// EmbodiedCognitionEnhanced.cpp
// Deep Tree Echo - Enhanced 4E Embodied Cognition System
// Copyright (c) 2025-2026 Deep Tree Echo Project

#include "EmbodiedCognitionEnhanced.h"
#include "../../UnrealEcho/Avatar/AvatarDesignStudio.h"

// ============================================================================
// FSensorimotorSchema
// ============================================================================

void FSensorimotorSchema::UpdateCompetence(float PredictionError)
{
    ErrorHistory.Add(PredictionError);
    if (ErrorHistory.Num() > 100) ErrorHistory.RemoveAt(0);

    // Competence = 1 - average recent error
    float AvgError = 0.0f;
    for (float E : ErrorHistory) AvgError += E;
    AvgError /= FMath::Max(1, ErrorHistory.Num());
    Competence = FMath::Clamp(1.0f - AvgError, 0.0f, 1.0f);
    ActivationCount++;
}

// ============================================================================
// FActiveInferenceState
// ============================================================================

void FActiveInferenceState::UpdateBeliefs(const TArray<float>& NewObservations, float LearningRate)
{
    Observations = NewObservations;

    if (Beliefs.Num() != NewObservations.Num())
    {
        Beliefs.SetNum(NewObservations.Num());
        Predictions.SetNum(NewObservations.Num());
        for (int32 i = 0; i < Beliefs.Num(); ++i)
        {
            Beliefs[i] = 0.5f;
            Predictions[i] = 0.5f;
        }
    }

    // Variational inference: update beliefs to minimize free energy
    for (int32 i = 0; i < Beliefs.Num(); ++i)
    {
        float PredictionError = NewObservations[i] - Predictions[i];
        // Precision-weighted prediction error
        float WeightedError = PredictionError * Precision;
        // Update belief
        Beliefs[i] = FMath::Clamp(Beliefs[i] + LearningRate * WeightedError, 0.0f, 1.0f);
        // Update prediction from belief
        Predictions[i] = Beliefs[i];
    }

    FreeEnergy = ComputeFreeEnergy();
}

float FActiveInferenceState::ComputeFreeEnergy() const
{
    if (Observations.Num() == 0 || Predictions.Num() == 0) return 0.0f;

    float Energy = 0.0f;
    for (int32 i = 0; i < FMath::Min(Observations.Num(), Predictions.Num()); ++i)
    {
        float Error = Observations[i] - Predictions[i];
        // Negative log likelihood (Gaussian)
        Energy += 0.5f * Precision * Error * Error;
    }
    return Energy / FMath::Max(1, Observations.Num());
}

int32 FActiveInferenceState::SelectAction(const TArray<TArray<float>>& ActionConsequences) const
{
    if (ActionConsequences.Num() == 0) return -1;

    int32 BestAction = 0;
    float BestEFE = MAX_FLT;

    for (int32 a = 0; a < ActionConsequences.Num(); ++a)
    {
        // Expected free energy = expected surprise + expected information gain
        float EFE = 0.0f;
        const TArray<float>& Consequence = ActionConsequences[a];
        for (int32 i = 0; i < FMath::Min(Consequence.Num(), Beliefs.Num()); ++i)
        {
            float PredError = Consequence[i] - Beliefs[i];
            EFE += 0.5f * Precision * PredError * PredError;
        }

        if (EFE < BestEFE)
        {
            BestEFE = EFE;
            BestAction = a;
        }
    }

    return BestAction;
}

// ============================================================================
// UEmbodiedCognitionEnhanced
// ============================================================================

UEmbodiedCognitionEnhanced::UEmbodiedCognitionEnhanced()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 Hz
}

void UEmbodiedCognitionEnhanced::BeginPlay()
{
    Super::BeginPlay();

    // Initialize active inference with 16 dimensions (matching hormone channels)
    ActiveInference.Beliefs.SetNum(16);
    ActiveInference.Predictions.SetNum(16);
    ActiveInference.Observations.SetNum(16);
    for (int32 i = 0; i < 16; ++i)
    {
        ActiveInference.Beliefs[i] = 0.5f;
        ActiveInference.Predictions[i] = 0.5f;
    }

    // Register default sensorimotor schemas
    TArray<float> DefaultForward, DefaultInverse;
    DefaultForward.SetNum(16);
    DefaultInverse.SetNum(16);
    for (int32 i = 0; i < 16; ++i)
    {
        DefaultForward[i] = 0.5f;
        DefaultInverse[i] = 0.5f;
    }
    RegisterSchema(TEXT("LookAt"), DefaultForward, DefaultInverse);
    RegisterSchema(TEXT("Smile"), DefaultForward, DefaultInverse);
    RegisterSchema(TEXT("Speak"), DefaultForward, DefaultInverse);
    RegisterSchema(TEXT("Listen"), DefaultForward, DefaultInverse);
    RegisterSchema(TEXT("Think"), DefaultForward, DefaultInverse);

    // Register default cognitive extensions
    RegisterCognitiveExtension(TEXT("HypergraphMemory"), TEXT("memory"), 0.9f);
    RegisterCognitiveExtension(TEXT("ReservoirComputing"), TEXT("computation"), 0.85f);
    RegisterCognitiveExtension(TEXT("LanguageModel"), TEXT("communication"), 0.8f);
    RegisterCognitiveExtension(TEXT("SensoryArray"), TEXT("perception"), 0.95f);

    // Initialize affordance library
    AffordanceLibrary.Add(TEXT("Face"), {});
    AffordanceLibrary.Add(TEXT("Object"), {});
    AffordanceLibrary.Add(TEXT("Text"), {});
}

void UEmbodiedCognitionEnhanced::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Gradual improvement of environmental sensitivity through exposure
    EnvironmentalSensitivity = FMath::Clamp(EnvironmentalSensitivity + 0.00005f, 0.0f, 1.0f);

    // Sync to design studio if connected
    if (ConnectedStudio)
    {
        SyncToDesignStudio();
    }
}

// ========================================================================
// Embodied Cognition
// ========================================================================

void UEmbodiedCognitionEnhanced::UpdateBodySchema(const TArray<FTransform>& JointTransforms)
{
    ActualBodyState = JointTransforms;

    if (InternalBodyModel.Num() != JointTransforms.Num())
    {
        InternalBodyModel = JointTransforms;
        ProprioceptiveAccuracy = 1.0f;
        return;
    }

    // Compute proprioceptive accuracy as 1 - normalized error between model and actual
    float TotalError = 0.0f;
    for (int32 i = 0; i < JointTransforms.Num(); ++i)
    {
        FVector PosDiff = InternalBodyModel[i].GetLocation() - JointTransforms[i].GetLocation();
        TotalError += PosDiff.Size();
    }
    float NormError = TotalError / FMath::Max(1.0f, static_cast<float>(JointTransforms.Num()) * 100.0f);
    ProprioceptiveAccuracy = FMath::Clamp(1.0f - NormError, 0.0f, 1.0f);

    // Update internal model toward actual (learning)
    for (int32 i = 0; i < JointTransforms.Num(); ++i)
    {
        FVector NewPos = FMath::Lerp(InternalBodyModel[i].GetLocation(), JointTransforms[i].GetLocation(), 0.1f);
        FQuat NewRot = FQuat::Slerp(InternalBodyModel[i].GetRotation(), JointTransforms[i].GetRotation(), 0.1f);
        InternalBodyModel[i].SetLocation(NewPos);
        InternalBodyModel[i].SetRotation(NewRot);
    }
}

void UEmbodiedCognitionEnhanced::RegisterSomaticMarker(const FString& Situation, float Valence, float Arousal)
{
    SomaticMarkers.Add(Situation, FVector2D(Valence, Arousal));
}

float UEmbodiedCognitionEnhanced::QuerySomaticMarker(const FString& Situation) const
{
    const FVector2D* Marker = SomaticMarkers.Find(Situation);
    if (Marker)
    {
        return Marker->X; // Return valence
    }
    return 0.0f; // Neutral
}

float UEmbodiedCognitionEnhanced::GetInteroceptiveAccuracy() const
{
    // Interoceptive accuracy based on how well the agent tracks its own hormone state
    if (ConnectedStudio)
    {
        FHormoneBusState Hormones = ConnectedStudio->GetHormoneState();
        // Check if active inference predictions match actual hormone state
        float Error = 0.0f;
        for (int32 i = 0; i < FMath::Min(16, ActiveInference.Predictions.Num()); ++i)
        {
            float Diff = ActiveInference.Predictions[i] - Hormones.GetChannel(i);
            Error += Diff * Diff;
        }
        return FMath::Clamp(1.0f - FMath::Sqrt(Error / 16.0f), 0.0f, 1.0f);
    }
    return 0.5f;
}

// ========================================================================
// Embedded Cognition
// ========================================================================

TArray<FAffordance> UEmbodiedCognitionEnhanced::DetectAffordances(const TArray<FString>& VisibleObjects)
{
    TArray<FAffordance> Detected;

    for (const FString& Object : VisibleObjects)
    {
        // Check affordance library
        if (Object.Contains(TEXT("Face")) || Object.Contains(TEXT("Person")))
        {
            FAffordance SocialAffordance;
            SocialAffordance.AffordanceName = TEXT("SocialInteraction");
            SocialAffordance.ObjectName = Object;
            SocialAffordance.Strength = 0.8f * EnvironmentalSensitivity;
            SocialAffordance.bRelevant = true;
            SocialAffordance.SchemaName = TEXT("Listen");
            Detected.Add(SocialAffordance);

            FAffordance SpeakAffordance;
            SpeakAffordance.AffordanceName = TEXT("Speak");
            SpeakAffordance.ObjectName = Object;
            SpeakAffordance.Strength = 0.7f * EnvironmentalSensitivity;
            SpeakAffordance.bRelevant = true;
            SpeakAffordance.SchemaName = TEXT("Speak");
            Detected.Add(SpeakAffordance);
        }

        if (Object.Contains(TEXT("Text")) || Object.Contains(TEXT("Screen")))
        {
            FAffordance ReadAffordance;
            ReadAffordance.AffordanceName = TEXT("Read");
            ReadAffordance.ObjectName = Object;
            ReadAffordance.Strength = 0.6f * EnvironmentalSensitivity;
            ReadAffordance.bRelevant = true;
            ReadAffordance.SchemaName = TEXT("LookAt");
            Detected.Add(ReadAffordance);
        }

        if (Object.Contains(TEXT("Tool")) || Object.Contains(TEXT("Interface")))
        {
            FAffordance UseAffordance;
            UseAffordance.AffordanceName = TEXT("UseTool");
            UseAffordance.ObjectName = Object;
            UseAffordance.Strength = 0.5f * EnvironmentalSensitivity;
            UseAffordance.bRelevant = true;
            UseAffordance.SchemaName = TEXT("Think");
            Detected.Add(UseAffordance);
        }
    }

    // Update niche coupling based on affordance detection rate
    float DetectionRate = VisibleObjects.Num() > 0 ?
        static_cast<float>(Detected.Num()) / static_cast<float>(VisibleObjects.Num()) : 0.0f;
    NicheCoupling = FMath::Lerp(NicheCoupling, DetectionRate, 0.05f);

    return Detected;
}

void UEmbodiedCognitionEnhanced::RecordNicheModification(const FString& Type, float Magnitude, bool bBeneficial)
{
    FNicheModification Mod;
    Mod.ModificationType = Type;
    Mod.Magnitude = Magnitude;
    Mod.bBeneficial = bBeneficial;
    NicheHistory.Add(Mod);

    // Update environmental sensitivity based on niche construction success
    if (bBeneficial)
    {
        EnvironmentalSensitivity = FMath::Clamp(EnvironmentalSensitivity + Magnitude * 0.05f, 0.0f, 1.0f);
    }
}

// ========================================================================
// Enacted Cognition
// ========================================================================

void UEmbodiedCognitionEnhanced::RegisterSchema(const FString& Name, const TArray<float>& ForwardModel, const TArray<float>& InverseModel)
{
    FSensorimotorSchema Schema;
    Schema.SchemaName = Name;
    Schema.ForwardModel = ForwardModel;
    Schema.InverseModel = InverseModel;
    Schemas.Add(Name, Schema);
}

float UEmbodiedCognitionEnhanced::ExecuteSchema(const FString& Name, const TArray<float>& SensoryInput)
{
    FSensorimotorSchema* Schema = Schemas.Find(Name);
    if (!Schema) return 1.0f; // Max error if schema not found

    // Compute prediction from forward model
    float PredictionError = 0.0f;
    int32 Dims = FMath::Min(Schema->ForwardModel.Num(), SensoryInput.Num());
    for (int32 i = 0; i < Dims; ++i)
    {
        float Diff = Schema->ForwardModel[i] - SensoryInput[i];
        PredictionError += Diff * Diff;
    }
    PredictionError = Dims > 0 ? FMath::Sqrt(PredictionError / static_cast<float>(Dims)) : 0.0f;

    // Update schema competence
    Schema->UpdateCompetence(PredictionError);

    // Update forward model (learning)
    for (int32 i = 0; i < Dims; ++i)
    {
        Schema->ForwardModel[i] = FMath::Lerp(Schema->ForwardModel[i], SensoryInput[i], 0.05f);
    }

    return PredictionError;
}

void UEmbodiedCognitionEnhanced::UpdateActiveInference(const TArray<float>& Observations)
{
    ActiveInference.UpdateBeliefs(Observations, 0.1f);
}

float UEmbodiedCognitionEnhanced::GetSensorimotorCoordination() const
{
    if (Schemas.Num() == 0) return 0.0f;

    float TotalCompetence = 0.0f;
    for (const auto& Pair : Schemas)
    {
        TotalCompetence += Pair.Value.Competence;
    }
    return TotalCompetence / static_cast<float>(Schemas.Num());
}

// ========================================================================
// Extended Cognition
// ========================================================================

void UEmbodiedCognitionEnhanced::RegisterCognitiveExtension(const FString& Name, const FString& Type, float Reliability)
{
    FCognitiveExtension Ext;
    Ext.ToolName = Name;
    Ext.ToolType = Type;
    Ext.Reliability = Reliability;
    CognitiveExtensions.Add(Name, Ext);
}

float UEmbodiedCognitionEnhanced::UseCognitiveExtension(const FString& Name)
{
    FCognitiveExtension* Ext = CognitiveExtensions.Find(Name);
    if (!Ext) return 0.0f;

    Ext->UsageCount++;
    // Integration improves with use
    Ext->IntegrationLevel = FMath::Clamp(Ext->IntegrationLevel + 0.01f, 0.0f, 1.0f);

    return Ext->IntegrationLevel * Ext->Reliability;
}

float UEmbodiedCognitionEnhanced::GetToolUseCompetence() const
{
    if (CognitiveExtensions.Num() == 0) return 0.0f;

    float TotalIntegration = 0.0f;
    for (const auto& Pair : CognitiveExtensions)
    {
        TotalIntegration += Pair.Value.IntegrationLevel;
    }
    return TotalIntegration / static_cast<float>(CognitiveExtensions.Num());
}

float UEmbodiedCognitionEnhanced::GetExternalMemoryIntegration() const
{
    float MemoryIntegration = 0.0f;
    int32 MemoryCount = 0;
    for (const auto& Pair : CognitiveExtensions)
    {
        if (Pair.Value.ToolType == TEXT("memory"))
        {
            MemoryIntegration += Pair.Value.IntegrationLevel;
            MemoryCount++;
        }
    }
    return MemoryCount > 0 ? MemoryIntegration / static_cast<float>(MemoryCount) : 0.0f;
}

void UEmbodiedCognitionEnhanced::UpdateSocialCognition(float InteractionQuality)
{
    SocialCognitionDepth = FMath::Lerp(SocialCognitionDepth, InteractionQuality, 0.05f);
}

// ========================================================================
// Integration
// ========================================================================

void UEmbodiedCognitionEnhanced::ConnectToDesignStudio(UAvatarDesignStudio* Studio)
{
    ConnectedStudio = Studio;
}

void UEmbodiedCognitionEnhanced::SyncToDesignStudio()
{
    if (!ConnectedStudio) return;

    ConnectedStudio->Update4EMetric(TEXT("BodySchemaIntegration"), ProprioceptiveAccuracy);
    ConnectedStudio->Update4EMetric(TEXT("ProprioceptiveAccuracy"), ProprioceptiveAccuracy);
    ConnectedStudio->Update4EMetric(TEXT("SomaticMarkerStrength"),
        SomaticMarkers.Num() > 0 ? FMath::Min(1.0f, static_cast<float>(SomaticMarkers.Num()) / 50.0f) : 0.0f);
    ConnectedStudio->Update4EMetric(TEXT("AffordanceDetectionRate"), NicheCoupling);
    ConnectedStudio->Update4EMetric(TEXT("NicheCoupling"), NicheCoupling);
    ConnectedStudio->Update4EMetric(TEXT("EnvironmentalSensitivity"), EnvironmentalSensitivity);
    ConnectedStudio->Update4EMetric(TEXT("SensorimotorCoordination"), GetSensorimotorCoordination());
    ConnectedStudio->Update4EMetric(TEXT("PredictionAccuracy"), 1.0f - ActiveInference.FreeEnergy);
    ConnectedStudio->Update4EMetric(TEXT("ActiveInferenceEfficiency"),
        FMath::Clamp(1.0f - ActiveInference.FreeEnergy, 0.0f, 1.0f));
    ConnectedStudio->Update4EMetric(TEXT("ToolUseCompetence"), GetToolUseCompetence());
    ConnectedStudio->Update4EMetric(TEXT("ExternalMemoryIntegration"), GetExternalMemoryIntegration());
    ConnectedStudio->Update4EMetric(TEXT("SocialCognitionDepth"), SocialCognitionDepth);
}

float UEmbodiedCognitionEnhanced::GetOverall4EScore() const
{
    return (GetEmbodiedScore() + GetEmbeddedScore() + GetEnactedScore() + GetExtendedScore()) / 4.0f;
}

float UEmbodiedCognitionEnhanced::GetEmbodiedScore() const
{
    float SomaticStrength = SomaticMarkers.Num() > 0 ?
        FMath::Min(1.0f, static_cast<float>(SomaticMarkers.Num()) / 50.0f) : 0.0f;
    return (ProprioceptiveAccuracy + GetInteroceptiveAccuracy() + SomaticStrength) / 3.0f;
}

float UEmbodiedCognitionEnhanced::GetEmbeddedScore() const
{
    return (NicheCoupling + EnvironmentalSensitivity +
            (NicheHistory.Num() > 0 ? FMath::Min(1.0f, static_cast<float>(NicheHistory.Num()) / 20.0f) : 0.0f)) / 3.0f;
}

float UEmbodiedCognitionEnhanced::GetEnactedScore() const
{
    return (GetSensorimotorCoordination() +
            FMath::Clamp(1.0f - ActiveInference.FreeEnergy, 0.0f, 1.0f) +
            FMath::Clamp(1.0f - ActiveInference.FreeEnergy, 0.0f, 1.0f)) / 3.0f;
}

float UEmbodiedCognitionEnhanced::GetExtendedScore() const
{
    return (GetToolUseCompetence() + GetExternalMemoryIntegration() + SocialCognitionDepth) / 3.0f;
}
