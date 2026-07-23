#pragma once
/**
 * @file CognitiveLODManager.h
 * @brief Cognitive LOD Manager for Deep Tree Echo
 *
 * Intelligent Level of Detail management that considers:
 * 1. Camera distance (standard LOD)
 * 2. Cognitive importance (ECAN attention value)
 * 3. Expression complexity (active AU count)
 * 4. Performance budget (target frame time)
 * 5. Social context (conversation partner proximity)
 *
 * LOD 0 = Maximum detail (~800 joints, all correctives, all animated maps)
 * LOD 7 = Minimum detail (~50 joints, no correctives, no animated maps)
 *
 * Integrates with RigLogicEvaluator to dynamically adjust evaluation complexity.
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CognitiveLODManager.generated.h"

class URigLogicEvaluator;
class UCognitiveDNACalibration;
class UMasterOrchestrator;

// ============================================================================
// LOD Level Statistics
// ============================================================================
USTRUCT(BlueprintType)
struct FLODLevelStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 LODLevel = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 JointCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 BlendShapeCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 AnimatedMapCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CorrectiveCount = 0;

    UPROPERTY(BlueprintReadOnly)
    float EstimatedEvalTimeMs = 0.0f;
};

// ============================================================================
// LOD Selection Criteria
// ============================================================================
USTRUCT(BlueprintType)
struct FLODSelectionCriteria
{
    GENERATED_BODY()

    /** Camera distance to avatar (world units) */
    UPROPERTY(BlueprintReadOnly)
    float CameraDistance = 100.0f;

    /** ECAN attention value from OpenCog [0,1] */
    UPROPERTY(BlueprintReadOnly)
    float CognitiveImportance = 0.5f;

    /** Number of currently active AUs */
    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAUCount = 0;

    /** Whether avatar is in conversation */
    UPROPERTY(BlueprintReadOnly)
    bool bInConversation = false;

    /** Current frame time budget remaining (ms) */
    UPROPERTY(BlueprintReadOnly)
    float FrameBudgetRemainingMs = 16.0f;

    /** Computed LOD recommendation */
    UPROPERTY(BlueprintReadOnly)
    int32 RecommendedLOD = 0;
};

// ============================================================================
// Cognitive LOD Manager Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UCognitiveLODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCognitiveLODManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    /** Get the current recommended LOD */
    UFUNCTION(BlueprintPure, Category="LOD")
    int32 GetCurrentLOD() const;

    /** Force a specific LOD (overrides automatic selection) */
    UFUNCTION(BlueprintCallable, Category="LOD")
    void ForceLOD(int32 LODLevel);

    /** Release forced LOD and return to automatic selection */
    UFUNCTION(BlueprintCallable, Category="LOD")
    void ReleaseForce();

    /** Get statistics for a specific LOD level */
    UFUNCTION(BlueprintCallable, Category="LOD")
    FLODLevelStats GetLODStats(int32 LODLevel) const;

    /** Get the current selection criteria */
    UFUNCTION(BlueprintPure, Category="LOD")
    FLODSelectionCriteria GetSelectionCriteria() const;

    /** Set camera distance (called by camera system) */
    UFUNCTION(BlueprintCallable, Category="LOD")
    void SetCameraDistance(float Distance);

    /** Set cognitive importance (called by ECAN bridge) */
    UFUNCTION(BlueprintCallable, Category="LOD")
    void SetCognitiveImportance(float Importance);

    /** Set conversation state */
    UFUNCTION(BlueprintCallable, Category="LOD")
    void SetInConversation(bool bConversing);

protected:
    int32 CurrentLOD;
    bool bForcedLOD;
    int32 ForcedLODLevel;

    FLODSelectionCriteria Criteria;

    // Connected components
    UPROPERTY()
    URigLogicEvaluator* RigLogicRef;

    UPROPERTY()
    UCognitiveDNACalibration* CalibrationRef;

    UPROPERTY()
    UMasterOrchestrator* OrchestratorRef;

    int32 ComputeOptimalLOD() const;
    void ApplyLOD(int32 LODLevel);
    void AutoDiscoverComponents();

    // LOD distance thresholds (world units)
    TArray<float> DistanceThresholds;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config")
    bool bAutoManageLOD = true;

    /** Weight for camera distance in LOD calculation [0,1] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DistanceWeight = 0.4f;

    /** Weight for cognitive importance in LOD calculation [0,1] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CognitiveWeight = 0.3f;

    /** Weight for expression complexity in LOD calculation [0,1] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ExpressionWeight = 0.2f;

    /** Weight for performance budget in LOD calculation [0,1] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PerformanceWeight = 0.1f;

    /** Hysteresis range to prevent LOD flickering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config")
    float HysteresisRange = 50.0f;

    /** Minimum time between LOD changes (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config")
    float MinLODChangeInterval = 0.5f;

    /** Target frame time budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LOD|Config")
    float TargetFrameTimeMs = 16.67f;
};
