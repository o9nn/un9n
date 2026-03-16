// EndocrineExpressionPipeline.cpp
// Deep Tree Echo Virtual Endocrine System → Avatar Expression Pipeline
// Full implementation of 10-gland, 16-channel hormone system with
// cognitive mode detection and FACS expression driving.

#include "EndocrineExpressionPipeline.h"

UEndocrineExpressionPipeline::UEndocrineExpressionPipeline()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Before cognitive cycle
}

void UEndocrineExpressionPipeline::BeginPlay()
{
    Super::BeginPlay();
    InitializeHormoneChannels();
    InitializeHormoneAUMappings();

    // Auto-find DNA bridge on owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        ConnectedBridge = Owner->FindComponentByClass<UMetaHumanDNACognitiveBridge>();
    }
}

void UEndocrineExpressionPipeline::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 1. Decay hormones toward baselines
    UpdateHormoneDecay(DeltaTime);

    // 2. Detect cognitive mode
    ECognitiveMode OldMode = BusState.CurrentMode;
    BusState.CurrentMode = DetectCognitiveMode();
    if (OldMode != BusState.CurrentMode)
    {
        OnCognitiveModeChanged.Broadcast(OldMode, BusState.CurrentMode);
    }

    // 3. Compute valence-arousal
    float OldValence = BusState.Valence;
    float OldArousal = BusState.Arousal;
    ComputeValenceArousal(BusState.Valence, BusState.Arousal);
    if (FMath::Abs(OldValence - BusState.Valence) > 0.05f ||
        FMath::Abs(OldArousal - BusState.Arousal) > 0.05f)
    {
        OnValenceArousalChanged.Broadcast(BusState.Valence, BusState.Arousal);
    }

    // 4. Apply hormone-driven expressions
    ApplyHormoneExpressions();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UEndocrineExpressionPipeline::InitializeHormoneChannels()
{
    BusState.Channels.Empty();
    BusState.Channels.SetNum(16);

    // Define all 16 channels per the VES specification
    auto SetChannel = [this](int32 Idx, FName Name, float HalfLife, float Baseline) {
        BusState.Channels[Idx].HormoneName = Name;
        BusState.Channels[Idx].HalfLife = HalfLife;
        BusState.Channels[Idx].Baseline = Baseline;
        BusState.Channels[Idx].Concentration = Baseline;
        BusState.Channels[Idx].DecayRate = FMath::Loge(2.0f) / HalfLife;
    };

    SetChannel(0,  FName("CRH"),              5.0f,  0.05f);
    SetChannel(1,  FName("ACTH"),             10.0f,  0.05f);
    SetChannel(2,  FName("Cortisol"),         30.0f,  0.15f);
    SetChannel(3,  FName("Dopamine_Tonic"),   20.0f,  0.30f);
    SetChannel(4,  FName("Dopamine_Phasic"),   3.0f,  0.00f);
    SetChannel(5,  FName("Serotonin"),        50.0f,  0.40f);
    SetChannel(6,  FName("Norepinephrine"),    8.0f,  0.10f);
    SetChannel(7,  FName("Oxytocin"),         15.0f,  0.10f);
    SetChannel(8,  FName("T3_T4"),           100.0f,  0.50f);
    SetChannel(9,  FName("Melatonin"),        12.0f,  0.00f);
    SetChannel(10, FName("Insulin"),          10.0f,  0.20f);
    SetChannel(11, FName("Glucagon"),          8.0f,  0.10f);
    SetChannel(12, FName("IL6"),              20.0f,  0.05f);
    SetChannel(13, FName("Anandamide"),        6.0f,  0.10f);
    SetChannel(14, FName("Reserved_A"),       10.0f,  0.00f);
    SetChannel(15, FName("Reserved_B"),       10.0f,  0.00f);
}

void UEndocrineExpressionPipeline::InitializeHormoneAUMappings()
{
    HormoneAUMappings.Empty();

    auto AddMapping = [this](FName Hormone, FName AU, float Weight, float Threshold = 0.0f, bool bInhibit = false) {
        FHormoneAUMapping M;
        M.HormoneName = Hormone;
        M.ActionUnitName = AU;
        M.Weight = Weight;
        M.Threshold = Threshold;
        M.bInhibitory = bInhibit;
        HormoneAUMappings.Add(M);
    };

    // Cortisol → stress expression
    AddMapping(FName("Cortisol"), FName("AU4"),  0.6f, 0.2f);  // Brow lowerer
    AddMapping(FName("Cortisol"), FName("AU1"),  0.4f, 0.2f);  // Inner brow raise
    AddMapping(FName("Cortisol"), FName("AU15"), 0.5f, 0.3f);  // Lip corner depress
    AddMapping(FName("Cortisol"), FName("AU7"),  0.3f, 0.2f);  // Lid tightener

    // Dopamine (phasic) → reward/joy expression
    AddMapping(FName("Dopamine_Phasic"), FName("AU12"), 0.8f, 0.1f);  // Smile
    AddMapping(FName("Dopamine_Phasic"), FName("AU6"),  0.7f, 0.1f);  // Cheek raise (Duchenne)
    AddMapping(FName("Dopamine_Phasic"), FName("AU25"), 0.3f, 0.2f);  // Lips part

    // Dopamine (tonic) → subtle contentment
    AddMapping(FName("Dopamine_Tonic"), FName("AU12"), 0.2f, 0.3f);   // Subtle smile
    AddMapping(FName("Dopamine_Tonic"), FName("AU6"),  0.15f, 0.3f);  // Slight cheek raise

    // Norepinephrine → alertness
    AddMapping(FName("Norepinephrine"), FName("AU5"),  0.7f, 0.15f);  // Upper lid raise
    AddMapping(FName("Norepinephrine"), FName("AU7"),  0.5f, 0.2f);   // Lid tightener
    AddMapping(FName("Norepinephrine"), FName("AU1"),  0.3f, 0.3f);   // Inner brow raise

    // Oxytocin → warm social expression
    AddMapping(FName("Oxytocin"), FName("AU6"),  0.5f, 0.15f);  // Cheek raise
    AddMapping(FName("Oxytocin"), FName("AU12"), 0.6f, 0.15f);  // Smile
    AddMapping(FName("Oxytocin"), FName("AU25"), 0.3f, 0.2f);   // Lips part
    AddMapping(FName("Oxytocin"), FName("AU4"),  0.2f, 0.0f, true); // Inhibit brow lowerer

    // Serotonin → calm, content expression
    AddMapping(FName("Serotonin"), FName("AU12"), 0.3f, 0.4f);  // Subtle smile
    AddMapping(FName("Serotonin"), FName("AU6"),  0.2f, 0.4f);  // Slight cheek raise
    AddMapping(FName("Serotonin"), FName("AU4"),  0.3f, 0.0f, true); // Inhibit stress brow

    // Melatonin → drowsiness
    AddMapping(FName("Melatonin"), FName("AU43"), 0.8f, 0.1f);  // Eyes closed
    AddMapping(FName("Melatonin"), FName("AU5"),  0.5f, 0.0f, true); // Inhibit lid raise

    // CRH → acute stress (precedes cortisol)
    AddMapping(FName("CRH"), FName("AU1"),  0.5f, 0.1f);  // Inner brow raise
    AddMapping(FName("CRH"), FName("AU5"),  0.4f, 0.1f);  // Upper lid raise (startle)
    AddMapping(FName("CRH"), FName("AU20"), 0.3f, 0.2f);  // Lip stretcher

    // Anandamide → relaxation, noise reduction
    AddMapping(FName("Anandamide"), FName("AU12"), 0.2f, 0.15f);  // Slight smile
    AddMapping(FName("Anandamide"), FName("AU7"),  0.3f, 0.0f, true); // Reduce lid tension

    // IL-6 → system distress
    AddMapping(FName("IL6"), FName("AU4"),  0.4f, 0.1f);  // Brow lowerer
    AddMapping(FName("IL6"), FName("AU43"), 0.3f, 0.2f);  // Partial eye close
}

// ============================================================================
// HORMONE DYNAMICS
// ============================================================================

void UEndocrineExpressionPipeline::UpdateHormoneDecay(float DeltaTime)
{
    for (FHormoneChannel& Channel : BusState.Channels)
    {
        // Exponential decay toward baseline: C(t) = baseline + (C - baseline) * e^(-decay * dt)
        float Delta = Channel.Concentration - Channel.Baseline;
        Channel.Concentration = Channel.Baseline + Delta * FMath::Exp(-Channel.DecayRate * DeltaTime);
    }
}

ECognitiveMode UEndocrineExpressionPipeline::DetectCognitiveMode() const
{
    float Cortisol = GetHormoneLevel(FName("Cortisol"));
    float DopamineTonic = GetHormoneLevel(FName("Dopamine_Tonic"));
    float DopaminePhasic = GetHormoneLevel(FName("Dopamine_Phasic"));
    float Serotonin = GetHormoneLevel(FName("Serotonin"));
    float NE = GetHormoneLevel(FName("Norepinephrine"));
    float Oxytocin = GetHormoneLevel(FName("Oxytocin"));
    float Melatonin = GetHormoneLevel(FName("Melatonin"));
    float Anandamide = GetHormoneLevel(FName("Anandamide"));
    float T3T4 = GetHormoneLevel(FName("T3_T4"));

    // Flow state: optimal dopamine + moderate NE + low cortisol + high serotonin
    if (DopamineTonic > 0.4f && NE > 0.15f && NE < 0.4f && Cortisol < 0.2f && Serotonin > 0.4f)
        return ECognitiveMode::Flow;

    // Transcendent: all positive hormones high, stress low
    if (DopamineTonic > 0.5f && Serotonin > 0.5f && Oxytocin > 0.3f && Cortisol < 0.15f)
        return ECognitiveMode::Transcendent;

    // Stressed: high cortisol
    if (Cortisol > 0.4f)
        return ECognitiveMode::Stressed;

    // Alert: high norepinephrine
    if (NE > 0.4f)
        return ECognitiveMode::Alert;

    // Creative: edge of chaos dopamine + moderate NE
    if (DopaminePhasic > 0.3f && NE > 0.2f && T3T4 > 0.5f)
        return ECognitiveMode::Creative;

    // Social: high oxytocin
    if (Oxytocin > 0.3f)
        return ECognitiveMode::Social;

    // Contemplative: high serotonin, low arousal
    if (Serotonin > 0.5f && NE < 0.15f)
        return ECognitiveMode::Contemplative;

    // Playful: high anandamide
    if (Anandamide > 0.3f && DopamineTonic > 0.3f)
        return ECognitiveMode::Playful;

    // Focused: moderate dopamine + NE
    if (DopamineTonic > 0.3f && NE > 0.1f)
        return ECognitiveMode::Focused;

    return ECognitiveMode::Resting;
}

void UEndocrineExpressionPipeline::ComputeValenceArousal(float& OutValence, float& OutArousal) const
{
    float DopamineTonic = GetHormoneLevel(FName("Dopamine_Tonic"));
    float DopaminePhasic = GetHormoneLevel(FName("Dopamine_Phasic"));
    float Serotonin = GetHormoneLevel(FName("Serotonin"));
    float Oxytocin = GetHormoneLevel(FName("Oxytocin"));
    float Cortisol = GetHormoneLevel(FName("Cortisol"));
    float NE = GetHormoneLevel(FName("Norepinephrine"));
    float Anandamide = GetHormoneLevel(FName("Anandamide"));

    // Valence: positive hormones minus negative hormones
    OutValence = FMath::Clamp(
        (DopamineTonic * 0.3f + DopaminePhasic * 0.5f + Serotonin * 0.3f +
         Oxytocin * 0.4f + Anandamide * 0.2f) -
        (Cortisol * 0.6f),
        -1.0f, 1.0f
    );

    // Arousal: activating hormones
    OutArousal = FMath::Clamp(
        NE * 0.4f + DopaminePhasic * 0.3f + Cortisol * 0.2f +
        GetHormoneLevel(FName("CRH")) * 0.3f -
        GetHormoneLevel(FName("Melatonin")) * 0.5f -
        Anandamide * 0.2f,
        0.0f, 1.0f
    );
}

// ============================================================================
// COGNITIVE EVENT HANDLING
// ============================================================================

void UEndocrineExpressionPipeline::SignalCognitiveEvent(FName EventType, float Intensity)
{
    FString Event = EventType.ToString();

    if (Event == TEXT("THREAT_DETECTED"))
        SignalThreat(Intensity);
    else if (Event == TEXT("REWARD_RECEIVED"))
        SignalReward(Intensity);
    else if (Event == TEXT("SOCIAL_CONNECTION"))
        SignalSocialConnection(Intensity);
    else if (Event == TEXT("NOVELTY_ENCOUNTERED"))
        SignalNovelty(Intensity);
    else if (Event == TEXT("GOAL_ACHIEVED"))
        SignalGoalAchieved(Intensity);
}

void UEndocrineExpressionPipeline::SignalThreat(float Intensity)
{
    // HPA axis cascade: CRH → ACTH → Cortisol
    InjectHormone(FName("CRH"), 0.5f * Intensity);
    InjectHormone(FName("ACTH"), 0.3f * Intensity);
    InjectHormone(FName("Cortisol"), 0.4f * Intensity);
    InjectHormone(FName("Norepinephrine"), 0.3f * Intensity);
    OnHormoneSpike.Broadcast(FName("Cortisol"), GetHormoneLevel(FName("Cortisol")));
}

void UEndocrineExpressionPipeline::SignalReward(float Intensity)
{
    InjectHormone(FName("Dopamine_Phasic"), 0.6f * Intensity);
    InjectHormone(FName("Dopamine_Tonic"), 0.1f * Intensity);
    InjectHormone(FName("Serotonin"), 0.1f * Intensity);
    OnHormoneSpike.Broadcast(FName("Dopamine_Phasic"), GetHormoneLevel(FName("Dopamine_Phasic")));
}

void UEndocrineExpressionPipeline::SignalSocialConnection(float Intensity)
{
    InjectHormone(FName("Oxytocin"), 0.5f * Intensity);
    InjectHormone(FName("Serotonin"), 0.15f * Intensity);
    InjectHormone(FName("Dopamine_Tonic"), 0.1f * Intensity);
    OnHormoneSpike.Broadcast(FName("Oxytocin"), GetHormoneLevel(FName("Oxytocin")));
}

void UEndocrineExpressionPipeline::SignalNovelty(float Intensity)
{
    InjectHormone(FName("Norepinephrine"), 0.4f * Intensity);
    InjectHormone(FName("Dopamine_Phasic"), 0.2f * Intensity);
    OnHormoneSpike.Broadcast(FName("Norepinephrine"), GetHormoneLevel(FName("Norepinephrine")));
}

void UEndocrineExpressionPipeline::SignalGoalAchieved(float Intensity)
{
    InjectHormone(FName("Dopamine_Phasic"), 0.7f * Intensity);
    InjectHormone(FName("Dopamine_Tonic"), 0.2f * Intensity);
    InjectHormone(FName("Serotonin"), 0.2f * Intensity);
    InjectHormone(FName("Oxytocin"), 0.1f * Intensity);
    InjectHormone(FName("Anandamide"), 0.15f * Intensity);
    OnHormoneSpike.Broadcast(FName("Dopamine_Phasic"), GetHormoneLevel(FName("Dopamine_Phasic")));
}

// ============================================================================
// EXPRESSION MAPPING
// ============================================================================

TMap<FName, float> UEndocrineExpressionPipeline::GetHormoneDriverAUValues() const
{
    TMap<FName, float> AUValues;

    for (const FHormoneAUMapping& Mapping : HormoneAUMappings)
    {
        float HormoneLevel = GetHormoneLevel(Mapping.HormoneName);
        if (HormoneLevel < Mapping.Threshold) continue;

        float Contribution = (HormoneLevel - Mapping.Threshold) * Mapping.Weight;

        if (Mapping.bInhibitory)
        {
            float* Existing = AUValues.Find(Mapping.ActionUnitName);
            if (Existing)
            {
                *Existing = FMath::Max(0.0f, *Existing - Contribution);
            }
        }
        else
        {
            float* Existing = AUValues.Find(Mapping.ActionUnitName);
            if (Existing)
            {
                *Existing = FMath::Min(1.0f, *Existing + Contribution);
            }
            else
            {
                AUValues.Add(Mapping.ActionUnitName, FMath::Min(1.0f, Contribution));
            }
        }
    }

    return AUValues;
}

void UEndocrineExpressionPipeline::ConnectToDNABridge(UMetaHumanDNACognitiveBridge* Bridge)
{
    ConnectedBridge = Bridge;
}

void UEndocrineExpressionPipeline::ApplyHormoneExpressions()
{
    if (!ConnectedBridge) return;

    TMap<FName, float> AUValues = GetHormoneDriverAUValues();

    for (const auto& Pair : AUValues)
    {
        // Find matching AU in the bridge and blend
        for (FMetaHumanActionUnit& AU : ConnectedBridge->ActionUnits)
        {
            if (AU.ActionUnitName == Pair.Key)
            {
                // Additive blend with existing cognitive-driven values
                AU.Activation = FMath::Clamp(AU.Activation + Pair.Value * 0.5f, 0.0f, 1.0f);
                break;
            }
        }
    }
}

// ============================================================================
// UTILITY
// ============================================================================

float UEndocrineExpressionPipeline::GetHormoneLevel(FName HormoneName) const
{
    int32 Idx = GetChannelIndex(HormoneName);
    if (Idx >= 0 && Idx < BusState.Channels.Num())
    {
        return BusState.Channels[Idx].Concentration;
    }
    return 0.0f;
}

void UEndocrineExpressionPipeline::SetHormoneBaseline(FName HormoneName, float Baseline)
{
    int32 Idx = GetChannelIndex(HormoneName);
    if (Idx >= 0 && Idx < BusState.Channels.Num())
    {
        BusState.Channels[Idx].Baseline = FMath::Clamp(Baseline, 0.0f, 1.0f);
    }
}

void UEndocrineExpressionPipeline::GetValenceArousal(float& OutValence, float& OutArousal) const
{
    OutValence = BusState.Valence;
    OutArousal = BusState.Arousal;
}

int32 UEndocrineExpressionPipeline::GetChannelIndex(FName HormoneName) const
{
    for (int32 i = 0; i < BusState.Channels.Num(); ++i)
    {
        if (BusState.Channels[i].HormoneName == HormoneName)
        {
            return i;
        }
    }
    return -1;
}

void UEndocrineExpressionPipeline::InjectHormone(FName HormoneName, float Amount)
{
    int32 Idx = GetChannelIndex(HormoneName);
    if (Idx >= 0 && Idx < BusState.Channels.Num())
    {
        BusState.Channels[Idx].Concentration = FMath::Min(
            1.0f, BusState.Channels[Idx].Concentration + Amount);
    }
}
