// EmbodiedCognitionEnhanced.h
// Deep Tree Echo - Enhanced 4E Embodied Cognition System
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// Complete 4E cognition framework with active inference, niche construction,
// sensorimotor coupling, and extended cognition through tool use.
// Integrates with the virtual endocrine system for affect-modulated cognition.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmbodiedCognitionEnhanced.generated.h"

// Forward declarations
class UAvatarDesignStudio;

/** Sensorimotor schema for enacted cognition */
USTRUCT(BlueprintType)
struct FSensorimotorSchema
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensorimotor")
    FString SchemaName;

    /** Prediction model: expected sensory consequence of action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensorimotor")
    TArray<float> ForwardModel;

    /** Inverse model: action required to achieve sensory goal */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensorimotor")
    TArray<float> InverseModel;

    /** Prediction error history for learning */
    TArray<float> ErrorHistory;

    /** Schema competence (0-1) based on prediction accuracy */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensorimotor")
    float Competence = 0.0f;

    /** Number of times this schema has been activated */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensorimotor")
    int32 ActivationCount = 0;

    /** Update competence from prediction error */
    void UpdateCompetence(float PredictionError);
};

/** Affordance detected in the environment */
USTRUCT(BlueprintType)
struct FAffordance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affordance")
    FString AffordanceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affordance")
    FString ObjectName;

    /** Affordance strength (0-1): how strongly the object affords this action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affordance")
    float Strength = 0.0f;

    /** Whether this affordance is currently relevant given the agent's goals */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affordance")
    bool bRelevant = false;

    /** Associated sensorimotor schema */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affordance")
    FString SchemaName;
};

/** Active inference state for free energy minimization */
USTRUCT(BlueprintType)
struct FActiveInferenceState
{
    GENERATED_BODY()

    /** Current beliefs about the world state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Inference")
    TArray<float> Beliefs;

    /** Predicted sensory observations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Inference")
    TArray<float> Predictions;

    /** Actual sensory observations */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Inference")
    TArray<float> Observations;

    /** Free energy (surprise) - lower is better */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Inference")
    float FreeEnergy = 0.0f;

    /** Expected free energy for action selection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Inference")
    float ExpectedFreeEnergy = 0.0f;

    /** Precision (inverse variance) of predictions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Inference")
    float Precision = 1.0f;

    /** Update beliefs via variational inference */
    void UpdateBeliefs(const TArray<float>& NewObservations, float LearningRate);

    /** Compute free energy: F = -log p(o|s) + KL(q(s)||p(s)) */
    float ComputeFreeEnergy() const;

    /** Select action that minimizes expected free energy */
    int32 SelectAction(const TArray<TArray<float>>& ActionConsequences) const;
};

/** Niche construction record */
USTRUCT(BlueprintType)
struct FNicheModification
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niche")
    FString ModificationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niche")
    float Magnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niche")
    float TimeStamp = 0.0f;

    /** Whether this modification improved fitness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niche")
    bool bBeneficial = false;
};

/** Extended cognition tool */
USTRUCT(BlueprintType)
struct FCognitiveExtension
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extended")
    FString ToolName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extended")
    FString ToolType; // "memory", "computation", "communication", "perception"

    /** How well the agent has integrated this tool (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extended")
    float IntegrationLevel = 0.0f;

    /** Usage count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extended")
    int32 UsageCount = 0;

    /** Reliability of the tool (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extended")
    float Reliability = 1.0f;
};

/**
 * UEmbodiedCognitionEnhanced
 *
 * Complete 4E cognition implementation for Deep Tree Echo:
 *
 * EMBODIED: Body schema, proprioception, somatic markers, interoception
 * EMBEDDED: Affordance detection, niche construction, environmental coupling
 * ENACTED: Sensorimotor schemas, active inference, prediction-action loops
 * EXTENDED: Tool use, external memory, social cognition, cognitive scaffolding
 *
 * Integrates with:
 * - Virtual endocrine system (affect modulates all 4E dimensions)
 * - ESN reservoir (temporal pattern processing)
 * - Avatar design studio (expression reflects 4E state)
 * - Evolution system (4E scores drive ontogenetic progression)
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class UEmbodiedCognitionEnhanced : public UActorComponent
{
    GENERATED_BODY()

public:
    UEmbodiedCognitionEnhanced();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // Embodied Cognition
    // ========================================================================

    /** Update body schema from skeletal mesh state */
    UFUNCTION(BlueprintCallable, Category = "4E|Embodied")
    void UpdateBodySchema(const TArray<FTransform>& JointTransforms);

    /** Get proprioceptive accuracy (how well internal model matches actual body) */
    UFUNCTION(BlueprintPure, Category = "4E|Embodied")
    float GetProprioceptiveAccuracy() const { return ProprioceptiveAccuracy; }

    /** Register a somatic marker (body-based emotional memory) */
    UFUNCTION(BlueprintCallable, Category = "4E|Embodied")
    void RegisterSomaticMarker(const FString& Situation, float Valence, float Arousal);

    /** Query somatic marker for a situation */
    UFUNCTION(BlueprintPure, Category = "4E|Embodied")
    float QuerySomaticMarker(const FString& Situation) const;

    /** Get interoceptive state (internal body sensing) */
    UFUNCTION(BlueprintPure, Category = "4E|Embodied")
    float GetInteroceptiveAccuracy() const;

    // ========================================================================
    // Embedded Cognition
    // ========================================================================

    /** Detect affordances in the environment */
    UFUNCTION(BlueprintCallable, Category = "4E|Embedded")
    TArray<FAffordance> DetectAffordances(const TArray<FString>& VisibleObjects);

    /** Get current niche coupling score */
    UFUNCTION(BlueprintPure, Category = "4E|Embedded")
    float GetNicheCoupling() const { return NicheCoupling; }

    /** Record a niche modification */
    UFUNCTION(BlueprintCallable, Category = "4E|Embedded")
    void RecordNicheModification(const FString& Type, float Magnitude, bool bBeneficial);

    /** Get environmental sensitivity */
    UFUNCTION(BlueprintPure, Category = "4E|Embedded")
    float GetEnvironmentalSensitivity() const { return EnvironmentalSensitivity; }

    // ========================================================================
    // Enacted Cognition
    // ========================================================================

    /** Register a sensorimotor schema */
    UFUNCTION(BlueprintCallable, Category = "4E|Enacted")
    void RegisterSchema(const FString& Name, const TArray<float>& ForwardModel, const TArray<float>& InverseModel);

    /** Execute a sensorimotor schema and update competence */
    UFUNCTION(BlueprintCallable, Category = "4E|Enacted")
    float ExecuteSchema(const FString& Name, const TArray<float>& SensoryInput);

    /** Get active inference state */
    UFUNCTION(BlueprintPure, Category = "4E|Enacted")
    FActiveInferenceState GetActiveInferenceState() const { return ActiveInference; }

    /** Update active inference with new observations */
    UFUNCTION(BlueprintCallable, Category = "4E|Enacted")
    void UpdateActiveInference(const TArray<float>& Observations);

    /** Get overall sensorimotor coordination score */
    UFUNCTION(BlueprintPure, Category = "4E|Enacted")
    float GetSensorimotorCoordination() const;

    // ========================================================================
    // Extended Cognition
    // ========================================================================

    /** Register a cognitive extension (tool) */
    UFUNCTION(BlueprintCallable, Category = "4E|Extended")
    void RegisterCognitiveExtension(const FString& Name, const FString& Type, float Reliability);

    /** Use a cognitive extension */
    UFUNCTION(BlueprintCallable, Category = "4E|Extended")
    float UseCognitiveExtension(const FString& Name);

    /** Get tool use competence */
    UFUNCTION(BlueprintPure, Category = "4E|Extended")
    float GetToolUseCompetence() const;

    /** Get external memory integration level */
    UFUNCTION(BlueprintPure, Category = "4E|Extended")
    float GetExternalMemoryIntegration() const;

    /** Get social cognition depth */
    UFUNCTION(BlueprintPure, Category = "4E|Extended")
    float GetSocialCognitionDepth() const { return SocialCognitionDepth; }

    /** Update social cognition from interaction */
    UFUNCTION(BlueprintCallable, Category = "4E|Extended")
    void UpdateSocialCognition(float InteractionQuality);

    // ========================================================================
    // Integration
    // ========================================================================

    /** Connect to the avatar design studio for bidirectional updates */
    UFUNCTION(BlueprintCallable, Category = "4E|Integration")
    void ConnectToDesignStudio(UAvatarDesignStudio* Studio);

    /** Get overall 4E score */
    UFUNCTION(BlueprintPure, Category = "4E|Integration")
    float GetOverall4EScore() const;

    /** Get dimension scores */
    UFUNCTION(BlueprintPure, Category = "4E|Integration")
    float GetEmbodiedScore() const;

    UFUNCTION(BlueprintPure, Category = "4E|Integration")
    float GetEmbeddedScore() const;

    UFUNCTION(BlueprintPure, Category = "4E|Integration")
    float GetEnactedScore() const;

    UFUNCTION(BlueprintPure, Category = "4E|Integration")
    float GetExtendedScore() const;

protected:
    // Embodied state
    TArray<FTransform> InternalBodyModel;
    TArray<FTransform> ActualBodyState;
    float ProprioceptiveAccuracy = 0.5f;
    TMap<FString, FVector2D> SomaticMarkers; // Situation → (Valence, Arousal)

    // Embedded state
    float NicheCoupling = 0.5f;
    float EnvironmentalSensitivity = 0.5f;
    TArray<FNicheModification> NicheHistory;
    TMap<FString, TArray<FAffordance>> AffordanceLibrary;

    // Enacted state
    TMap<FString, FSensorimotorSchema> Schemas;
    FActiveInferenceState ActiveInference;

    // Extended state
    TMap<FString, FCognitiveExtension> CognitiveExtensions;
    float SocialCognitionDepth = 0.5f;

    // Integration
    UPROPERTY()
    UAvatarDesignStudio* ConnectedStudio = nullptr;

    /** Sync 4E metrics to the connected design studio */
    void SyncToDesignStudio();
};
