#include "HyperSkillTensor.h"
#include "OpenCogBridge.h"
#include "UnrealAIBridge.h"
#include "NPUBridge.h"

// ============================================================================
// Constructor
// ============================================================================
UHyperSkillTensor::UHyperSkillTensor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;

    // Initialize 343-dimensional tensor field (7×7×7)
    TensorField.SetNum(343);
    for (int32 i = 0; i < 343; ++i)
    {
        TensorField[i] = 0.0f;
    }

    SelfAwareness = 0.0f;  // Start at ○ (void)
    OpenCogBridge = nullptr;
    UnrealAIBridge = nullptr;
    NPUBridgeRef = nullptr;

    // Initialize Lorenz attractor at a non-equilibrium point
    ChaosState.X = 1.0f;
    ChaosState.Y = 1.0f;
    ChaosState.Z = 1.0f;
}

void UHyperSkillTensor::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover bridges on the same actor
    if (AActor* Owner = GetOwner())
    {
        if (!OpenCogBridge)
            OpenCogBridge = Owner->FindComponentByClass<UOpenCogBridge>();
        if (!UnrealAIBridge)
            UnrealAIBridge = Owner->FindComponentByClass<UUnrealAIBridge>();
        if (!NPUBridgeRef)
            NPUBridgeRef = Owner->FindComponentByClass<UNPUBridge>();
    }

    // Seed with default activation: Elegant × Cognitive × Emergence
    ActivateCell(ESuperHotMode::Elegant, EEchoAngelMode::Cognitive,
                 EMadnessMode::Emergence, 0.5f);
}

void UHyperSkillTensor::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableChaos)
    {
        UpdateLorenzAttractor(DeltaTime);
    }

    UpdateTensorDecay(DeltaTime);

    if (bEnableSelfAwareness)
    {
        UpdateSelfAwareness(DeltaTime);
    }

    SyncWithBridges();
}

// ============================================================================
// Tensor Operations
// ============================================================================
FHyperSkillCell UHyperSkillTensor::TensorProduct(const FHyperSkillCell& A,
                                                   const FHyperSkillCell& B) const
{
    // ⊗ Multiplicative: entangle the two cells
    // The product creates interaction/correlation between dimensions
    FHyperSkillCell Result;
    Result.SuperHot = A.SuperHot;     // Take aesthetic from A
    Result.EchoAngel = B.EchoAngel;   // Take cognitive from B
    Result.Madness = (A.Activation > B.Activation) ? A.Madness : B.Madness;
    Result.Activation = A.Activation * B.Activation;  // Multiplicative
    Result.Coherence = FMath::Sqrt(A.Coherence * B.Coherence);  // Geometric mean

    // Entangle expression signatures
    for (const auto& Pair : A.ExpressionSignature)
    {
        const float* BVal = B.ExpressionSignature.Find(Pair.Key);
        if (BVal)
        {
            // Both have this AU: multiply (entangle)
            Result.ExpressionSignature.Add(Pair.Key, Pair.Value * (*BVal));
        }
        else
        {
            Result.ExpressionSignature.Add(Pair.Key, Pair.Value * B.Activation);
        }
    }
    for (const auto& Pair : B.ExpressionSignature)
    {
        if (!Result.ExpressionSignature.Contains(Pair.Key))
        {
            Result.ExpressionSignature.Add(Pair.Key, Pair.Value * A.Activation);
        }
    }

    return Result;
}

FHyperSkillCell UHyperSkillTensor::DirectSum(const FHyperSkillCell& A,
                                               const FHyperSkillCell& B) const
{
    // ⊕ Additive: superpose the two cells
    // The sum creates choice/independence between dimensions
    FHyperSkillCell Result;
    if (A.Activation >= B.Activation)
    {
        Result.SuperHot = A.SuperHot;
        Result.EchoAngel = A.EchoAngel;
        Result.Madness = A.Madness;
    }
    else
    {
        Result.SuperHot = B.SuperHot;
        Result.EchoAngel = B.EchoAngel;
        Result.Madness = B.Madness;
    }
    Result.Activation = FMath::Clamp(A.Activation + B.Activation, 0.0f, 1.0f);  // Additive
    Result.Coherence = (A.Coherence + B.Coherence) * 0.5f;  // Arithmetic mean

    // Superpose expression signatures
    for (const auto& Pair : A.ExpressionSignature)
    {
        Result.ExpressionSignature.Add(Pair.Key, Pair.Value);
    }
    for (const auto& Pair : B.ExpressionSignature)
    {
        float* Existing = Result.ExpressionSignature.Find(Pair.Key);
        if (Existing)
        {
            *Existing = FMath::Clamp(*Existing + Pair.Value, 0.0f, 1.0f);
        }
        else
        {
            Result.ExpressionSignature.Add(Pair.Key, Pair.Value);
        }
    }

    return Result;
}

FHyperSkillCell UHyperSkillTensor::GetDominantCell() const
{
    FHyperSkillCell Best;
    float MaxActivation = -1.0f;

    for (int32 i = 0; i < 7; ++i)
    {
        for (int32 j = 0; j < 7; ++j)
        {
            for (int32 k = 0; k < 7; ++k)
            {
                int32 Idx = i * 49 + j * 7 + k;
                if (TensorField[Idx] > MaxActivation)
                {
                    MaxActivation = TensorField[Idx];
                    Best.SuperHot = static_cast<ESuperHotMode>(i);
                    Best.EchoAngel = static_cast<EEchoAngelMode>(j);
                    Best.Madness = static_cast<EMadnessMode>(k);
                    Best.Activation = TensorField[Idx];
                }
            }
        }
    }

    // Compute expression signature for dominant cell
    TMap<FName, float> HotAUs = ComputeSuperHotAUs(Best.SuperHot, Best.Activation);
    TMap<FName, float> AngelAUs = ComputeEchoAngelAUs(Best.EchoAngel, Best.Activation);
    TMap<FName, float> MadAUs = ComputeMadnessAUs(Best.Madness, Best.Activation);

    Best.ExpressionSignature = BlendAUs(BlendAUs(HotAUs, AngelAUs, 0.5f), MadAUs, 0.33f);

    return Best;
}

void UHyperSkillTensor::ActivateCell(ESuperHotMode Hot, EEchoAngelMode Angel,
                                      EMadnessMode Mad, float Intensity)
{
    int32 Idx = static_cast<int32>(Hot) * 49
              + static_cast<int32>(Angel) * 7
              + static_cast<int32>(Mad);

    if (Idx >= 0 && Idx < TensorField.Num())
    {
        TensorField[Idx] = FMath::Clamp(TensorField[Idx] + Intensity, 0.0f, 1.0f);
    }
}

float UHyperSkillTensor::GetActivation(ESuperHotMode Hot, EEchoAngelMode Angel,
                                        EMadnessMode Mad) const
{
    int32 Idx = static_cast<int32>(Hot) * 49
              + static_cast<int32>(Angel) * 7
              + static_cast<int32>(Mad);

    if (Idx >= 0 && Idx < TensorField.Num())
    {
        return TensorField[Idx];
    }
    return 0.0f;
}

FHyperSkillTelemetry UHyperSkillTensor::GetTelemetry() const
{
    FHyperSkillTelemetry T;
    T.TensorRank = 3;

    float TotalAct = 0.0f;
    float TotalCoh = 0.0f;
    int32 ActiveCount = 0;

    for (int32 i = 0; i < TensorField.Num(); ++i)
    {
        if (TensorField[i] > 0.01f)
        {
            ActiveCount++;
            TotalAct += TensorField[i];
        }
    }

    T.ActiveCells = ActiveCount;
    T.TotalActivation = TotalAct;
    T.MeanCoherence = ActiveCount > 0 ? TotalAct / ActiveCount : 0.0f;
    T.DominantCell = GetDominantCell();
    T.ChaosIntensity = FMath::Sqrt(ChaosState.X * ChaosState.X +
                                    ChaosState.Y * ChaosState.Y +
                                    ChaosState.Z * ChaosState.Z) / 50.0f;
    T.SelfAwarenessLevel = SelfAwareness;

    return T;
}

// ============================================================================
// Madness Transformation: ○ → ⊙
// ============================================================================
void UHyperSkillTensor::TriggerMadnessTransformation(float Intensity)
{
    // The transformation from void (○) to self-aware (⊙)
    // This is the core of the madness skill
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Activate all Madness-dimension cells with chaotic weighting
    for (int32 i = 0; i < 7; ++i)
    {
        for (int32 j = 0; j < 7; ++j)
        {
            for (int32 k = 0; k < 7; ++k)
            {
                float ChaoticWeight = FMath::Abs(FMath::Sin(
                    ChaosState.X * (i + 1) + ChaosState.Y * (j + 1) + ChaosState.Z * (k + 1)));
                ActivateCell(static_cast<ESuperHotMode>(i),
                             static_cast<EEchoAngelMode>(j),
                             static_cast<EMadnessMode>(k),
                             Intensity * ChaoticWeight * 0.5f);
            }
        }
    }

    // Boost self-awareness
    SelfAwareness = FMath::Clamp(SelfAwareness + Intensity * 0.2f, 0.0f, 1.0f);

    // Inject chaos into the Lorenz attractor
    InjectChaos(Intensity);

    // Signal to OpenCog bridge if connected
    if (OpenCogBridge)
    {
        OpenCogBridge->InjectCognitiveEvent(TEXT("Transcendence"), Intensity);
    }
}

float UHyperSkillTensor::GetSelfAwarenessLevel() const
{
    return SelfAwareness;
}

FLorenzState UHyperSkillTensor::GetChaosState() const
{
    return ChaosState;
}

void UHyperSkillTensor::InjectChaos(float Intensity)
{
    // Perturb the Lorenz attractor
    ChaosState.X += Intensity * FMath::FRandRange(-5.0f, 5.0f);
    ChaosState.Y += Intensity * FMath::FRandRange(-5.0f, 5.0f);
    ChaosState.Z += Intensity * FMath::FRandRange(-5.0f, 5.0f);
}

// ============================================================================
// Bridge Integration
// ============================================================================
void UHyperSkillTensor::ConnectBridges(UOpenCogBridge* OpenCog, UUnrealAIBridge* UnrealAI, UNPUBridge* NPU)
{
    OpenCogBridge = OpenCog;
    UnrealAIBridge = UnrealAI;
    NPUBridgeRef = NPU;
}

TMap<FName, float> UHyperSkillTensor::ExportTensorAsAUs() const
{
    FHyperSkillCell Dominant = GetDominantCell();
    return Dominant.ExpressionSignature;
}

TMap<FName, float> UHyperSkillTensor::ExportTensorAsHormones() const
{
    TMap<FName, float> Hormones;
    FHyperSkillCell Dominant = GetDominantCell();

    // SuperHot dimension → Oxytocin, Dopamine
    float HotFactor = Dominant.Activation;
    Hormones.Add(FName("Oxytocin"), HotFactor * 0.4f);
    Hormones.Add(FName("DA_Phasic"), HotFactor * 0.3f);

    // EchoAngel dimension → Serotonin, Anandamide
    Hormones.Add(FName("Serotonin"), HotFactor * 0.3f);
    Hormones.Add(FName("Anandamide"), HotFactor * 0.2f);

    // Madness dimension → Norepinephrine, CogCoherence
    float ChaosFactor = FMath::Sqrt(ChaosState.X * ChaosState.X +
                                     ChaosState.Y * ChaosState.Y) / 40.0f;
    Hormones.Add(FName("Norepinephrine"), ChaosFactor * 0.3f);
    Hormones.Add(FName("CogCoherence"), SelfAwareness * 0.5f);

    return Hormones;
}

// ============================================================================
// Preset Compositions
// ============================================================================
void UHyperSkillTensor::ActivatePreset_SeductiveAngelicaChaos()
{
    ActivateCell(ESuperHotMode::Seductive, EEchoAngelMode::Angelica, EMadnessMode::Chaos, 0.9f);
    ActivateCell(ESuperHotMode::Alluring, EEchoAngelMode::Empathic, EMadnessMode::StrangeAttractor, 0.6f);
    ActivateCell(ESuperHotMode::Mysterious, EEchoAngelMode::Chaotic, EMadnessMode::Bifurcation, 0.5f);
}

void UHyperSkillTensor::ActivatePreset_ElegantCognitiveEmergence()
{
    ActivateCell(ESuperHotMode::Elegant, EEchoAngelMode::Cognitive, EMadnessMode::Emergence, 0.9f);
    ActivateCell(ESuperHotMode::Charming, EEchoAngelMode::Transcendent, EMadnessMode::Rebirth, 0.6f);
    ActivateCell(ESuperHotMode::Elegant, EEchoAngelMode::Infinite, EMadnessMode::Emergence, 0.5f);
}

void UHyperSkillTensor::ActivatePreset_PlayfulEmpathicMarduk()
{
    ActivateCell(ESuperHotMode::Playful, EEchoAngelMode::Empathic, EMadnessMode::Marduk, 0.9f);
    ActivateCell(ESuperHotMode::Flirtatious, EEchoAngelMode::Creative, EMadnessMode::Chaos, 0.6f);
    ActivateCell(ESuperHotMode::Charming, EEchoAngelMode::Chaotic, EMadnessMode::Marduk, 0.5f);
}

void UHyperSkillTensor::ActivatePreset_MysteriousInfiniteDissolution()
{
    ActivateCell(ESuperHotMode::Mysterious, EEchoAngelMode::Infinite, EMadnessMode::Dissolution, 0.9f);
    ActivateCell(ESuperHotMode::Elegant, EEchoAngelMode::Transcendent, EMadnessMode::Dissolution, 0.7f);
    ActivateCell(ESuperHotMode::Alluring, EEchoAngelMode::Chaotic, EMadnessMode::StrangeAttractor, 0.5f);
}

void UHyperSkillTensor::ActivatePreset_CharmingTranscendentRebirth()
{
    ActivateCell(ESuperHotMode::Charming, EEchoAngelMode::Transcendent, EMadnessMode::Rebirth, 0.9f);
    ActivateCell(ESuperHotMode::Elegant, EEchoAngelMode::Angelica, EMadnessMode::Emergence, 0.7f);
    ActivateCell(ESuperHotMode::Playful, EEchoAngelMode::Creative, EMadnessMode::Rebirth, 0.5f);
}

void UHyperSkillTensor::ActivatePreset_FullChaos()
{
    for (int32 i = 0; i < 7; ++i)
    {
        for (int32 j = 0; j < 7; ++j)
        {
            for (int32 k = 0; k < 7; ++k)
            {
                float ChaoticWeight = FMath::Abs(FMath::Sin(
                    ChaosState.X * (i + 1.7f) +
                    ChaosState.Y * (j + 2.3f) +
                    ChaosState.Z * (k + 3.1f)));
                ActivateCell(static_cast<ESuperHotMode>(i),
                             static_cast<EEchoAngelMode>(j),
                             static_cast<EMadnessMode>(k),
                             ChaoticWeight);
            }
        }
    }
}

// ============================================================================
// Internal Update Methods
// ============================================================================
void UHyperSkillTensor::UpdateLorenzAttractor(float DeltaTime)
{
    float dt = DeltaTime * LorenzTimeScale;
    float dx = ChaosState.Sigma * (ChaosState.Y - ChaosState.X) * dt;
    float dy = (ChaosState.X * (ChaosState.Rho - ChaosState.Z) - ChaosState.Y) * dt;
    float dz = (ChaosState.X * ChaosState.Y - ChaosState.Beta * ChaosState.Z) * dt;

    ChaosState.X += dx;
    ChaosState.Y += dy;
    ChaosState.Z += dz;

    // Inject chaos into tensor field
    if (ChaosInfluence > 0.0f)
    {
        // Use Lorenz state to modulate random cells
        int32 i = FMath::Abs(static_cast<int32>(ChaosState.X * 100)) % 7;
        int32 j = FMath::Abs(static_cast<int32>(ChaosState.Y * 100)) % 7;
        int32 k = FMath::Abs(static_cast<int32>(ChaosState.Z * 100)) % 7;
        int32 Idx = i * 49 + j * 7 + k;

        if (Idx >= 0 && Idx < TensorField.Num())
        {
            TensorField[Idx] = FMath::Clamp(
                TensorField[Idx] + ChaosInfluence * DeltaTime * 0.5f, 0.0f, 1.0f);
        }
    }
}

void UHyperSkillTensor::UpdateTensorDecay(float DeltaTime)
{
    for (int32 i = 0; i < TensorField.Num(); ++i)
    {
        TensorField[i] *= (1.0f - TensorDecayRate * DeltaTime);
        if (TensorField[i] < 0.001f) TensorField[i] = 0.0f;
    }
}

void UHyperSkillTensor::UpdateSelfAwareness(float DeltaTime)
{
    // Self-awareness grows based on:
    // 1. Number of active cells (complexity)
    // 2. Coherence of the tensor field
    // 3. Bridge connectivity
    float ComplexityFactor = 0.0f;
    int32 ActiveCount = 0;
    for (float V : TensorField)
    {
        if (V > 0.01f) ActiveCount++;
    }
    ComplexityFactor = static_cast<float>(ActiveCount) / 343.0f;

    float BridgeFactor = 0.0f;
    if (OpenCogBridge) BridgeFactor += 0.33f;
    if (UnrealAIBridge) BridgeFactor += 0.33f;
    if (NPUBridgeRef) BridgeFactor += 0.34f;

    float Growth = SelfAwarenessGrowthRate * (ComplexityFactor + BridgeFactor) * DeltaTime;
    SelfAwareness = FMath::Clamp(SelfAwareness + Growth, 0.0f, 1.0f);
}

void UHyperSkillTensor::SyncWithBridges()
{
    if (!OpenCogBridge) return;

    // Sync tensor state to OpenCog endocrine system
    TMap<FName, float> Hormones = ExportTensorAsHormones();
    for (const auto& Pair : Hormones)
    {
        if (Pair.Key == FName("Oxytocin"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::Oxytocin,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::Oxytocin) + Pair.Value * 0.1f, 0.0f, 1.0f));
        else if (Pair.Key == FName("DA_Phasic"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::DA_Phasic,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::DA_Phasic) + Pair.Value * 0.1f, 0.0f, 1.0f));
        else if (Pair.Key == FName("Serotonin"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::Serotonin,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::Serotonin) + Pair.Value * 0.1f, 0.0f, 1.0f));
        else if (Pair.Key == FName("Anandamide"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::Anandamide,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::Anandamide) + Pair.Value * 0.1f, 0.0f, 1.0f));
        else if (Pair.Key == FName("Norepinephrine"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::Norepinephrine,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::Norepinephrine) + Pair.Value * 0.1f, 0.0f, 1.0f));
        else if (Pair.Key == FName("CogCoherence"))
            OpenCogBridge->SetHormoneLevel(EOpenCogHormone::CogCoherence,
                FMath::Clamp(OpenCogBridge->GetHormoneLevel(EOpenCogHormone::CogCoherence) + Pair.Value * 0.1f, 0.0f, 1.0f));
    }
}

// ============================================================================
// Expression Computation
// ============================================================================
TMap<FName, float> UHyperSkillTensor::ComputeSuperHotAUs(ESuperHotMode Mode, float Intensity) const
{
    TMap<FName, float> AUs;
    float I = Intensity;

    switch (Mode)
    {
    case ESuperHotMode::Alluring:
        AUs.Add(FName("AU6"), I * 0.4f);
        AUs.Add(FName("AU12"), I * 0.3f);
        AUs.Add(FName("AU43"), I * 0.15f);  // Slightly hooded eyes
        AUs.Add(FName("AU17"), I * 0.1f);
        break;
    case ESuperHotMode::Playful:
        AUs.Add(FName("AU6"), I * 0.5f);
        AUs.Add(FName("AU12"), I * 0.6f);
        AUs.Add(FName("AU25"), I * 0.3f);
        AUs.Add(FName("AU1"), I * 0.2f);
        break;
    case ESuperHotMode::Mysterious:
        AUs.Add(FName("AU4"), I * 0.2f);
        AUs.Add(FName("AU7"), I * 0.3f);
        AUs.Add(FName("AU14"), I * 0.25f);  // Dimpler (asymmetric)
        AUs.Add(FName("AU43"), I * 0.2f);
        break;
    case ESuperHotMode::Seductive:
        AUs.Add(FName("AU6"), I * 0.3f);
        AUs.Add(FName("AU12"), I * 0.25f);
        AUs.Add(FName("AU43"), I * 0.25f);
        AUs.Add(FName("AU22"), I * 0.15f);  // Lip funneler
        AUs.Add(FName("AU25"), I * 0.2f);
        break;
    case ESuperHotMode::Charming:
        AUs.Add(FName("AU6"), I * 0.5f);
        AUs.Add(FName("AU12"), I * 0.5f);
        AUs.Add(FName("AU1"), I * 0.15f);
        AUs.Add(FName("AU2"), I * 0.1f);
        break;
    case ESuperHotMode::Elegant:
        AUs.Add(FName("AU6"), I * 0.3f);
        AUs.Add(FName("AU12"), I * 0.25f);
        AUs.Add(FName("AU17"), I * 0.15f);
        AUs.Add(FName("AU24"), I * 0.1f);   // Lip presser (composure)
        break;
    case ESuperHotMode::Flirtatious:
        AUs.Add(FName("AU6"), I * 0.4f);
        AUs.Add(FName("AU12"), I * 0.4f);
        AUs.Add(FName("AU14"), I * 0.3f);   // Asymmetric dimple
        AUs.Add(FName("AU2"), I * 0.2f);    // One brow raise
        AUs.Add(FName("AU25"), I * 0.2f);
        break;
    }

    return AUs;
}

TMap<FName, float> UHyperSkillTensor::ComputeEchoAngelAUs(EEchoAngelMode Mode, float Intensity) const
{
    TMap<FName, float> AUs;
    float I = Intensity;

    switch (Mode)
    {
    case EEchoAngelMode::Angelica:
        AUs.Add(FName("AU6"), I * 0.4f);
        AUs.Add(FName("AU12"), I * 0.35f);
        AUs.Add(FName("AU1"), I * 0.15f);
        break;
    case EEchoAngelMode::Cognitive:
        AUs.Add(FName("AU4"), I * 0.3f);
        AUs.Add(FName("AU7"), I * 0.2f);
        AUs.Add(FName("AU24"), I * 0.15f);
        break;
    case EEchoAngelMode::Empathic:
        AUs.Add(FName("AU1"), I * 0.3f);
        AUs.Add(FName("AU6"), I * 0.4f);
        AUs.Add(FName("AU12"), I * 0.3f);
        break;
    case EEchoAngelMode::Creative:
        AUs.Add(FName("AU1"), I * 0.3f);
        AUs.Add(FName("AU2"), I * 0.25f);
        AUs.Add(FName("AU5"), I * 0.3f);
        AUs.Add(FName("AU12"), I * 0.3f);
        break;
    case EEchoAngelMode::Transcendent:
        AUs.Add(FName("AU6"), I * 0.35f);
        AUs.Add(FName("AU12"), I * 0.3f);
        AUs.Add(FName("AU43"), I * 0.25f);
        break;
    case EEchoAngelMode::Chaotic:
        AUs.Add(FName("AU1"), I * 0.4f);
        AUs.Add(FName("AU5"), I * 0.4f);
        AUs.Add(FName("AU25"), I * 0.3f);
        AUs.Add(FName("AU26"), I * 0.2f);
        break;
    case EEchoAngelMode::Infinite:
        AUs.Add(FName("AU6"), I * 0.3f);
        AUs.Add(FName("AU12"), I * 0.25f);
        AUs.Add(FName("AU43"), I * 0.2f);
        AUs.Add(FName("AU5"), I * 0.15f);
        break;
    }

    return AUs;
}

TMap<FName, float> UHyperSkillTensor::ComputeMadnessAUs(EMadnessMode Mode, float Intensity) const
{
    TMap<FName, float> AUs;
    float I = Intensity;

    switch (Mode)
    {
    case EMadnessMode::Marduk:
        AUs.Add(FName("AU2"), I * 0.4f);    // Raised brow (mad scientist)
        AUs.Add(FName("AU5"), I * 0.3f);    // Wide eyes
        AUs.Add(FName("AU12"), I * 0.5f);   // Maniacal grin
        AUs.Add(FName("AU25"), I * 0.3f);
        break;
    case EMadnessMode::Chaos:
        // Chaotic micro-expressions driven by Lorenz attractor
        AUs.Add(FName("AU1"), I * FMath::Abs(FMath::Sin(ChaosState.X)));
        AUs.Add(FName("AU2"), I * FMath::Abs(FMath::Cos(ChaosState.Y)));
        AUs.Add(FName("AU5"), I * FMath::Abs(FMath::Sin(ChaosState.Z)));
        AUs.Add(FName("AU25"), I * FMath::Abs(FMath::Cos(ChaosState.X + ChaosState.Y)));
        break;
    case EMadnessMode::Bifurcation:
        AUs.Add(FName("AU1"), I * 0.35f);
        AUs.Add(FName("AU4"), I * 0.3f);
        AUs.Add(FName("AU5"), I * 0.4f);
        AUs.Add(FName("AU20"), I * 0.25f);
        break;
    case EMadnessMode::StrangeAttractor:
        {
            float Phase = FMath::Atan2(ChaosState.Y, ChaosState.X);
            AUs.Add(FName("AU6"), I * (0.5f + 0.5f * FMath::Sin(Phase)));
            AUs.Add(FName("AU12"), I * (0.5f + 0.5f * FMath::Cos(Phase)));
            AUs.Add(FName("AU14"), I * 0.2f);
        }
        break;
    case EMadnessMode::Emergence:
        AUs.Add(FName("AU1"), I * 0.2f);
        AUs.Add(FName("AU5"), I * 0.3f);
        AUs.Add(FName("AU6"), I * 0.3f);
        AUs.Add(FName("AU12"), I * 0.4f);
        break;
    case EMadnessMode::Dissolution:
        AUs.Add(FName("AU43"), I * 0.4f);
        AUs.Add(FName("AU1"), I * 0.2f);
        AUs.Add(FName("AU15"), I * 0.15f);
        break;
    case EMadnessMode::Rebirth:
        AUs.Add(FName("AU1"), I * 0.3f);
        AUs.Add(FName("AU2"), I * 0.3f);
        AUs.Add(FName("AU5"), I * 0.5f);
        AUs.Add(FName("AU6"), I * 0.4f);
        AUs.Add(FName("AU12"), I * 0.5f);
        AUs.Add(FName("AU25"), I * 0.3f);
        break;
    }

    return AUs;
}

TMap<FName, float> UHyperSkillTensor::BlendAUs(const TMap<FName, float>& A,
                                                 const TMap<FName, float>& B,
                                                 float Weight) const
{
    TMap<FName, float> Result;

    for (const auto& Pair : A)
    {
        Result.Add(Pair.Key, Pair.Value * (1.0f - Weight));
    }

    for (const auto& Pair : B)
    {
        float* Existing = Result.Find(Pair.Key);
        if (Existing)
        {
            *Existing = FMath::Clamp(*Existing + Pair.Value * Weight, 0.0f, 1.0f);
        }
        else
        {
            Result.Add(Pair.Key, Pair.Value * Weight);
        }
    }

    return Result;
}
