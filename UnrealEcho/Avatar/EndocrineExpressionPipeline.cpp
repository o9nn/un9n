// EndocrineExpressionPipeline.cpp
// Deep Tree Echo - Virtual Endocrine to Avatar Expression Pipeline Implementation
// Copyright (c) 2025-2026 Deep Tree Echo Project

#include "EndocrineExpressionPipeline.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogExpressionPipeline, Log, All);

// ============================================================================
// CONSTRUCTOR
// ============================================================================

UEndocrineExpressionPipeline::UEndocrineExpressionPipeline()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UEndocrineExpressionPipeline::BeginPlay()
{
    Super::BeginPlay();

    // Initialize chaotic dynamics engine
    ChaoticEngine = NewObject<ULorenzChaoticDynamics>(this);
    ChaoticEngine->Initialize();

    // Cache mapping tables
    MorphTargetMappings = FFACSMetaHumanMappingTable::GetCompleteMappingTable();
    EndocrineMappings = FFACSMetaHumanMappingTable::GetEndocrineToFACSMappings();
    AestheticBiases = FFACSMetaHumanMappingTable::GetSuperHotGirlAUBiases();

    UE_LOG(LogExpressionPipeline, Log,
        TEXT("EndocrineExpressionPipeline initialized: %d morph mappings, %d endocrine mappings, %d aesthetic biases"),
        MorphTargetMappings.Num(), EndocrineMappings.Num(), AestheticBiases.Num());
}

void UEndocrineExpressionPipeline::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Auto-detect cognitive mode from endocrine state
    CurrentMode = DetectCognitiveMode();
}

// ============================================================================
// PIPELINE STEP 1-2: INPUT STATES
// ============================================================================

void UEndocrineExpressionPipeline::SetEndocrineState(const FEndocrineState& NewState)
{
    EndocrineInput = NewState;
}

void UEndocrineExpressionPipeline::SetCognitiveMode(ECognitiveMode Mode)
{
    CurrentMode = Mode;
}

// ============================================================================
// PIPELINE STEP 3: ENDOCRINE-DRIVEN AU ACTIVATIONS
// ============================================================================

void UEndocrineExpressionPipeline::ComputeEndocrineAUs()
{
    EndocrineAUValues.Empty();

    for (const FEndocrineAUMapping& Mapping : EndocrineMappings)
    {
        float HormoneValue = 0.0f;

        // Look up hormone concentration from endocrine state
        if (Mapping.HormoneName == TEXT("Cortisol")) HormoneValue = EndocrineInput.Cortisol;
        else if (Mapping.HormoneName == TEXT("DopaminePhasic")) HormoneValue = EndocrineInput.DopaminePhasic;
        else if (Mapping.HormoneName == TEXT("DopamineTonic")) HormoneValue = EndocrineInput.DopamineTonic;
        else if (Mapping.HormoneName == TEXT("Norepinephrine")) HormoneValue = EndocrineInput.Norepinephrine;
        else if (Mapping.HormoneName == TEXT("Serotonin")) HormoneValue = EndocrineInput.Serotonin;
        else if (Mapping.HormoneName == TEXT("Oxytocin")) HormoneValue = EndocrineInput.Oxytocin;
        else if (Mapping.HormoneName == TEXT("Melatonin")) HormoneValue = EndocrineInput.Melatonin;
        else if (Mapping.HormoneName == TEXT("Endorphin")) HormoneValue = EndocrineInput.Endorphin;
        else if (Mapping.HormoneName == TEXT("Adrenaline")) HormoneValue = EndocrineInput.Adrenaline;
        else if (Mapping.HormoneName == TEXT("GABA")) HormoneValue = EndocrineInput.Anandamide; // Approximate
        else if (Mapping.HormoneName == TEXT("Testosterone")) HormoneValue = 0.5f; // Default baseline

        // Apply mapping curve
        float AUActivation = FFACSMetaHumanMappingTable::ApplyMappingCurve(
            HormoneValue, Mapping.Sensitivity, Mapping.Threshold, Mapping.CurveType);

        // Accumulate into endocrine AU buffer
        AccumulateAU(EndocrineAUValues, Mapping.TargetAU, AUActivation);
    }
}

// ============================================================================
// PIPELINE STEP 4: COGNITIVE-DRIVEN AU ACTIVATIONS
// ============================================================================

void UEndocrineExpressionPipeline::ComputeCognitiveAUs(float EmotionValence, float Arousal, float CognitiveLoad)
{
    CognitiveAUValues.Empty();

    // Valence mapping: positive → smile, negative → frown
    if (EmotionValence > 0.0f)
    {
        AccumulateAU(CognitiveAUValues, TEXT("AU12"), EmotionValence * 0.8f);  // Smile
        AccumulateAU(CognitiveAUValues, TEXT("AU6"), EmotionValence * 0.6f);   // Cheek raise
    }
    else
    {
        AccumulateAU(CognitiveAUValues, TEXT("AU15"), -EmotionValence * 0.7f); // Frown
        AccumulateAU(CognitiveAUValues, TEXT("AU1"), -EmotionValence * 0.5f);  // Inner brow raise
    }

    // Arousal mapping: high → wide eyes, open mouth
    AccumulateAU(CognitiveAUValues, TEXT("AU5"), Arousal * 0.6f);   // Upper lid raise
    AccumulateAU(CognitiveAUValues, TEXT("AU25"), Arousal * 0.4f);  // Lips part
    AccumulateAU(CognitiveAUValues, TEXT("AU26"), Arousal * 0.2f);  // Jaw drop

    // Cognitive load mapping: high → furrowed brow, tightened lids
    AccumulateAU(CognitiveAUValues, TEXT("AU4"), CognitiveLoad * 0.7f);  // Brow lowerer
    AccumulateAU(CognitiveAUValues, TEXT("AU7"), CognitiveLoad * 0.5f);  // Lid tightener
    AccumulateAU(CognitiveAUValues, TEXT("AU23"), CognitiveLoad * 0.3f); // Lip tightener

    // Cognitive mode-specific modulations
    switch (CurrentMode)
    {
    case ECognitiveMode::Explore:
        AccumulateAU(CognitiveAUValues, TEXT("AU2"), 0.2f);   // Curious brow raise
        AccumulateAU(CognitiveAUValues, TEXT("AU5"), 0.15f);  // Alert eyes
        break;
    case ECognitiveMode::Social:
        AccumulateAU(CognitiveAUValues, TEXT("AU12"), 0.25f); // Social smile
        AccumulateAU(CognitiveAUValues, TEXT("AU6"), 0.2f);   // Warm cheeks
        AccumulateAU(CognitiveAUValues, TEXT("AU53"), 0.1f);  // Head up (openness)
        break;
    case ECognitiveMode::Flow:
        AccumulateAU(CognitiveAUValues, TEXT("AU12"), 0.15f); // Subtle satisfaction
        AccumulateAU(CognitiveAUValues, TEXT("AU7"), 0.1f);   // Focused eyes
        break;
    case ECognitiveMode::Creative:
        AccumulateAU(CognitiveAUValues, TEXT("AU1"), 0.15f);  // Wondering brows
        AccumulateAU(CognitiveAUValues, TEXT("AU25"), 0.1f);  // Slightly parted lips
        break;
    case ECognitiveMode::Alert:
        AccumulateAU(CognitiveAUValues, TEXT("AU5"), 0.3f);   // Wide eyes
        AccumulateAU(CognitiveAUValues, TEXT("AU2"), 0.2f);   // Raised brows
        break;
    case ECognitiveMode::Stress:
        AccumulateAU(CognitiveAUValues, TEXT("AU4"), 0.3f);   // Furrowed brow
        AccumulateAU(CognitiveAUValues, TEXT("AU15"), 0.15f); // Slight frown
        break;
    case ECognitiveMode::Rest:
        AccumulateAU(CognitiveAUValues, TEXT("AU43"), 0.2f);  // Droopy eyes
        AccumulateAU(CognitiveAUValues, TEXT("AU12"), 0.1f);  // Peaceful smile
        break;
    default:
        break;
    }
}

// ============================================================================
// PIPELINE STEP 5: BLEND ALL AU SOURCES
// ============================================================================

void UEndocrineExpressionPipeline::BlendAllAUs()
{
    FinalAUValues.Empty();

    // Collect all unique AU IDs
    TSet<FName> AllAUIDs;
    for (auto& Pair : EndocrineAUValues) AllAUIDs.Add(Pair.Key);
    for (auto& Pair : CognitiveAUValues) AllAUIDs.Add(Pair.Key);

    // Sum contributions from all sources and clamp to [0, 1]
    for (const FName& AUID : AllAUIDs)
    {
        float Total = 0.0f;

        if (float* EndoVal = EndocrineAUValues.Find(AUID))
            Total += *EndoVal;

        if (float* CogVal = CognitiveAUValues.Find(AUID))
            Total += *CogVal;

        FinalAUValues.Add(AUID, FMath::Clamp(Total, 0.0f, 1.0f));
    }
}

// ============================================================================
// PIPELINE STEP 6: CHAOTIC MICRO-EXPRESSIONS
// ============================================================================

void UEndocrineExpressionPipeline::ApplyChaoticMicroExpressions(float DeltaTime)
{
    if (!bEnableMicroExpressions || !ChaoticEngine)
        return;

    // Advance chaotic dynamics
    ChaoticEngine->StepByTime(DeltaTime);

    // Generate micro-expression noise
    ChaoticNoiseValues = ChaoticEngine->GenerateMicroExpressionNoise(ChaosIntensity);

    // Add noise to final AU values
    for (auto& Pair : ChaoticNoiseValues)
    {
        float* CurrentVal = FinalAUValues.Find(Pair.Key);
        if (CurrentVal)
        {
            *CurrentVal = FMath::Clamp(*CurrentVal + Pair.Value, 0.0f, 1.0f);
        }
        else
        {
            FinalAUValues.Add(Pair.Key, FMath::Clamp(Pair.Value, 0.0f, 1.0f));
        }
    }
}

// ============================================================================
// PIPELINE STEP 7: SUPERHOT GIRL AESTHETIC BIASES
// ============================================================================

void UEndocrineExpressionPipeline::ApplyAestheticBiases()
{
    if (!bEnableAestheticBiases)
        return;

    // Apply static biases from mapping table
    for (auto& Pair : AestheticBiases)
    {
        float* CurrentVal = FinalAUValues.Find(Pair.Key);
        if (CurrentVal)
        {
            *CurrentVal = FMath::Clamp(*CurrentVal + Pair.Value, 0.0f, 1.0f);
        }
        else if (Pair.Value > 0.0f)
        {
            FinalAUValues.Add(Pair.Key, FMath::Clamp(Pair.Value, 0.0f, 1.0f));
        }
    }

    // Dynamic aesthetic modulation based on parameters
    // Confidence → head up, slight smile
    float ConfidenceBias = Aesthetics.ConfidencePosture * 0.15f;
    AccumulateAU(FinalAUValues, TEXT("AU53"), ConfidenceBias);
    AccumulateAU(FinalAUValues, TEXT("AU12"), ConfidenceBias * 0.5f);

    // Charisma → warm expression
    float CharismaBias = Aesthetics.Charisma * 0.1f;
    AccumulateAU(FinalAUValues, TEXT("AU6"), CharismaBias);
    AccumulateAU(FinalAUValues, TEXT("AU12"), CharismaBias * 0.7f);

    // Eye sparkle → upper lid raise
    float SparkleBias = Aesthetics.EyeSparkle * 0.12f;
    AccumulateAU(FinalAUValues, TEXT("AU5"), SparkleBias);

    // Clamp all values
    for (auto& Pair : FinalAUValues)
    {
        Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }
}

// ============================================================================
// PIPELINE STEP 8: MAP TO METAHUMAN MORPH TARGETS
// ============================================================================

TMap<FString, float> UEndocrineExpressionPipeline::MapToMorphTargets() const
{
    TMap<FString, float> MorphTargets;

    for (const FMorphTargetMapping& Mapping : MorphTargetMappings)
    {
        const float* AUValue = FinalAUValues.Find(Mapping.ActionUnitID);
        if (!AUValue || *AUValue < 0.001f)
            continue;

        float WeightedValue = *AUValue * Mapping.WeightMultiplier;

        // Select correct morph target names based on rig version
        FString LeftTarget = bUseDHIRig ? Mapping.DHI_MorphTarget_Left : Mapping.MH4_MorphTarget_Left;
        FString RightTarget = bUseDHIRig ? Mapping.DHI_MorphTarget_Right : Mapping.MH4_MorphTarget_Right;

        if (!LeftTarget.IsEmpty())
        {
            float* Existing = MorphTargets.Find(LeftTarget);
            float NewVal = Existing ? FMath::Clamp(*Existing + WeightedValue, 0.0f, 1.0f) : WeightedValue;
            MorphTargets.Add(LeftTarget, NewVal);
        }

        if (!RightTarget.IsEmpty())
        {
            float* Existing = MorphTargets.Find(RightTarget);
            float NewVal = Existing ? FMath::Clamp(*Existing + WeightedValue, 0.0f, 1.0f) : WeightedValue;
            MorphTargets.Add(RightTarget, NewVal);
        }
    }

    return MorphTargets;
}

// ============================================================================
// PIPELINE STEP 9: DYNAMIC MATERIALS
// ============================================================================

void UEndocrineExpressionPipeline::UpdateDynamicMaterials(USkeletalMeshComponent* Mesh)
{
    if (!bEnableDynamicMaterials || !Mesh)
        return;

    int32 NumMaterials = Mesh->GetNumMaterials();
    for (int32 i = 0; i < NumMaterials; ++i)
    {
        UMaterialInterface* Material = Mesh->GetMaterial(i);
        if (!Material) continue;

        UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Material);
        if (!DynMat)
        {
            DynMat = Mesh->CreateDynamicMaterialInstance(i);
        }

        if (DynMat)
        {
            // Skin subsurface scattering boost
            DynMat->SetScalarParameterValue(TEXT("SubsurfaceScatteringBoost"),
                Aesthetics.SkinSSSBoost);

            // Eye sparkle modulated by dopamine
            float SparkleIntensity = Aesthetics.EyeSparkle *
                (1.0f + EndocrineInput.DopaminePhasic * 0.5f);
            DynMat->SetScalarParameterValue(TEXT("EyeSparkleIntensity"), SparkleIntensity);

            // Iris specular boost
            DynMat->SetScalarParameterValue(TEXT("IrisSpecularBoost"),
                Aesthetics.IrisSpecularBoost);

            // Lip gloss modulated by serotonin
            float LipGloss = Aesthetics.LipGlossSheen *
                (0.8f + EndocrineInput.Serotonin * 0.4f);
            DynMat->SetScalarParameterValue(TEXT("LipGlossSheen"), LipGloss);

            // Charisma glow modulated by oxytocin
            float GlowIntensity = Aesthetics.EmissiveGlow *
                (1.0f + EndocrineInput.Oxytocin * 0.3f);
            DynMat->SetScalarParameterValue(TEXT("CharismaGlowIntensity"), GlowIntensity);

            // Chaotic shimmer from Lorenz attractor
            if (ChaoticEngine)
            {
                float ChaoticShimmer = FMath::Sin(ChaoticEngine->GetCurrentPosition().Z * 0.5f) * 0.02f;
                DynMat->SetScalarParameterValue(TEXT("SkinShimmer"),
                    FMath::Clamp(ChaoticShimmer + 0.02f, 0.0f, 0.1f));
            }

            // Hair sheen
            DynMat->SetScalarParameterValue(TEXT("HairSheen"), Aesthetics.HairSheen);
        }
    }
}

// ============================================================================
// PIPELINE STEP 10: APPLY TO SKELETAL MESH
// ============================================================================

void UEndocrineExpressionPipeline::ApplyToSkeletalMesh(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return;

    TMap<FString, float> MorphTargets = MapToMorphTargets();

    for (auto& Pair : MorphTargets)
    {
        Mesh->SetMorphTarget(FName(*Pair.Key), Pair.Value);
    }
}

// ============================================================================
// PIPELINE STEP 11: DEBUG EXPORT
// ============================================================================

FString UEndocrineExpressionPipeline::ExportGlyphProjection() const
{
    FString Output = TEXT("{\n  \"expression_state\": {\n");

    bool bFirst = true;
    for (auto& Pair : FinalAUValues)
    {
        if (!bFirst) Output += TEXT(",\n");
        Output += FString::Printf(TEXT("    \"%s\": %.4f"), *Pair.Key.ToString(), Pair.Value);
        bFirst = false;
    }

    Output += TEXT("\n  },\n  \"cognitive_mode\": \"");
    switch (CurrentMode)
    {
    case ECognitiveMode::Explore: Output += TEXT("explore"); break;
    case ECognitiveMode::Exploit: Output += TEXT("exploit"); break;
    case ECognitiveMode::Stress: Output += TEXT("stress"); break;
    case ECognitiveMode::Social: Output += TEXT("social"); break;
    case ECognitiveMode::Rest: Output += TEXT("rest"); break;
    case ECognitiveMode::Flow: Output += TEXT("flow"); break;
    case ECognitiveMode::Creative: Output += TEXT("creative"); break;
    case ECognitiveMode::Alert: Output += TEXT("alert"); break;
    }
    Output += TEXT("\",\n");

    if (ChaoticEngine)
    {
        FChaoticDynamicsTelemetry Tel = ChaoticEngine->GetTelemetry();
        Output += FString::Printf(TEXT("  \"chaotic_dynamics\": {\n"));
        Output += FString::Printf(TEXT("    \"lyapunov_exponent\": %.6f,\n"), Tel.LyapunovExponent);
        Output += FString::Printf(TEXT("    \"is_chaotic\": %s,\n"), Tel.bIsInChaoticRegime ? TEXT("true") : TEXT("false"));
        Output += FString::Printf(TEXT("    \"attractor_energy\": %.4f,\n"), Tel.AttractorEnergy);
        Output += FString::Printf(TEXT("    \"total_steps\": %d\n"), Tel.TotalSteps);
        Output += TEXT("  }\n");
    }

    Output += TEXT("}");
    return Output;
}

// ============================================================================
// FULL PIPELINE
// ============================================================================

void UEndocrineExpressionPipeline::ExecuteFullPipeline(float DeltaTime, USkeletalMeshComponent* Mesh,
    float EmotionValence, float Arousal, float CognitiveLoad)
{
    // Steps 1-2: Inputs already set via SetEndocrineState / auto-detected mode
    // Step 3: Endocrine AUs
    ComputeEndocrineAUs();
    // Step 4: Cognitive AUs
    ComputeCognitiveAUs(EmotionValence, Arousal, CognitiveLoad);
    // Step 5: Blend
    BlendAllAUs();
    // Step 6: Chaos
    ApplyChaoticMicroExpressions(DeltaTime);
    // Step 7: Aesthetics
    ApplyAestheticBiases();
    // Step 8-10: Output
    if (Mesh)
    {
        ApplyToSkeletalMesh(Mesh);
        UpdateDynamicMaterials(Mesh);
    }
}

// ============================================================================
// ACCESSORS
// ============================================================================

float UEndocrineExpressionPipeline::GetFinalAUValue(FName AUID) const
{
    const float* Val = FinalAUValues.Find(AUID);
    return Val ? *Val : 0.0f;
}

FChaoticDynamicsTelemetry UEndocrineExpressionPipeline::GetChaoticTelemetry() const
{
    return ChaoticEngine ? ChaoticEngine->GetTelemetry() : FChaoticDynamicsTelemetry();
}

// ============================================================================
// HELPERS
// ============================================================================

ECognitiveMode UEndocrineExpressionPipeline::DetectCognitiveMode() const
{
    // Detect emergent cognitive mode from hormone concentrations
    float NE = EndocrineInput.Norepinephrine;
    float DA_P = EndocrineInput.DopaminePhasic;
    float DA_T = EndocrineInput.DopamineTonic;
    float Cort = EndocrineInput.Cortisol;
    float OT = EndocrineInput.Oxytocin;
    float Mel = EndocrineInput.Melatonin;
    float Adr = EndocrineInput.Adrenaline;
    float Anan = EndocrineInput.Anandamide;

    if (Mel > 0.6f) return ECognitiveMode::Rest;
    if (Adr > 0.7f && NE > 0.6f) return ECognitiveMode::Alert;
    if (Cort > 0.7f) return ECognitiveMode::Stress;
    if (OT > 0.6f) return ECognitiveMode::Social;
    if (DA_P > 0.5f && Anan > 0.4f) return ECognitiveMode::Creative;
    if (NE > 0.5f && DA_P > 0.4f) return ECognitiveMode::Explore;
    if (DA_T > 0.6f) return ECognitiveMode::Exploit;

    // Check for flow state: balanced hormones
    float Variance = FMath::Abs(NE - 0.5f) + FMath::Abs(DA_T - 0.5f) +
        FMath::Abs(EndocrineInput.Serotonin - 0.5f);
    if (Variance < 0.5f) return ECognitiveMode::Flow;

    return ECognitiveMode::Explore; // Default
}

void UEndocrineExpressionPipeline::AccumulateAU(TMap<FName, float>& Target, FName AUID, float Value)
{
    float* Existing = Target.Find(AUID);
    if (Existing)
    {
        *Existing = FMath::Clamp(*Existing + Value, 0.0f, 1.0f);
    }
    else
    {
        Target.Add(AUID, FMath::Clamp(Value, 0.0f, 1.0f));
    }
}
