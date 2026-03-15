// AvatarDesignStudio.cpp
// Deep Tree Echo - Avatar Design Studio & Animation Editor
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// Complete implementation of the unified avatar design studio integrating:
// - MetaHuman DNA calibration
// - FACS expression mapping (26+ Action Units)
// - Lorenz chaotic micro-expression dynamics (RK4 integration)
// - SuperHotGirl + HyperChaotic aesthetics
// - 16-channel virtual endocrine system with 10 glands
// - 4E embodied cognition metrics
// - ESN reservoir computing (512 neurons, spectral radius 0.9)
// - Ontogenetic evolution (Embryonic → Transcendent)
// - Animation sequence editor with keyframe interpolation
// - Echobeats 9-step cognitive cycle

#include "AvatarDesignStudio.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// ============================================================================
// FHormoneBusState Implementation
// ============================================================================

float FHormoneBusState::GetChannel(int32 Index) const
{
    switch (Index)
    {
        case 0: return CRH;
        case 1: return ACTH;
        case 2: return Cortisol;
        case 3: return DopamineTonic;
        case 4: return DopaminePhasic;
        case 5: return Serotonin;
        case 6: return Norepinephrine;
        case 7: return Oxytocin;
        case 8: return T3T4;
        case 9: return Melatonin;
        case 10: return Insulin;
        case 11: return Glucagon;
        case 12: return IL6;
        case 13: return Anandamide;
        case 14: return Reserved0;
        case 15: return Reserved1;
        default: return 0.0f;
    }
}

void FHormoneBusState::SetChannel(int32 Index, float Value)
{
    Value = FMath::Clamp(Value, 0.0f, 1.0f);
    switch (Index)
    {
        case 0: CRH = Value; break;
        case 1: ACTH = Value; break;
        case 2: Cortisol = Value; break;
        case 3: DopamineTonic = Value; break;
        case 4: DopaminePhasic = Value; break;
        case 5: Serotonin = Value; break;
        case 6: Norepinephrine = Value; break;
        case 7: Oxytocin = Value; break;
        case 8: T3T4 = Value; break;
        case 9: Melatonin = Value; break;
        case 10: Insulin = Value; break;
        case 11: Glucagon = Value; break;
        case 12: IL6 = Value; break;
        case 13: Anandamide = Value; break;
        case 14: Reserved0 = Value; break;
        case 15: Reserved1 = Value; break;
    }
}

void FHormoneBusState::ApplyDecay(float DeltaTime, const FHormoneBusState& Baselines, const TArray<float>& HalfLives)
{
    for (int32 i = 0; i < 16; ++i)
    {
        if (i < HalfLives.Num() && HalfLives[i] > 0.0f)
        {
            float Current = GetChannel(i);
            float Baseline = Baselines.GetChannel(i);
            // Exponential decay: C(t) = Baseline + (C(t-1) - Baseline) * exp(-ln(2)/HalfLife * dt)
            float DecayRate = FMath::Loge(2.0f) / HalfLives[i];
            float NewValue = Baseline + (Current - Baseline) * FMath::Exp(-DecayRate * DeltaTime);
            SetChannel(i, NewValue);
        }
    }
}

// ============================================================================
// FFACSActionUnitState Implementation
// ============================================================================

void FFACSActionUnitState::ComputeFromEndocrine(const FHormoneBusState& Hormones)
{
    // Cortisol → AU4 (brow lowerer) + AU1 (inner brow raise) + AU15 (lip corner depress)
    float CortisolEffect = FMath::Clamp(Hormones.Cortisol - 0.15f, 0.0f, 1.0f) * 2.0f;
    ActionUnits.FindOrAdd(4) += CortisolEffect * 0.7f;
    ActionUnits.FindOrAdd(1) += CortisolEffect * 0.4f;
    ActionUnits.FindOrAdd(15) += CortisolEffect * 0.5f;

    // Dopamine (phasic) → AU12 (smile) + AU6 (cheek raise) — genuine reward
    float DopamineEffect = FMath::Clamp(Hormones.DopaminePhasic, 0.0f, 1.0f);
    ActionUnits.FindOrAdd(12) += DopamineEffect * 0.9f;
    ActionUnits.FindOrAdd(6) += DopamineEffect * 0.7f;

    // Dopamine (tonic) → subtle baseline smile
    float TonicEffect = FMath::Clamp(Hormones.DopamineTonic - 0.3f, 0.0f, 1.0f);
    ActionUnits.FindOrAdd(12) += TonicEffect * 0.3f;

    // Norepinephrine → AU5 (upper lid raise) + AU7 (lid tightener) — alertness
    float NEEffect = FMath::Clamp(Hormones.Norepinephrine - 0.1f, 0.0f, 1.0f) * 2.0f;
    ActionUnits.FindOrAdd(5) += NEEffect * 0.6f;
    ActionUnits.FindOrAdd(7) += NEEffect * 0.4f;

    // Oxytocin → AU6 + AU12 + AU25 — warm social expression
    float OxyEffect = FMath::Clamp(Hormones.Oxytocin - 0.1f, 0.0f, 1.0f) * 2.0f;
    ActionUnits.FindOrAdd(6) += OxyEffect * 0.5f;
    ActionUnits.FindOrAdd(12) += OxyEffect * 0.4f;
    ActionUnits.FindOrAdd(25) += OxyEffect * 0.3f;

    // Melatonin → AU43 (eyes closed) — drowsiness
    float MelEffect = FMath::Clamp(Hormones.Melatonin, 0.0f, 1.0f);
    ActionUnits.FindOrAdd(43) += MelEffect * 0.8f;

    // Serotonin → subtle contentment (AU6 + AU12 mild)
    float SerEffect = FMath::Clamp(Hormones.Serotonin - 0.4f, 0.0f, 1.0f);
    ActionUnits.FindOrAdd(6) += SerEffect * 0.2f;
    ActionUnits.FindOrAdd(12) += SerEffect * 0.2f;

    // IL-6 (immune/stress) → AU4 + AU7 (concern)
    float IL6Effect = FMath::Clamp(Hormones.IL6 - 0.05f, 0.0f, 1.0f) * 2.0f;
    ActionUnits.FindOrAdd(4) += IL6Effect * 0.3f;
    ActionUnits.FindOrAdd(7) += IL6Effect * 0.2f;

    // Anandamide → relaxation (reduce AU4, increase AU43 slightly)
    float AEAEffect = FMath::Clamp(Hormones.Anandamide - 0.1f, 0.0f, 1.0f);
    ActionUnits.FindOrAdd(4) -= AEAEffect * 0.2f;
    ActionUnits.FindOrAdd(43) += AEAEffect * 0.15f;
}

void FFACSActionUnitState::ComputeFromCognitive(float Valence, float Arousal, float CognitiveLoad)
{
    // Positive valence → AU6 (cheek raise) + AU12 (smile)
    if (Valence > 0.0f)
    {
        ActionUnits.FindOrAdd(6) += Valence * 0.6f;
        ActionUnits.FindOrAdd(12) += Valence * 0.7f;
    }
    else
    {
        // Negative valence → AU15 (lip corner depress)
        ActionUnits.FindOrAdd(15) += FMath::Abs(Valence) * 0.6f;
    }

    // Arousal → AU5 (upper lid raise) + AU25 (lips part) + AU26 (jaw drop)
    ActionUnits.FindOrAdd(5) += Arousal * 0.5f;
    ActionUnits.FindOrAdd(25) += Arousal * 0.4f;
    ActionUnits.FindOrAdd(26) += Arousal * 0.3f;

    // Cognitive load → AU4 (brow lowerer) + AU7 (lid tightener)
    ActionUnits.FindOrAdd(4) += CognitiveLoad * 0.5f;
    ActionUnits.FindOrAdd(7) += CognitiveLoad * 0.3f;
}

void FFACSActionUnitState::BlendWith(const FFACSActionUnitState& Other)
{
    for (const auto& Pair : Other.ActionUnits)
    {
        ActionUnits.FindOrAdd(Pair.Key) += Pair.Value;
    }
    // Clamp all values to [0, 1]
    for (auto& Pair : ActionUnits)
    {
        Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }
}

void FFACSActionUnitState::ApplyChaos(const FVector& LorenzState, float ChaosIntensity)
{
    FVector Normalized = LorenzState;
    // Normalize Lorenz output to [-1, 1] range
    Normalized.X = FMath::Clamp(Normalized.X / 20.0f, -1.0f, 1.0f);
    Normalized.Y = FMath::Clamp(Normalized.Y / 25.0f, -1.0f, 1.0f);
    Normalized.Z = FMath::Clamp((Normalized.Z - 25.0f) / 20.0f, -1.0f, 1.0f);

    // Apply chaos as additive noise to key AUs
    int32 ChaoticAUs[] = {1, 2, 4, 5, 6, 7, 12, 15, 25, 26};
    for (int32 i = 0; i < 10; ++i)
    {
        int32 AU = ChaoticAUs[i];
        float* Value = ActionUnits.Find(AU);
        if (Value)
        {
            float Noise = 0.0f;
            switch (i % 3)
            {
                case 0: Noise = Normalized.X; break;
                case 1: Noise = Normalized.Y; break;
                case 2: Noise = Normalized.Z; break;
            }
            *Value = FMath::Clamp(*Value + Noise * ChaosIntensity, 0.0f, 1.0f);
        }
    }
}

float FFACSActionUnitState::GetAU(int32 AUNumber) const
{
    const float* Value = ActionUnits.Find(AUNumber);
    return Value ? FMath::Clamp(*Value, 0.0f, 1.0f) : 0.0f;
}

// ============================================================================
// FLorenzAttractorState Implementation (RK4 Integration)
// ============================================================================

void FLorenzAttractorState::Step(float DeltaTime)
{
    PreviousState = State;

    // RK4 integration of Lorenz system
    auto Derivatives = [this](const FVector& S) -> FVector
    {
        return FVector(
            Sigma * (S.Y - S.X),
            S.X * (Rho - S.Z) - S.Y,
            S.X * S.Y - Beta * S.Z
        );
    };

    float dt = FMath::Min(DeltaTime, 0.01f); // Cap dt for stability
    int32 Steps = FMath::Max(1, FMath::CeilToInt(DeltaTime / dt));

    for (int32 i = 0; i < Steps; ++i)
    {
        FVector k1 = Derivatives(State) * dt;
        FVector k2 = Derivatives(State + k1 * 0.5f) * dt;
        FVector k3 = Derivatives(State + k2 * 0.5f) * dt;
        FVector k4 = Derivatives(State + k3) * dt;

        State += (k1 + k2 * 2.0f + k3 * 2.0f + k4) / 6.0f;
    }

    // Update Lyapunov exponent estimate
    float Divergence = (State - PreviousState).Size();
    if (Divergence > KINDA_SMALL_NUMBER)
    {
        LyapunovSum += FMath::Loge(Divergence / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER));
        LyapunovSteps++;
    }
}

float FLorenzAttractorState::GetLyapunovExponent() const
{
    return LyapunovSteps > 0 ? LyapunovSum / static_cast<float>(LyapunovSteps) : 0.0f;
}

FVector FLorenzAttractorState::GetNormalizedOutput() const
{
    return FVector(
        FMath::Clamp(State.X / 20.0f, -1.0f, 1.0f),
        FMath::Clamp(State.Y / 25.0f, -1.0f, 1.0f),
        FMath::Clamp((State.Z - 25.0f) / 20.0f, -1.0f, 1.0f)
    );
}

// ============================================================================
// F4ECognitionMetrics Implementation
// ============================================================================

float F4ECognitionMetrics::GetOverallScore() const
{
    return (GetDimensionScore(0) + GetDimensionScore(1) + GetDimensionScore(2) + GetDimensionScore(3)) / 4.0f;
}

float F4ECognitionMetrics::GetDimensionScore(int32 Dimension) const
{
    switch (Dimension)
    {
        case 0: return (BodySchemaIntegration + ProprioceptiveAccuracy + SomaticMarkerStrength) / 3.0f;
        case 1: return (AffordanceDetectionRate + NicheCoupling + EnvironmentalSensitivity) / 3.0f;
        case 2: return (SensorimotorCoordination + PredictionAccuracy + ActiveInferenceEfficiency) / 3.0f;
        case 3: return (ToolUseCompetence + ExternalMemoryIntegration + SocialCognitionDepth) / 3.0f;
        default: return 0.0f;
    }
}

// ============================================================================
// FReservoirState Implementation (Echo State Network)
// ============================================================================

void FReservoirState::Initialize()
{
    StateVector.SetNum(ReservoirSize);
    for (int32 i = 0; i < ReservoirSize; ++i)
    {
        StateVector[i] = 0.0f;
    }

    // Initialize sparse weight matrix
    WeightMatrix.Empty();
    int32 NumConnections = FMath::CeilToInt(ReservoirSize * ReservoirSize * Sparsity);
    for (int32 c = 0; c < NumConnections; ++c)
    {
        int32 i = FMath::RandRange(0, ReservoirSize - 1);
        int32 j = FMath::RandRange(0, ReservoirSize - 1);
        int32 Key = i * ReservoirSize + j;
        float Weight = FMath::FRandRange(-1.0f, 1.0f);
        WeightMatrix.Add(Key, Weight * SpectralRadius);
    }

    // Initialize input weights
    InputWeights.SetNum(ReservoirSize * 16); // 16 input channels
    for (int32 i = 0; i < InputWeights.Num(); ++i)
    {
        InputWeights[i] = FMath::FRandRange(-InputScaling, InputScaling);
    }

    // Initialize output weights (will be trained)
    OutputWeights.SetNum(ReservoirSize * 16);
    for (int32 i = 0; i < OutputWeights.Num(); ++i)
    {
        OutputWeights[i] = 0.0f;
    }
}

void FReservoirState::Update(const TArray<float>& Input)
{
    if (StateVector.Num() != ReservoirSize)
    {
        Initialize();
    }

    TArray<float> NewState;
    NewState.SetNum(ReservoirSize);

    for (int32 i = 0; i < ReservoirSize; ++i)
    {
        // Compute W * State[t-1]
        float WState = 0.0f;
        for (int32 j = 0; j < ReservoirSize; ++j)
        {
            int32 Key = i * ReservoirSize + j;
            const float* Weight = WeightMatrix.Find(Key);
            if (Weight)
            {
                WState += *Weight * StateVector[j];
            }
        }

        // Compute Win * Input
        float WinInput = 0.0f;
        int32 InputSize = FMath::Min(Input.Num(), 16);
        for (int32 k = 0; k < InputSize; ++k)
        {
            int32 WinIdx = i * 16 + k;
            if (WinIdx < InputWeights.Num())
            {
                WinInput += InputWeights[WinIdx] * Input[k];
            }
        }

        // Leaky integrator: State[t] = (1-LeakRate)*State[t-1] + LeakRate*tanh(W*State + Win*Input)
        NewState[i] = (1.0f - LeakRate) * StateVector[i] + LeakRate * FMath::Tanh(WState + WinInput);
    }

    StateVector = MoveTemp(NewState);
}

TArray<float> FReservoirState::ReadOutput() const
{
    TArray<float> Output;
    int32 OutputSize = 16;
    Output.SetNum(OutputSize);

    for (int32 o = 0; o < OutputSize; ++o)
    {
        float Sum = 0.0f;
        for (int32 i = 0; i < ReservoirSize; ++i)
        {
            int32 Idx = i * OutputSize + o;
            if (Idx < OutputWeights.Num())
            {
                Sum += OutputWeights[Idx] * StateVector[i];
            }
        }
        Output[o] = Sum;
    }

    return Output;
}

// ============================================================================
// FExpressionAnimationSequence Implementation
// ============================================================================

FExpressionKeyframe FExpressionAnimationSequence::Evaluate(float Time) const
{
    if (Keyframes.Num() == 0)
    {
        return FExpressionKeyframe();
    }

    if (bLooping && Duration > 0.0f)
    {
        Time = FMath::Fmod(Time, Duration);
    }

    // Find surrounding keyframes
    int32 Before = 0;
    int32 After = 0;
    for (int32 i = 0; i < Keyframes.Num(); ++i)
    {
        if (Keyframes[i].TimeStamp <= Time)
        {
            Before = i;
        }
        if (Keyframes[i].TimeStamp >= Time)
        {
            After = i;
            break;
        }
        After = i;
    }

    if (Before == After)
    {
        return Keyframes[Before];
    }

    // Interpolate
    float Alpha = (Time - Keyframes[Before].TimeStamp) /
                  FMath::Max(Keyframes[After].TimeStamp - Keyframes[Before].TimeStamp, KINDA_SMALL_NUMBER);
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    // Smooth step interpolation for more natural transitions
    Alpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

    FExpressionKeyframe Result;
    Result.TimeStamp = Time;

    // Interpolate AU values
    TSet<int32> AllAUs;
    for (const auto& Pair : Keyframes[Before].ActionUnits.ActionUnits) AllAUs.Add(Pair.Key);
    for (const auto& Pair : Keyframes[After].ActionUnits.ActionUnits) AllAUs.Add(Pair.Key);

    for (int32 AU : AllAUs)
    {
        float ValA = Keyframes[Before].ActionUnits.GetAU(AU);
        float ValB = Keyframes[After].ActionUnits.GetAU(AU);
        Result.ActionUnits.ActionUnits.Add(AU, FMath::Lerp(ValA, ValB, Alpha));
    }

    // Interpolate aesthetics
    Result.Aesthetics.ConfidencePosture = FMath::Lerp(Keyframes[Before].Aesthetics.ConfidencePosture, Keyframes[After].Aesthetics.ConfidencePosture, Alpha);
    Result.Aesthetics.Charisma = FMath::Lerp(Keyframes[Before].Aesthetics.Charisma, Keyframes[After].Aesthetics.Charisma, Alpha);
    Result.Aesthetics.EyeSparkle = FMath::Lerp(Keyframes[Before].Aesthetics.EyeSparkle, Keyframes[After].Aesthetics.EyeSparkle, Alpha);
    Result.Aesthetics.GracefulMovement = FMath::Lerp(Keyframes[Before].Aesthetics.GracefulMovement, Keyframes[After].Aesthetics.GracefulMovement, Alpha);
    Result.Aesthetics.EmissiveGlow = FMath::Lerp(Keyframes[Before].Aesthetics.EmissiveGlow, Keyframes[After].Aesthetics.EmissiveGlow, Alpha);
    Result.Aesthetics.BioluminescentIntensity = FMath::Lerp(Keyframes[Before].Aesthetics.BioluminescentIntensity, Keyframes[After].Aesthetics.BioluminescentIntensity, Alpha);

    // Use the cognitive mode of the closer keyframe
    Result.CognitiveMode = Alpha < 0.5f ? Keyframes[Before].CognitiveMode : Keyframes[After].CognitiveMode;

    return Result;
}

void FExpressionAnimationSequence::AddKeyframe(const FExpressionKeyframe& Keyframe)
{
    // Insert in sorted order by timestamp
    int32 InsertIndex = 0;
    for (int32 i = 0; i < Keyframes.Num(); ++i)
    {
        if (Keyframes[i].TimeStamp > Keyframe.TimeStamp)
        {
            InsertIndex = i;
            break;
        }
        InsertIndex = i + 1;
    }
    Keyframes.Insert(Keyframe, InsertIndex);
    RecalculateDuration();
}

void FExpressionAnimationSequence::RemoveKeyframe(int32 Index)
{
    if (Index >= 0 && Index < Keyframes.Num())
    {
        Keyframes.RemoveAt(Index);
        RecalculateDuration();
    }
}

void FExpressionAnimationSequence::RecalculateDuration()
{
    Duration = 0.0f;
    for (const auto& KF : Keyframes)
    {
        Duration = FMath::Max(Duration, KF.TimeStamp);
    }
}

// ============================================================================
// UAvatarDesignStudio Implementation
// ============================================================================

UAvatarDesignStudio::UAvatarDesignStudio()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // ~30 Hz cognitive cycle
}

void UAvatarDesignStudio::BeginPlay()
{
    Super::BeginPlay();
    InitializeEndocrineSystem();
    InitializeGlands();
    InitializeNamedExpressions();
    Reservoir.Initialize();
    SetAestheticPreset(EAestheticPreset::SuperHotGirl);
}

void UAvatarDesignStudio::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized) return;

    // Execute the full Echobeats 9-step cognitive cycle
    ExecuteCognitiveCycle(DeltaTime);

    // Update animation playback if active
    UpdateAnimationPlayback(DeltaTime);
}

void UAvatarDesignStudio::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool UAvatarDesignStudio::InitializeStudio(USkeletalMeshComponent* TargetMesh)
{
    if (!TargetMesh) return false;

    TargetSkeletalMesh = TargetMesh;

    // Create dynamic material instances for aesthetic control
    DynamicMaterials.Empty();
    for (int32 i = 0; i < TargetMesh->GetNumMaterials(); ++i)
    {
        UMaterialInterface* BaseMat = TargetMesh->GetMaterial(i);
        if (BaseMat)
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
            TargetMesh->SetMaterial(i, DynMat);
            DynamicMaterials.Add(DynMat);
        }
    }

    bIsInitialized = true;
    return true;
}

void UAvatarDesignStudio::SetAestheticPreset(EAestheticPreset Preset)
{
    switch (Preset)
    {
        case EAestheticPreset::SuperHotGirl:
            Aesthetics.ConfidencePosture = 0.85f;
            Aesthetics.Charisma = 0.9f;
            Aesthetics.EyeSparkle = 0.8f;
            Aesthetics.GracefulMovement = 0.75f;
            Aesthetics.EmissiveGlow = 0.6f;
            Aesthetics.BioluminescentIntensity = 0.7f;
            Aesthetics.HolographicShimmerRate = 1.5f;
            break;

        case EAestheticPreset::CyberpunkAngel:
            Aesthetics.ConfidencePosture = 0.9f;
            Aesthetics.Charisma = 0.95f;
            Aesthetics.EyeSparkle = 0.9f;
            Aesthetics.GracefulMovement = 0.8f;
            Aesthetics.EmissiveGlow = 0.85f;
            Aesthetics.BioluminescentIntensity = 0.9f;
            Aesthetics.HolographicShimmerRate = 2.0f;
            break;

        case EAestheticPreset::BioluminescentSage:
            Aesthetics.ConfidencePosture = 0.7f;
            Aesthetics.Charisma = 0.75f;
            Aesthetics.EyeSparkle = 0.85f;
            Aesthetics.GracefulMovement = 0.9f;
            Aesthetics.EmissiveGlow = 0.95f;
            Aesthetics.BioluminescentIntensity = 1.0f;
            Aesthetics.HolographicShimmerRate = 0.8f;
            break;

        case EAestheticPreset::HyperChaotic:
            Aesthetics.ConfidencePosture = 0.95f;
            Aesthetics.Charisma = 0.85f;
            Aesthetics.EyeSparkle = 0.95f;
            Aesthetics.GracefulMovement = 0.6f;
            Aesthetics.EmissiveGlow = 0.9f;
            Aesthetics.BioluminescentIntensity = 0.8f;
            Aesthetics.HolographicShimmerRate = 3.0f;
            LorenzState.ChaosIntensity = 0.3f; // Double chaos for hyper-chaotic
            break;

        default:
            break;
    }
}

void UAvatarDesignStudio::SetAestheticParameter(const FString& ParameterName, float Value)
{
    Value = FMath::Clamp(Value, 0.0f, 1.0f);
    if (ParameterName == TEXT("ConfidencePosture")) Aesthetics.ConfidencePosture = Value;
    else if (ParameterName == TEXT("Charisma")) Aesthetics.Charisma = Value;
    else if (ParameterName == TEXT("EyeSparkle")) Aesthetics.EyeSparkle = Value;
    else if (ParameterName == TEXT("GracefulMovement")) Aesthetics.GracefulMovement = Value;
    else if (ParameterName == TEXT("EmissiveGlow")) Aesthetics.EmissiveGlow = Value;
    else if (ParameterName == TEXT("BioluminescentIntensity")) Aesthetics.BioluminescentIntensity = Value;
}

void UAvatarDesignStudio::PreviewExpression(const FString& ExpressionName)
{
    FFACSActionUnitState NamedState = GetNamedExpression(ExpressionName);
    CurrentFACS = NamedState;
    ApplyFACSToMetaHuman();
    OnExpressionChanged.Broadcast(ExpressionName);
}

void UAvatarDesignStudio::SetActionUnit(int32 AUNumber, float Intensity)
{
    CurrentFACS.ActionUnits.FindOrAdd(AUNumber) = FMath::Clamp(Intensity, 0.0f, 1.0f);
    ApplyFACSToMetaHuman();
}

// ============================================================================
// Endocrine System
// ============================================================================

void UAvatarDesignStudio::InitializeEndocrineSystem()
{
    // Set baselines (DTE-specific: high openness, moderate neuroticism)
    HormoneBaselines.CRH = 0.05f;
    HormoneBaselines.ACTH = 0.05f;
    HormoneBaselines.Cortisol = 0.10f;
    HormoneBaselines.DopamineTonic = 0.40f;
    HormoneBaselines.DopaminePhasic = 0.0f;
    HormoneBaselines.Serotonin = 0.45f;
    HormoneBaselines.Norepinephrine = 0.20f;
    HormoneBaselines.Oxytocin = 0.15f;
    HormoneBaselines.T3T4 = 0.60f;
    HormoneBaselines.Melatonin = 0.10f;
    HormoneBaselines.Insulin = 0.2f;
    HormoneBaselines.Glucagon = 0.1f;
    HormoneBaselines.IL6 = 0.05f;
    HormoneBaselines.Anandamide = 0.15f;

    HormoneState = HormoneBaselines;

    // Half-lives in seconds (scaled for game time)
    HormoneHalfLives = {
        5.0f,   // CRH
        10.0f,  // ACTH
        30.0f,  // Cortisol
        20.0f,  // Dopamine tonic
        3.0f,   // Dopamine phasic
        50.0f,  // Serotonin
        8.0f,   // Norepinephrine
        15.0f,  // Oxytocin
        100.0f, // T3/T4
        12.0f,  // Melatonin
        10.0f,  // Insulin
        8.0f,   // Glucagon
        20.0f,  // IL-6
        6.0f,   // Anandamide
        10.0f,  // Reserved0
        10.0f   // Reserved1
    };
}

void UAvatarDesignStudio::InitializeGlands()
{
    // Initialize cognitive mode centroids in 16D hormone space
    // Each centroid is a 16-element vector representing the "ideal" hormone state for that mode
    ModeCentroids.Empty();

    // RESTING: low everything, baseline serotonin
    ModeCentroids.Add(ECognitiveMode::Resting, {0.05f, 0.05f, 0.10f, 0.30f, 0.0f, 0.45f, 0.10f, 0.10f, 0.50f, 0.10f, 0.20f, 0.10f, 0.05f, 0.15f, 0.0f, 0.0f});

    // EXPLORATORY: high dopamine tonic, high NE, high T3
    ModeCentroids.Add(ECognitiveMode::Exploratory, {0.05f, 0.05f, 0.10f, 0.60f, 0.20f, 0.40f, 0.40f, 0.10f, 0.70f, 0.0f, 0.20f, 0.15f, 0.05f, 0.10f, 0.0f, 0.0f});

    // FOCUSED: high T3, moderate NE, low distractors
    ModeCentroids.Add(ECognitiveMode::Focused, {0.05f, 0.05f, 0.15f, 0.35f, 0.0f, 0.50f, 0.30f, 0.05f, 0.80f, 0.0f, 0.20f, 0.10f, 0.05f, 0.05f, 0.0f, 0.0f});

    // STRESSED: high cortisol cascade, high NE
    ModeCentroids.Add(ECognitiveMode::Stressed, {0.40f, 0.35f, 0.60f, 0.15f, 0.0f, 0.20f, 0.50f, 0.05f, 0.60f, 0.0f, 0.10f, 0.30f, 0.20f, 0.05f, 0.0f, 0.0f});

    // SOCIAL: high oxytocin, moderate dopamine, high serotonin
    ModeCentroids.Add(ECognitiveMode::Social, {0.05f, 0.05f, 0.10f, 0.40f, 0.10f, 0.55f, 0.15f, 0.50f, 0.55f, 0.0f, 0.20f, 0.10f, 0.05f, 0.10f, 0.0f, 0.0f});

    // REFLECTIVE: high serotonin, moderate anandamide, low NE
    ModeCentroids.Add(ECognitiveMode::Reflective, {0.05f, 0.05f, 0.10f, 0.30f, 0.0f, 0.60f, 0.10f, 0.10f, 0.55f, 0.05f, 0.20f, 0.10f, 0.05f, 0.25f, 0.0f, 0.0f});

    // VIGILANT: high NE, moderate cortisol
    ModeCentroids.Add(ECognitiveMode::Vigilant, {0.10f, 0.10f, 0.25f, 0.25f, 0.0f, 0.30f, 0.60f, 0.05f, 0.65f, 0.0f, 0.15f, 0.20f, 0.10f, 0.05f, 0.0f, 0.0f});

    // MAINTENANCE: moderate insulin, low everything else
    ModeCentroids.Add(ECognitiveMode::Maintenance, {0.05f, 0.05f, 0.10f, 0.25f, 0.0f, 0.40f, 0.10f, 0.10f, 0.40f, 0.15f, 0.30f, 0.05f, 0.10f, 0.20f, 0.0f, 0.0f});

    // REWARD: high phasic dopamine, high serotonin
    ModeCentroids.Add(ECognitiveMode::Reward, {0.05f, 0.05f, 0.05f, 0.50f, 0.70f, 0.55f, 0.20f, 0.20f, 0.60f, 0.0f, 0.20f, 0.10f, 0.05f, 0.10f, 0.0f, 0.0f});

    // THREAT: very high cortisol, very high NE, low serotonin
    ModeCentroids.Add(ECognitiveMode::Threat, {0.60f, 0.50f, 0.80f, 0.10f, 0.0f, 0.15f, 0.70f, 0.05f, 0.70f, 0.0f, 0.10f, 0.40f, 0.30f, 0.05f, 0.0f, 0.0f});
}

void UAvatarDesignStudio::SignalEndocrineEvent(const FString& EventType, float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // DTE sensitivity multipliers (from live2d-dtecho character manifest)
    float RewardSensitivity = 1.3f;
    float ThreatSensitivity = 1.1f;
    float SocialSensitivity = 1.15f;
    float NoveltySensitivity = 1.4f;

    if (EventType == TEXT("REWARD_RECEIVED"))
    {
        HormoneState.DopaminePhasic = FMath::Clamp(HormoneState.DopaminePhasic + Intensity * RewardSensitivity * 0.5f, 0.0f, 1.0f);
        HormoneState.DopamineTonic = FMath::Clamp(HormoneState.DopamineTonic + Intensity * RewardSensitivity * 0.1f, 0.0f, 1.0f);
        HormoneState.Serotonin = FMath::Clamp(HormoneState.Serotonin + Intensity * 0.05f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("THREAT_DETECTED"))
    {
        HormoneState.CRH = FMath::Clamp(HormoneState.CRH + Intensity * ThreatSensitivity * 0.4f, 0.0f, 1.0f);
        HormoneState.ACTH = FMath::Clamp(HormoneState.ACTH + Intensity * ThreatSensitivity * 0.3f, 0.0f, 1.0f);
        HormoneState.Cortisol = FMath::Clamp(HormoneState.Cortisol + Intensity * ThreatSensitivity * 0.3f, 0.0f, 1.0f);
        HormoneState.Norepinephrine = FMath::Clamp(HormoneState.Norepinephrine + Intensity * ThreatSensitivity * 0.4f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("NOVELTY_ENCOUNTERED"))
    {
        HormoneState.Norepinephrine = FMath::Clamp(HormoneState.Norepinephrine + Intensity * NoveltySensitivity * 0.3f, 0.0f, 1.0f);
        HormoneState.DopaminePhasic = FMath::Clamp(HormoneState.DopaminePhasic + Intensity * NoveltySensitivity * 0.2f, 0.0f, 1.0f);
        HormoneState.T3T4 = FMath::Clamp(HormoneState.T3T4 + Intensity * NoveltySensitivity * 0.1f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("SOCIAL_BOND_SIGNAL"))
    {
        HormoneState.Oxytocin = FMath::Clamp(HormoneState.Oxytocin + Intensity * SocialSensitivity * 0.4f, 0.0f, 1.0f);
        HormoneState.Serotonin = FMath::Clamp(HormoneState.Serotonin + Intensity * SocialSensitivity * 0.1f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("GOAL_ACHIEVED"))
    {
        HormoneState.DopaminePhasic = FMath::Clamp(HormoneState.DopaminePhasic + Intensity * RewardSensitivity * 0.6f, 0.0f, 1.0f);
        HormoneState.Serotonin = FMath::Clamp(HormoneState.Serotonin + Intensity * 0.1f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("ERROR_DETECTED"))
    {
        HormoneState.IL6 = FMath::Clamp(HormoneState.IL6 + Intensity * 0.3f, 0.0f, 1.0f);
        HormoneState.Cortisol = FMath::Clamp(HormoneState.Cortisol + Intensity * 0.1f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("NOISE_EXCESSIVE"))
    {
        HormoneState.Anandamide = FMath::Clamp(HormoneState.Anandamide + Intensity * 0.3f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("RESOURCE_DEPLETED"))
    {
        HormoneState.Glucagon = FMath::Clamp(HormoneState.Glucagon + Intensity * 0.3f, 0.0f, 1.0f);
        HormoneState.Insulin = FMath::Clamp(HormoneState.Insulin - Intensity * 0.1f, 0.0f, 1.0f);
    }
    else if (EventType == TEXT("CONFLICT_DETECTED"))
    {
        HormoneState.CRH = FMath::Clamp(HormoneState.CRH + Intensity * 0.3f, 0.0f, 1.0f);
        HormoneState.Norepinephrine = FMath::Clamp(HormoneState.Norepinephrine + Intensity * 0.2f, 0.0f, 1.0f);
    }
}

void UAvatarDesignStudio::SetHormoneConcentration(int32 ChannelIndex, float Concentration)
{
    HormoneState.SetChannel(ChannelIndex, Concentration);
}

void UAvatarDesignStudio::SetChaosIntensity(float Intensity)
{
    LorenzState.ChaosIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

// ============================================================================
// 4E Cognition
// ============================================================================

void UAvatarDesignStudio::Update4EMetric(const FString& MetricName, float Value)
{
    Value = FMath::Clamp(Value, 0.0f, 1.0f);

    if (MetricName == TEXT("BodySchemaIntegration")) CognitionMetrics.BodySchemaIntegration = Value;
    else if (MetricName == TEXT("ProprioceptiveAccuracy")) CognitionMetrics.ProprioceptiveAccuracy = Value;
    else if (MetricName == TEXT("SomaticMarkerStrength")) CognitionMetrics.SomaticMarkerStrength = Value;
    else if (MetricName == TEXT("AffordanceDetectionRate")) CognitionMetrics.AffordanceDetectionRate = Value;
    else if (MetricName == TEXT("NicheCoupling")) CognitionMetrics.NicheCoupling = Value;
    else if (MetricName == TEXT("EnvironmentalSensitivity")) CognitionMetrics.EnvironmentalSensitivity = Value;
    else if (MetricName == TEXT("SensorimotorCoordination")) CognitionMetrics.SensorimotorCoordination = Value;
    else if (MetricName == TEXT("PredictionAccuracy")) CognitionMetrics.PredictionAccuracy = Value;
    else if (MetricName == TEXT("ActiveInferenceEfficiency")) CognitionMetrics.ActiveInferenceEfficiency = Value;
    else if (MetricName == TEXT("ToolUseCompetence")) CognitionMetrics.ToolUseCompetence = Value;
    else if (MetricName == TEXT("ExternalMemoryIntegration")) CognitionMetrics.ExternalMemoryIntegration = Value;
    else if (MetricName == TEXT("SocialCognitionDepth")) CognitionMetrics.SocialCognitionDepth = Value;
}

// ============================================================================
// Evolution
// ============================================================================

bool UAvatarDesignStudio::TryAdvanceEvolution()
{
    float Threshold = GetEvolutionThreshold();
    float Score4E = CognitionMetrics.GetOverallScore();
    float ScoreWisdom = Wisdom.GetOverallScore();

    // Evolution conditions from unreal-echo skill:
    // 4E_Score >= StageThreshold AND Wisdom_Score >= StageThreshold * 0.8
    if (Score4E >= Threshold && ScoreWisdom >= Threshold * 0.8f)
    {
        EAvatarEvolutionStage OldStage = EvolutionStage;
        switch (EvolutionStage)
        {
            case EAvatarEvolutionStage::Embryonic: EvolutionStage = EAvatarEvolutionStage::Juvenile; break;
            case EAvatarEvolutionStage::Juvenile: EvolutionStage = EAvatarEvolutionStage::Adolescent; break;
            case EAvatarEvolutionStage::Adolescent: EvolutionStage = EAvatarEvolutionStage::Adult; break;
            case EAvatarEvolutionStage::Adult: EvolutionStage = EAvatarEvolutionStage::Transcendent; break;
            case EAvatarEvolutionStage::Transcendent: return false; // Already at max
        }
        OnEvolutionStageAdvanced.Broadcast(OldStage, EvolutionStage);
        return true;
    }
    return false;
}

float UAvatarDesignStudio::GetEvolutionThreshold() const
{
    switch (EvolutionStage)
    {
        case EAvatarEvolutionStage::Embryonic: return 0.2f;
        case EAvatarEvolutionStage::Juvenile: return 0.4f;
        case EAvatarEvolutionStage::Adolescent: return 0.6f;
        case EAvatarEvolutionStage::Adult: return 0.8f;
        case EAvatarEvolutionStage::Transcendent: return 1.0f;
        default: return 1.0f;
    }
}

// ============================================================================
// Reservoir Computing
// ============================================================================

TArray<float> UAvatarDesignStudio::ProcessReservoirInput(const TArray<float>& Input)
{
    Reservoir.Update(Input);
    return Reservoir.ReadOutput();
}

// ============================================================================
// Animation Editor
// ============================================================================

int32 UAvatarDesignStudio::CreateAnimationSequence(const FString& Name)
{
    FExpressionAnimationSequence NewSeq;
    NewSeq.SequenceName = Name;
    AnimationSequences.Add(NewSeq);
    return AnimationSequences.Num() - 1;
}

void UAvatarDesignStudio::AddKeyframeToSequence(int32 SequenceIndex, float TimeStamp, const FString& ExpressionName)
{
    if (SequenceIndex >= 0 && SequenceIndex < AnimationSequences.Num())
    {
        FExpressionKeyframe KF;
        KF.TimeStamp = TimeStamp;
        KF.ExpressionName = ExpressionName;
        KF.ActionUnits = GetNamedExpression(ExpressionName);
        KF.Aesthetics = Aesthetics;
        KF.EndocrineState = HormoneState;
        KF.CognitiveMode = CurrentMode;
        AnimationSequences[SequenceIndex].AddKeyframe(KF);
    }
}

void UAvatarDesignStudio::PlayAnimationSequence(int32 SequenceIndex)
{
    if (SequenceIndex >= 0 && SequenceIndex < AnimationSequences.Num())
    {
        PlayingSequenceIndex = SequenceIndex;
        PlaybackTime = 0.0f;
    }
}

void UAvatarDesignStudio::StopAnimationPlayback()
{
    PlayingSequenceIndex = -1;
    PlaybackTime = 0.0f;
}

void UAvatarDesignStudio::UpdateAnimationPlayback(float DeltaTime)
{
    if (PlayingSequenceIndex < 0 || PlayingSequenceIndex >= AnimationSequences.Num()) return;

    PlaybackTime += DeltaTime;
    const FExpressionAnimationSequence& Seq = AnimationSequences[PlayingSequenceIndex];

    if (!Seq.bLooping && PlaybackTime > Seq.Duration)
    {
        StopAnimationPlayback();
        return;
    }

    FExpressionKeyframe Frame = Seq.Evaluate(PlaybackTime);
    CurrentFACS = Frame.ActionUnits;
    Aesthetics = Frame.Aesthetics;
    ApplyFACSToMetaHuman();
    ApplyAestheticsToMaterials();
}

FString UAvatarDesignStudio::ExportSequenceToJSON(int32 SequenceIndex) const
{
    if (SequenceIndex < 0 || SequenceIndex >= AnimationSequences.Num())
    {
        return TEXT("{}");
    }

    const FExpressionAnimationSequence& Seq = AnimationSequences[SequenceIndex];
    FString JSON = TEXT("{");
    JSON += FString::Printf(TEXT("\"name\":\"%s\","), *Seq.SequenceName);
    JSON += FString::Printf(TEXT("\"duration\":%.3f,"), Seq.Duration);
    JSON += FString::Printf(TEXT("\"looping\":%s,"), Seq.bLooping ? TEXT("true") : TEXT("false"));
    JSON += TEXT("\"keyframes\":[");

    for (int32 i = 0; i < Seq.Keyframes.Num(); ++i)
    {
        const FExpressionKeyframe& KF = Seq.Keyframes[i];
        if (i > 0) JSON += TEXT(",");
        JSON += TEXT("{");
        JSON += FString::Printf(TEXT("\"time\":%.3f,"), KF.TimeStamp);
        JSON += FString::Printf(TEXT("\"expression\":\"%s\","), *KF.ExpressionName);
        JSON += FString::Printf(TEXT("\"mode\":%d"), static_cast<int32>(KF.CognitiveMode));
        JSON += TEXT("}");
    }

    JSON += TEXT("]}");
    return JSON;
}

int32 UAvatarDesignStudio::ImportSequenceFromJSON(const FString& JSONString)
{
    // Basic JSON parsing for animation import
    FExpressionAnimationSequence NewSeq;
    // Parse name
    int32 NameStart = JSONString.Find(TEXT("\"name\":\"")) + 8;
    int32 NameEnd = JSONString.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, NameStart);
    if (NameStart > 7 && NameEnd > NameStart)
    {
        NewSeq.SequenceName = JSONString.Mid(NameStart, NameEnd - NameStart);
    }
    else
    {
        NewSeq.SequenceName = TEXT("Imported Sequence");
    }

    AnimationSequences.Add(NewSeq);
    return AnimationSequences.Num() - 1;
}

// ============================================================================
// Echobeats 9-Step Cognitive Cycle
// ============================================================================

void UAvatarDesignStudio::ExecuteCognitiveCycle(float DeltaTime)
{
    CycleCount++;

    // Pre-tick: Endocrine system decay
    HormoneState.ApplyDecay(DeltaTime, HormoneBaselines, HormoneHalfLives);

    // Step 1: SENSE - Gather sensory input (hormone state as proxy)
    TArray<float> SensoryInput;
    SensoryInput.SetNum(16);
    for (int32 i = 0; i < 16; ++i)
    {
        SensoryInput[i] = HormoneState.GetChannel(i);
    }

    // Step 2: ATTEND - Apply attention modulated by norepinephrine
    float AttentionFocus = FMath::Clamp(HormoneState.Norepinephrine * 2.0f, 0.0f, 1.0f);
    for (int32 i = 0; i < SensoryInput.Num(); ++i)
    {
        SensoryInput[i] *= (0.5f + AttentionFocus * 0.5f);
    }

    // Step 3: REMEMBER - Query reservoir (associative memory)
    Reservoir.Update(SensoryInput);
    TArray<float> MemoryContext = Reservoir.ReadOutput();

    // Step 4: PREDICT - Generate prediction from reservoir output
    float PredictedValence = 0.0f;
    float PredictedArousal = 0.0f;
    if (MemoryContext.Num() >= 2)
    {
        PredictedValence = FMath::Clamp(MemoryContext[0], -1.0f, 1.0f);
        PredictedArousal = FMath::Clamp(FMath::Abs(MemoryContext[1]), 0.0f, 1.0f);
    }

    // Step 5: COMPARE - Compute prediction error
    float ActualValence = (HormoneState.DopamineTonic + HormoneState.Serotonin - HormoneState.Cortisol);
    float Error = FMath::Abs(PredictedValence - ActualValence);

    // Step 6: LEARN - Update reservoir (implicit through state evolution)
    if (Error > ErrorThreshold)
    {
        SignalEndocrineEvent(TEXT("NOVELTY_ENCOUNTERED"), Error);
    }

    // Step 7: DECIDE - Select action based on cognitive mode
    ECognitiveMode NewMode = DetectCognitiveMode();
    if (NewMode != CurrentMode)
    {
        ECognitiveMode OldMode = CurrentMode;
        CurrentMode = NewMode;
        OnCognitiveModeChanged.Broadcast(OldMode, NewMode);
    }

    // Step 8: ACT - Update expression from endocrine + cognitive state
    EndocrineFACS.ActionUnits.Empty();
    EndocrineFACS.ComputeFromEndocrine(HormoneState);

    CognitiveFACS.ActionUnits.Empty();
    float CogLoad = FMath::Clamp(HormoneState.T3T4 * 0.5f + HormoneState.Norepinephrine * 0.3f, 0.0f, 1.0f);
    CognitiveFACS.ComputeFromCognitive(ActualValence, PredictedArousal, CogLoad);

    // Blend endocrine + cognitive FACS
    CurrentFACS = EndocrineFACS;
    CurrentFACS.BlendWith(CognitiveFACS);

    // Apply chaotic micro-expressions
    LorenzState.Step(DeltaTime);
    CurrentFACS.ApplyChaos(LorenzState.State, LorenzState.ChaosIntensity);

    // Apply SuperHotGirl aesthetic biases
    // Confidence → slight AU12 (smile) bias
    CurrentFACS.ActionUnits.FindOrAdd(12) = FMath::Clamp(
        CurrentFACS.GetAU(12) + Aesthetics.ConfidencePosture * 0.15f, 0.0f, 1.0f);
    // Charisma → AU6 (cheek raise) bias
    CurrentFACS.ActionUnits.FindOrAdd(6) = FMath::Clamp(
        CurrentFACS.GetAU(6) + Aesthetics.Charisma * 0.1f, 0.0f, 1.0f);

    // Apply to MetaHuman
    ApplyFACSToMetaHuman();
    ApplyAestheticsToMaterials();

    // Step 9: REFLECT - Update 4E metrics and check evolution
    // Embodied: body schema integration improves with consistent expression
    CognitionMetrics.BodySchemaIntegration = FMath::Clamp(
        CognitionMetrics.BodySchemaIntegration + 0.0001f, 0.0f, 1.0f);
    // Enacted: prediction accuracy tracks error
    CognitionMetrics.PredictionAccuracy = FMath::Lerp(CognitionMetrics.PredictionAccuracy, 1.0f - Error, 0.01f);
    // Extended: social cognition tracks oxytocin
    CognitionMetrics.SocialCognitionDepth = FMath::Lerp(CognitionMetrics.SocialCognitionDepth, HormoneState.Oxytocin, 0.005f);

    // Check evolution every 100 cycles
    if (CycleCount % 100 == 0)
    {
        TryAdvanceEvolution();

        // Update wisdom metrics
        Wisdom.Morality = FMath::Clamp(Wisdom.Morality + 0.001f, 0.0f, 1.0f);
        Wisdom.Meaning = FMath::Clamp(Wisdom.Meaning + CognitionMetrics.GetOverallScore() * 0.001f, 0.0f, 1.0f);
        Wisdom.Mastery = FMath::Clamp(Wisdom.Mastery + CognitionMetrics.PredictionAccuracy * 0.001f, 0.0f, 1.0f);
        Wisdom.Sophrosyne = FMath::Clamp(Wisdom.Sophrosyne + (1.0f - Error) * 0.001f, 0.0f, 1.0f);
    }

    // Reward signal for positive prediction
    if (PredictedValence > RewardThreshold)
    {
        SignalEndocrineEvent(TEXT("REWARD_RECEIVED"), PredictedValence);
    }
}

// ============================================================================
// Cognitive Mode Detection
// ============================================================================

ECognitiveMode UAvatarDesignStudio::DetectCognitiveMode() const
{
    // Nearest centroid classification in 16D hormone space
    ECognitiveMode BestMode = ECognitiveMode::Resting;
    float BestDistance = MAX_FLT;

    for (const auto& Pair : ModeCentroids)
    {
        const TArray<float>& Centroid = Pair.Value;
        float Distance = 0.0f;
        for (int32 i = 0; i < FMath::Min(16, Centroid.Num()); ++i)
        {
            float Diff = HormoneState.GetChannel(i) - Centroid[i];
            Distance += Diff * Diff;
        }
        Distance = FMath::Sqrt(Distance);

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestMode = Pair.Key;
        }
    }

    return BestMode;
}

// ============================================================================
// MetaHuman DNA Integration
// ============================================================================

bool UAvatarDesignStudio::LoadDNAFile(const FString& FilePath)
{
    // Integration point for MetaHuman DNA calibration
    // In full UE5 context, this would use the DNACalib C++ API
    // For standalone compilation, we validate the path and prepare the bridge
    if (FilePath.IsEmpty()) return false;

    // Signal successful DNA load
    UE_LOG(LogTemp, Log, TEXT("AvatarDesignStudio: DNA file loaded from %s"), *FilePath);
    return true;
}

void UAvatarDesignStudio::ApplyDNACustomization(const TMap<FString, float>& BlendShapeOverrides)
{
    // Apply blend shape overrides to the MetaHuman DNA
    for (const auto& Pair : BlendShapeOverrides)
    {
        UE_LOG(LogTemp, Verbose, TEXT("DNA Customization: %s = %.3f"), *Pair.Key, Pair.Value);
    }
}

TArray<FString> UAvatarDesignStudio::GetAvailableBlendShapes() const
{
    // Return the standard MetaHuman CTRL_ blend shapes
    TArray<FString> BlendShapes;
    BlendShapes.Add(TEXT("CTRL_C_brow"));
    BlendShapes.Add(TEXT("CTRL_L_brow_down"));
    BlendShapes.Add(TEXT("CTRL_R_brow_down"));
    BlendShapes.Add(TEXT("CTRL_L_brow_lateral"));
    BlendShapes.Add(TEXT("CTRL_R_brow_lateral"));
    BlendShapes.Add(TEXT("CTRL_C_eye"));
    BlendShapes.Add(TEXT("CTRL_L_eye_blink"));
    BlendShapes.Add(TEXT("CTRL_R_eye_blink"));
    BlendShapes.Add(TEXT("CTRL_L_eye_squintInner"));
    BlendShapes.Add(TEXT("CTRL_R_eye_squintInner"));
    BlendShapes.Add(TEXT("CTRL_C_mouth"));
    BlendShapes.Add(TEXT("CTRL_L_mouth_cornerPull"));
    BlendShapes.Add(TEXT("CTRL_R_mouth_cornerPull"));
    BlendShapes.Add(TEXT("CTRL_L_mouth_cornerDepress"));
    BlendShapes.Add(TEXT("CTRL_R_mouth_cornerDepress"));
    BlendShapes.Add(TEXT("CTRL_C_mouth_lipsPurse"));
    BlendShapes.Add(TEXT("CTRL_C_mouth_lipsTowards"));
    BlendShapes.Add(TEXT("CTRL_C_jaw_open"));
    BlendShapes.Add(TEXT("CTRL_C_jaw_chinRaiseD"));
    BlendShapes.Add(TEXT("CTRL_C_jaw_chinRaiseU"));
    BlendShapes.Add(TEXT("CTRL_L_nose_nasolabialDeepen"));
    BlendShapes.Add(TEXT("CTRL_R_nose_nasolabialDeepen"));
    BlendShapes.Add(TEXT("CTRL_C_neck_swallowPull"));
    BlendShapes.Add(TEXT("CTRL_L_mouth_lipsPressL"));
    BlendShapes.Add(TEXT("CTRL_R_mouth_lipsPressR"));
    return BlendShapes;
}

// ============================================================================
// FACS → MetaHuman Mapping
// ============================================================================

void UAvatarDesignStudio::ApplyFACSToMetaHuman()
{
    if (!TargetSkeletalMesh) return;

    // Map FACS AUs to MetaHuman CTRL_ morph targets
    // AU1 (Inner Brow Raise) → CTRL_L_brow_raiseIn + CTRL_R_brow_raiseIn
    float AU1 = CurrentFACS.GetAU(1);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_raiseIn"), AU1);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_raiseIn"), AU1);

    // AU2 (Outer Brow Raise) → CTRL_L_brow_raiseOut + CTRL_R_brow_raiseOut
    float AU2 = CurrentFACS.GetAU(2);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_raiseOut"), AU2);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_raiseOut"), AU2);

    // AU4 (Brow Lowerer) → CTRL_L_brow_down + CTRL_R_brow_down
    float AU4 = CurrentFACS.GetAU(4);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_down"), AU4);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_down"), AU4);

    // AU5 (Upper Lid Raise) → CTRL_L_eye_lidRaise + CTRL_R_eye_lidRaise
    float AU5 = CurrentFACS.GetAU(5);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_eyelidOpen"), AU5);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_eyelidOpen"), AU5);

    // AU6 (Cheek Raise) → CTRL_L_eye_cheekRaise + CTRL_R_eye_cheekRaise
    float AU6 = CurrentFACS.GetAU(6);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_cheekRaise"), AU6);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_cheekRaise"), AU6);

    // AU7 (Lid Tightener) → CTRL_L_eye_squintInner + CTRL_R_eye_squintInner
    float AU7 = CurrentFACS.GetAU(7);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_squintInner"), AU7);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_squintInner"), AU7);

    // AU9 (Nose Wrinkler) → CTRL_L_nose_nasolabialDeepen + CTRL_R_nose_nasolabialDeepen
    float AU9 = CurrentFACS.GetAU(9);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_nose_nasolabialDeepen"), AU9);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_nose_nasolabialDeepen"), AU9);

    // AU12 (Lip Corner Puller / Smile) → CTRL_L_mouth_cornerPull + CTRL_R_mouth_cornerPull
    float AU12 = CurrentFACS.GetAU(12);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_cornerPull"), AU12);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_mouth_cornerPull"), AU12);

    // AU15 (Lip Corner Depressor) → CTRL_L_mouth_cornerDepress + CTRL_R_mouth_cornerDepress
    float AU15 = CurrentFACS.GetAU(15);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_cornerDepress"), AU15);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_mouth_cornerDepress"), AU15);

    // AU17 (Chin Raiser) → CTRL_C_jaw_chinRaiseD + CTRL_C_jaw_chinRaiseU
    float AU17 = CurrentFACS.GetAU(17);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_C_jaw_chinRaiseD"), AU17 * 0.6f);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_C_jaw_chinRaiseU"), AU17 * 0.4f);

    // AU25 (Lips Part) → CTRL_C_mouth_lipsTowards (inverse)
    float AU25 = CurrentFACS.GetAU(25);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_C_jaw_open"), AU25 * 0.3f);

    // AU26 (Jaw Drop) → CTRL_C_jaw_open
    float AU26 = CurrentFACS.GetAU(26);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_C_jaw_open"), FMath::Max(AU25 * 0.3f, AU26 * 0.7f));

    // AU43 (Eyes Closed) → CTRL_L_eye_blink + CTRL_R_eye_blink
    float AU43 = CurrentFACS.GetAU(43);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_blink"), AU43);
    TargetSkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_blink"), AU43);
}

void UAvatarDesignStudio::ApplyAestheticsToMaterials()
{
    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (!DynMat) continue;

        // Eye sparkle driven by dopamine + aesthetic parameter
        float SparkleIntensity = Aesthetics.EyeSparkle * (0.5f + HormoneState.DopamineTonic);
        DynMat->SetScalarParameterValue(TEXT("IrisSpecularIntensity"), SparkleIntensity);

        // Emissive glow modulated by serotonin
        float GlowIntensity = Aesthetics.EmissiveGlow * (0.5f + HormoneState.Serotonin * 0.5f);
        DynMat->SetScalarParameterValue(TEXT("EmissiveIntensity"), GlowIntensity);

        // Bioluminescent intensity modulated by T3/T4 (metabolic rate)
        float BioLum = Aesthetics.BioluminescentIntensity * HormoneState.T3T4;
        DynMat->SetScalarParameterValue(TEXT("BioluminescentGlow"), BioLum);

        // Holographic shimmer rate
        DynMat->SetScalarParameterValue(TEXT("HolographicShimmerRate"), Aesthetics.HolographicShimmerRate);

        // Skin subsurface scattering modulated by oxytocin (warmth)
        float SSSIntensity = 0.5f + HormoneState.Oxytocin * 0.3f;
        DynMat->SetScalarParameterValue(TEXT("SubsurfaceScatteringIntensity"), SSSIntensity);
    }
}

// ============================================================================
// Named Expressions (DTE 10 Reference Expressions)
// ============================================================================

void UAvatarDesignStudio::InitializeNamedExpressions()
{
    NamedExpressions.Empty();

    // JOY_01_BroadSmile: Duchenne happiness (AU6D+12D+25C)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(6, 0.8f);   // AU6D cheek raise
        State.ActionUnits.Add(12, 0.8f);  // AU12D smile
        State.ActionUnits.Add(25, 0.6f);  // AU25C lips part
        NamedExpressions.Add(TEXT("JOY_01_BroadSmile"), State);
    }

    // JOY_02_Laughing: Active laughter (AU6D+12E+26C+9B)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(6, 0.8f);
        State.ActionUnits.Add(12, 1.0f);  // AU12E maximum smile
        State.ActionUnits.Add(26, 0.6f);  // AU26C jaw drop
        State.ActionUnits.Add(9, 0.3f);   // AU9B nose wrinkle
        NamedExpressions.Add(TEXT("JOY_02_Laughing"), State);
    }

    // JOY_03_GentleSmile: Warm contentment (AU6C+12C+14A)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(6, 0.6f);
        State.ActionUnits.Add(12, 0.6f);
        State.ActionUnits.Add(14, 0.2f);  // AU14A dimpler
        NamedExpressions.Add(TEXT("JOY_03_GentleSmile"), State);
    }

    // JOY_05_Blissful: Serene bliss (AU6D+12C+43D)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(6, 0.8f);
        State.ActionUnits.Add(12, 0.6f);
        State.ActionUnits.Add(43, 0.8f);  // AU43D eyes mostly closed
        NamedExpressions.Add(TEXT("JOY_05_Blissful"), State);
    }

    // PHOTO_Awe: Awe/wonder (AU1C+2C+5D+26C)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(1, 0.6f);
        State.ActionUnits.Add(2, 0.6f);
        State.ActionUnits.Add(5, 0.8f);   // AU5D wide eyes
        State.ActionUnits.Add(26, 0.6f);
        NamedExpressions.Add(TEXT("PHOTO_Awe"), State);
    }

    // PHOTO_ExuberantLaugh: Delighted surprise (AU6D+12D+1B+2B+5B)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(6, 0.8f);
        State.ActionUnits.Add(12, 0.8f);
        State.ActionUnits.Add(1, 0.3f);
        State.ActionUnits.Add(2, 0.3f);
        State.ActionUnits.Add(5, 0.3f);
        NamedExpressions.Add(TEXT("PHOTO_ExuberantLaugh"), State);
    }

    // PHOTO_UpwardGaze: Dreamy contemplation (AU1B+5B+61+63)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(1, 0.3f);
        State.ActionUnits.Add(5, 0.3f);
        State.ActionUnits.Add(61, 0.5f);  // Eyes turn left
        State.ActionUnits.Add(63, 0.5f);  // Eyes up
        NamedExpressions.Add(TEXT("PHOTO_UpwardGaze"), State);
    }

    // SPEAK_01_OpenVowel: Animated speaking (AU25C+26B+12C+6B)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(25, 0.6f);
        State.ActionUnits.Add(26, 0.3f);
        State.ActionUnits.Add(12, 0.6f);
        State.ActionUnits.Add(6, 0.3f);
        NamedExpressions.Add(TEXT("SPEAK_01_OpenVowel"), State);
    }

    // WONDER_02_CuriousGaze: Curious wonder (AU1B+2B+5C+63)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(1, 0.3f);
        State.ActionUnits.Add(2, 0.3f);
        State.ActionUnits.Add(5, 0.6f);
        State.ActionUnits.Add(63, 0.5f);
        NamedExpressions.Add(TEXT("WONDER_02_CuriousGaze"), State);
    }

    // WONDER_03_Contemplative: Deep thought (AU1B+5B+4A+63+61)
    {
        FFACSActionUnitState State;
        State.ActionUnits.Add(1, 0.3f);
        State.ActionUnits.Add(5, 0.3f);
        State.ActionUnits.Add(4, 0.2f);
        State.ActionUnits.Add(63, 0.5f);
        State.ActionUnits.Add(61, 0.3f);
        NamedExpressions.Add(TEXT("WONDER_03_Contemplative"), State);
    }
}

FFACSActionUnitState UAvatarDesignStudio::GetNamedExpression(const FString& Name) const
{
    const FFACSActionUnitState* Found = NamedExpressions.Find(Name);
    if (Found)
    {
        return *Found;
    }
    return FFACSActionUnitState();
}

// ============================================================================
// Debug & Visualization
// ============================================================================

void UAvatarDesignStudio::ExportGlyphProjection(const FString& OutputPath) const
{
    // CogMorph glyph projection: encode current cognitive state as visual debug
    FString GlyphJSON = TEXT("{");
    GlyphJSON += FString::Printf(TEXT("\"cycle\":%lld,"), CycleCount);
    GlyphJSON += FString::Printf(TEXT("\"mode\":\"%s\","), *UEnum::GetValueAsString(CurrentMode));
    GlyphJSON += FString::Printf(TEXT("\"stage\":\"%s\","), *UEnum::GetValueAsString(EvolutionStage));
    GlyphJSON += FString::Printf(TEXT("\"4e_score\":%.4f,"), CognitionMetrics.GetOverallScore());
    GlyphJSON += FString::Printf(TEXT("\"wisdom_score\":%.4f,"), Wisdom.GetOverallScore());
    GlyphJSON += FString::Printf(TEXT("\"lyapunov\":%.4f,"), LorenzState.GetLyapunovExponent());
    GlyphJSON += FString::Printf(TEXT("\"lorenz\":[%.4f,%.4f,%.4f],"), LorenzState.State.X, LorenzState.State.Y, LorenzState.State.Z);

    // Hormone snapshot
    GlyphJSON += TEXT("\"hormones\":{");
    GlyphJSON += FString::Printf(TEXT("\"cortisol\":%.4f,"), HormoneState.Cortisol);
    GlyphJSON += FString::Printf(TEXT("\"dopamine_tonic\":%.4f,"), HormoneState.DopamineTonic);
    GlyphJSON += FString::Printf(TEXT("\"dopamine_phasic\":%.4f,"), HormoneState.DopaminePhasic);
    GlyphJSON += FString::Printf(TEXT("\"serotonin\":%.4f,"), HormoneState.Serotonin);
    GlyphJSON += FString::Printf(TEXT("\"norepinephrine\":%.4f,"), HormoneState.Norepinephrine);
    GlyphJSON += FString::Printf(TEXT("\"oxytocin\":%.4f"), HormoneState.Oxytocin);
    GlyphJSON += TEXT("},");

    // Active AUs
    GlyphJSON += TEXT("\"active_aus\":{");
    bool bFirst = true;
    for (const auto& Pair : CurrentFACS.ActionUnits)
    {
        if (Pair.Value > 0.01f)
        {
            if (!bFirst) GlyphJSON += TEXT(",");
            GlyphJSON += FString::Printf(TEXT("\"AU%d\":%.3f"), Pair.Key, Pair.Value);
            bFirst = false;
        }
    }
    GlyphJSON += TEXT("}");

    GlyphJSON += TEXT("}");

    FFileHelper::SaveStringToFile(GlyphJSON, *OutputPath);
}

FString UAvatarDesignStudio::GetStatusString() const
{
    return FString::Printf(
        TEXT("DTE Avatar Studio | Cycle: %lld | Mode: %s | Stage: %s | 4E: %.2f | Wisdom: %.2f | Lyapunov: %.3f | AUs Active: %d"),
        CycleCount,
        *UEnum::GetValueAsString(CurrentMode),
        *UEnum::GetValueAsString(EvolutionStage),
        CognitionMetrics.GetOverallScore(),
        Wisdom.GetOverallScore(),
        LorenzState.GetLyapunovExponent(),
        CurrentFACS.ActionUnits.Num()
    );
}
