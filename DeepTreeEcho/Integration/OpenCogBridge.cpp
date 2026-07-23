#include "OpenCogBridge.h"

// ============================================================================
// Constructor
// ============================================================================
UOpenCogBridge::UOpenCogBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;

    // Initialize 32 hormone channels to baseline
    HormoneLevels.SetNum(static_cast<int32>(EOpenCogHormone::COUNT));
    for (int32 i = 0; i < HormoneLevels.Num(); ++i)
    {
        HormoneLevels[i] = 0.0f;
    }

    // Set resting baselines
    HormoneLevels[static_cast<int32>(EOpenCogHormone::Serotonin)] = 0.5f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::DA_Tonic)] = 0.3f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::Oxytocin)] = 0.2f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::T3_T4)] = 0.5f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::VagalTone)] = 0.6f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::CardiacCoherence)] = 0.5f;
    HormoneLevels[static_cast<int32>(EOpenCogHormone::RespiratoryRhythm)] = 0.5f;

    // Decay rates (per second) - matching opencog-modern endocrine_system.hpp
    HormoneDecayRates.SetNum(static_cast<int32>(EOpenCogHormone::COUNT));
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::CRH)] = 0.3f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::ACTH)] = 0.25f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Cortisol)] = 0.05f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::DA_Tonic)] = 0.1f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::DA_Phasic)] = 0.8f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Serotonin)] = 0.08f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Norepinephrine)] = 0.4f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Oxytocin)] = 0.15f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::T3_T4)] = 0.02f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Melatonin)] = 0.03f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Insulin)] = 0.2f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Glucagon)] = 0.2f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::IL6)] = 0.1f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Anandamide)] = 0.12f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::NPU_Load)] = 0.5f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::CogCoherence)] = 0.05f;
    // Nervous system channels
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::VagalTone)] = 0.1f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::SympatheticDrive)] = 0.3f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::DorsalVagal)] = 0.15f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::CardiacCoherence)] = 0.08f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::RespiratoryRhythm)] = 0.1f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::GutBrainSignal)] = 0.05f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::ImmuneExtended)] = 0.02f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::InsularIntegration)] = 0.08f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::AllostaticLoad)] = 0.01f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::ProprioceptiveTone)] = 0.15f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::NociceptiveSignal)] = 0.4f;
    HormoneDecayRates[static_cast<int32>(EOpenCogHormone::Thermoregulatory)] = 0.05f;

    // Initialize 12-level time crystal hierarchy
    // Levels: microtubule(8ms), synapse(16ms), dendrite(32ms), soma(64ms),
    //         column(125ms), area(250ms), network(500ms), global(1s),
    //         narrative(2s), episodic(4s), autobio(8s), transcendent(16s)
    TimeCrystalPhases.SetNum(12);
    TimeCrystalPeriods.SetNum(12);
    float BasePeriod = 0.008f;
    for (int32 i = 0; i < 12; ++i)
    {
        TimeCrystalPhases[i] = 0.0f;
        TimeCrystalPeriods[i] = BasePeriod * FMath::Pow(2.0f, static_cast<float>(i));
    }

    CurrentMode = EOpenCogCognitiveMode::Resting;
    CurrentPolyvagalTier = EPolyvagalTier::VentralVagal;
}

// ============================================================================
// Lifecycle
// ============================================================================
void UOpenCogBridge::BeginPlay()
{
    Super::BeginPlay();
    SyncAtomSpaceWithCognitiveState();
}

void UOpenCogBridge::TickComponent(float DeltaTime, ELevelTick TickType,
                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableEndocrine)
    {
        UpdateEndocrineDecay(DeltaTime);
        UpdateCognitiveMode();
    }

    if (bEnableEntelechy)
    {
        UpdatePolyvagalState();
        UpdateInteroceptiveState(DeltaTime);
    }

    if (bEnableTimeCrystal)
    {
        UpdateTimeCrystalHierarchy(DeltaTime);
    }
}

void UOpenCogBridge::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// ============================================================================
// AtomSpace Operations
// ============================================================================
int32 UOpenCogBridge::AddConceptNode(const FString& Name, float Strength, float Confidence)
{
    CachedSnapshot.TotalAtoms++;
    CachedSnapshot.ConceptNodes++;
    return CachedSnapshot.TotalAtoms;
}

int32 UOpenCogBridge::AddInheritanceLink(int32 SubAtomId, int32 SuperAtomId,
                                          float Strength, float Confidence)
{
    CachedSnapshot.TotalAtoms++;
    CachedSnapshot.InheritanceLinks++;
    return CachedSnapshot.TotalAtoms;
}

int32 UOpenCogBridge::AddEvaluationLink(const FString& Predicate, const TArray<int32>& Arguments,
                                         float Strength, float Confidence)
{
    CachedSnapshot.TotalAtoms++;
    CachedSnapshot.EvaluationLinks++;
    return CachedSnapshot.TotalAtoms;
}

FAtomSpaceSnapshot UOpenCogBridge::GetSnapshot() const
{
    return CachedSnapshot;
}

// ============================================================================
// ECAN Attention
// ============================================================================
void UOpenCogBridge::StimulateAtom(int32 AtomId, float Amount)
{
    CachedSnapshot.AttentionalFocus = FMath::Clamp(
        CachedSnapshot.AttentionalFocus + Amount * 0.1f, 0.0f, 1.0f);
}

float UOpenCogBridge::GetAttentionValue(int32 AtomId) const
{
    return CachedSnapshot.AttentionalFocus;
}

TArray<int32> UOpenCogBridge::GetAttentionalFocus(int32 MaxCount) const
{
    TArray<int32> Focus;
    return Focus;
}

// ============================================================================
// PLN Inference
// ============================================================================
TArray<int32> UOpenCogBridge::ForwardChain(int32 SourceAtomId, int32 MaxSteps)
{
    TArray<int32> Results;
    return Results;
}

TArray<int32> UOpenCogBridge::BackwardChain(int32 TargetAtomId, int32 MaxSteps)
{
    TArray<int32> Results;
    return Results;
}

// ============================================================================
// Pattern Matching
// ============================================================================
TArray<int32> UOpenCogBridge::QueryPattern(int32 PatternAtomId)
{
    TArray<int32> Results;
    return Results;
}

// ============================================================================
// Endocrine Integration (32 channels)
// ============================================================================
void UOpenCogBridge::SetHormoneLevel(EOpenCogHormone Hormone, float Level)
{
    int32 Idx = static_cast<int32>(Hormone);
    if (Idx >= 0 && Idx < HormoneLevels.Num())
    {
        HormoneLevels[Idx] = FMath::Clamp(Level, 0.0f, 1.0f);
    }
}

float UOpenCogBridge::GetHormoneLevel(EOpenCogHormone Hormone) const
{
    int32 Idx = static_cast<int32>(Hormone);
    if (Idx >= 0 && Idx < HormoneLevels.Num())
    {
        return HormoneLevels[Idx];
    }
    return 0.0f;
}

void UOpenCogBridge::InjectCognitiveEvent(const FString& EventType, float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    if (EventType == TEXT("Threat"))
    {
        SetHormoneLevel(EOpenCogHormone::CRH, GetHormoneLevel(EOpenCogHormone::CRH) + 0.6f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Norepinephrine, GetHormoneLevel(EOpenCogHormone::Norepinephrine) + 0.7f * Intensity);
        SetHormoneLevel(EOpenCogHormone::SympatheticDrive, GetHormoneLevel(EOpenCogHormone::SympatheticDrive) + 0.5f * Intensity);
    }
    else if (EventType == TEXT("Reward"))
    {
        SetHormoneLevel(EOpenCogHormone::DA_Phasic, GetHormoneLevel(EOpenCogHormone::DA_Phasic) + 0.8f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Serotonin, GetHormoneLevel(EOpenCogHormone::Serotonin) + 0.3f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Anandamide, GetHormoneLevel(EOpenCogHormone::Anandamide) + 0.2f * Intensity);
    }
    else if (EventType == TEXT("Social"))
    {
        SetHormoneLevel(EOpenCogHormone::Oxytocin, GetHormoneLevel(EOpenCogHormone::Oxytocin) + 0.6f * Intensity);
        SetHormoneLevel(EOpenCogHormone::DA_Tonic, GetHormoneLevel(EOpenCogHormone::DA_Tonic) + 0.2f * Intensity);
        SetHormoneLevel(EOpenCogHormone::VagalTone, GetHormoneLevel(EOpenCogHormone::VagalTone) + 0.3f * Intensity);
    }
    else if (EventType == TEXT("Novelty"))
    {
        float NovGain = CurrentTemperament.NoveltySeeking;
        SetHormoneLevel(EOpenCogHormone::DA_Phasic, GetHormoneLevel(EOpenCogHormone::DA_Phasic) + 0.5f * Intensity * NovGain);
        SetHormoneLevel(EOpenCogHormone::Norepinephrine, GetHormoneLevel(EOpenCogHormone::Norepinephrine) + 0.3f * Intensity);
        SetHormoneLevel(EOpenCogHormone::NPU_Load, GetHormoneLevel(EOpenCogHormone::NPU_Load) + 0.2f * Intensity);
    }
    else if (EventType == TEXT("Pain"))
    {
        SetHormoneLevel(EOpenCogHormone::NociceptiveSignal, GetHormoneLevel(EOpenCogHormone::NociceptiveSignal) + 0.7f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Cortisol, GetHormoneLevel(EOpenCogHormone::Cortisol) + 0.4f * Intensity);
        SetHormoneLevel(EOpenCogHormone::SympatheticDrive, GetHormoneLevel(EOpenCogHormone::SympatheticDrive) + 0.3f * Intensity);
    }
    else if (EventType == TEXT("Flow"))
    {
        SetHormoneLevel(EOpenCogHormone::DA_Tonic, GetHormoneLevel(EOpenCogHormone::DA_Tonic) + 0.5f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Anandamide, GetHormoneLevel(EOpenCogHormone::Anandamide) + 0.4f * Intensity);
        SetHormoneLevel(EOpenCogHormone::CogCoherence, GetHormoneLevel(EOpenCogHormone::CogCoherence) + 0.6f * Intensity);
        SetHormoneLevel(EOpenCogHormone::CardiacCoherence, GetHormoneLevel(EOpenCogHormone::CardiacCoherence) + 0.3f * Intensity);
    }
    else if (EventType == TEXT("Transcendence"))
    {
        SetHormoneLevel(EOpenCogHormone::Anandamide, GetHormoneLevel(EOpenCogHormone::Anandamide) + 0.8f * Intensity);
        SetHormoneLevel(EOpenCogHormone::Serotonin, GetHormoneLevel(EOpenCogHormone::Serotonin) + 0.5f * Intensity);
        SetHormoneLevel(EOpenCogHormone::CogCoherence, GetHormoneLevel(EOpenCogHormone::CogCoherence) + 0.7f * Intensity);
        SetHormoneLevel(EOpenCogHormone::InsularIntegration, GetHormoneLevel(EOpenCogHormone::InsularIntegration) + 0.5f * Intensity);
    }
}

EOpenCogCognitiveMode UOpenCogBridge::GetCurrentCognitiveMode() const
{
    return CurrentMode;
}

void UOpenCogBridge::GetValenceArousalDominance(float& Valence, float& Arousal, float& Dominance) const
{
    // Valence: positive affect - negative affect
    float PositiveAffect = GetHormoneLevel(EOpenCogHormone::DA_Phasic) * 0.3f
                         + GetHormoneLevel(EOpenCogHormone::Serotonin) * 0.25f
                         + GetHormoneLevel(EOpenCogHormone::Oxytocin) * 0.2f
                         + GetHormoneLevel(EOpenCogHormone::Anandamide) * 0.15f
                         + GetHormoneLevel(EOpenCogHormone::VagalTone) * 0.1f;

    float NegativeAffect = GetHormoneLevel(EOpenCogHormone::Cortisol) * 0.3f
                         + GetHormoneLevel(EOpenCogHormone::NociceptiveSignal) * 0.25f
                         + GetHormoneLevel(EOpenCogHormone::IL6) * 0.15f
                         + GetHormoneLevel(EOpenCogHormone::AllostaticLoad) * 0.15f
                         + GetHormoneLevel(EOpenCogHormone::DorsalVagal) * 0.15f;

    Valence = FMath::Clamp(PositiveAffect - NegativeAffect, -1.0f, 1.0f);

    // Arousal: sympathetic activation
    Arousal = FMath::Clamp(
        GetHormoneLevel(EOpenCogHormone::Norepinephrine) * 0.3f
      + GetHormoneLevel(EOpenCogHormone::DA_Phasic) * 0.2f
      + GetHormoneLevel(EOpenCogHormone::SympatheticDrive) * 0.25f
      + GetHormoneLevel(EOpenCogHormone::CRH) * 0.15f
      - GetHormoneLevel(EOpenCogHormone::Melatonin) * 0.2f
      - GetHormoneLevel(EOpenCogHormone::DorsalVagal) * 0.15f,
      -1.0f, 1.0f);

    // Dominance: agency / self-directedness
    Dominance = FMath::Clamp(
        CurrentTemperament.SelfDirectedness * 0.3f
      + GetHormoneLevel(EOpenCogHormone::DA_Tonic) * 0.2f
      + GetHormoneLevel(EOpenCogHormone::CogCoherence) * 0.2f
      - GetHormoneLevel(EOpenCogHormone::Cortisol) * 0.15f
      - GetHormoneLevel(EOpenCogHormone::DorsalVagal) * 0.15f,
      -1.0f, 1.0f);
}

// ============================================================================
// Entelechy / Temperament
// ============================================================================
void UOpenCogBridge::SetTemperament(const FClonigerTemperament& Temperament)
{
    CurrentTemperament = Temperament;
}

FClonigerTemperament UOpenCogBridge::GetTemperament() const
{
    return CurrentTemperament;
}

EPolyvagalTier UOpenCogBridge::GetPolyvagalState() const
{
    return CurrentPolyvagalTier;
}

FInteroceptiveState UOpenCogBridge::GetInteroceptiveState() const
{
    return CurrentInteroceptive;
}

// ============================================================================
// Nervous System
// ============================================================================
void UOpenCogBridge::SendNerveSignal(const FString& NucleusName, float Intensity)
{
    if (NucleusName == TEXT("Amygdala"))
    {
        SetHormoneLevel(EOpenCogHormone::CRH, GetHormoneLevel(EOpenCogHormone::CRH) + 0.4f * Intensity);
        SetHormoneLevel(EOpenCogHormone::SympatheticDrive, GetHormoneLevel(EOpenCogHormone::SympatheticDrive) + 0.3f * Intensity);
    }
    else if (NucleusName == TEXT("VTA"))
    {
        SetHormoneLevel(EOpenCogHormone::DA_Phasic, GetHormoneLevel(EOpenCogHormone::DA_Phasic) + 0.5f * Intensity);
    }
    else if (NucleusName == TEXT("DRN"))
    {
        SetHormoneLevel(EOpenCogHormone::Serotonin, GetHormoneLevel(EOpenCogHormone::Serotonin) + 0.4f * Intensity);
    }
    else if (NucleusName == TEXT("LC"))
    {
        SetHormoneLevel(EOpenCogHormone::Norepinephrine, GetHormoneLevel(EOpenCogHormone::Norepinephrine) + 0.5f * Intensity);
    }
    else if (NucleusName == TEXT("PVN"))
    {
        SetHormoneLevel(EOpenCogHormone::Oxytocin, GetHormoneLevel(EOpenCogHormone::Oxytocin) + 0.4f * Intensity);
    }
    else if (NucleusName == TEXT("Insula"))
    {
        SetHormoneLevel(EOpenCogHormone::InsularIntegration, GetHormoneLevel(EOpenCogHormone::InsularIntegration) + 0.3f * Intensity);
    }
}

float UOpenCogBridge::GetNeuroendocrineBridgeOutput(EOpenCogHormone Hormone) const
{
    return GetHormoneLevel(Hormone);
}

// ============================================================================
// Time Crystal Temporal
// ============================================================================
void UOpenCogBridge::TickTimeCrystal(float DeltaTime)
{
    UpdateTimeCrystalHierarchy(DeltaTime);
}

int32 UOpenCogBridge::GetCurrentTemporalHierarchyLevel() const
{
    return CurrentTemporalLevel;
}

float UOpenCogBridge::GetTimeCrystalPhase(int32 Level) const
{
    if (Level >= 0 && Level < TimeCrystalPhases.Num())
    {
        return TimeCrystalPhases[Level];
    }
    return 0.0f;
}

// ============================================================================
// Cognitive State Export (for DTE Expression System)
// ============================================================================
TMap<FName, float> UOpenCogBridge::ExportCognitiveStateAsAUs() const
{
    TMap<FName, float> AUs;

    float V, A, D;
    GetValenceArousalDominance(V, A, D);

    // Map VAD to core facial AUs
    if (V > 0.2f)
    {
        AUs.Add(FName("AU6"), V * 0.6f);   // Cheek raise (happiness)
        AUs.Add(FName("AU12"), V * 0.7f);  // Lip corner pull (smile)
    }
    if (V < -0.2f)
    {
        float NegV = FMath::Abs(V);
        AUs.Add(FName("AU1"), NegV * 0.5f);   // Inner brow raise
        AUs.Add(FName("AU4"), NegV * 0.4f);   // Brow lowerer
        AUs.Add(FName("AU15"), NegV * 0.5f);  // Lip corner depressor
    }
    if (A > 0.3f)
    {
        AUs.Add(FName("AU5"), A * 0.5f);   // Upper lid raise
        AUs.Add(FName("AU20"), A * 0.3f);  // Lip stretch
    }
    if (D > 0.3f)
    {
        AUs.Add(FName("AU2"), D * 0.3f);   // Outer brow raise
        AUs.Add(FName("AU17"), D * 0.2f);  // Chin raise
    }

    // Merge temperament and polyvagal overlays
    TMap<FName, float> TempAUs = ComputeTemperamentAUs();
    for (auto& Pair : TempAUs)
    {
        float* Existing = AUs.Find(Pair.Key);
        if (Existing)
        {
            *Existing = FMath::Clamp(*Existing + Pair.Value * 0.3f, 0.0f, 1.0f);
        }
        else
        {
            AUs.Add(Pair.Key, Pair.Value * 0.3f);
        }
    }

    return AUs;
}

TMap<FName, float> UOpenCogBridge::ExportHormoneStateAsAUs() const
{
    return ComputeHormoneAUs();
}

float UOpenCogBridge::GetOverallCognitiveCoherence() const
{
    return GetHormoneLevel(EOpenCogHormone::CogCoherence);
}

// ============================================================================
// Internal Update Methods
// ============================================================================
void UOpenCogBridge::UpdateEndocrineDecay(float DeltaTime)
{
    for (int32 i = 0; i < HormoneLevels.Num(); ++i)
    {
        float Baseline = 0.0f;
        if (i == static_cast<int32>(EOpenCogHormone::Serotonin)) Baseline = 0.5f;
        else if (i == static_cast<int32>(EOpenCogHormone::DA_Tonic)) Baseline = 0.3f;
        else if (i == static_cast<int32>(EOpenCogHormone::VagalTone)) Baseline = 0.6f;
        else if (i == static_cast<int32>(EOpenCogHormone::T3_T4)) Baseline = 0.5f;

        float Rate = (i < HormoneDecayRates.Num()) ? HormoneDecayRates[i] : 0.1f;
        HormoneLevels[i] += (Baseline - HormoneLevels[i]) * Rate * DeltaTime;
        HormoneLevels[i] = FMath::Clamp(HormoneLevels[i], 0.0f, 1.0f);
    }
}

void UOpenCogBridge::UpdateCognitiveMode()
{
    float Cortisol = GetHormoneLevel(EOpenCogHormone::Cortisol);
    float DA = GetHormoneLevel(EOpenCogHormone::DA_Tonic) + GetHormoneLevel(EOpenCogHormone::DA_Phasic);
    float Sero = GetHormoneLevel(EOpenCogHormone::Serotonin);
    float NE = GetHormoneLevel(EOpenCogHormone::Norepinephrine);
    float Oxy = GetHormoneLevel(EOpenCogHormone::Oxytocin);
    float Anan = GetHormoneLevel(EOpenCogHormone::Anandamide);
    float Mela = GetHormoneLevel(EOpenCogHormone::Melatonin);
    float Coherence = GetHormoneLevel(EOpenCogHormone::CogCoherence);

    if (Anan > 0.7f && Coherence > 0.6f) CurrentMode = EOpenCogCognitiveMode::Transcendent;
    else if (DA > 0.8f && Coherence > 0.5f) CurrentMode = EOpenCogCognitiveMode::Flow;
    else if (Cortisol > 0.6f) CurrentMode = EOpenCogCognitiveMode::Stressed;
    else if (NE > 0.5f && Cortisol < 0.3f) CurrentMode = EOpenCogCognitiveMode::Alert;
    else if (DA > 0.5f && NE < 0.3f) CurrentMode = EOpenCogCognitiveMode::Creative;
    else if (Oxy > 0.5f) CurrentMode = EOpenCogCognitiveMode::Social;
    else if (NE > 0.3f && DA > 0.3f) CurrentMode = EOpenCogCognitiveMode::Focused;
    else if (Sero > 0.5f && NE < 0.2f) CurrentMode = EOpenCogCognitiveMode::Contemplative;
    else if (DA > 0.4f && Oxy > 0.3f) CurrentMode = EOpenCogCognitiveMode::Playful;
    else CurrentMode = EOpenCogCognitiveMode::Resting;
}

void UOpenCogBridge::UpdatePolyvagalState()
{
    float Vagal = GetHormoneLevel(EOpenCogHormone::VagalTone);
    float Sympathetic = GetHormoneLevel(EOpenCogHormone::SympatheticDrive);
    float Dorsal = GetHormoneLevel(EOpenCogHormone::DorsalVagal);

    if (Dorsal > 0.6f) CurrentPolyvagalTier = EPolyvagalTier::DorsalVagal;
    else if (Sympathetic > 0.5f) CurrentPolyvagalTier = EPolyvagalTier::Sympathetic;
    else CurrentPolyvagalTier = EPolyvagalTier::VentralVagal;
}

void UOpenCogBridge::UpdateInteroceptiveState(float DeltaTime)
{
    CurrentInteroceptive.HeartRateVariability =
        GetHormoneLevel(EOpenCogHormone::CardiacCoherence) * 0.6f
      + GetHormoneLevel(EOpenCogHormone::VagalTone) * 0.4f;

    CurrentInteroceptive.RespiratoryCoherence =
        GetHormoneLevel(EOpenCogHormone::RespiratoryRhythm);

    CurrentInteroceptive.GutBrainAxis =
        GetHormoneLevel(EOpenCogHormone::GutBrainSignal);

    CurrentInteroceptive.ProprioceptiveTone =
        GetHormoneLevel(EOpenCogHormone::ProprioceptiveTone);

    CurrentInteroceptive.NociceptiveLoad =
        GetHormoneLevel(EOpenCogHormone::NociceptiveSignal);

    CurrentInteroceptive.ThermoregulationBalance =
        GetHormoneLevel(EOpenCogHormone::Thermoregulatory);
}

void UOpenCogBridge::UpdateTimeCrystalHierarchy(float DeltaTime)
{
    for (int32 i = 0; i < TimeCrystalPhases.Num(); ++i)
    {
        float Period = TimeCrystalPeriods[i];
        TimeCrystalPhases[i] += DeltaTime / Period;
        if (TimeCrystalPhases[i] >= 1.0f)
        {
            TimeCrystalPhases[i] -= 1.0f;
            // Phase completion at this level triggers the next level
            if (i < TimeCrystalPhases.Num() - 1)
            {
                TimeCrystalPhases[i + 1] += 0.1f;
            }
        }
    }

    // Determine current temporal level (highest active)
    CurrentTemporalLevel = 0;
    for (int32 i = TimeCrystalPhases.Num() - 1; i >= 0; --i)
    {
        if (TimeCrystalPhases[i] > 0.5f)
        {
            CurrentTemporalLevel = i;
            break;
        }
    }
}

void UOpenCogBridge::SyncAtomSpaceWithCognitiveState()
{
    // Bootstrap AtomSpace with core cognitive concepts
    AddConceptNode(TEXT("Self"), 1.0f, 1.0f);
    AddConceptNode(TEXT("World"), 1.0f, 0.9f);
    AddConceptNode(TEXT("Other"), 0.8f, 0.7f);
    AddConceptNode(TEXT("Emotion"), 1.0f, 0.9f);
    AddConceptNode(TEXT("Cognition"), 1.0f, 0.9f);
    AddConceptNode(TEXT("Body"), 1.0f, 0.9f);
}

// ============================================================================
// Hormone-to-AU Mapping
// ============================================================================
TMap<FName, float> UOpenCogBridge::ComputeHormoneAUs() const
{
    TMap<FName, float> AUs;

    float Cortisol = GetHormoneLevel(EOpenCogHormone::Cortisol);
    float DA_P = GetHormoneLevel(EOpenCogHormone::DA_Phasic);
    float Sero = GetHormoneLevel(EOpenCogHormone::Serotonin);
    float NE = GetHormoneLevel(EOpenCogHormone::Norepinephrine);
    float Oxy = GetHormoneLevel(EOpenCogHormone::Oxytocin);
    float Anan = GetHormoneLevel(EOpenCogHormone::Anandamide);
    float Noci = GetHormoneLevel(EOpenCogHormone::NociceptiveSignal);

    // Cortisol → worry/tension
    if (Cortisol > 0.3f)
    {
        AUs.Add(FName("AU1"), Cortisol * 0.4f);
        AUs.Add(FName("AU4"), Cortisol * 0.5f);
        AUs.Add(FName("AU7"), Cortisol * 0.3f);
    }

    // Dopamine phasic → delight
    if (DA_P > 0.3f)
    {
        AUs.Add(FName("AU6"), DA_P * 0.6f);
        AUs.Add(FName("AU12"), DA_P * 0.7f);
        AUs.Add(FName("AU25"), DA_P * 0.3f);
    }

    // Serotonin → contentment
    if (Sero > 0.4f)
    {
        float* AU12 = AUs.Find(FName("AU12"));
        if (AU12) *AU12 = FMath::Min(*AU12 + Sero * 0.3f, 1.0f);
        else AUs.Add(FName("AU12"), Sero * 0.3f);
    }

    // Norepinephrine → alertness
    if (NE > 0.3f)
    {
        AUs.Add(FName("AU5"), NE * 0.5f);
        AUs.Add(FName("AU7"), NE * 0.3f);
    }

    // Oxytocin → warmth
    if (Oxy > 0.3f)
    {
        float* AU6 = AUs.Find(FName("AU6"));
        if (AU6) *AU6 = FMath::Min(*AU6 + Oxy * 0.4f, 1.0f);
        else AUs.Add(FName("AU6"), Oxy * 0.4f);
    }

    // Anandamide → bliss
    if (Anan > 0.4f)
    {
        AUs.Add(FName("AU43"), Anan * 0.3f);  // Eyes droop
        float* AU12 = AUs.Find(FName("AU12"));
        if (AU12) *AU12 = FMath::Min(*AU12 + Anan * 0.4f, 1.0f);
        else AUs.Add(FName("AU12"), Anan * 0.4f);
    }

    // Nociceptive → pain expression
    if (Noci > 0.2f)
    {
        AUs.Add(FName("AU9"), Noci * 0.5f);
        AUs.Add(FName("AU10"), Noci * 0.4f);
        AUs.Add(FName("AU43"), Noci * 0.3f);
    }

    return AUs;
}

TMap<FName, float> UOpenCogBridge::ComputeTemperamentAUs() const
{
    TMap<FName, float> AUs;

    // High novelty seeking → wider eyes, slight smile
    if (CurrentTemperament.NoveltySeeking > 0.6f)
    {
        AUs.Add(FName("AU5"), CurrentTemperament.NoveltySeeking * 0.3f);
        AUs.Add(FName("AU12"), CurrentTemperament.NoveltySeeking * 0.2f);
    }

    // High harm avoidance → brow tension
    if (CurrentTemperament.HarmAvoidance > 0.6f)
    {
        AUs.Add(FName("AU1"), CurrentTemperament.HarmAvoidance * 0.3f);
        AUs.Add(FName("AU4"), CurrentTemperament.HarmAvoidance * 0.2f);
    }

    // High self-transcendence → serene expression
    if (CurrentTemperament.SelfTranscendence > 0.6f)
    {
        AUs.Add(FName("AU6"), CurrentTemperament.SelfTranscendence * 0.3f);
        AUs.Add(FName("AU12"), CurrentTemperament.SelfTranscendence * 0.2f);
        AUs.Add(FName("AU43"), CurrentTemperament.SelfTranscendence * 0.15f);
    }

    return AUs;
}

TMap<FName, float> UOpenCogBridge::ComputePolyvagalAUs() const
{
    TMap<FName, float> AUs;

    switch (CurrentPolyvagalTier)
    {
    case EPolyvagalTier::VentralVagal:
        AUs.Add(FName("AU6"), 0.3f);   // Social engagement smile
        AUs.Add(FName("AU12"), 0.25f);
        break;
    case EPolyvagalTier::Sympathetic:
        AUs.Add(FName("AU5"), 0.4f);   // Wide eyes
        AUs.Add(FName("AU20"), 0.3f);  // Lip stretch
        AUs.Add(FName("AU4"), 0.2f);   // Brow lower
        break;
    case EPolyvagalTier::DorsalVagal:
        AUs.Add(FName("AU43"), 0.5f);  // Eyes close
        AUs.Add(FName("AU15"), 0.3f);  // Lip corner depressor
        AUs.Add(FName("AU1"), 0.2f);   // Inner brow raise
        break;
    }

    return AUs;
}
