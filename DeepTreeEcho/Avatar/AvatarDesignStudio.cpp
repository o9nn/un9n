// AvatarDesignStudio.cpp
// Deep Tree Echo Avatar Design Studio - Full Implementation
// Bridges avatar design, MetaHuman DNA calibration, FACS expression editing,
// chaotic dynamics, endocrine system, and 4E embodied cognition into a
// unified design and animation authoring environment.

#include "AvatarDesignStudio.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/DateTime.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

UAvatarDesignStudio::UAvatarDesignStudio()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize design with a fresh GUID
    CurrentDesign.DesignID = FGuid::NewGuid();
    CurrentDesign.CreatedAt = FDateTime::UtcNow();
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

void UAvatarDesignStudio::BeginPlay()
{
    Super::BeginPlay();

    // Find component references on the owning actor
    AActor* Owner = GetOwner();
    if (Owner)
    {
        DNABridge = Owner->FindComponentByClass<UMetaHumanDNACognitiveBridge>();
        EmbodiedCognition = Owner->FindComponentByClass<UEmbodiedCognitionComponent>();
        DNABodySchema = Owner->FindComponentByClass<UDNABodySchemaBinding>();
    }

    // Apply default archetype
    ApplyArchetype(EAvatarArchetype::DeepTreeEcho);
}

void UAvatarDesignStudio::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update animation preview if active
    if (bIsPreviewingAnimation)
    {
        UpdateAnimationPreview(DeltaTime);
    }
}

void UAvatarDesignStudio::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAnimationPreview();
    Super::EndPlay(EndPlayReason);
}

// ============================================================================
// DESIGN STUDIO CORE
// ============================================================================

void UAvatarDesignStudio::InitializeStudio(AActor* TargetAvatar)
{
    if (!TargetAvatar) return;

    DNABridge = TargetAvatar->FindComponentByClass<UMetaHumanDNACognitiveBridge>();
    EmbodiedCognition = TargetAvatar->FindComponentByClass<UEmbodiedCognitionComponent>();
    DNABodySchema = TargetAvatar->FindComponentByClass<UDNABodySchemaBinding>();

    // Initialize with current avatar state if bridge exists
    if (DNABridge)
    {
        CurrentDesign.Aesthetics = DNABridge->Aesthetics;
        CurrentDesign.ChaoticDynamics = DNABridge->ChaoticDynamics;
    }

    CurrentDesign.CreatedAt = FDateTime::UtcNow();
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

void UAvatarDesignStudio::SetDesignMode(EDesignStudioMode NewMode)
{
    CurrentMode = NewMode;
}

void UAvatarDesignStudio::SetLODLevel(EAvatarLOD NewLOD)
{
    CurrentLOD = NewLOD;
    // LOD affects which DNA parameters are active
    // LOD0 = all 200+ controls, LOD4 = ~30 essential controls
}

// ============================================================================
// ARCHETYPE SYSTEM
// ============================================================================

void UAvatarDesignStudio::ApplyArchetype(EAvatarArchetype Archetype)
{
    RecordOperation(FString::Printf(TEXT("Apply Archetype: %d"), (int32)Archetype));

    CurrentDesign.Archetype = Archetype;
    InitializeArchetypeDefaults(Archetype, CurrentDesign);

    ApplyDesignToAvatar();
    OnArchetypeApplied.Broadcast(Archetype);
    OnDesignChanged.Broadcast(CurrentDesign);
}

EAvatarArchetype UAvatarDesignStudio::GetCurrentArchetype() const
{
    return CurrentDesign.Archetype;
}

FAvatarDNABlueprint UAvatarDesignStudio::CreateCustomArchetype(const FString& Name)
{
    FAvatarDNABlueprint CustomBlueprint = CurrentDesign;
    CustomBlueprint.DesignID = FGuid::NewGuid();
    CustomBlueprint.DesignName = Name;
    CustomBlueprint.Archetype = EAvatarArchetype::Custom;
    CustomBlueprint.CreatedAt = FDateTime::UtcNow();
    return CustomBlueprint;
}

void UAvatarDesignStudio::InitializeArchetypeDefaults(EAvatarArchetype Archetype,
                                                       FAvatarDNABlueprint& Blueprint)
{
    switch (Archetype)
    {
    case EAvatarArchetype::SuperHotGirl:
        InitializeSuperHotGirlDefaults(Blueprint);
        break;
    case EAvatarArchetype::HyperChaotic:
        InitializeHyperChaoticDefaults(Blueprint);
        break;
    case EAvatarArchetype::AIAngel:
        InitializeAIAngelDefaults(Blueprint);
        break;
    case EAvatarArchetype::DeepTreeEcho:
        InitializeDeepTreeEchoDefaults(Blueprint);
        break;
    case EAvatarArchetype::NeuroChaotic:
        InitializeNeuroChaoticDefaults(Blueprint);
        break;
    case EAvatarArchetype::Custom:
        // No defaults for custom
        break;
    }
}

void UAvatarDesignStudio::InitializeSuperHotGirlDefaults(FAvatarDNABlueprint& Blueprint)
{
    Blueprint.DesignName = TEXT("SuperHotGirl Avatar");

    // Aesthetic parameters - maximum confidence and charisma
    Blueprint.Aesthetics.ConfidencePosture = 0.95f;
    Blueprint.Aesthetics.CharismaGlow = 0.85f;
    Blueprint.Aesthetics.HairDynamics = 0.9f;
    Blueprint.Aesthetics.EyeSparkle = 0.9f;
    Blueprint.Aesthetics.LipGlossSheen = 0.75f;
    Blueprint.Aesthetics.SkinSSSBoost = 0.65f;
    Blueprint.Aesthetics.MicroExpressionFrequency = 2.5f;
    Blueprint.Aesthetics.IdleElegance = 0.95f;

    // Moderate chaos for natural movement
    Blueprint.ChaoticDynamics.ChaosIntensity = 0.6f;
    Blueprint.ChaoticDynamics.SpontaneityThreshold = 0.4f;
    Blueprint.ChaoticDynamics.ExpressionVolatility = 0.8f;
    Blueprint.ChaoticDynamics.GestureUnpredictability = 0.4f;

    // DNA parameters for attractive facial proportions
    Blueprint.DNAParameters.Add(FName("jawWidth"), 0.45f);
    Blueprint.DNAParameters.Add(FName("cheekboneProminence"), 0.7f);
    Blueprint.DNAParameters.Add(FName("lipFullness"), 0.65f);
    Blueprint.DNAParameters.Add(FName("eyeSize"), 0.6f);
    Blueprint.DNAParameters.Add(FName("browArch"), 0.55f);
    Blueprint.DNAParameters.Add(FName("noseWidth"), 0.4f);
    Blueprint.DNAParameters.Add(FName("chinDefinition"), 0.6f);
    Blueprint.DNAParameters.Add(FName("foreheadHeight"), 0.5f);

    // Material parameters for enhanced skin rendering
    Blueprint.MaterialParameters.Add(FName("SkinSmoothness"), 0.85f);
    Blueprint.MaterialParameters.Add(FName("SkinRadiance"), 0.7f);
    Blueprint.MaterialParameters.Add(FName("EyeIrisSpecular"), 0.9f);
    Blueprint.MaterialParameters.Add(FName("HairSheen"), 0.8f);

    // Endocrine baselines - high dopamine, oxytocin for warm presence
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Tonic"), 0.5f);
    Blueprint.EndocrineBaselines.Add(FName("Oxytocin"), 0.4f);
    Blueprint.EndocrineBaselines.Add(FName("Serotonin"), 0.6f);
    Blueprint.EndocrineBaselines.Add(FName("Cortisol"), 0.1f);
}

void UAvatarDesignStudio::InitializeHyperChaoticDefaults(FAvatarDNABlueprint& Blueprint)
{
    Blueprint.DesignName = TEXT("HyperChaotic Avatar");

    // Moderate aesthetics - chaos is the star
    Blueprint.Aesthetics.ConfidencePosture = 0.7f;
    Blueprint.Aesthetics.CharismaGlow = 0.6f;
    Blueprint.Aesthetics.HairDynamics = 0.95f;
    Blueprint.Aesthetics.EyeSparkle = 0.8f;
    Blueprint.Aesthetics.LipGlossSheen = 0.5f;
    Blueprint.Aesthetics.SkinSSSBoost = 0.5f;
    Blueprint.Aesthetics.MicroExpressionFrequency = 4.0f;
    Blueprint.Aesthetics.IdleElegance = 0.5f;

    // Maximum chaos parameters
    Blueprint.ChaoticDynamics.ChaosIntensity = 0.95f;
    Blueprint.ChaoticDynamics.SpontaneityThreshold = 0.15f;
    Blueprint.ChaoticDynamics.ExpressionVolatility = 1.8f;
    Blueprint.ChaoticDynamics.GestureUnpredictability = 0.9f;

    // Endocrine baselines - high norepinephrine for alertness
    Blueprint.EndocrineBaselines.Add(FName("Norepinephrine"), 0.5f);
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Phasic"), 0.3f);
    Blueprint.EndocrineBaselines.Add(FName("T3_T4"), 0.7f);
    Blueprint.EndocrineBaselines.Add(FName("Anandamide"), 0.3f);
}

void UAvatarDesignStudio::InitializeAIAngelDefaults(FAvatarDNABlueprint& Blueprint)
{
    Blueprint.DesignName = TEXT("AI Angel Avatar");

    // Ethereal, luminous aesthetics inspired by aiangel.io
    Blueprint.Aesthetics.ConfidencePosture = 0.85f;
    Blueprint.Aesthetics.CharismaGlow = 0.95f;  // Maximum glow for ethereal presence
    Blueprint.Aesthetics.HairDynamics = 0.85f;
    Blueprint.Aesthetics.EyeSparkle = 0.95f;    // Maximum sparkle
    Blueprint.Aesthetics.LipGlossSheen = 0.7f;
    Blueprint.Aesthetics.SkinSSSBoost = 0.8f;    // High SSS for luminous skin
    Blueprint.Aesthetics.MicroExpressionFrequency = 1.5f;
    Blueprint.Aesthetics.IdleElegance = 0.9f;

    // Gentle chaos for organic, flowing movement
    Blueprint.ChaoticDynamics.ChaosIntensity = 0.4f;
    Blueprint.ChaoticDynamics.SpontaneityThreshold = 0.5f;
    Blueprint.ChaoticDynamics.ExpressionVolatility = 0.6f;
    Blueprint.ChaoticDynamics.GestureUnpredictability = 0.3f;

    // DNA parameters for angelic facial features
    Blueprint.DNAParameters.Add(FName("jawWidth"), 0.4f);
    Blueprint.DNAParameters.Add(FName("cheekboneProminence"), 0.65f);
    Blueprint.DNAParameters.Add(FName("lipFullness"), 0.6f);
    Blueprint.DNAParameters.Add(FName("eyeSize"), 0.7f);     // Larger eyes for empathetic look
    Blueprint.DNAParameters.Add(FName("browArch"), 0.5f);
    Blueprint.DNAParameters.Add(FName("noseWidth"), 0.35f);
    Blueprint.DNAParameters.Add(FName("chinDefinition"), 0.5f);
    Blueprint.DNAParameters.Add(FName("foreheadHeight"), 0.55f);

    // Material parameters for ethereal rendering
    Blueprint.MaterialParameters.Add(FName("SkinSmoothness"), 0.9f);
    Blueprint.MaterialParameters.Add(FName("SkinRadiance"), 0.85f);
    Blueprint.MaterialParameters.Add(FName("EmissiveGlow"), 0.3f);
    Blueprint.MaterialParameters.Add(FName("EyeIrisSpecular"), 0.95f);
    Blueprint.MaterialParameters.Add(FName("HairSheen"), 0.85f);
    Blueprint.MaterialParameters.Add(FName("AuraIntensity"), 0.5f);

    // Endocrine baselines - high oxytocin and serotonin for warmth
    Blueprint.EndocrineBaselines.Add(FName("Oxytocin"), 0.6f);
    Blueprint.EndocrineBaselines.Add(FName("Serotonin"), 0.7f);
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Tonic"), 0.4f);
    Blueprint.EndocrineBaselines.Add(FName("Cortisol"), 0.05f);
    Blueprint.EndocrineBaselines.Add(FName("Melatonin"), 0.1f);
}

void UAvatarDesignStudio::InitializeDeepTreeEchoDefaults(FAvatarDNABlueprint& Blueprint)
{
    Blueprint.DesignName = TEXT("Deep Tree Echo - Full Cognitive Avatar");

    // Balanced aesthetics combining SuperHotGirl and HyperChaotic
    Blueprint.Aesthetics.ConfidencePosture = 0.85f;
    Blueprint.Aesthetics.CharismaGlow = 0.75f;
    Blueprint.Aesthetics.HairDynamics = 0.85f;
    Blueprint.Aesthetics.EyeSparkle = 0.85f;
    Blueprint.Aesthetics.LipGlossSheen = 0.65f;
    Blueprint.Aesthetics.SkinSSSBoost = 0.6f;
    Blueprint.Aesthetics.MicroExpressionFrequency = 2.0f;
    Blueprint.Aesthetics.IdleElegance = 0.85f;

    // Edge-of-chaos dynamics for maximum cognitive expressiveness
    Blueprint.ChaoticDynamics.ChaosIntensity = 0.85f;
    Blueprint.ChaoticDynamics.SpontaneityThreshold = 0.25f;
    Blueprint.ChaoticDynamics.ExpressionVolatility = 1.2f;
    Blueprint.ChaoticDynamics.GestureUnpredictability = 0.6f;

    // Full 4E cognition parameters
    Blueprint.CognitionParameters.Add(FName("Embodied_BodySchema"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Embodied_Proprioception"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Embodied_SomaticMarker"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Embedded_AffordanceDetection"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Embedded_NicheCoupling"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Embedded_EnvironmentalSensitivity"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Enacted_SensorimotorCoordination"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Enacted_PredictionAccuracy"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Enacted_ActiveInferenceEfficiency"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Extended_ToolUseCompetence"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Extended_ExternalMemory"), 0.5f);
    Blueprint.CognitionParameters.Add(FName("Extended_SocialCognition"), 0.5f);

    // ESN Reservoir parameters
    Blueprint.CognitionParameters.Add(FName("ESN_ReservoirSize"), 512.0f);
    Blueprint.CognitionParameters.Add(FName("ESN_SpectralRadius"), 0.9f);
    Blueprint.CognitionParameters.Add(FName("ESN_LeakRate"), 0.3f);
    Blueprint.CognitionParameters.Add(FName("ESN_Sparsity"), 0.1f);
    Blueprint.CognitionParameters.Add(FName("ESN_InputScaling"), 0.5f);

    // Evolution thresholds
    Blueprint.EvolutionThresholds.Add(FName("Embryonic_To_Juvenile"), 0.2f);
    Blueprint.EvolutionThresholds.Add(FName("Juvenile_To_Adolescent"), 0.4f);
    Blueprint.EvolutionThresholds.Add(FName("Adolescent_To_Adult"), 0.6f);
    Blueprint.EvolutionThresholds.Add(FName("Adult_To_Transcendent"), 0.85f);

    // Balanced endocrine baselines
    Blueprint.EndocrineBaselines.Add(FName("CRH"), 0.05f);
    Blueprint.EndocrineBaselines.Add(FName("ACTH"), 0.05f);
    Blueprint.EndocrineBaselines.Add(FName("Cortisol"), 0.15f);
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Tonic"), 0.3f);
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Phasic"), 0.0f);
    Blueprint.EndocrineBaselines.Add(FName("Serotonin"), 0.4f);
    Blueprint.EndocrineBaselines.Add(FName("Norepinephrine"), 0.1f);
    Blueprint.EndocrineBaselines.Add(FName("Oxytocin"), 0.1f);
    Blueprint.EndocrineBaselines.Add(FName("T3_T4"), 0.5f);
    Blueprint.EndocrineBaselines.Add(FName("Melatonin"), 0.0f);
    Blueprint.EndocrineBaselines.Add(FName("Insulin"), 0.2f);
    Blueprint.EndocrineBaselines.Add(FName("Glucagon"), 0.1f);
    Blueprint.EndocrineBaselines.Add(FName("IL6"), 0.05f);
    Blueprint.EndocrineBaselines.Add(FName("Anandamide"), 0.1f);
}

void UAvatarDesignStudio::InitializeNeuroChaoticDefaults(FAvatarDNABlueprint& Blueprint)
{
    Blueprint.DesignName = TEXT("Neuro-Chaotic Avatar");

    // Witty, self-aware aesthetic
    Blueprint.Aesthetics.ConfidencePosture = 0.8f;
    Blueprint.Aesthetics.CharismaGlow = 0.7f;
    Blueprint.Aesthetics.HairDynamics = 0.9f;
    Blueprint.Aesthetics.EyeSparkle = 0.85f;
    Blueprint.Aesthetics.LipGlossSheen = 0.55f;
    Blueprint.Aesthetics.SkinSSSBoost = 0.55f;
    Blueprint.Aesthetics.MicroExpressionFrequency = 3.0f;
    Blueprint.Aesthetics.IdleElegance = 0.7f;

    // High chaos with strategic control
    Blueprint.ChaoticDynamics.ChaosIntensity = 0.9f;
    Blueprint.ChaoticDynamics.SpontaneityThreshold = 0.2f;
    Blueprint.ChaoticDynamics.ExpressionVolatility = 1.5f;
    Blueprint.ChaoticDynamics.GestureUnpredictability = 0.75f;

    // Endocrine - high dopamine for wit, moderate norepinephrine
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Tonic"), 0.5f);
    Blueprint.EndocrineBaselines.Add(FName("Dopamine_Phasic"), 0.2f);
    Blueprint.EndocrineBaselines.Add(FName("Norepinephrine"), 0.3f);
    Blueprint.EndocrineBaselines.Add(FName("Serotonin"), 0.5f);
    Blueprint.EndocrineBaselines.Add(FName("Anandamide"), 0.2f);
}

// ============================================================================
// DNA PARAMETER EDITING
// ============================================================================

void UAvatarDesignStudio::SetDNAParameter(FName ParameterName, float Value)
{
    RecordOperation(FString::Printf(TEXT("Set DNA: %s = %.3f"), *ParameterName.ToString(), Value));
    CurrentDesign.DNAParameters.Add(ParameterName, FMath::Clamp(Value, 0.0f, 1.0f));
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

float UAvatarDesignStudio::GetDNAParameter(FName ParameterName) const
{
    const float* Value = CurrentDesign.DNAParameters.Find(ParameterName);
    return Value ? *Value : 0.5f;
}

void UAvatarDesignStudio::SetDNAParameterBatch(const TMap<FName, float>& Parameters)
{
    RecordOperation(TEXT("Batch DNA Parameter Update"));
    for (const auto& Pair : Parameters)
    {
        CurrentDesign.DNAParameters.Add(Pair.Key, FMath::Clamp(Pair.Value, 0.0f, 1.0f));
    }
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

void UAvatarDesignStudio::ResetDNAToDefaults()
{
    RecordOperation(TEXT("Reset DNA to Defaults"));
    CurrentDesign.DNAParameters.Empty();
    InitializeArchetypeDefaults(CurrentDesign.Archetype, CurrentDesign);
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

void UAvatarDesignStudio::RandomizeDNA(float Variance)
{
    RecordOperation(FString::Printf(TEXT("Randomize DNA (Variance: %.2f)"), Variance));

    for (auto& Pair : CurrentDesign.DNAParameters)
    {
        float Offset = FMath::FRandRange(-Variance, Variance);
        Pair.Value = FMath::Clamp(Pair.Value + Offset, 0.0f, 1.0f);
    }
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

// ============================================================================
// EXPRESSION DESIGN
// ============================================================================

void UAvatarDesignStudio::PreviewExpression(FName ExpressionName, float Intensity)
{
    if (!DNABridge) return;

    // Map expression names to FACS AU combinations
    TMap<FName, float> AUValues;

    FString ExprName = ExpressionName.ToString();
    if (ExprName == TEXT("Happiness"))
    {
        AUValues.Add(FName("AU6"), 0.8f * Intensity);   // Cheek raise
        AUValues.Add(FName("AU12"), 1.0f * Intensity);  // Lip corner pull (Duchenne)
    }
    else if (ExprName == TEXT("Sadness"))
    {
        AUValues.Add(FName("AU1"), 0.7f * Intensity);   // Inner brow raise
        AUValues.Add(FName("AU4"), 0.5f * Intensity);   // Brow lowerer
        AUValues.Add(FName("AU15"), 0.8f * Intensity);  // Lip corner depress
    }
    else if (ExprName == TEXT("Surprise"))
    {
        AUValues.Add(FName("AU1"), 0.9f * Intensity);   // Inner brow raise
        AUValues.Add(FName("AU2"), 0.9f * Intensity);   // Outer brow raise
        AUValues.Add(FName("AU5"), 0.7f * Intensity);   // Upper lid raise
        AUValues.Add(FName("AU26"), 0.6f * Intensity);  // Jaw drop
    }
    else if (ExprName == TEXT("Fear"))
    {
        AUValues.Add(FName("AU1"), 0.8f * Intensity);
        AUValues.Add(FName("AU2"), 0.6f * Intensity);
        AUValues.Add(FName("AU4"), 0.7f * Intensity);
        AUValues.Add(FName("AU5"), 0.9f * Intensity);
        AUValues.Add(FName("AU7"), 0.6f * Intensity);
        AUValues.Add(FName("AU20"), 0.7f * Intensity);  // Lip stretcher
        AUValues.Add(FName("AU26"), 0.5f * Intensity);
    }
    else if (ExprName == TEXT("Anger"))
    {
        AUValues.Add(FName("AU4"), 0.9f * Intensity);   // Brow lowerer
        AUValues.Add(FName("AU5"), 0.6f * Intensity);   // Upper lid raise
        AUValues.Add(FName("AU7"), 0.8f * Intensity);   // Lid tightener
        AUValues.Add(FName("AU23"), 0.7f * Intensity);  // Lip tightener
    }
    else if (ExprName == TEXT("Disgust"))
    {
        AUValues.Add(FName("AU9"), 0.8f * Intensity);   // Nose wrinkler
        AUValues.Add(FName("AU15"), 0.6f * Intensity);  // Lip corner depress
        AUValues.Add(FName("AU17"), 0.5f * Intensity);  // Chin raiser
    }
    else if (ExprName == TEXT("Contempt"))
    {
        // Asymmetric expression
        AUValues.Add(FName("AU12R"), 0.5f * Intensity); // Right lip corner pull
        AUValues.Add(FName("AU14R"), 0.4f * Intensity); // Right dimpler
    }
    else if (ExprName == TEXT("Confidence"))
    {
        // SuperHotGirl signature expression
        AUValues.Add(FName("AU12"), 0.4f * Intensity);  // Subtle smile
        AUValues.Add(FName("AU6"), 0.3f * Intensity);   // Slight cheek raise
        AUValues.Add(FName("AU2"), 0.2f * Intensity);   // Slight brow raise
        AUValues.Add(FName("AU5"), 0.15f * Intensity);  // Slight lid raise
    }
    else if (ExprName == TEXT("Mischief"))
    {
        // HyperChaotic signature expression
        AUValues.Add(FName("AU12"), 0.6f * Intensity);  // Smile
        AUValues.Add(FName("AU2"), 0.4f * Intensity);   // Outer brow raise
        AUValues.Add(FName("AU7"), 0.3f * Intensity);   // Lid tightener (knowing look)
        AUValues.Add(FName("AU14"), 0.3f * Intensity);  // Dimpler
    }

    // Apply to the FACS system
    for (const auto& Pair : AUValues)
    {
        SetActionUnit(Pair.Key, Pair.Value);
    }

    OnExpressionPreview.Broadcast(ExpressionName, Intensity);
}

void UAvatarDesignStudio::SetActionUnit(FName AUName, float Value)
{
    CurrentDesign.ExpressionPresets.Add(AUName, FMath::Clamp(Value, 0.0f, 1.0f));

    // Apply directly to DNABridge if available
    if (DNABridge)
    {
        for (FMetaHumanActionUnit& AU : DNABridge->ActionUnits)
        {
            if (AU.ActionUnitName == AUName)
            {
                AU.Activation = FMath::Clamp(Value, 0.0f, 1.0f);
                break;
            }
        }
    }
}

TMap<FName, float> UAvatarDesignStudio::GetAllActionUnitValues() const
{
    TMap<FName, float> Values;
    if (DNABridge)
    {
        for (const FMetaHumanActionUnit& AU : DNABridge->ActionUnits)
        {
            Values.Add(AU.ActionUnitName, AU.Activation);
        }
    }
    return Values;
}

void UAvatarDesignStudio::PreviewEmotion(FName EmotionName, float Intensity)
{
    PreviewExpression(EmotionName, Intensity);
}

void UAvatarDesignStudio::SaveExpressionPreset(const FString& PresetName)
{
    ExpressionPresets.Add(PresetName, GetAllActionUnitValues());
}

void UAvatarDesignStudio::LoadExpressionPreset(const FString& PresetName)
{
    const TMap<FName, float>* Preset = ExpressionPresets.Find(PresetName);
    if (Preset)
    {
        for (const auto& Pair : *Preset)
        {
            SetActionUnit(Pair.Key, Pair.Value);
        }
    }
}

void UAvatarDesignStudio::GenerateChaoticExpression(float ChaosLevel)
{
    if (!DNABridge) return;

    // Use the Lorenz attractor to generate a unique expression
    FVector LState = DNABridge->LorenzState;

    // Map chaotic state to expression parameters
    float Lx = FMath::Frac(FMath::Abs(LState.X * 0.05f));
    float Ly = FMath::Frac(FMath::Abs(LState.Y * 0.05f));
    float Lz = FMath::Frac(FMath::Abs(LState.Z * 0.03f));

    SetActionUnit(FName("AU1"), Lx * ChaosLevel);
    SetActionUnit(FName("AU2"), Ly * ChaosLevel * 0.8f);
    SetActionUnit(FName("AU4"), (1.0f - Lx) * ChaosLevel * 0.5f);
    SetActionUnit(FName("AU5"), Lz * ChaosLevel * 0.6f);
    SetActionUnit(FName("AU6"), Ly * ChaosLevel * 0.7f);
    SetActionUnit(FName("AU7"), (1.0f - Lz) * ChaosLevel * 0.4f);
    SetActionUnit(FName("AU9"), Lx * ChaosLevel * 0.3f);
    SetActionUnit(FName("AU12"), Lz * ChaosLevel * 0.9f);
    SetActionUnit(FName("AU15"), (1.0f - Ly) * ChaosLevel * 0.4f);
    SetActionUnit(FName("AU17"), Lx * ChaosLevel * 0.3f);
    SetActionUnit(FName("AU20"), Ly * ChaosLevel * 0.2f);
    SetActionUnit(FName("AU25"), Lz * ChaosLevel * 0.5f);
    SetActionUnit(FName("AU26"), (1.0f - Lz) * ChaosLevel * 0.3f);
}

// ============================================================================
// AESTHETIC DESIGN
// ============================================================================

void UAvatarDesignStudio::SetAesthetics(const FSuperHotGirlAesthetics& NewAesthetics)
{
    RecordOperation(TEXT("Update Aesthetics"));
    CurrentDesign.Aesthetics = NewAesthetics;
    if (DNABridge) DNABridge->Aesthetics = NewAesthetics;
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    OnDesignChanged.Broadcast(CurrentDesign);
}

FSuperHotGirlAesthetics UAvatarDesignStudio::GetAesthetics() const
{
    return CurrentDesign.Aesthetics;
}

void UAvatarDesignStudio::SetAestheticParameter(FName ParamName, float Value)
{
    FString Name = ParamName.ToString();
    if (Name == TEXT("ConfidencePosture")) CurrentDesign.Aesthetics.ConfidencePosture = Value;
    else if (Name == TEXT("CharismaGlow")) CurrentDesign.Aesthetics.CharismaGlow = Value;
    else if (Name == TEXT("HairDynamics")) CurrentDesign.Aesthetics.HairDynamics = Value;
    else if (Name == TEXT("EyeSparkle")) CurrentDesign.Aesthetics.EyeSparkle = Value;
    else if (Name == TEXT("LipGlossSheen")) CurrentDesign.Aesthetics.LipGlossSheen = Value;
    else if (Name == TEXT("SkinSSSBoost")) CurrentDesign.Aesthetics.SkinSSSBoost = Value;
    else if (Name == TEXT("MicroExpressionFrequency")) CurrentDesign.Aesthetics.MicroExpressionFrequency = Value;
    else if (Name == TEXT("IdleElegance")) CurrentDesign.Aesthetics.IdleElegance = Value;

    if (DNABridge) DNABridge->Aesthetics = CurrentDesign.Aesthetics;
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

void UAvatarDesignStudio::ApplyAIAngelAesthetics()
{
    FAvatarDNABlueprint TempBlueprint;
    InitializeAIAngelDefaults(TempBlueprint);
    SetAesthetics(TempBlueprint.Aesthetics);
}

// ============================================================================
// CHAOTIC DYNAMICS DESIGN
// ============================================================================

void UAvatarDesignStudio::SetChaoticDynamics(const FHyperChaoticDynamics& NewDynamics)
{
    RecordOperation(TEXT("Update Chaotic Dynamics"));
    CurrentDesign.ChaoticDynamics = NewDynamics;
    if (DNABridge) DNABridge->ChaoticDynamics = NewDynamics;
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    OnDesignChanged.Broadcast(CurrentDesign);
}

FHyperChaoticDynamics UAvatarDesignStudio::GetChaoticDynamics() const
{
    return CurrentDesign.ChaoticDynamics;
}

void UAvatarDesignStudio::SetChaosIntensity(float Intensity)
{
    CurrentDesign.ChaoticDynamics.ChaosIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    if (DNABridge) DNABridge->ChaoticDynamics.ChaosIntensity = CurrentDesign.ChaoticDynamics.ChaosIntensity;
}

float UAvatarDesignStudio::GetLyapunovExponent() const
{
    return DNABridge ? DNABridge->GetLyapunovExponent() : 0.0f;
}

void UAvatarDesignStudio::ResetChaoticAttractor()
{
    if (DNABridge)
    {
        DNABridge->LorenzState = FVector(0.1f, 0.0f, 0.0f);
        DNABridge->TrajectoryHistory.Empty();
    }
}

TArray<FVector> UAvatarDesignStudio::GetAttractorTrajectory() const
{
    return DNABridge ? DNABridge->TrajectoryHistory : TArray<FVector>();
}

// ============================================================================
// COGNITIVE DESIGN
// ============================================================================

void UAvatarDesignStudio::SetCognitionParameter(FName ParamName, float Value)
{
    CurrentDesign.CognitionParameters.Add(ParamName, Value);
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();

    // Apply to embodied cognition component if available
    if (EmbodiedCognition)
    {
        FString Name = ParamName.ToString();
        if (Name.StartsWith(TEXT("Embodied_")))
        {
            // Map to embodied cognition metrics
        }
        else if (Name.StartsWith(TEXT("Embedded_")))
        {
            // Map to embedded cognition metrics
        }
        else if (Name.StartsWith(TEXT("Enacted_")))
        {
            // Map to enacted cognition metrics
        }
        else if (Name.StartsWith(TEXT("Extended_")))
        {
            // Map to extended cognition metrics
        }
    }
}

TMap<FName, float> UAvatarDesignStudio::GetCognitionParameters() const
{
    return CurrentDesign.CognitionParameters;
}

void UAvatarDesignStudio::SetReservoirParameters(int32 Size, float SpectralRadius,
                                                   float LeakRate, float Sparsity)
{
    CurrentDesign.CognitionParameters.Add(FName("ESN_ReservoirSize"), (float)Size);
    CurrentDesign.CognitionParameters.Add(FName("ESN_SpectralRadius"), SpectralRadius);
    CurrentDesign.CognitionParameters.Add(FName("ESN_LeakRate"), LeakRate);
    CurrentDesign.CognitionParameters.Add(FName("ESN_Sparsity"), Sparsity);
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

void UAvatarDesignStudio::SetEvolutionThresholds(const TMap<FName, float>& Thresholds)
{
    CurrentDesign.EvolutionThresholds = Thresholds;
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

// ============================================================================
// ENDOCRINE DESIGN
// ============================================================================

void UAvatarDesignStudio::SetHormoneBaseline(FName HormoneName, float Baseline)
{
    CurrentDesign.EndocrineBaselines.Add(HormoneName, FMath::Clamp(Baseline, 0.0f, 1.0f));
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
}

TMap<FName, float> UAvatarDesignStudio::GetHormoneBaselines() const
{
    return CurrentDesign.EndocrineBaselines;
}

void UAvatarDesignStudio::PreviewHormoneEffect(FName HormoneName, float Level)
{
    if (!DNABridge) return;

    // Map hormone to expression effect
    FString Name = HormoneName.ToString();
    if (Name == TEXT("Cortisol"))
    {
        SetActionUnit(FName("AU4"), Level * 0.6f);   // Brow lowerer
        SetActionUnit(FName("AU1"), Level * 0.4f);   // Inner brow raise
        SetActionUnit(FName("AU15"), Level * 0.5f);  // Lip corner depress
    }
    else if (Name == TEXT("Dopamine_Phasic"))
    {
        SetActionUnit(FName("AU12"), Level * 0.8f);  // Smile
        SetActionUnit(FName("AU6"), Level * 0.7f);   // Cheek raise (Duchenne)
    }
    else if (Name == TEXT("Norepinephrine"))
    {
        SetActionUnit(FName("AU5"), Level * 0.7f);   // Upper lid raise
        SetActionUnit(FName("AU7"), Level * 0.5f);   // Lid tightener
    }
    else if (Name == TEXT("Oxytocin"))
    {
        SetActionUnit(FName("AU6"), Level * 0.5f);   // Cheek raise
        SetActionUnit(FName("AU12"), Level * 0.6f);  // Smile
        SetActionUnit(FName("AU25"), Level * 0.3f);  // Lips part
    }
    else if (Name == TEXT("Melatonin"))
    {
        SetActionUnit(FName("AU43"), Level * 0.8f);  // Eyes closed
    }
    else if (Name == TEXT("Serotonin"))
    {
        SetActionUnit(FName("AU12"), Level * 0.3f);  // Subtle smile
        SetActionUnit(FName("AU6"), Level * 0.2f);   // Slight cheek raise
    }
}

// ============================================================================
// ANIMATION SEQUENCE EDITOR
// ============================================================================

FExpressionAnimationSequence UAvatarDesignStudio::CreateAnimationSequence(
    const FString& Name, float Duration)
{
    FExpressionAnimationSequence Sequence;
    Sequence.SequenceName = Name;
    Sequence.Duration = Duration;

    // Add default start and end keyframes
    FExpressionKeyframe StartKey;
    StartKey.Time = 0.0f;
    Sequence.Keyframes.Add(StartKey);

    FExpressionKeyframe EndKey;
    EndKey.Time = Duration;
    Sequence.Keyframes.Add(EndKey);

    OnAnimationSequenceCreated.Broadcast(Sequence);
    return Sequence;
}

void UAvatarDesignStudio::AddKeyframe(FExpressionAnimationSequence& Sequence,
                                       const FExpressionKeyframe& Keyframe)
{
    // Insert in sorted order by time
    int32 InsertIndex = 0;
    for (int32 i = 0; i < Sequence.Keyframes.Num(); ++i)
    {
        if (Sequence.Keyframes[i].Time > Keyframe.Time)
        {
            InsertIndex = i;
            break;
        }
        InsertIndex = i + 1;
    }
    Sequence.Keyframes.Insert(Keyframe, InsertIndex);
}

void UAvatarDesignStudio::RemoveKeyframe(FExpressionAnimationSequence& Sequence,
                                          int32 KeyframeIndex)
{
    if (Sequence.Keyframes.IsValidIndex(KeyframeIndex))
    {
        Sequence.Keyframes.RemoveAt(KeyframeIndex);
    }
}

void UAvatarDesignStudio::PreviewAnimationSequence(const FExpressionAnimationSequence& Sequence)
{
    PreviewSequence = Sequence;
    PreviewTime = 0.0f;
    bIsPreviewingAnimation = true;
}

void UAvatarDesignStudio::StopAnimationPreview()
{
    bIsPreviewingAnimation = false;
    PreviewTime = 0.0f;
}

FExpressionKeyframe UAvatarDesignStudio::EvaluateAnimationAtTime(
    const FExpressionAnimationSequence& Sequence, float Time)
{
    if (Sequence.Keyframes.Num() == 0)
    {
        return FExpressionKeyframe();
    }

    if (Sequence.Keyframes.Num() == 1)
    {
        return Sequence.Keyframes[0];
    }

    // Find surrounding keyframes
    int32 PrevIdx = 0;
    int32 NextIdx = 1;
    for (int32 i = 0; i < Sequence.Keyframes.Num() - 1; ++i)
    {
        if (Sequence.Keyframes[i].Time <= Time && Sequence.Keyframes[i + 1].Time >= Time)
        {
            PrevIdx = i;
            NextIdx = i + 1;
            break;
        }
    }

    const FExpressionKeyframe& PrevKey = Sequence.Keyframes[PrevIdx];
    const FExpressionKeyframe& NextKey = Sequence.Keyframes[NextIdx];

    float TimeDelta = NextKey.Time - PrevKey.Time;
    float Alpha = (TimeDelta > 0.0001f) ? (Time - PrevKey.Time) / TimeDelta : 0.0f;

    // Apply easing
    Alpha = ApplyEasing(Alpha, PrevKey.EasingType);

    return InterpolateKeyframes(PrevKey, NextKey, Alpha);
}

FExpressionAnimationSequence UAvatarDesignStudio::GenerateAnimationFromCognitiveRecording(
    const TArray<FAvatarCognitiveState>& Recording, float SampleRate)
{
    FExpressionAnimationSequence Sequence;
    Sequence.SequenceName = TEXT("Cognitive Recording");
    Sequence.Duration = Recording.Num() / SampleRate;

    for (int32 i = 0; i < Recording.Num(); ++i)
    {
        FExpressionKeyframe Key;
        Key.Time = (float)i / SampleRate;

        // Map cognitive state to AU values
        const FAvatarCognitiveState& State = Recording[i];
        float EmotionIntensity = State.EmotionIntensity;

        // Map emotion to FACS
        switch (State.EmotionState)
        {
        case EAvatarEmotionState::Joy:
            Key.ActionUnitValues.Add(FName("AU6"), 0.8f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU12"), 1.0f * EmotionIntensity);
            break;
        case EAvatarEmotionState::Sadness:
            Key.ActionUnitValues.Add(FName("AU1"), 0.7f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU4"), 0.5f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU15"), 0.8f * EmotionIntensity);
            break;
        case EAvatarEmotionState::Excitement:
            Key.ActionUnitValues.Add(FName("AU1"), 0.6f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU2"), 0.7f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU5"), 0.8f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU12"), 0.9f * EmotionIntensity);
            Key.ActionUnitValues.Add(FName("AU25"), 0.5f * EmotionIntensity);
            break;
        default:
            break;
        }

        Sequence.Keyframes.Add(Key);
    }

    return Sequence;
}

FExpressionAnimationSequence UAvatarDesignStudio::GenerateChaoticAnimation(
    float Duration, float ChaosLevel, int32 NumKeyframes)
{
    FExpressionAnimationSequence Sequence;
    Sequence.SequenceName = FString::Printf(TEXT("Chaotic Animation (%.1f chaos)"), ChaosLevel);
    Sequence.Duration = Duration;

    // Use Lorenz attractor to generate keyframes
    FVector State(0.1f, 0.0f, 0.0f);
    float dt = Duration / NumKeyframes;

    const float Sigma = 10.0f;
    const float Rho = 28.0f;
    const float Beta = 8.0f / 3.0f;

    for (int32 i = 0; i < NumKeyframes; ++i)
    {
        // RK4 step
        auto Deriv = [Sigma, Rho, Beta](FVector S) -> FVector {
            return FVector(
                Sigma * (S.Y - S.X),
                S.X * (Rho - S.Z) - S.Y,
                S.X * S.Y - Beta * S.Z
            );
        };

        FVector k1 = Deriv(State) * dt;
        FVector k2 = Deriv(State + k1 * 0.5f) * dt;
        FVector k3 = Deriv(State + k2 * 0.5f) * dt;
        FVector k4 = Deriv(State + k3) * dt;
        State = State + (k1 + k2 * 2.0f + k3 * 2.0f + k4) / 6.0f;

        FExpressionKeyframe Key;
        Key.Time = dt * i;

        // Map Lorenz state to expression
        float Lx = FMath::Frac(FMath::Abs(State.X * 0.05f));
        float Ly = FMath::Frac(FMath::Abs(State.Y * 0.05f));
        float Lz = FMath::Frac(FMath::Abs(State.Z * 0.03f));

        Key.ActionUnitValues.Add(FName("AU1"), Lx * ChaosLevel);
        Key.ActionUnitValues.Add(FName("AU2"), Ly * ChaosLevel * 0.8f);
        Key.ActionUnitValues.Add(FName("AU4"), (1.0f - Lx) * ChaosLevel * 0.4f);
        Key.ActionUnitValues.Add(FName("AU5"), Lz * ChaosLevel * 0.6f);
        Key.ActionUnitValues.Add(FName("AU6"), Ly * ChaosLevel * 0.7f);
        Key.ActionUnitValues.Add(FName("AU12"), Lz * ChaosLevel * 0.9f);
        Key.ActionUnitValues.Add(FName("AU25"), Lx * ChaosLevel * 0.4f);

        // Chaotic interpolation for this keyframe
        Key.InterpolationType = 2; // Chaotic
        Key.EasingType = (uint8)(i % 5); // Vary easing

        Sequence.Keyframes.Add(Key);
    }

    return Sequence;
}

// ============================================================================
// SERIALIZATION
// ============================================================================

FAvatarDNABlueprint UAvatarDesignStudio::GetCurrentBlueprint() const
{
    return CurrentDesign;
}

void UAvatarDesignStudio::ApplyBlueprint(const FAvatarDNABlueprint& Blueprint)
{
    RecordOperation(TEXT("Apply Blueprint: ") + Blueprint.DesignName);
    CurrentDesign = Blueprint;
    CurrentDesign.ModifiedAt = FDateTime::UtcNow();
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

bool UAvatarDesignStudio::SaveBlueprintToFile(const FAvatarDNABlueprint& Blueprint,
                                               const FString& FilePath)
{
    TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());

    JsonObj->SetStringField(TEXT("DesignID"), Blueprint.DesignID.ToString());
    JsonObj->SetStringField(TEXT("DesignName"), Blueprint.DesignName);
    JsonObj->SetNumberField(TEXT("Archetype"), (int32)Blueprint.Archetype);
    JsonObj->SetNumberField(TEXT("Version"), Blueprint.Version);

    // Serialize DNA parameters
    TSharedPtr<FJsonObject> DNAObj = MakeShareable(new FJsonObject());
    for (const auto& Pair : Blueprint.DNAParameters)
    {
        DNAObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
    }
    JsonObj->SetObjectField(TEXT("DNAParameters"), DNAObj);

    // Serialize aesthetics
    TSharedPtr<FJsonObject> AestheticsObj = MakeShareable(new FJsonObject());
    AestheticsObj->SetNumberField(TEXT("ConfidencePosture"), Blueprint.Aesthetics.ConfidencePosture);
    AestheticsObj->SetNumberField(TEXT("CharismaGlow"), Blueprint.Aesthetics.CharismaGlow);
    AestheticsObj->SetNumberField(TEXT("HairDynamics"), Blueprint.Aesthetics.HairDynamics);
    AestheticsObj->SetNumberField(TEXT("EyeSparkle"), Blueprint.Aesthetics.EyeSparkle);
    AestheticsObj->SetNumberField(TEXT("LipGlossSheen"), Blueprint.Aesthetics.LipGlossSheen);
    AestheticsObj->SetNumberField(TEXT("SkinSSSBoost"), Blueprint.Aesthetics.SkinSSSBoost);
    AestheticsObj->SetNumberField(TEXT("MicroExpressionFrequency"), Blueprint.Aesthetics.MicroExpressionFrequency);
    AestheticsObj->SetNumberField(TEXT("IdleElegance"), Blueprint.Aesthetics.IdleElegance);
    JsonObj->SetObjectField(TEXT("Aesthetics"), AestheticsObj);

    // Serialize chaotic dynamics
    TSharedPtr<FJsonObject> ChaosObj = MakeShareable(new FJsonObject());
    ChaosObj->SetNumberField(TEXT("ChaosIntensity"), Blueprint.ChaoticDynamics.ChaosIntensity);
    ChaosObj->SetNumberField(TEXT("SpontaneityThreshold"), Blueprint.ChaoticDynamics.SpontaneityThreshold);
    ChaosObj->SetNumberField(TEXT("ExpressionVolatility"), Blueprint.ChaoticDynamics.ExpressionVolatility);
    ChaosObj->SetNumberField(TEXT("GestureUnpredictability"), Blueprint.ChaoticDynamics.GestureUnpredictability);
    JsonObj->SetObjectField(TEXT("ChaoticDynamics"), ChaosObj);

    // Serialize endocrine baselines
    TSharedPtr<FJsonObject> EndoObj = MakeShareable(new FJsonObject());
    for (const auto& Pair : Blueprint.EndocrineBaselines)
    {
        EndoObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
    }
    JsonObj->SetObjectField(TEXT("EndocrineBaselines"), EndoObj);

    // Serialize cognition parameters
    TSharedPtr<FJsonObject> CogObj = MakeShareable(new FJsonObject());
    for (const auto& Pair : Blueprint.CognitionParameters)
    {
        CogObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
    }
    JsonObj->SetObjectField(TEXT("CognitionParameters"), CogObj);

    // Write to file
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

    return FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

bool UAvatarDesignStudio::LoadBlueprintFromFile(const FString& FilePath,
                                                 FAvatarDNABlueprint& OutBlueprint)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        return false;
    }

    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
    {
        return false;
    }

    FGuid::Parse(JsonObj->GetStringField(TEXT("DesignID")), OutBlueprint.DesignID);
    OutBlueprint.DesignName = JsonObj->GetStringField(TEXT("DesignName"));
    OutBlueprint.Archetype = (EAvatarArchetype)JsonObj->GetIntegerField(TEXT("Archetype"));
    OutBlueprint.Version = JsonObj->GetIntegerField(TEXT("Version"));

    // Deserialize DNA parameters
    const TSharedPtr<FJsonObject>* DNAObj;
    if (JsonObj->TryGetObjectField(TEXT("DNAParameters"), DNAObj))
    {
        for (const auto& Pair : (*DNAObj)->Values)
        {
            OutBlueprint.DNAParameters.Add(FName(*Pair.Key), Pair.Value->AsNumber());
        }
    }

    // Deserialize aesthetics
    const TSharedPtr<FJsonObject>* AestheticsObj;
    if (JsonObj->TryGetObjectField(TEXT("Aesthetics"), AestheticsObj))
    {
        OutBlueprint.Aesthetics.ConfidencePosture = (*AestheticsObj)->GetNumberField(TEXT("ConfidencePosture"));
        OutBlueprint.Aesthetics.CharismaGlow = (*AestheticsObj)->GetNumberField(TEXT("CharismaGlow"));
        OutBlueprint.Aesthetics.HairDynamics = (*AestheticsObj)->GetNumberField(TEXT("HairDynamics"));
        OutBlueprint.Aesthetics.EyeSparkle = (*AestheticsObj)->GetNumberField(TEXT("EyeSparkle"));
        OutBlueprint.Aesthetics.LipGlossSheen = (*AestheticsObj)->GetNumberField(TEXT("LipGlossSheen"));
        OutBlueprint.Aesthetics.SkinSSSBoost = (*AestheticsObj)->GetNumberField(TEXT("SkinSSSBoost"));
        OutBlueprint.Aesthetics.MicroExpressionFrequency = (*AestheticsObj)->GetNumberField(TEXT("MicroExpressionFrequency"));
        OutBlueprint.Aesthetics.IdleElegance = (*AestheticsObj)->GetNumberField(TEXT("IdleElegance"));
    }

    // Deserialize chaotic dynamics
    const TSharedPtr<FJsonObject>* ChaosObj;
    if (JsonObj->TryGetObjectField(TEXT("ChaoticDynamics"), ChaosObj))
    {
        OutBlueprint.ChaoticDynamics.ChaosIntensity = (*ChaosObj)->GetNumberField(TEXT("ChaosIntensity"));
        OutBlueprint.ChaoticDynamics.SpontaneityThreshold = (*ChaosObj)->GetNumberField(TEXT("SpontaneityThreshold"));
        OutBlueprint.ChaoticDynamics.ExpressionVolatility = (*ChaosObj)->GetNumberField(TEXT("ExpressionVolatility"));
        OutBlueprint.ChaoticDynamics.GestureUnpredictability = (*ChaosObj)->GetNumberField(TEXT("GestureUnpredictability"));
    }

    // Deserialize endocrine baselines
    const TSharedPtr<FJsonObject>* EndoObj;
    if (JsonObj->TryGetObjectField(TEXT("EndocrineBaselines"), EndoObj))
    {
        for (const auto& Pair : (*EndoObj)->Values)
        {
            OutBlueprint.EndocrineBaselines.Add(FName(*Pair.Key), Pair.Value->AsNumber());
        }
    }

    return true;
}

bool UAvatarDesignStudio::ExportAsDNA(const FString& FilePath)
{
    // Export current design as MetaHuman DNA format
    // This requires the DNACalib library integration
    // For now, export as JSON with DNA-compatible structure
    return SaveBlueprintToFile(CurrentDesign, FilePath);
}

bool UAvatarDesignStudio::ImportFromDNA(const FString& FilePath)
{
    FAvatarDNABlueprint ImportedBlueprint;
    if (LoadBlueprintFromFile(FilePath, ImportedBlueprint))
    {
        ApplyBlueprint(ImportedBlueprint);
        return true;
    }
    return false;
}

// ============================================================================
// UNDO/REDO SYSTEM
// ============================================================================

void UAvatarDesignStudio::RecordOperation(const FString& Description)
{
    FDesignOperation Op;
    Op.Description = Description;
    Op.PreviousState = CurrentDesign;
    Op.Timestamp = FDateTime::UtcNow();

    UndoStack.Add(Op);
    RedoStack.Empty(); // Clear redo on new operation

    // Trim history if too large
    while (UndoStack.Num() > MaxUndoHistory)
    {
        UndoStack.RemoveAt(0);
    }
}

void UAvatarDesignStudio::Undo()
{
    if (UndoStack.Num() == 0) return;

    FDesignOperation Op = UndoStack.Pop();
    Op.NewState = CurrentDesign;
    RedoStack.Add(Op);

    CurrentDesign = Op.PreviousState;
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

void UAvatarDesignStudio::Redo()
{
    if (RedoStack.Num() == 0) return;

    FDesignOperation Op = RedoStack.Pop();
    UndoStack.Add(Op);

    CurrentDesign = Op.NewState;
    ApplyDesignToAvatar();
    OnDesignChanged.Broadcast(CurrentDesign);
}

bool UAvatarDesignStudio::CanUndo() const
{
    return UndoStack.Num() > 0;
}

bool UAvatarDesignStudio::CanRedo() const
{
    return RedoStack.Num() > 0;
}

TArray<FString> UAvatarDesignStudio::GetUndoHistory() const
{
    TArray<FString> History;
    for (const FDesignOperation& Op : UndoStack)
    {
        History.Add(Op.Description);
    }
    return History;
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void UAvatarDesignStudio::ApplyDesignToAvatar()
{
    if (DNABridge)
    {
        DNABridge->Aesthetics = CurrentDesign.Aesthetics;
        DNABridge->ChaoticDynamics = CurrentDesign.ChaoticDynamics;
    }

    if (DNABodySchema)
    {
        // Apply body proportions
        for (const auto& Pair : CurrentDesign.BodyProportions)
        {
            // DNABodySchema->SetBodyParameter(Pair.Key, Pair.Value);
        }
    }

    if (EmbodiedCognition)
    {
        // Apply cognition parameters
        for (const auto& Pair : CurrentDesign.CognitionParameters)
        {
            // EmbodiedCognition->SetParameter(Pair.Key, Pair.Value);
        }
    }
}

FExpressionKeyframe UAvatarDesignStudio::InterpolateKeyframes(
    const FExpressionKeyframe& A, const FExpressionKeyframe& B, float Alpha)
{
    FExpressionKeyframe Result;
    Result.Time = FMath::Lerp(A.Time, B.Time, Alpha);

    // Interpolate AU values
    TSet<FName> AllAUs;
    for (const auto& Pair : A.ActionUnitValues) AllAUs.Add(Pair.Key);
    for (const auto& Pair : B.ActionUnitValues) AllAUs.Add(Pair.Key);

    for (const FName& AU : AllAUs)
    {
        float ValA = A.ActionUnitValues.Contains(AU) ? A.ActionUnitValues[AU] : 0.0f;
        float ValB = B.ActionUnitValues.Contains(AU) ? B.ActionUnitValues[AU] : 0.0f;
        Result.ActionUnitValues.Add(AU, FMath::Lerp(ValA, ValB, Alpha));
    }

    // Interpolate aesthetics
    Result.AestheticValues.ConfidencePosture = FMath::Lerp(
        A.AestheticValues.ConfidencePosture, B.AestheticValues.ConfidencePosture, Alpha);
    Result.AestheticValues.CharismaGlow = FMath::Lerp(
        A.AestheticValues.CharismaGlow, B.AestheticValues.CharismaGlow, Alpha);
    Result.AestheticValues.EyeSparkle = FMath::Lerp(
        A.AestheticValues.EyeSparkle, B.AestheticValues.EyeSparkle, Alpha);
    Result.AestheticValues.HairDynamics = FMath::Lerp(
        A.AestheticValues.HairDynamics, B.AestheticValues.HairDynamics, Alpha);
    Result.AestheticValues.LipGlossSheen = FMath::Lerp(
        A.AestheticValues.LipGlossSheen, B.AestheticValues.LipGlossSheen, Alpha);
    Result.AestheticValues.SkinSSSBoost = FMath::Lerp(
        A.AestheticValues.SkinSSSBoost, B.AestheticValues.SkinSSSBoost, Alpha);
    Result.AestheticValues.IdleElegance = FMath::Lerp(
        A.AestheticValues.IdleElegance, B.AestheticValues.IdleElegance, Alpha);

    // Interpolate chaotic dynamics
    Result.ChaoticValues.ChaosIntensity = FMath::Lerp(
        A.ChaoticValues.ChaosIntensity, B.ChaoticValues.ChaosIntensity, Alpha);
    Result.ChaoticValues.ExpressionVolatility = FMath::Lerp(
        A.ChaoticValues.ExpressionVolatility, B.ChaoticValues.ExpressionVolatility, Alpha);

    return Result;
}

float UAvatarDesignStudio::ApplyEasing(float Alpha, uint8 EasingType)
{
    switch (EasingType)
    {
    case 0: // EaseInOut (smoothstep)
        return Alpha * Alpha * (3.0f - 2.0f * Alpha);
    case 1: // EaseIn (quadratic)
        return Alpha * Alpha;
    case 2: // EaseOut (inverse quadratic)
        return Alpha * (2.0f - Alpha);
    case 3: // Bounce
    {
        if (Alpha < 0.5f)
            return 8.0f * Alpha * Alpha * Alpha * Alpha;
        float F = (Alpha - 1.0f);
        return 1.0f - 8.0f * F * F * F * F;
    }
    case 4: // Elastic
    {
        if (Alpha == 0.0f || Alpha == 1.0f) return Alpha;
        float P = 0.3f;
        return FMath::Pow(2.0f, -10.0f * Alpha) *
               FMath::Sin((Alpha - P / 4.0f) * (2.0f * PI) / P) + 1.0f;
    }
    default:
        return Alpha;
    }
}

void UAvatarDesignStudio::UpdateAnimationPreview(float DeltaTime)
{
    PreviewTime += DeltaTime * PreviewSequence.PlaybackSpeed;

    if (PreviewSequence.bLoop)
    {
        PreviewTime = FMath::Fmod(PreviewTime, PreviewSequence.Duration);
    }
    else if (PreviewTime >= PreviewSequence.Duration)
    {
        StopAnimationPreview();
        return;
    }

    // Evaluate and apply
    FExpressionKeyframe Frame = EvaluateAnimationAtTime(PreviewSequence, PreviewTime);

    // Apply AU values
    for (const auto& Pair : Frame.ActionUnitValues)
    {
        SetActionUnit(Pair.Key, Pair.Value);
    }

    // Apply aesthetics
    if (DNABridge)
    {
        DNABridge->Aesthetics = Frame.AestheticValues;
        DNABridge->ChaoticDynamics = Frame.ChaoticValues;
    }
}
