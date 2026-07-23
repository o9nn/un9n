#pragma once
/**
 * @file RigLogicEvaluator.h
 * @brief Runtime Rig Logic Evaluation Engine for Deep Tree Echo
 *
 * Implements the complete MetaHuman Rig Logic evaluation pipeline:
 *
 *   FACS AUs → GUI Controls → Corrective Expressions → [Joints + BlendShapes + AnimatedMaps]
 *
 * Based on the 3Lateral/Epic Games Rig Logic whitepaper (v2, 2022).
 *
 * Key features:
 * - Joint group-based sparse matrix evaluation (8 facial regions)
 * - Corrective expression network for multi-AU combinations
 * - 8-level LOD system with zero-cost switching
 * - SIMD-friendly dense submatrix layout within joint groups
 * - Animated map multipliers for wrinkles and subsurface blood flow
 * - Direct integration with MasterOrchestrator AU output
 *
 * Pipeline: MasterOrchestrator → GetFinalExpressionAUs() → RigLogicEvaluator → Mesh Deformation
 *
 * @see rig-logic skill for whitepaper reference
 * @see FACSToMetaHumanMapping for AU↔CTRL_ bidirectional lookup
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RigLogicEvaluator.generated.h"

// Forward declarations
class UFACSToMetaHumanMapping;
class UMasterOrchestrator;

// ============================================================================
// Joint Group Definition (8 facial regions)
// ============================================================================
UENUM(BlueprintType)
enum class EJointGroup : uint8
{
    Forehead = 0,   // Brow, forehead wrinkles (~60 joints)
    EyeLeft,        // Left eyelids, eye socket (~80 joints)
    EyeRight,       // Right eyelids, eye socket (~80 joints)
    Nose,           // Nasal bridge, nostrils, nasolabial (~40 joints)
    Jaw,            // Jaw bone, chin (~30 joints)
    Mouth,          // Lips, oral cavity (~120 joints)
    CheekLeft,      // Left cheek, zygomatic (~50 joints)
    CheekRight,     // Right cheek, zygomatic (~50 joints)
    Neck,           // Platysma, neck muscles (~40 joints)
    COUNT
};

// ============================================================================
// Joint Transform (9 channels per joint: T.xyz + R.xyz + S.xyz)
// ============================================================================
USTRUCT(BlueprintType)
struct FRigJointTransform
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Translation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector::OneVector;

    /** Joint name in the MetaHuman skeleton */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName JointName;

    /** Parent joint index (-1 for root) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ParentIndex = -1;

    /** Which joint group this belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EJointGroup Group = EJointGroup::Forehead;

    /** Minimum LOD level at which this joint is active (0 = always) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLOD = 0;
};

// ============================================================================
// Corrective Expression Definition
// ============================================================================
USTRUCT(BlueprintType)
struct FCorrectiveExpression
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    /** Controls that must be active for this corrective to fire */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> TriggerControls;  // ControlName → MinThreshold

    /** Joint correction deltas (applied additively) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FVector> JointTranslationDeltas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FRotator> JointRotationDeltas;

    /** Blend shape correction deltas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> BlendShapeDeltas;

    /** Current activation level [0,1] */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Activation = 0.0f;

    /** Minimum LOD for this corrective */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLOD = 0;
};

// ============================================================================
// Blend Shape Channel
// ============================================================================
USTRUCT(BlueprintType)
struct FRigBlendShapeChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChannelName;

    /** Current weight [0,1] */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Weight = 0.0f;

    /** Minimum LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLOD = 0;

    /** Which mesh this blend shape belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MeshName;
};

// ============================================================================
// Animated Map Channel (wrinkles, blood flow, etc.)
// ============================================================================
USTRUCT(BlueprintType)
struct FRigAnimatedMapChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChannelName;

    /** Current multiplier [0,1] */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Multiplier = 0.0f;

    /** Minimum LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLOD = 0;

    /** Shader parameter name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ShaderParameterName;
};

// ============================================================================
// Rig Logic Evaluation Telemetry
// ============================================================================
USTRUCT(BlueprintType)
struct FRigLogicTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentLOD = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveJoints = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveBlendShapes = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAnimatedMaps = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveCorrectiveExpressions = 0;

    UPROPERTY(BlueprintReadOnly)
    float EvaluationTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalControlInputs = 0;

    UPROPERTY(BlueprintReadOnly)
    float MaxControlValue = 0.0f;
};

// ============================================================================
// Rig Logic Evaluator Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class URigLogicEvaluator : public UActorComponent
{
    GENERATED_BODY()

public:
    URigLogicEvaluator();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- Control Input ---

    /** Set a single GUI control value (FACS AU mapped to CTRL_) */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Input")
    void SetControlValue(FName ControlName, float Value);

    /** Set multiple control values at once (batch update) */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Input")
    void SetControlValues(const TMap<FName, float>& Controls);

    /** Set controls from FACS AU activations (auto-converts via FACSToMetaHumanMapping) */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Input")
    void SetControlsFromFACS(const TMap<FName, float>& FACSActivations);

    /** Connect to MasterOrchestrator for automatic AU feeding */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Input")
    void ConnectToOrchestrator(UMasterOrchestrator* Orchestrator);

    // --- Evaluation ---

    /** Evaluate the full rig logic pipeline (called automatically each tick) */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Evaluate")
    void Evaluate();

    /** Force evaluation at a specific LOD */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Evaluate")
    void EvaluateAtLOD(int32 LODLevel);

    // --- Output: Joints ---

    /** Get all joint transforms for the current LOD */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    TArray<FRigJointTransform> GetJointTransforms() const;

    /** Get joint transforms for a specific joint group */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    TArray<FRigJointTransform> GetJointTransformsForGroup(EJointGroup Group) const;

    /** Get a single joint transform by name */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    FRigJointTransform GetJointTransform(FName JointName) const;

    // --- Output: Blend Shapes ---

    /** Get all blend shape weights for the current LOD */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    TMap<FName, float> GetBlendShapeWeights() const;

    /** Get blend shape weights for a specific mesh */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    TMap<FName, float> GetBlendShapeWeightsForMesh(FName MeshName) const;

    // --- Output: Animated Maps ---

    /** Get all animated map multipliers for the current LOD */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Output")
    TMap<FName, float> GetAnimatedMapMultipliers() const;

    // --- LOD Management ---

    /** Set the current LOD level (0 = maximum, 7 = minimum) */
    UFUNCTION(BlueprintCallable, Category="RigLogic|LOD")
    void SetLOD(int32 LODLevel);

    /** Get the current LOD level */
    UFUNCTION(BlueprintPure, Category="RigLogic|LOD")
    int32 GetCurrentLOD() const;

    /** Get joint count at a specific LOD */
    UFUNCTION(BlueprintPure, Category="RigLogic|LOD")
    int32 GetJointCountAtLOD(int32 LODLevel) const;

    // --- Corrective Expressions ---

    /** Get all currently active corrective expressions */
    UFUNCTION(BlueprintCallable, Category="RigLogic|Correctives")
    TArray<FCorrectiveExpression> GetActiveCorrectiveExpressions() const;

    /** Get the total number of corrective expressions */
    UFUNCTION(BlueprintPure, Category="RigLogic|Correctives")
    int32 GetCorrectiveExpressionCount() const;

    // --- Telemetry ---

    UFUNCTION(BlueprintCallable, Category="RigLogic|Telemetry")
    FRigLogicTelemetry GetTelemetry() const;

    // --- FACS Mapping Reference ---

    /** Get the FACS-to-MetaHuman mapping database */
    UFUNCTION(BlueprintPure, Category="RigLogic|Mapping")
    UFACSToMetaHumanMapping* GetFACSMapping() const;

protected:
    // State
    int32 CurrentLOD;
    TMap<FName, float> CurrentControlValues;

    // Rig data
    TArray<FRigJointTransform> Joints;
    TArray<FRigBlendShapeChannel> BlendShapeChannels;
    TArray<FRigAnimatedMapChannel> AnimatedMapChannels;
    TArray<FCorrectiveExpression> CorrectiveExpressions;

    // Joint group index ranges (start, count) for fast group lookup
    TMap<EJointGroup, TPair<int32, int32>> JointGroupRanges;

    // Mapping database
    UPROPERTY()
    UFACSToMetaHumanMapping* FACSMapping;

    // Connected orchestrator
    UPROPERTY()
    UMasterOrchestrator* ConnectedOrchestrator;

    // Internal evaluation methods
    void InitializeRigData();
    void InitializeJoints();
    void InitializeBlendShapes();
    void InitializeAnimatedMaps();
    void InitializeCorrectiveExpressions();

    void EvaluateJointGroups();
    void EvaluateCorrectiveExpressions();
    void EvaluateBlendShapes();
    void EvaluateAnimatedMaps();

    void EvaluateJointGroup(EJointGroup Group);
    float EvaluateJointChannel(int32 JointIndex, int32 Channel) const;

    bool IsCorrectiveActive(const FCorrectiveExpression& Corrective) const;
    void ApplyCorrectiveDelta(const FCorrectiveExpression& Corrective);

    void FeedFromOrchestrator();

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RigLogic|Config")
    bool bAutoFeedFromOrchestrator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RigLogic|Config")
    float MinTransitionTime = 0.15f;  // Minimum seconds for expression transitions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RigLogic|Config")
    bool bEnableCorrectiveExpressions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RigLogic|Config")
    bool bEnableAnimatedMaps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RigLogic|Config")
    int32 MaxLOD = 7;
};
