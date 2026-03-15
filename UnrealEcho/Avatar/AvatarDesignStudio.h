// AvatarDesignStudio.h
// Deep Tree Echo - Avatar Design Studio & Animation Editor
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// A comprehensive avatar design studio that integrates MetaHuman DNA calibration,
// FACS expression mapping, Lorenz chaotic dynamics, SuperHotGirl aesthetics,
// 4E embodied cognition, and the virtual endocrine system into a unified
// design-time and runtime avatar creation pipeline.
//
// Composition: meta-echo-dna ⊗ (facs ⊗ rig-logic ⊗ virtual-endocrine-system)
//              ⊕ unreal-blueprint(expression_pipeline)
//              ⊕ live2d-dtecho(cyberpunk-bioluminescent aesthetic)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AvatarDesignStudio.generated.h"

// ============================================================================
// Forward Declarations
// ============================================================================
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
struct FNeurochemicalState;
struct FEmotionalState;

// ============================================================================
// Enums
// ============================================================================

/** Ontogenetic evolution stages for the avatar */
UENUM(BlueprintType)
enum class EAvatarEvolutionStage : uint8
{
    Embryonic       UMETA(DisplayName = "Embryonic"),
    Juvenile        UMETA(DisplayName = "Juvenile"),
    Adolescent      UMETA(DisplayName = "Adolescent"),
    Adult           UMETA(DisplayName = "Adult"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

/** Cognitive modes emergent from endocrine state */
UENUM(BlueprintType)
enum class ECognitiveMode : uint8
{
    Resting         UMETA(DisplayName = "Resting"),
    Exploratory     UMETA(DisplayName = "Exploratory"),
    Focused         UMETA(DisplayName = "Focused"),
    Stressed        UMETA(DisplayName = "Stressed"),
    Social          UMETA(DisplayName = "Social"),
    Reflective      UMETA(DisplayName = "Reflective"),
    Vigilant        UMETA(DisplayName = "Vigilant"),
    Maintenance     UMETA(DisplayName = "Maintenance"),
    Reward          UMETA(DisplayName = "Reward"),
    Threat          UMETA(DisplayName = "Threat")
};

/** SuperHotGirl aesthetic parameter set */
UENUM(BlueprintType)
enum class EAestheticPreset : uint8
{
    Default             UMETA(DisplayName = "Default"),
    SuperHotGirl        UMETA(DisplayName = "SuperHotGirl"),
    CyberpunkAngel      UMETA(DisplayName = "Cyberpunk Angel"),
    BioluminescentSage  UMETA(DisplayName = "Bioluminescent Sage"),
    HyperChaotic        UMETA(DisplayName = "Hyper-Chaotic"),
    Custom              UMETA(DisplayName = "Custom")
};

// ============================================================================
// Structs
// ============================================================================

/** 16-channel hormone bus state */
USTRUCT(BlueprintType)
struct FHormoneBusState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float CRH = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float ACTH = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Cortisol = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float DopamineTonic = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float DopaminePhasic = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Serotonin = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Norepinephrine = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Oxytocin = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float T3T4 = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Melatonin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Insulin = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Glucagon = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float IL6 = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Anandamide = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Reserved0 = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endocrine")
    float Reserved1 = 0.0f;

    /** Get concentration by channel index (0-15) */
    float GetChannel(int32 Index) const;

    /** Set concentration by channel index */
    void SetChannel(int32 Index, float Value);

    /** Apply exponential decay toward baselines */
    void ApplyDecay(float DeltaTime, const FHormoneBusState& Baselines, const TArray<float>& HalfLives);
};

/** FACS Action Unit activation state */
USTRUCT(BlueprintType)
struct FFACSActionUnitState
{
    GENERATED_BODY()

    /** Map of AU number to activation intensity [0.0, 1.0] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FACS")
    TMap<int32, float> ActionUnits;

    /** Compute AU from endocrine state */
    void ComputeFromEndocrine(const FHormoneBusState& Hormones);

    /** Compute AU from cognitive state (valence, arousal, load) */
    void ComputeFromCognitive(float Valence, float Arousal, float CognitiveLoad);

    /** Blend two AU states (sum + clamp) */
    void BlendWith(const FFACSActionUnitState& Other);

    /** Apply chaotic micro-expression noise */
    void ApplyChaos(const FVector& LorenzState, float ChaosIntensity);

    /** Get final AU value clamped to [0,1] */
    float GetAU(int32 AUNumber) const;
};

/** SuperHotGirl aesthetic parameters */
USTRUCT(BlueprintType)
struct FSuperHotGirlAesthetics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidencePosture = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Charisma = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSparkle = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GracefulMovement = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmissiveGlow = 0.6f;

    /** Bioluminescent teal intensity for DTE cyberpunk aesthetic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BioluminescentIntensity = 0.7f;

    /** Holographic decal shimmer rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aesthetics", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float HolographicShimmerRate = 1.5f;
};

/** Lorenz attractor state for chaotic micro-expressions */
USTRUCT(BlueprintType)
struct FLorenzAttractorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    FVector State = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float Sigma = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float Rho = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float Beta = 2.6667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float ChaosIntensity = 0.15f;

    /** Step the attractor using RK4 integration */
    void Step(float DeltaTime);

    /** Get the current Lyapunov exponent estimate */
    float GetLyapunovExponent() const;

    /** Get normalized output for AU modulation [-1, 1] */
    FVector GetNormalizedOutput() const;

private:
    float LyapunovSum = 0.0f;
    int32 LyapunovSteps = 0;
    FVector PreviousState = FVector(1.0f, 1.0f, 1.0f);
};

/** 4E Cognition metrics */
USTRUCT(BlueprintType)
struct F4ECognitionMetrics
{
    GENERATED_BODY()

    // Embodied
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embodied", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BodySchemaIntegration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embodied", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ProprioceptiveAccuracy = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embodied", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SomaticMarkerStrength = 0.5f;

    // Embedded
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embedded", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AffordanceDetectionRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embedded", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NicheCoupling = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Embedded", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EnvironmentalSensitivity = 0.5f;

    // Enacted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Enacted", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SensorimotorCoordination = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Enacted", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PredictionAccuracy = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Enacted", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActiveInferenceEfficiency = 0.5f;

    // Extended
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Extended", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ToolUseCompetence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Extended", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ExternalMemoryIntegration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4E|Extended", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialCognitionDepth = 0.5f;

    /** Get overall 4E score (average of all dimensions) */
    float GetOverallScore() const;

    /** Get score for a specific E dimension (0=Embodied, 1=Embedded, 2=Enacted, 3=Extended) */
    float GetDimensionScore(int32 Dimension) const;
};

/** ESN Reservoir state */
USTRUCT(BlueprintType)
struct FReservoirState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservoir")
    int32 ReservoirSize = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservoir")
    float SpectralRadius = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservoir")
    float LeakRate = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservoir")
    float Sparsity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reservoir")
    float InputScaling = 0.5f;

    /** Internal state vector (not exposed to BP for performance) */
    TArray<float> StateVector;

    /** Weight matrix (sparse) */
    TMap<int32, float> WeightMatrix;

    /** Input weight matrix */
    TArray<float> InputWeights;

    /** Output weight matrix */
    TArray<float> OutputWeights;

    /** Initialize reservoir with random weights */
    void Initialize();

    /** Update reservoir state: State[t] = (1-LeakRate)*State[t-1] + LeakRate*tanh(W*State[t-1] + Win*Input) */
    void Update(const TArray<float>& Input);

    /** Read output: Wout * State */
    TArray<float> ReadOutput() const;
};

/** Wisdom cultivation metrics */
USTRUCT(BlueprintType)
struct FWisdomMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Morality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Meaning = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Mastery = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sophrosyne = 0.5f;

    float GetOverallScore() const { return (Morality + Meaning + Mastery + Sophrosyne) / 4.0f; }
};

/** Animation keyframe for the animation editor */
USTRUCT(BlueprintType)
struct FExpressionKeyframe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FFACSActionUnitState ActionUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FSuperHotGirlAesthetics Aesthetics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FHormoneBusState EndocrineState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    ECognitiveMode CognitiveMode = ECognitiveMode::Resting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString ExpressionName;
};

/** Animation sequence for the animation editor */
USTRUCT(BlueprintType)
struct FExpressionAnimationSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FExpressionKeyframe> Keyframes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping = false;

    /** Evaluate the sequence at a given time, interpolating between keyframes */
    FExpressionKeyframe Evaluate(float Time) const;

    /** Add a keyframe at the specified time */
    void AddKeyframe(const FExpressionKeyframe& Keyframe);

    /** Remove keyframe at index */
    void RemoveKeyframe(int32 Index);

    /** Recalculate duration from keyframes */
    void RecalculateDuration();
};

// ============================================================================
// Main Component: Avatar Design Studio
// ============================================================================

/**
 * UAvatarDesignStudio
 *
 * The unified avatar design studio component that integrates all Deep Tree Echo
 * avatar subsystems into a single, coherent design-time and runtime pipeline.
 *
 * Composition:
 *   meta-echo-dna ⊗ (facs ⊗ rig-logic ⊗ virtual-endocrine-system)
 *   ⊕ unreal-blueprint(expression_pipeline)
 *   ⊕ live2d-dtecho(cyberpunk-bioluminescent aesthetic)
 *
 * Features:
 *   - Real-time FACS expression preview with 26+ Action Units
 *   - Lorenz attractor chaotic micro-expression generation
 *   - 16-channel virtual endocrine system with 10 glands
 *   - SuperHotGirl + HyperChaotic aesthetic parameter control
 *   - 4E embodied cognition metrics tracking
 *   - ESN reservoir computing for temporal pattern processing
 *   - Ontogenetic evolution stage management
 *   - Animation sequence editor with keyframe interpolation
 *   - MetaHuman DNA calibration integration
 *   - CogMorph glyph projection for debug visualization
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class UAvatarDesignStudio : public UActorComponent
{
    GENERATED_BODY()

public:
    UAvatarDesignStudio();

    // ========================================================================
    // Lifecycle
    // ========================================================================

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========================================================================
    // Design Studio API
    // ========================================================================

    /** Initialize the design studio with a target skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Design Studio")
    bool InitializeStudio(USkeletalMeshComponent* TargetMesh);

    /** Set the aesthetic preset */
    UFUNCTION(BlueprintCallable, Category = "Design Studio")
    void SetAestheticPreset(EAestheticPreset Preset);

    /** Get current aesthetic parameters */
    UFUNCTION(BlueprintPure, Category = "Design Studio")
    FSuperHotGirlAesthetics GetCurrentAesthetics() const { return Aesthetics; }

    /** Set individual aesthetic parameter */
    UFUNCTION(BlueprintCallable, Category = "Design Studio")
    void SetAestheticParameter(const FString& ParameterName, float Value);

    /** Preview an expression by name */
    UFUNCTION(BlueprintCallable, Category = "Design Studio")
    void PreviewExpression(const FString& ExpressionName);

    /** Get the current FACS AU state */
    UFUNCTION(BlueprintPure, Category = "Design Studio")
    FFACSActionUnitState GetCurrentFACSState() const { return CurrentFACS; }

    /** Manually set an Action Unit value */
    UFUNCTION(BlueprintCallable, Category = "Design Studio")
    void SetActionUnit(int32 AUNumber, float Intensity);

    /** Get the current cognitive mode */
    UFUNCTION(BlueprintPure, Category = "Design Studio")
    ECognitiveMode GetCurrentCognitiveMode() const { return CurrentMode; }

    // ========================================================================
    // Endocrine System API
    // ========================================================================

    /** Signal an endocrine event */
    UFUNCTION(BlueprintCallable, Category = "Endocrine")
    void SignalEndocrineEvent(const FString& EventType, float Intensity);

    /** Get current hormone bus state */
    UFUNCTION(BlueprintPure, Category = "Endocrine")
    FHormoneBusState GetHormoneState() const { return HormoneState; }

    /** Set hormone concentration directly (for design-time preview) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine")
    void SetHormoneConcentration(int32 ChannelIndex, float Concentration);

    /** Get the current Lorenz attractor state */
    UFUNCTION(BlueprintPure, Category = "Chaos")
    FLorenzAttractorState GetLorenzState() const { return LorenzState; }

    /** Set chaos intensity */
    UFUNCTION(BlueprintCallable, Category = "Chaos")
    void SetChaosIntensity(float Intensity);

    // ========================================================================
    // 4E Cognition API
    // ========================================================================

    /** Get current 4E cognition metrics */
    UFUNCTION(BlueprintPure, Category = "4E Cognition")
    F4ECognitionMetrics Get4EMetrics() const { return CognitionMetrics; }

    /** Update a specific 4E metric */
    UFUNCTION(BlueprintCallable, Category = "4E Cognition")
    void Update4EMetric(const FString& MetricName, float Value);

    /** Get the overall 4E score */
    UFUNCTION(BlueprintPure, Category = "4E Cognition")
    float GetOverall4EScore() const { return CognitionMetrics.GetOverallScore(); }

    // ========================================================================
    // Evolution API
    // ========================================================================

    /** Get current evolution stage */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    EAvatarEvolutionStage GetEvolutionStage() const { return EvolutionStage; }

    /** Check if evolution conditions are met and advance if possible */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    bool TryAdvanceEvolution();

    /** Get wisdom metrics */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    FWisdomMetrics GetWisdomMetrics() const { return Wisdom; }

    /** Get the cycle count since initialization */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    int64 GetCycleCount() const { return CycleCount; }

    // ========================================================================
    // Reservoir Computing API
    // ========================================================================

    /** Get reservoir state */
    UFUNCTION(BlueprintPure, Category = "Reservoir")
    FReservoirState GetReservoirState() const { return Reservoir; }

    /** Feed input to the reservoir and get output */
    UFUNCTION(BlueprintCallable, Category = "Reservoir")
    TArray<float> ProcessReservoirInput(const TArray<float>& Input);

    // ========================================================================
    // Animation Editor API
    // ========================================================================

    /** Create a new animation sequence */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    int32 CreateAnimationSequence(const FString& Name);

    /** Add a keyframe to a sequence */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void AddKeyframeToSequence(int32 SequenceIndex, float TimeStamp, const FString& ExpressionName);

    /** Play an animation sequence */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void PlayAnimationSequence(int32 SequenceIndex);

    /** Stop current animation playback */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void StopAnimationPlayback();

    /** Get all animation sequences */
    UFUNCTION(BlueprintPure, Category = "Animation Editor")
    TArray<FExpressionAnimationSequence> GetAnimationSequences() const { return AnimationSequences; }

    /** Export animation sequence to JSON */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    FString ExportSequenceToJSON(int32 SequenceIndex) const;

    /** Import animation sequence from JSON */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    int32 ImportSequenceFromJSON(const FString& JSONString);

    // ========================================================================
    // Echobeats 9-Step Cognitive Cycle
    // ========================================================================

    /** Execute one full Echobeats cognitive cycle */
    UFUNCTION(BlueprintCallable, Category = "Echobeats")
    void ExecuteCognitiveCycle(float DeltaTime);

    // ========================================================================
    // MetaHuman DNA Integration
    // ========================================================================

    /** Load a MetaHuman DNA file */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman DNA")
    bool LoadDNAFile(const FString& FilePath);

    /** Apply DNA modifications for unique facial features */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman DNA")
    void ApplyDNACustomization(const TMap<FString, float>& BlendShapeOverrides);

    /** Get available DNA blend shapes */
    UFUNCTION(BlueprintPure, Category = "MetaHuman DNA")
    TArray<FString> GetAvailableBlendShapes() const;

    // ========================================================================
    // Debug & Visualization
    // ========================================================================

    /** Export current state as CogMorph glyph projection */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ExportGlyphProjection(const FString& OutputPath) const;

    /** Get a human-readable status string */
    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetStatusString() const;

    /** Get Lyapunov exponent of the chaotic system */
    UFUNCTION(BlueprintPure, Category = "Debug")
    float GetLyapunovExponent() const { return LorenzState.GetLyapunovExponent(); }

    // ========================================================================
    // Events
    // ========================================================================

    /** Fired when cognitive mode changes */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCognitiveModeChanged OnCognitiveModeChanged;

    /** Fired when evolution stage advances */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEvolutionStageAdvanced OnEvolutionStageAdvanced;

    /** Fired when an expression changes */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnExpressionChanged OnExpressionChanged;

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Target skeletal mesh for expression application */
    UPROPERTY()
    USkeletalMeshComponent* TargetSkeletalMesh = nullptr;

    /** Dynamic material instances for aesthetic control */
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    /** Current FACS action unit state */
    UPROPERTY(VisibleAnywhere, Category = "State")
    FFACSActionUnitState CurrentFACS;

    /** Endocrine-driven FACS state */
    FFACSActionUnitState EndocrineFACS;

    /** Cognitive-driven FACS state */
    FFACSActionUnitState CognitiveFACS;

    /** Current hormone bus state */
    UPROPERTY(VisibleAnywhere, Category = "State")
    FHormoneBusState HormoneState;

    /** Hormone baselines for decay */
    FHormoneBusState HormoneBaselines;

    /** Hormone half-lives */
    TArray<float> HormoneHalfLives;

    /** Lorenz attractor for chaotic dynamics */
    UPROPERTY(VisibleAnywhere, Category = "State")
    FLorenzAttractorState LorenzState;

    /** SuperHotGirl aesthetic parameters */
    UPROPERTY(EditAnywhere, Category = "Aesthetics")
    FSuperHotGirlAesthetics Aesthetics;

    /** 4E cognition metrics */
    UPROPERTY(VisibleAnywhere, Category = "State")
    F4ECognitionMetrics CognitionMetrics;

    /** ESN reservoir */
    UPROPERTY(VisibleAnywhere, Category = "State")
    FReservoirState Reservoir;

    /** Wisdom metrics */
    UPROPERTY(VisibleAnywhere, Category = "State")
    FWisdomMetrics Wisdom;

    /** Current cognitive mode */
    UPROPERTY(VisibleAnywhere, Category = "State")
    ECognitiveMode CurrentMode = ECognitiveMode::Resting;

    /** Current evolution stage */
    UPROPERTY(VisibleAnywhere, Category = "State")
    EAvatarEvolutionStage EvolutionStage = EAvatarEvolutionStage::Embryonic;

    /** Cognitive cycle count */
    int64 CycleCount = 0;

    /** Animation sequences */
    UPROPERTY(VisibleAnywhere, Category = "Animation")
    TArray<FExpressionAnimationSequence> AnimationSequences;

    /** Currently playing animation index (-1 = none) */
    int32 PlayingSequenceIndex = -1;

    /** Current playback time */
    float PlaybackTime = 0.0f;

    /** Whether the studio is initialized */
    bool bIsInitialized = false;

    /** Error threshold for learning signal */
    float ErrorThreshold = 0.3f;

    /** Reward threshold for positive reinforcement */
    float RewardThreshold = 0.7f;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Initialize the 16-channel hormone bus with baselines and half-lives */
    void InitializeEndocrineSystem();

    /** Initialize the 10 virtual glands */
    void InitializeGlands();

    /** Detect cognitive mode from 16D hormone space (nearest centroid) */
    ECognitiveMode DetectCognitiveMode() const;

    /** Map FACS AU values to MetaHuman CTRL_ morph targets */
    void ApplyFACSToMetaHuman();

    /** Apply aesthetic parameters to materials */
    void ApplyAestheticsToMaterials();

    /** Update animation playback */
    void UpdateAnimationPlayback(float DeltaTime);

    /** Interpolate between two keyframes */
    FExpressionKeyframe InterpolateKeyframes(const FExpressionKeyframe& A, const FExpressionKeyframe& B, float Alpha) const;

    /** Get evolution threshold for current stage */
    float GetEvolutionThreshold() const;

    /** Map named expression to FACS AU configuration */
    FFACSActionUnitState GetNamedExpression(const FString& Name) const;

    /** Initialize the 10 named DTE expressions */
    void InitializeNamedExpressions();

    /** Named expression library */
    TMap<FString, FFACSActionUnitState> NamedExpressions;

    /** Cognitive mode centroid vectors in 16D hormone space */
    TMap<ECognitiveMode, TArray<float>> ModeCentroids;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCognitiveModeChanged, ECognitiveMode, OldMode, ECognitiveMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEvolutionStageAdvanced, EAvatarEvolutionStage, OldStage, EAvatarEvolutionStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpressionChanged, const FString&, ExpressionName);
