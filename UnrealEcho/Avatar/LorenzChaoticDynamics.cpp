// LorenzChaoticDynamics.cpp
// Deep Tree Echo - Lorenz Attractor Chaotic Dynamics Implementation
// Copyright (c) 2025-2026 Deep Tree Echo Project

#include "LorenzChaoticDynamics.h"

DEFINE_LOG_CATEGORY_STATIC(LogLorenzChaos, Log, All);

// ============================================================================
// CONSTRUCTOR
// ============================================================================

ULorenzChaoticDynamics::ULorenzChaoticDynamics()
{
    Initialize();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void ULorenzChaoticDynamics::Initialize()
{
    State.Position = FVector(1.0f, 1.0f, 1.0f);
    State.Sigma = 10.0f;
    State.Rho = 28.0f;
    State.Beta = 8.0f / 3.0f;
    State.DeltaT = 0.01f;

    ShadowPosition = State.Position + FVector(ShadowSeparation, 0.0f, 0.0f);
    TrajectoryHistory.Empty();
    Telemetry = FChaoticDynamicsTelemetry();

    UE_LOG(LogLorenzChaos, Log, TEXT("Lorenz chaotic dynamics initialized (σ=%.1f, ρ=%.1f, β=%.3f)"),
        State.Sigma, State.Rho, State.Beta);
}

void ULorenzChaoticDynamics::InitializeWithParams(float Sigma, float Rho, float Beta, float DeltaT)
{
    State.Sigma = Sigma;
    State.Rho = Rho;
    State.Beta = Beta;
    State.DeltaT = DeltaT;
    State.Position = FVector(1.0f, 1.0f, 1.0f);

    ShadowPosition = State.Position + FVector(ShadowSeparation, 0.0f, 0.0f);
    TrajectoryHistory.Empty();
    Telemetry = FChaoticDynamicsTelemetry();

    UE_LOG(LogLorenzChaos, Log, TEXT("Lorenz chaotic dynamics initialized with custom params (σ=%.1f, ρ=%.1f, β=%.3f, dt=%.4f)"),
        Sigma, Rho, Beta, DeltaT);
}

void ULorenzChaoticDynamics::Reset()
{
    State.Position = FVector(1.0f, 1.0f, 1.0f);
    ShadowPosition = State.Position + FVector(ShadowSeparation, 0.0f, 0.0f);
    TrajectoryHistory.Empty();
    Telemetry = FChaoticDynamicsTelemetry();

    UE_LOG(LogLorenzChaos, Log, TEXT("Lorenz chaotic dynamics reset"));
}

// ============================================================================
// STEPPING
// ============================================================================

FVector ULorenzChaoticDynamics::Step()
{
    State.Position = RK4Step(State.Position, State.DeltaT);
    Telemetry.TotalSteps++;

    UpdateLyapunovEstimate();
    RecordTrajectory();

    // Auto-perturb if system falls into periodic orbit
    if (Telemetry.TotalSteps > 100 && !Telemetry.bIsInChaoticRegime)
    {
        Perturb(0.001f);
        UE_LOG(LogLorenzChaos, Warning, TEXT("Auto-perturbation triggered: system entered periodic orbit"));
    }

    return State.Position;
}

FVector ULorenzChaoticDynamics::StepMultiple(int32 NumSteps)
{
    for (int32 i = 0; i < NumSteps; ++i)
    {
        Step();
    }
    return State.Position;
}

FVector ULorenzChaoticDynamics::StepByTime(float DeltaTime)
{
    // Compute number of integration steps for the given real time
    // Use a fixed integration timestep for stability
    int32 NumSteps = FMath::Max(1, FMath::RoundToInt(DeltaTime / State.DeltaT));

    // Cap to prevent excessive computation
    NumSteps = FMath::Min(NumSteps, 100);

    return StepMultiple(NumSteps);
}

// ============================================================================
// MICRO-EXPRESSION GENERATION
// ============================================================================

TMap<FName, float> ULorenzChaoticDynamics::GenerateMicroExpressionNoise(float ChaosIntensity)
{
    TMap<FName, float> Noise;

    // Normalize Lorenz coordinates to [0, 1] range
    // Lorenz attractor typically ranges: X ∈ [-20, 20], Y ∈ [-30, 30], Z ∈ [0, 50]
    float Lx = FMath::Frac(FMath::Abs(State.Position.X * 0.025f));
    float Ly = FMath::Frac(FMath::Abs(State.Position.Y * 0.017f));
    float Lz = FMath::Frac(FMath::Abs(State.Position.Z * 0.02f));

    // Cross-channel mixing for richer dynamics
    float LxLy = FMath::Frac(Lx * Ly * 3.14159f);
    float LyLz = FMath::Frac(Ly * Lz * 2.71828f);
    float LzLx = FMath::Frac(Lz * Lx * 1.61803f);

    // Upper face micro-expressions
    Noise.Add(TEXT("AU1"), Lx * ChaosIntensity * 0.8f);                    // Inner brow raise
    Noise.Add(TEXT("AU2"), (1.0f - Lx) * ChaosIntensity * 0.6f);          // Outer brow raise
    Noise.Add(TEXT("AU4"), LxLy * ChaosIntensity * 0.4f);                 // Brow lowerer (subtle)
    Noise.Add(TEXT("AU5"), Ly * ChaosIntensity * 0.5f);                    // Upper lid raise
    Noise.Add(TEXT("AU6"), Lz * ChaosIntensity * 0.7f);                   // Cheek raise
    Noise.Add(TEXT("AU7"), (1.0f - Ly) * ChaosIntensity * 0.3f);          // Lid tightener

    // Lower face micro-expressions
    Noise.Add(TEXT("AU9"), LxLy * ChaosIntensity * 0.2f);                 // Nose wrinkle (very subtle)
    Noise.Add(TEXT("AU12"), Lz * ChaosIntensity * 0.9f);                  // Smile (most expressive)
    Noise.Add(TEXT("AU14"), LzLx * ChaosIntensity * 0.4f);               // Dimpler
    Noise.Add(TEXT("AU15"), (1.0f - Lz) * ChaosIntensity * 0.3f);        // Lip corner depress (subtle)
    Noise.Add(TEXT("AU17"), LyLz * ChaosIntensity * 0.2f);               // Chin raise
    Noise.Add(TEXT("AU25"), Ly * ChaosIntensity * 0.4f);                  // Lips part
    Noise.Add(TEXT("AU26"), LzLx * ChaosIntensity * 0.15f);              // Jaw drop (very subtle)

    // Eye gaze micro-movements (saccades)
    float SaccadeX = FMath::Sin(State.Position.X * 0.5f) * ChaosIntensity * 0.1f;
    float SaccadeY = FMath::Cos(State.Position.Y * 0.5f) * ChaosIntensity * 0.08f;
    Noise.Add(TEXT("AU61"), FMath::Max(0.0f, SaccadeX));   // Eyes left
    Noise.Add(TEXT("AU62"), FMath::Max(0.0f, -SaccadeX));  // Eyes right
    Noise.Add(TEXT("AU63"), FMath::Max(0.0f, SaccadeY));   // Eyes up
    Noise.Add(TEXT("AU64"), FMath::Max(0.0f, -SaccadeY));  // Eyes down

    return Noise;
}

float ULorenzChaoticDynamics::GetChaoticFloat(int32 Channel) const
{
    switch (Channel % 3)
    {
    case 0: return FMath::Frac(FMath::Abs(State.Position.X * 0.025f));
    case 1: return FMath::Frac(FMath::Abs(State.Position.Y * 0.017f));
    case 2: return FMath::Frac(FMath::Abs(State.Position.Z * 0.02f));
    default: return 0.5f;
    }
}

// ============================================================================
// MONITORING
// ============================================================================

float ULorenzChaoticDynamics::GetLyapunovExponent() const
{
    return Telemetry.LyapunovExponent;
}

bool ULorenzChaoticDynamics::IsInChaoticRegime() const
{
    return Telemetry.bIsInChaoticRegime;
}

FChaoticDynamicsTelemetry ULorenzChaoticDynamics::GetTelemetry() const
{
    return Telemetry;
}

// ============================================================================
// PARAMETER CONTROL
// ============================================================================

void ULorenzChaoticDynamics::SetExpressionVolatility(float Volatility)
{
    Telemetry.ExpressionVolatility = FMath::Clamp(Volatility, 0.0f, 1.0f);
}

void ULorenzChaoticDynamics::Perturb(float Magnitude)
{
    State.Position.X += FMath::FRandRange(-Magnitude, Magnitude);
    State.Position.Y += FMath::FRandRange(-Magnitude, Magnitude);
    State.Position.Z += FMath::FRandRange(-Magnitude, Magnitude);

    UE_LOG(LogLorenzChaos, Verbose, TEXT("Perturbation applied (magnitude=%.4f)"), Magnitude);
}
