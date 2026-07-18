#pragma once
/**
 * @file CognitiveDNACalibration.h
 * @brief Cognitive DNA Calibration System for Deep Tree Echo
 *
 * The final bridge in the pipeline:
 *
 *   MasterOrchestrator → RigLogicEvaluator → CognitiveDNACalibration → Mesh Deformation
 *
 * This component:
 * 1. Reads MetaHuman DNA files (Descriptor, Definition, Behavior, Geometry layers)
 * 2. Calibrates the rig to the specific avatar's facial proportions
 * 3. Applies cognitive-driven expression through the Rig Logic evaluation
 * 4. Manages the SuperHotGirl aesthetic overlay and HyperChaotic micro-expressions
 * 5. Outputs final joint transforms, blend shapes, and animated maps to the skeletal mesh
 * 6. Supports runtime DNA modification via DNACalib for ontogenetic evolution
 *
 * Integrates with:
 * - RigLogicEvaluator: receives evaluated joint/blendshape/animmap outputs
 * - EndocrineExpressionPipeline: receives hormone-driven AU modulations
 * - HyperSkillTensor: receives tensor-composed aesthetic/chaotic overlays
 * - AvatarDesignStudio: receives design-time calibration parameters
 *
 * @see rig-logic skill (3Lateral whitepaper v2)
 * @see meta-echo-dna skill
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RigLogicEvaluator.h"
#include "CognitiveDNACalibration.generated.h"

// Forward declarations
class URigLogicEvaluator;
class UMasterOrchestrator;
class UEndocrineExpressionPipeline;

// ============================================================================
// DNA Layer Status
// ============================================================================
UENUM(BlueprintType)
enum class EDNALayerStatus : uint8
{
    NotLoaded,
    Loading,
    Loaded,
    Error
};

// ============================================================================
// DNA Character Descriptor
// ============================================================================
USTRUCT(BlueprintType)
struct FDNACharacterDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName = TEXT("DeepTreeEcho");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Archetype = TEXT("CognitiveAvatar");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Gender = TEXT("Female");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Metadata;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LODCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CoordinateSystem = TEXT("UE5_LeftHanded_ZUp");
};

// ============================================================================
// Aesthetic Overlay (SuperHotGirl + HyperChaotic combined)
// ============================================================================
USTRUCT(BlueprintType)
struct FAestheticOverlay
{
    GENERATED_BODY()

    // --- SuperHotGirl Aesthetics ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidencePosture = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CharismaGlow = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSparkle = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LipGlossSheen = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinSSSBoost = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IdleElegance = 0.9f;

    // --- HyperChaotic Dynamics ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MicroExpressionAmplitude = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MicroExpressionFrequency = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ChaosIntensity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AsymmetryFactor = 0.1f;

    /** Lorenz attractor state for deterministic chaos */
    FVector LorenzState = FVector(0.1f, 0.0f, 0.0f);
};

// ============================================================================
// Calibration Snapshot (for undo/redo and evolution)
// ============================================================================
USTRUCT(BlueprintType)
struct FCalibrationSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    TMap<FName, FVector> NeutralPoseOffsets;

    UPROPERTY(BlueprintReadOnly)
    FAestheticOverlay AestheticState;

    UPROPERTY(BlueprintReadOnly)
    int32 EvolutionGeneration = 0;
};

// ============================================================================
// Calibration Telemetry
// ============================================================================
USTRUCT(BlueprintType)
struct FCalibrationTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EDNALayerStatus DescriptorStatus = EDNALayerStatus::NotLoaded;

    UPROPERTY(BlueprintReadOnly)
    EDNALayerStatus DefinitionStatus = EDNALayerStatus::NotLoaded;

    UPROPERTY(BlueprintReadOnly)
    EDNALayerStatus BehaviorStatus = EDNALayerStatus::NotLoaded;

    UPROPERTY(BlueprintReadOnly)
    EDNALayerStatus GeometryStatus = EDNALayerStatus::NotLoaded;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalControls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalJoints = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalBlendShapes = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 SnapshotCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EvolutionGeneration = 0;

    UPROPERTY(BlueprintReadOnly)
    float AestheticCoherence = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float ChaosLevel = 0.0f;
};

// ============================================================================
// Cognitive DNA Calibration Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UCognitiveDNACalibration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCognitiveDNACalibration();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- DNA File Operations ---

    /** Load a MetaHuman DNA file */
    UFUNCTION(BlueprintCallable, Category="DNA|File")
    bool LoadDNAFile(const FString& FilePath);

    /** Save the current calibrated DNA to file */
    UFUNCTION(BlueprintCallable, Category="DNA|File")
    bool SaveDNAFile(const FString& FilePath);

    /** Get the character descriptor */
    UFUNCTION(BlueprintPure, Category="DNA|Descriptor")
    FDNACharacterDescriptor GetDescriptor() const;

    /** Set the character descriptor */
    UFUNCTION(BlueprintCallable, Category="DNA|Descriptor")
    void SetDescriptor(const FDNACharacterDescriptor& Desc);

    // --- Calibration ---

    /** Calibrate the rig to a neutral pose with aesthetic overlay */
    UFUNCTION(BlueprintCallable, Category="DNA|Calibration")
    void CalibrateNeutralPose();

    /** Set a neutral pose offset for a specific joint */
    UFUNCTION(BlueprintCallable, Category="DNA|Calibration")
    void SetNeutralPoseOffset(FName JointName, FVector Offset);

    /** Apply the aesthetic overlay to the current rig state */
    UFUNCTION(BlueprintCallable, Category="DNA|Calibration")
    void ApplyAestheticOverlay();

    /** Get the current aesthetic overlay */
    UFUNCTION(BlueprintPure, Category="DNA|Calibration")
    FAestheticOverlay GetAestheticOverlay() const;

    /** Set the aesthetic overlay */
    UFUNCTION(BlueprintCallable, Category="DNA|Calibration")
    void SetAestheticOverlay(const FAestheticOverlay& Overlay);

    // --- Cognitive Expression Pipeline ---

    /** Process one frame of cognitive expression through the full pipeline */
    UFUNCTION(BlueprintCallable, Category="DNA|Expression")
    void ProcessCognitiveExpression(float DeltaTime);

    /** Get the final joint transforms after full pipeline processing */
    UFUNCTION(BlueprintCallable, Category="DNA|Expression")
    TArray<FRigJointTransform> GetFinalJointTransforms() const;

    /** Get the final blend shape weights after full pipeline processing */
    UFUNCTION(BlueprintCallable, Category="DNA|Expression")
    TMap<FName, float> GetFinalBlendShapeWeights() const;

    /** Get the final animated map multipliers after full pipeline processing */
    UFUNCTION(BlueprintCallable, Category="DNA|Expression")
    TMap<FName, float> GetFinalAnimatedMaps() const;

    // --- Snapshots (Undo/Redo/Evolution) ---

    /** Take a snapshot of the current calibration state */
    UFUNCTION(BlueprintCallable, Category="DNA|Snapshot")
    void TakeSnapshot(const FString& Description);

    /** Restore from a snapshot */
    UFUNCTION(BlueprintCallable, Category="DNA|Snapshot")
    bool RestoreSnapshot(int32 Index);

    /** Get all snapshots */
    UFUNCTION(BlueprintCallable, Category="DNA|Snapshot")
    TArray<FCalibrationSnapshot> GetSnapshots() const;

    // --- Evolution (Ontogenetic DNA Modification) ---

    /** Evolve the DNA based on cognitive experience */
    UFUNCTION(BlueprintCallable, Category="DNA|Evolution")
    void EvolveFromExperience(float ExperienceIntensity);

    /** Get the current evolution generation */
    UFUNCTION(BlueprintPure, Category="DNA|Evolution")
    int32 GetEvolutionGeneration() const;

    // --- Bridge Connections ---

    /** Connect to the Rig Logic Evaluator */
    UFUNCTION(BlueprintCallable, Category="DNA|Bridges")
    void ConnectRigLogic(URigLogicEvaluator* RigLogic);

    /** Connect to the Master Orchestrator */
    UFUNCTION(BlueprintCallable, Category="DNA|Bridges")
    void ConnectOrchestrator(UMasterOrchestrator* Orchestrator);

    // --- Telemetry ---

    UFUNCTION(BlueprintCallable, Category="DNA|Telemetry")
    FCalibrationTelemetry GetTelemetry() const;

protected:
    // DNA state
    FDNACharacterDescriptor Descriptor;
    EDNALayerStatus LayerStatus[4];  // Descriptor, Definition, Behavior, Geometry

    // Aesthetic overlay
    FAestheticOverlay Aesthetic;

    // Neutral pose offsets
    TMap<FName, FVector> NeutralPoseOffsets;

    // Snapshots
    TArray<FCalibrationSnapshot> Snapshots;
    int32 EvolutionGeneration;

    // Connected components
    UPROPERTY()
    URigLogicEvaluator* RigLogicRef;

    UPROPERTY()
    UMasterOrchestrator* OrchestratorRef;

    // Cached final outputs
    TArray<FRigJointTransform> FinalJoints;
    TMap<FName, float> FinalBlendShapes;
    TMap<FName, float> FinalAnimatedMaps;

    // Internal methods
    void ApplyMicroExpressions(float DeltaTime);
    void UpdateLorenzAttractor(float DeltaTime);
    FVector ComputeLorenzDerivative(const FVector& State) const;
    void ApplyNeutralPoseOffsets();
    void ApplyAestheticToAnimatedMaps();
    void AutoDiscoverBridges();

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    bool bAutoDiscoverBridges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    bool bEnableAestheticOverlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    bool bEnableMicroExpressions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    int32 MaxSnapshots = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    float EvolutionRate = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DNA|Config")
    FString DNAFilePath;
};
