#include "CognitiveLODManager.h"
#include "RigLogicEvaluator.h"
#include "CognitiveDNACalibration.h"
#include "../Integration/MasterOrchestrator.h"

UCognitiveLODManager::UCognitiveLODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;  // 10Hz LOD evaluation (no need for 60Hz)

    CurrentLOD = 0;
    bForcedLOD = false;
    ForcedLODLevel = 0;
    RigLogicRef = nullptr;
    CalibrationRef = nullptr;
    OrchestratorRef = nullptr;

    // Default distance thresholds for 8 LOD levels
    DistanceThresholds = { 0.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
}

void UCognitiveLODManager::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverComponents();
}

void UCognitiveLODManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAutoManageLOD || bForcedLOD) return;

    // Update active AU count from orchestrator
    if (OrchestratorRef)
    {
        TMap<FName, float> AUs = OrchestratorRef->GetFinalExpressionAUs();
        int32 ActiveCount = 0;
        for (const auto& P : AUs)
        {
            if (P.Value > 0.05f) ActiveCount++;
        }
        Criteria.ActiveAUCount = ActiveCount;
    }

    // Compute and apply optimal LOD
    int32 Optimal = ComputeOptimalLOD();
    if (Optimal != CurrentLOD)
    {
        ApplyLOD(Optimal);
    }
}

int32 UCognitiveLODManager::ComputeOptimalLOD() const
{
    // Distance-based LOD (0-7)
    int32 DistanceLOD = 7;
    for (int32 i = 0; i < DistanceThresholds.Num() - 1; ++i)
    {
        float Low = DistanceThresholds[i] - (i > 0 ? HysteresisRange : 0.0f);
        float High = DistanceThresholds[i + 1] + HysteresisRange;
        if (Criteria.CameraDistance >= Low && Criteria.CameraDistance < High)
        {
            DistanceLOD = i;
            break;
        }
    }

    // Cognitive importance boost (high importance → lower LOD number → more detail)
    float CogBoost = Criteria.CognitiveImportance * 3.0f;  // Can reduce LOD by up to 3 levels

    // Expression complexity boost (more active AUs → more detail needed)
    float ExprBoost = FMath::Clamp(Criteria.ActiveAUCount / 10.0f, 0.0f, 2.0f);

    // Conversation boost (always high detail in conversation)
    float ConvBoost = Criteria.bInConversation ? 3.0f : 0.0f;

    // Performance penalty (low budget → higher LOD number → less detail)
    float PerfPenalty = 0.0f;
    if (Criteria.FrameBudgetRemainingMs < 2.0f)
    {
        PerfPenalty = 3.0f;  // Severe budget pressure
    }
    else if (Criteria.FrameBudgetRemainingMs < 5.0f)
    {
        PerfPenalty = 1.0f;  // Moderate budget pressure
    }

    // Weighted combination
    float FinalLOD = static_cast<float>(DistanceLOD)
        - CogBoost * CognitiveWeight
        - ExprBoost * ExpressionWeight
        - ConvBoost
        + PerfPenalty * PerformanceWeight;

    return FMath::Clamp(FMath::RoundToInt(FinalLOD), 0, 7);
}

void UCognitiveLODManager::ApplyLOD(int32 LODLevel)
{
    CurrentLOD = FMath::Clamp(LODLevel, 0, 7);
    Criteria.RecommendedLOD = CurrentLOD;

    if (RigLogicRef)
    {
        RigLogicRef->SetLOD(CurrentLOD);
    }
}

// ============================================================================
// Public Interface
// ============================================================================
int32 UCognitiveLODManager::GetCurrentLOD() const
{
    return bForcedLOD ? ForcedLODLevel : CurrentLOD;
}

void UCognitiveLODManager::ForceLOD(int32 LODLevel)
{
    bForcedLOD = true;
    ForcedLODLevel = FMath::Clamp(LODLevel, 0, 7);
    ApplyLOD(ForcedLODLevel);
}

void UCognitiveLODManager::ReleaseForce()
{
    bForcedLOD = false;
}

FLODLevelStats UCognitiveLODManager::GetLODStats(int32 LODLevel) const
{
    FLODLevelStats Stats;
    Stats.LODLevel = LODLevel;

    if (RigLogicRef)
    {
        Stats.JointCount = RigLogicRef->GetJointCountAtLOD(LODLevel);
    }

    // Estimated evaluation time (rough model)
    Stats.EstimatedEvalTimeMs = Stats.JointCount * 0.002f
                               + Stats.BlendShapeCount * 0.001f
                               + Stats.CorrectiveCount * 0.005f;

    return Stats;
}

FLODSelectionCriteria UCognitiveLODManager::GetSelectionCriteria() const { return Criteria; }

void UCognitiveLODManager::SetCameraDistance(float Distance)
{
    Criteria.CameraDistance = FMath::Max(Distance, 0.0f);
}

void UCognitiveLODManager::SetCognitiveImportance(float Importance)
{
    Criteria.CognitiveImportance = FMath::Clamp(Importance, 0.0f, 1.0f);
}

void UCognitiveLODManager::SetInConversation(bool bConversing)
{
    Criteria.bInConversation = bConversing;
}

void UCognitiveLODManager::AutoDiscoverComponents()
{
    if (AActor* Owner = GetOwner())
    {
        RigLogicRef = Owner->FindComponentByClass<URigLogicEvaluator>();
        CalibrationRef = Owner->FindComponentByClass<UCognitiveDNACalibration>();
        OrchestratorRef = Owner->FindComponentByClass<UMasterOrchestrator>();
    }
}
