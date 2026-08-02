// StandaloneReservoirVerification.cpp
// Executable verification of the DeepTreeEchoReservoir Echo State Network core.
//
// UDeepTreeEchoReservoir cannot run outside Unreal, so this harness mirrors CreateReservoir /
// EnsureWeightsBuilt / ProcessInput 1:1 (same CSR layout, same power-iteration normalization,
// same leaky-tanh update) and tests the properties that make a reservoir a reservoir:
//
//   [1] Reproducibility  - same input sequence from the same initial state gives the same
//                          trajectory. The pre-fix code resampled W every step and fails this.
//   [2] Echo State Property (fading memory) - two different initial states driven by the same
//                          input converge. This is THE defining property; without it the
//                          reservoir cannot encode temporal structure.
//   [3] Input sensitivity - different input sequences give different states (not just a
//                          collapsed fixed point, which would satisfy [2] trivially).
//   [4] Spectral radius  - the normalized W's measured spectral radius matches the configured
//                          one. The pre-fix code never normalized at all.
//   [5] Input diversity  - units receive distinct input drive. The pre-fix code computed one
//                          identical InputSum for every unit.
//   [6] No hot-path RNG  - ProcessInput draws zero random numbers (structural: the mirror
//                          counts every RNG call and asserts the step loop makes none).
//
// Mirrored from DeepTreeEcho/Reservoir/DeepTreeEchoReservoir.{h,cpp} - keep in lockstep.
//
// Build & run:  g++ -std=c++17 -O2 -o resverify StandaloneReservoirVerification.cpp && ./resverify

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

namespace
{

int RngCallCount = 0;  // instrumentation for test [6]

// Mirrors FRandomStream closely enough for structural testing: deterministic, seeded.
struct SeededStream
{
    std::mt19937 Engine;
    explicit SeededStream(int Seed) : Engine(static_cast<unsigned>(Seed)) {}

    float FRand()
    {
        ++RngCallCount;
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(Engine);
    }
    float FRandRange(float Min, float Max)
    {
        ++RngCallCount;
        return std::uniform_real_distribution<float>(Min, Max)(Engine);
    }
};

constexpr int MaxInputDimension = 16;
constexpr float RecurrentDensity = 0.1f;

// Mirrors FReservoirState's weight storage (CSR sparse W + dense Win).
struct ReservoirState
{
    int Units = 100;
    float SpectralRadius = 0.9f;
    float LeakRate = 0.3f;
    float InputScaling = 1.0f;
    int WeightSeed = 0;
    bool bWeightsBuilt = false;

    std::vector<float> ActivationState;
    std::vector<int> RecurrentRowStart;
    std::vector<int> RecurrentColIndex;
    std::vector<float> RecurrentWeight;
    std::vector<float> InputWeight;
};

// Mirrors UDeepTreeEchoReservoir::EnsureWeightsBuilt.
void EnsureWeightsBuilt(ReservoirState& R)
{
    if (R.bWeightsBuilt && static_cast<int>(R.RecurrentRowStart.size()) == R.Units + 1)
    {
        return;
    }
    if (R.Units <= 0)
    {
        R.bWeightsBuilt = true;
        return;
    }

    SeededStream Stream(R.WeightSeed);

    R.RecurrentRowStart.clear();
    R.RecurrentColIndex.clear();
    R.RecurrentWeight.clear();
    R.RecurrentRowStart.reserve(R.Units + 1);

    for (int Row = 0; Row < R.Units; ++Row)
    {
        R.RecurrentRowStart.push_back(static_cast<int>(R.RecurrentColIndex.size()));
        for (int Col = 0; Col < R.Units; ++Col)
        {
            if (Stream.FRand() < RecurrentDensity)
            {
                R.RecurrentColIndex.push_back(Col);
                R.RecurrentWeight.push_back(Stream.FRandRange(-1.0f, 1.0f));
            }
        }
    }
    R.RecurrentRowStart.push_back(static_cast<int>(R.RecurrentColIndex.size()));

    // Power iteration -> normalize to the configured spectral radius
    {
        std::vector<float> Vec(R.Units, 1.0f / std::sqrt(static_cast<float>(R.Units)));
        std::vector<float> Next(R.Units, 0.0f);
        float Eigenvalue = 0.0f;

        // Converge rather than fixed-count: a fixed 20 iterations under-estimates the
        // eigenvalue by ~3% here, overshooting the requested radius by the same factor.
        constexpr int MaxPowerIterations = 200;
        constexpr float ConvergenceTolerance = 1.0e-5f;

        for (int Iter = 0; Iter < MaxPowerIterations; ++Iter)
        {
            for (int Row = 0; Row < R.Units; ++Row)
            {
                float Sum = 0.0f;
                for (int k = R.RecurrentRowStart[Row]; k < R.RecurrentRowStart[Row + 1]; ++k)
                {
                    Sum += R.RecurrentWeight[k] * Vec[R.RecurrentColIndex[k]];
                }
                Next[Row] = Sum;
            }
            float Norm = 0.0f;
            for (float V : Next) { Norm += V * V; }
            Norm = std::sqrt(Norm);
            if (Norm < 1e-6f) { Eigenvalue = 0.0f; break; }

            const float PreviousEigenvalue = Eigenvalue;
            Eigenvalue = Norm;
            for (int i = 0; i < R.Units; ++i) { Vec[i] = Next[i] / Norm; }

            if (Iter > 0 && std::fabs(Eigenvalue - PreviousEigenvalue) <= ConvergenceTolerance * Eigenvalue)
            {
                break;
            }
        }

        if (Eigenvalue > 1e-6f)
        {
            const float Scale = R.SpectralRadius / Eigenvalue;
            for (float& W : R.RecurrentWeight) { W *= Scale; }
        }
    }

    R.InputWeight.resize(static_cast<size_t>(R.Units) * MaxInputDimension);
    for (auto& W : R.InputWeight) { W = Stream.FRandRange(-1.0f, 1.0f); }

    R.bWeightsBuilt = true;
}

// Mirrors UDeepTreeEchoReservoir::CreateReservoir (activation init + seed + weight build).
ReservoirState CreateReservoir(int Units, float SpectralRadius, float LeakRate, int Seed)
{
    ReservoirState R;
    R.Units = Units;
    R.SpectralRadius = SpectralRadius;
    R.LeakRate = LeakRate;
    R.WeightSeed = Seed;
    R.ActivationState.assign(Units, 0.0f);
    EnsureWeightsBuilt(R);
    return R;
}

// Mirrors the per-step update inside UDeepTreeEchoReservoir::ProcessInput.
std::vector<float> ProcessInput(ReservoirState& R, const std::vector<float>& Input)
{
    EnsureWeightsBuilt(R);

    std::vector<float> NewState(R.Units, 0.0f);
    const int InputDim = std::min<int>(static_cast<int>(Input.size()), MaxInputDimension);

    for (int i = 0; i < R.Units; ++i)
    {
        float InputSum = 0.0f;
        const int RowBase = i * MaxInputDimension;
        for (int j = 0; j < InputDim; ++j)
        {
            InputSum += R.InputWeight[RowBase + j] * Input[j];
        }
        InputSum *= R.InputScaling;

        float RecurrentSum = 0.0f;
        for (int k = R.RecurrentRowStart[i]; k < R.RecurrentRowStart[i + 1]; ++k)
        {
            RecurrentSum += R.RecurrentWeight[k] * R.ActivationState[R.RecurrentColIndex[k]];
        }

        const float Activation = std::tanh(InputSum + RecurrentSum);
        NewState[i] = (1.0f - R.LeakRate) * R.ActivationState[i] + R.LeakRate * Activation;
    }

    R.ActivationState = NewState;
    return NewState;
}

// Measured spectral radius of the built W, via independent power iteration.
float MeasureSpectralRadius(const ReservoirState& R)
{
    std::vector<float> Vec(R.Units, 1.0f / std::sqrt(static_cast<float>(R.Units)));
    std::vector<float> Next(R.Units, 0.0f);
    float Eigenvalue = 0.0f;

    for (int Iter = 0; Iter < 200; ++Iter)
    {
        for (int Row = 0; Row < R.Units; ++Row)
        {
            float Sum = 0.0f;
            for (int k = R.RecurrentRowStart[Row]; k < R.RecurrentRowStart[Row + 1]; ++k)
            {
                Sum += R.RecurrentWeight[k] * Vec[R.RecurrentColIndex[k]];
            }
            Next[Row] = Sum;
        }
        float Norm = 0.0f;
        for (float V : Next) { Norm += V * V; }
        Norm = std::sqrt(Norm);
        if (Norm < 1e-6f) { return 0.0f; }
        Eigenvalue = Norm;
        for (int i = 0; i < R.Units; ++i) { Vec[i] = Next[i] / Norm; }
    }
    return Eigenvalue;
}

float L2Distance(const std::vector<float>& A, const std::vector<float>& B)
{
    float Sum = 0.0f;
    for (size_t i = 0; i < A.size() && i < B.size(); ++i)
    {
        const float D = A[i] - B[i];
        Sum += D * D;
    }
    return std::sqrt(Sum);
}

// Deterministic pseudo-input sequence (not from the instrumented RNG).
std::vector<std::vector<float>> MakeInputSequence(int Steps, float Phase)
{
    std::vector<std::vector<float>> Seq;
    for (int t = 0; t < Steps; ++t)
    {
        std::vector<float> U(8);
        for (int j = 0; j < 8; ++j)
        {
            U[j] = std::sin(0.3f * t + 0.7f * j + Phase);
        }
        Seq.push_back(U);
    }
    return Seq;
}

int TestsFailed = 0;

void Check(bool bCondition, const char* Label)
{
    std::printf("  [%s] %s\n", bCondition ? "PASS" : "FAIL", Label);
    if (!bCondition) { ++TestsFailed; }
}

} // namespace

int main()
{
    std::printf("=== Standalone verification: DeepTreeEchoReservoir ESN core ===\n\n");

    constexpr int Units = 100;
    constexpr int Seed = 1337;
    const auto SeqA = MakeInputSequence(60, 0.0f);
    const auto SeqB = MakeInputSequence(60, 1.9f);

    // ------------------------------------------------------------------ [1] reproducibility
    std::printf("[1] trajectory reproducibility (fails if W is resampled per step)\n");
    ReservoirState R1 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    ReservoirState R2 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    std::vector<float> Final1, Final2;
    for (const auto& U : SeqA) { Final1 = ProcessInput(R1, U); }
    for (const auto& U : SeqA) { Final2 = ProcessInput(R2, U); }
    const float ReproDist = L2Distance(Final1, Final2);
    std::printf("      identical runs, final-state L2 distance: %.3e\n", ReproDist);
    Check(ReproDist < 1e-5f, "same seed + same input sequence -> identical trajectory");

    // ------------------------------------------------------------------ [2] echo state property
    std::printf("\n[2] Echo State Property: state contraction / fading memory\n");
    ReservoirState E1 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    ReservoirState E2 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    // Drive from two very different initial states with the SAME input.
    for (int i = 0; i < Units; ++i)
    {
        E1.ActivationState[i] = 0.9f;
        E2.ActivationState[i] = -0.9f;
    }
    const float StartDist = L2Distance(E1.ActivationState, E2.ActivationState);
    for (const auto& U : SeqA)
    {
        ProcessInput(E1, U);
        ProcessInput(E2, U);
    }
    const float EndDist = L2Distance(E1.ActivationState, E2.ActivationState);
    std::printf("      initial-state separation %.4f -> %.4f after %zu steps\n",
                StartDist, EndDist, SeqA.size());
    Check(EndDist < StartDist * 0.01f, "differing initial states converge under identical input");

    // ------------------------------------------------------------------ [3] input sensitivity
    std::printf("\n[3] input sensitivity (state is not a collapsed fixed point)\n");
    ReservoirState S1 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    ReservoirState S2 = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    for (const auto& U : SeqA) { ProcessInput(S1, U); }
    for (const auto& U : SeqB) { ProcessInput(S2, U); }
    const float InputDist = L2Distance(S1.ActivationState, S2.ActivationState);
    std::printf("      different input sequences, final-state L2 distance: %.4f\n", InputDist);
    Check(InputDist > 0.1f, "different inputs produce measurably different states");

    // ------------------------------------------------------------------ [4] spectral radius
    std::printf("\n[4] spectral radius normalization\n");
    bool bAllRadiiOk = true;
    for (float Target : {0.5f, 0.9f, 1.1f})
    {
        ReservoirState R = CreateReservoir(Units, Target, 0.3f, Seed + 7);
        const float Measured = MeasureSpectralRadius(R);
        std::printf("      configured %.2f -> measured %.4f\n", Target, Measured);
        if (std::fabs(Measured - Target) > 0.02f) { bAllRadiiOk = false; }
    }
    Check(bAllRadiiOk, "measured spectral radius matches configured within 0.02");

    // ------------------------------------------------------------------ [5] input diversity
    std::printf("\n[5] per-unit input diversity\n");
    ReservoirState D = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    const std::vector<float> Impulse(8, 1.0f);
    const std::vector<float> Response = ProcessInput(D, Impulse);
    float MinR = Response[0], MaxR = Response[0];
    for (float V : Response) { MinR = std::min(MinR, V); MaxR = std::max(MaxR, V); }
    std::printf("      impulse response spread across units: [%.4f, %.4f]\n", MinR, MaxR);
    Check(MaxR - MinR > 0.05f, "units receive distinct input drive (Win is a real matrix)");

    // ------------------------------------------------------------------ [6] no hot-path RNG
    std::printf("\n[6] no RNG draws on the ProcessInput hot path\n");
    ReservoirState P = CreateReservoir(Units, 0.9f, 0.3f, Seed);
    const int RngAfterBuild = RngCallCount;
    for (const auto& U : SeqA) { ProcessInput(P, U); }
    const int RngDuringSteps = RngCallCount - RngAfterBuild;
    std::printf("      RNG draws during %zu ProcessInput calls: %d (pre-fix: ~%d)\n",
                SeqA.size(), RngDuringSteps,
                static_cast<int>(SeqA.size()) * Units * Units);
    Check(RngDuringSteps == 0, "ProcessInput performs zero random draws");

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "Reservoir=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
