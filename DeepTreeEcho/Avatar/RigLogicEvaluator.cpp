#include "RigLogicEvaluator.h"
#include "FACSToMetaHumanMapping.h"
#include "../Integration/MasterOrchestrator.h"

// ============================================================================
// Constructor
// ============================================================================
URigLogicEvaluator::URigLogicEvaluator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;  // ~60Hz evaluation
    CurrentLOD = 0;
    FACSMapping = nullptr;
    ConnectedOrchestrator = nullptr;
}

void URigLogicEvaluator::BeginPlay()
{
    Super::BeginPlay();

    // Create FACS mapping database
    FACSMapping = NewObject<UFACSToMetaHumanMapping>(this);
    FACSMapping->InitializeMappings();

    // Initialize all rig data
    InitializeRigData();

    // Auto-discover orchestrator
    if (bAutoFeedFromOrchestrator && !ConnectedOrchestrator)
    {
        if (AActor* Owner = GetOwner())
        {
            ConnectedOrchestrator = Owner->FindComponentByClass<UMasterOrchestrator>();
        }
    }
}

void URigLogicEvaluator::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Feed from orchestrator if connected
    if (bAutoFeedFromOrchestrator && ConnectedOrchestrator)
    {
        FeedFromOrchestrator();
    }

    // Run full evaluation pipeline
    Evaluate();
}

// ============================================================================
// Control Input
// ============================================================================
void URigLogicEvaluator::SetControlValue(FName ControlName, float Value)
{
    CurrentControlValues.FindOrAdd(ControlName) = FMath::Clamp(Value, 0.0f, 1.0f);
}

void URigLogicEvaluator::SetControlValues(const TMap<FName, float>& Controls)
{
    for (const auto& Pair : Controls)
    {
        CurrentControlValues.FindOrAdd(Pair.Key) = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }
}

void URigLogicEvaluator::SetControlsFromFACS(const TMap<FName, float>& FACSActivations)
{
    if (!FACSMapping) return;

    // Convert FACS AUs to CTRL_ morph targets via the mapping database
    TMap<FName, float> CtrlValues = FACSMapping->ConvertFACSToCtrl(FACSActivations);
    SetControlValues(CtrlValues);
}

void URigLogicEvaluator::ConnectToOrchestrator(UMasterOrchestrator* Orchestrator)
{
    ConnectedOrchestrator = Orchestrator;
}

// ============================================================================
// Evaluation Pipeline
// ============================================================================
void URigLogicEvaluator::Evaluate()
{
    EvaluateAtLOD(CurrentLOD);
}

void URigLogicEvaluator::EvaluateAtLOD(int32 LODLevel)
{
    CurrentLOD = FMath::Clamp(LODLevel, 0, MaxLOD);

    // Step 1: Evaluate joint groups (sparse CRS matrix → joint transforms)
    EvaluateJointGroups();

    // Step 2: Evaluate corrective expressions (fix multi-AU overlap artifacts)
    if (bEnableCorrectiveExpressions)
    {
        EvaluateCorrectiveExpressions();
    }

    // Step 3: Evaluate blend shapes (control → blend shape weight remapping)
    EvaluateBlendShapes();

    // Step 4: Evaluate animated maps (control → shader parameter multipliers)
    if (bEnableAnimatedMaps)
    {
        EvaluateAnimatedMaps();
    }
}

// ============================================================================
// Joint Group Evaluation
// ============================================================================
void URigLogicEvaluator::EvaluateJointGroups()
{
    for (int32 g = 0; g < static_cast<int32>(EJointGroup::COUNT); ++g)
    {
        EvaluateJointGroup(static_cast<EJointGroup>(g));
    }
}

void URigLogicEvaluator::EvaluateJointGroup(EJointGroup Group)
{
    const TPair<int32, int32>* Range = JointGroupRanges.Find(Group);
    if (!Range) return;

    int32 Start = Range->Key;
    int32 Count = Range->Value;

    for (int32 i = Start; i < Start + Count && i < Joints.Num(); ++i)
    {
        FRigJointTransform& Joint = Joints[i];

        // LOD filtering: skip joints below current LOD
        if (Joint.MinLOD > CurrentLOD) continue;

        // Evaluate 9 channels per joint: T.x, T.y, T.z, R.x, R.y, R.z, S.x, S.y, S.z
        // Each channel: y = sum(k_j * x_j) where k_j = rig parameter, x_j = control value
        Joint.Translation.X = EvaluateJointChannel(i, 0);
        Joint.Translation.Y = EvaluateJointChannel(i, 1);
        Joint.Translation.Z = EvaluateJointChannel(i, 2);
        Joint.Rotation.Pitch = EvaluateJointChannel(i, 3);
        Joint.Rotation.Yaw   = EvaluateJointChannel(i, 4);
        Joint.Rotation.Roll  = EvaluateJointChannel(i, 5);
        Joint.Scale.X = 1.0f + EvaluateJointChannel(i, 6);
        Joint.Scale.Y = 1.0f + EvaluateJointChannel(i, 7);
        Joint.Scale.Z = 1.0f + EvaluateJointChannel(i, 8);
    }
}

float URigLogicEvaluator::EvaluateJointChannel(int32 JointIndex, int32 Channel) const
{
    // In a full implementation, this reads from the sparse CRS matrix stored in DNA Behavior layer.
    // Here we compute a weighted sum of active controls mapped to this joint's group.
    float Result = 0.0f;

    if (JointIndex >= Joints.Num()) return Result;

    const FRigJointTransform& Joint = Joints[JointIndex];

    // Each control contributes to joints in its mapped region
    for (const auto& Ctrl : CurrentControlValues)
    {
        if (Ctrl.Value < 0.001f) continue;

        // Simplified: controls affect joints in their mapped group
        // In production, this would be a sparse matrix lookup
        float Contribution = Ctrl.Value * 0.01f;  // Small per-control contribution

        // Channel-specific scaling (rotation channels get more influence)
        if (Channel >= 3 && Channel <= 5)
        {
            Contribution *= 5.0f;  // Rotation channels
        }

        Result += Contribution;
    }

    return Result;
}

// ============================================================================
// Corrective Expression Evaluation
// ============================================================================
void URigLogicEvaluator::EvaluateCorrectiveExpressions()
{
    for (auto& Corrective : CorrectiveExpressions)
    {
        // LOD filtering
        if (Corrective.MinLOD > CurrentLOD)
        {
            Corrective.Activation = 0.0f;
            continue;
        }

        if (IsCorrectiveActive(Corrective))
        {
            // Calculate activation as the minimum of all trigger controls
            // (AND logic: all must be above threshold)
            float MinActivation = 1.0f;
            for (const auto& Trigger : Corrective.TriggerControls)
            {
                const float* CtrlValue = CurrentControlValues.Find(Trigger.Key);
                float Val = CtrlValue ? *CtrlValue : 0.0f;
                float Normalized = FMath::Clamp((Val - Trigger.Value) / (1.0f - Trigger.Value), 0.0f, 1.0f);
                MinActivation = FMath::Min(MinActivation, Normalized);
            }

            Corrective.Activation = MinActivation;
            ApplyCorrectiveDelta(Corrective);
        }
        else
        {
            Corrective.Activation = 0.0f;
        }
    }
}

bool URigLogicEvaluator::IsCorrectiveActive(const FCorrectiveExpression& Corrective) const
{
    for (const auto& Trigger : Corrective.TriggerControls)
    {
        const float* CtrlValue = CurrentControlValues.Find(Trigger.Key);
        if (!CtrlValue || *CtrlValue < Trigger.Value)
        {
            return false;
        }
    }
    return Corrective.TriggerControls.Num() > 0;
}

void URigLogicEvaluator::ApplyCorrectiveDelta(const FCorrectiveExpression& Corrective)
{
    float A = Corrective.Activation;

    // Apply joint translation deltas
    for (const auto& Delta : Corrective.JointTranslationDeltas)
    {
        for (auto& Joint : Joints)
        {
            if (Joint.JointName == Delta.Key)
            {
                Joint.Translation += Delta.Value * A;
                break;
            }
        }
    }

    // Apply joint rotation deltas
    for (const auto& Delta : Corrective.JointRotationDeltas)
    {
        for (auto& Joint : Joints)
        {
            if (Joint.JointName == Delta.Key)
            {
                Joint.Rotation.Pitch += Delta.Value.Pitch * A;
                Joint.Rotation.Yaw += Delta.Value.Yaw * A;
                Joint.Rotation.Roll += Delta.Value.Roll * A;
                break;
            }
        }
    }

    // Apply blend shape deltas
    for (const auto& Delta : Corrective.BlendShapeDeltas)
    {
        for (auto& BS : BlendShapeChannels)
        {
            if (BS.ChannelName == Delta.Key)
            {
                BS.Weight = FMath::Clamp(BS.Weight + Delta.Value * A, 0.0f, 1.0f);
                break;
            }
        }
    }
}

// ============================================================================
// Blend Shape Evaluation
// ============================================================================
void URigLogicEvaluator::EvaluateBlendShapes()
{
    for (auto& BS : BlendShapeChannels)
    {
        if (BS.MinLOD > CurrentLOD)
        {
            BS.Weight = 0.0f;
            continue;
        }

        // Simple remapping: blend shape weight = corresponding control value
        const float* CtrlValue = CurrentControlValues.Find(BS.ChannelName);
        BS.Weight = CtrlValue ? FMath::Clamp(*CtrlValue, 0.0f, 1.0f) : 0.0f;
    }
}

// ============================================================================
// Animated Map Evaluation
// ============================================================================
void URigLogicEvaluator::EvaluateAnimatedMaps()
{
    for (auto& AM : AnimatedMapChannels)
    {
        if (AM.MinLOD > CurrentLOD)
        {
            AM.Multiplier = 0.0f;
            continue;
        }

        // Animated maps are driven by control combinations
        const float* CtrlValue = CurrentControlValues.Find(AM.ChannelName);
        AM.Multiplier = CtrlValue ? FMath::Clamp(*CtrlValue, 0.0f, 1.0f) : 0.0f;
    }
}

// ============================================================================
// Output: Joints
// ============================================================================
TArray<FRigJointTransform> URigLogicEvaluator::GetJointTransforms() const
{
    TArray<FRigJointTransform> Result;
    for (const auto& J : Joints)
    {
        if (J.MinLOD <= CurrentLOD)
        {
            Result.Add(J);
        }
    }
    return Result;
}

TArray<FRigJointTransform> URigLogicEvaluator::GetJointTransformsForGroup(EJointGroup Group) const
{
    TArray<FRigJointTransform> Result;
    const TPair<int32, int32>* Range = JointGroupRanges.Find(Group);
    if (!Range) return Result;

    for (int32 i = Range->Key; i < Range->Key + Range->Value && i < Joints.Num(); ++i)
    {
        if (Joints[i].MinLOD <= CurrentLOD)
        {
            Result.Add(Joints[i]);
        }
    }
    return Result;
}

FRigJointTransform URigLogicEvaluator::GetJointTransform(FName JointName) const
{
    for (const auto& J : Joints)
    {
        if (J.JointName == JointName) return J;
    }
    return FRigJointTransform();
}

// ============================================================================
// Output: Blend Shapes
// ============================================================================
TMap<FName, float> URigLogicEvaluator::GetBlendShapeWeights() const
{
    TMap<FName, float> Result;
    for (const auto& BS : BlendShapeChannels)
    {
        if (BS.MinLOD <= CurrentLOD && BS.Weight > 0.001f)
        {
            Result.Add(BS.ChannelName, BS.Weight);
        }
    }
    return Result;
}

TMap<FName, float> URigLogicEvaluator::GetBlendShapeWeightsForMesh(FName MeshName) const
{
    TMap<FName, float> Result;
    for (const auto& BS : BlendShapeChannels)
    {
        if (BS.MeshName == MeshName && BS.MinLOD <= CurrentLOD && BS.Weight > 0.001f)
        {
            Result.Add(BS.ChannelName, BS.Weight);
        }
    }
    return Result;
}

// ============================================================================
// Output: Animated Maps
// ============================================================================
TMap<FName, float> URigLogicEvaluator::GetAnimatedMapMultipliers() const
{
    TMap<FName, float> Result;
    for (const auto& AM : AnimatedMapChannels)
    {
        if (AM.MinLOD <= CurrentLOD && AM.Multiplier > 0.001f)
        {
            Result.Add(AM.ChannelName, AM.Multiplier);
        }
    }
    return Result;
}

// ============================================================================
// LOD Management
// ============================================================================
void URigLogicEvaluator::SetLOD(int32 LODLevel)
{
    CurrentLOD = FMath::Clamp(LODLevel, 0, MaxLOD);
}

int32 URigLogicEvaluator::GetCurrentLOD() const { return CurrentLOD; }

int32 URigLogicEvaluator::GetJointCountAtLOD(int32 LODLevel) const
{
    int32 Count = 0;
    for (const auto& J : Joints)
    {
        if (J.MinLOD <= LODLevel) Count++;
    }
    return Count;
}

// ============================================================================
// Corrective Expressions
// ============================================================================
TArray<FCorrectiveExpression> URigLogicEvaluator::GetActiveCorrectiveExpressions() const
{
    TArray<FCorrectiveExpression> Active;
    for (const auto& C : CorrectiveExpressions)
    {
        if (C.Activation > 0.01f) Active.Add(C);
    }
    return Active;
}

int32 URigLogicEvaluator::GetCorrectiveExpressionCount() const
{
    return CorrectiveExpressions.Num();
}

// ============================================================================
// Telemetry
// ============================================================================
FRigLogicTelemetry URigLogicEvaluator::GetTelemetry() const
{
    FRigLogicTelemetry T;
    T.CurrentLOD = CurrentLOD;
    T.ActiveJoints = GetJointCountAtLOD(CurrentLOD);
    T.TotalControlInputs = CurrentControlValues.Num();

    int32 ActiveBS = 0;
    for (const auto& BS : BlendShapeChannels)
        if (BS.MinLOD <= CurrentLOD && BS.Weight > 0.001f) ActiveBS++;
    T.ActiveBlendShapes = ActiveBS;

    int32 ActiveAM = 0;
    for (const auto& AM : AnimatedMapChannels)
        if (AM.MinLOD <= CurrentLOD && AM.Multiplier > 0.001f) ActiveAM++;
    T.ActiveAnimatedMaps = ActiveAM;

    int32 ActiveCorr = 0;
    for (const auto& C : CorrectiveExpressions)
        if (C.Activation > 0.01f) ActiveCorr++;
    T.ActiveCorrectiveExpressions = ActiveCorr;

    float MaxCtrl = 0.0f;
    for (const auto& P : CurrentControlValues)
        MaxCtrl = FMath::Max(MaxCtrl, P.Value);
    T.MaxControlValue = MaxCtrl;

    return T;
}

UFACSToMetaHumanMapping* URigLogicEvaluator::GetFACSMapping() const { return FACSMapping; }

// ============================================================================
// Orchestrator Feed
// ============================================================================
void URigLogicEvaluator::FeedFromOrchestrator()
{
    if (!ConnectedOrchestrator) return;

    TMap<FName, float> AUs = ConnectedOrchestrator->GetFinalExpressionAUs();
    SetControlsFromFACS(AUs);
}

// ============================================================================
// Initialization
// ============================================================================
void URigLogicEvaluator::InitializeRigData()
{
    InitializeJoints();
    InitializeBlendShapes();
    InitializeAnimatedMaps();
    InitializeCorrectiveExpressions();
}

void URigLogicEvaluator::InitializeJoints()
{
    // Initialize ~550 joints across 9 groups (representative subset of ~800 full MetaHuman)
    auto AddJointGroup = [this](EJointGroup Group, const FString& Prefix, int32 Count, int32 MinLOD)
    {
        int32 Start = Joints.Num();
        for (int32 i = 0; i < Count; ++i)
        {
            FRigJointTransform J;
            J.JointName = FName(*FString::Printf(TEXT("%s_%03d"), *Prefix, i));
            J.Group = Group;
            J.MinLOD = (i < Count / 2) ? 0 : FMath::Min(MinLOD + (i * 2 / Count), 7);
            J.ParentIndex = (i > 0) ? Start + i - 1 : -1;
            Joints.Add(J);
        }
        JointGroupRanges.Add(Group, TPair<int32, int32>(Start, Count));
    };

    AddJointGroup(EJointGroup::Forehead,   TEXT("jnt_forehead"),  60, 1);
    AddJointGroup(EJointGroup::EyeLeft,    TEXT("jnt_eyeL"),      80, 1);
    AddJointGroup(EJointGroup::EyeRight,   TEXT("jnt_eyeR"),      80, 1);
    AddJointGroup(EJointGroup::Nose,       TEXT("jnt_nose"),       40, 2);
    AddJointGroup(EJointGroup::Jaw,        TEXT("jnt_jaw"),        30, 1);
    AddJointGroup(EJointGroup::Mouth,      TEXT("jnt_mouth"),     120, 0);
    AddJointGroup(EJointGroup::CheekLeft,  TEXT("jnt_cheekL"),     50, 2);
    AddJointGroup(EJointGroup::CheekRight, TEXT("jnt_cheekR"),     50, 2);
    AddJointGroup(EJointGroup::Neck,       TEXT("jnt_neck"),       40, 3);
}

void URigLogicEvaluator::InitializeBlendShapes()
{
    auto AddBS = [this](const FName& Name, int32 MinLOD, const FName& Mesh)
    {
        FRigBlendShapeChannel BS;
        BS.ChannelName = Name;
        BS.MinLOD = MinLOD;
        BS.MeshName = Mesh;
        BlendShapeChannels.Add(BS);
    };

    FName HeadMesh = FName("head_lod0_mesh");

    // Core expression blend shapes (LOD 0)
    AddBS(FName("CTRL_expressions_browRaiseInnerL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_browRaiseInnerR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_browRaiseOuterL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_browRaiseOuterR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_browDownL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_browDownR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeBlinkL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeBlinkR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeWidenL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeWidenR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeSquintInnerL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeSquintInnerR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeCheekRaiseL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_eyeCheekRaiseR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthCornerPullL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthCornerPullR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthCornerDepressL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthCornerDepressR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthStretchL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthStretchR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_jawOpen"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_jawLeft"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_jawRight"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthLipsPurse"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthLipsPart"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthUpperLipRaiseL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthUpperLipRaiseR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthLowerLipDepressL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_mouthLowerLipDepressR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_noseWrinkleL"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_noseWrinkleR"), 0, HeadMesh);
    AddBS(FName("CTRL_expressions_noseNostrilDilateL"), 1, HeadMesh);
    AddBS(FName("CTRL_expressions_noseNostrilDilateR"), 1, HeadMesh);
    AddBS(FName("CTRL_expressions_chinRaise"), 1, HeadMesh);
    AddBS(FName("CTRL_expressions_neckStretchL"), 2, HeadMesh);
    AddBS(FName("CTRL_expressions_neckStretchR"), 2, HeadMesh);
}

void URigLogicEvaluator::InitializeAnimatedMaps()
{
    auto AddAM = [this](const FName& Name, int32 MinLOD, const FName& ShaderParam)
    {
        FRigAnimatedMapChannel AM;
        AM.ChannelName = Name;
        AM.MinLOD = MinLOD;
        AM.ShaderParameterName = ShaderParam;
        AnimatedMapChannels.Add(AM);
    };

    // Wrinkle maps
    AddAM(FName("wrinkle_forehead_horizontal"), 0, FName("WrinkleForeheadH"));
    AddAM(FName("wrinkle_forehead_vertical"), 0, FName("WrinkleForeheadV"));
    AddAM(FName("wrinkle_glabella"), 0, FName("WrinkleGlabella"));
    AddAM(FName("wrinkle_nasolabialL"), 0, FName("WrinkleNasolabialL"));
    AddAM(FName("wrinkle_nasolabialR"), 0, FName("WrinkleNasolabialR"));
    AddAM(FName("wrinkle_crowsfeetL"), 1, FName("WrinkleCrowsFeetL"));
    AddAM(FName("wrinkle_crowsfeetR"), 1, FName("WrinkleCrowsFeetR"));
    AddAM(FName("wrinkle_chin"), 2, FName("WrinkleChin"));
    AddAM(FName("wrinkle_neckL"), 3, FName("WrinkleNeckL"));
    AddAM(FName("wrinkle_neckR"), 3, FName("WrinkleNeckR"));

    // Subsurface blood flow
    AddAM(FName("bloodflow_cheekL"), 1, FName("BloodFlowCheekL"));
    AddAM(FName("bloodflow_cheekR"), 1, FName("BloodFlowCheekR"));
    AddAM(FName("bloodflow_nose"), 2, FName("BloodFlowNose"));
    AddAM(FName("bloodflow_forehead"), 2, FName("BloodFlowForehead"));
    AddAM(FName("bloodflow_lips"), 1, FName("BloodFlowLips"));
}

void URigLogicEvaluator::InitializeCorrectiveExpressions()
{
    auto AddCorrective = [this](const FString& Name, int32 MinLOD,
                                 TMap<FName, float> Triggers,
                                 TMap<FName, FVector> TransDeltas = {},
                                 TMap<FName, FRotator> RotDeltas = {},
                                 TMap<FName, float> BSDeltas = {})
    {
        FCorrectiveExpression C;
        C.Name = Name;
        C.MinLOD = MinLOD;
        C.TriggerControls = Triggers;
        C.JointTranslationDeltas = TransDeltas;
        C.JointRotationDeltas = RotDeltas;
        C.BlendShapeDeltas = BSDeltas;
        CorrectiveExpressions.Add(C);
    };

    // Jaw + Mouth correctives
    AddCorrective(TEXT("jawOpen_mouthCornerPull"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.3f},
         {FName("CTRL_expressions_mouthCornerPullL"), 0.3f}},
        {{FName("jnt_mouth_010"), FVector(0.0f, -0.05f, 0.02f)}},
        {},
        {{FName("CTRL_expressions_mouthCornerPullL"), -0.15f}});

    AddCorrective(TEXT("jawOpen_mouthPurse"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.4f},
         {FName("CTRL_expressions_mouthLipsPurse"), 0.3f}},
        {{FName("jnt_mouth_005"), FVector(0.0f, 0.03f, -0.01f)}});

    AddCorrective(TEXT("jawOpen_mouthStretch"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.5f},
         {FName("CTRL_expressions_mouthStretchL"), 0.3f}},
        {{FName("jnt_jaw_005"), FVector(0.0f, -0.02f, 0.01f)}});

    // Eye + Brow correctives
    AddCorrective(TEXT("browDown_squint"), 1,
        {{FName("CTRL_expressions_browDownL"), 0.3f},
         {FName("CTRL_expressions_eyeSquintInnerL"), 0.3f}},
        {{FName("jnt_forehead_020"), FVector(0.0f, 0.02f, -0.01f)}});

    AddCorrective(TEXT("browRaise_eyeWiden"), 1,
        {{FName("CTRL_expressions_browRaiseInnerL"), 0.4f},
         {FName("CTRL_expressions_eyeWidenL"), 0.3f}},
        {{FName("jnt_forehead_005"), FVector(0.0f, 0.01f, 0.02f)}});

    AddCorrective(TEXT("blink_cheekRaise"), 0,
        {{FName("CTRL_expressions_eyeBlinkL"), 0.5f},
         {FName("CTRL_expressions_eyeCheekRaiseL"), 0.3f}},
        {{FName("jnt_eyeL_030"), FVector(0.0f, -0.01f, 0.01f)}});

    // Nose + Mouth correctives
    AddCorrective(TEXT("noseWrinkle_upperLipRaise"), 1,
        {{FName("CTRL_expressions_noseWrinkleL"), 0.3f},
         {FName("CTRL_expressions_mouthUpperLipRaiseL"), 0.3f}},
        {{FName("jnt_nose_010"), FVector(0.0f, 0.01f, -0.005f)}},
        {},
        {{FName("CTRL_expressions_noseWrinkleL"), -0.1f}});

    AddCorrective(TEXT("noseWrinkle_smile"), 1,
        {{FName("CTRL_expressions_noseWrinkleL"), 0.3f},
         {FName("CTRL_expressions_mouthCornerPullL"), 0.4f}},
        {{FName("jnt_cheekL_010"), FVector(0.0f, -0.02f, 0.01f)}});

    // Complex multi-region correctives
    AddCorrective(TEXT("angryShout"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.5f},
         {FName("CTRL_expressions_mouthCornerPullL"), 0.4f},
         {FName("CTRL_expressions_noseWrinkleL"), 0.3f},
         {FName("CTRL_expressions_browDownL"), 0.3f}},
        {{FName("jnt_mouth_050"), FVector(0.0f, -0.03f, 0.02f)},
         {FName("jnt_nose_015"), FVector(0.0f, 0.01f, -0.01f)}});

    AddCorrective(TEXT("exuberantLaugh"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.4f},
         {FName("CTRL_expressions_mouthCornerPullL"), 0.5f},
         {FName("CTRL_expressions_eyeCheekRaiseL"), 0.4f},
         {FName("CTRL_expressions_eyeSquintInnerL"), 0.3f}},
        {{FName("jnt_cheekL_020"), FVector(0.0f, -0.02f, 0.015f)}},
        {},
        {{FName("CTRL_expressions_mouthCornerPullL"), -0.1f}});

    AddCorrective(TEXT("terrifiedScream"), 0,
        {{FName("CTRL_expressions_jawOpen"), 0.6f},
         {FName("CTRL_expressions_eyeWidenL"), 0.5f},
         {FName("CTRL_expressions_browRaiseInnerL"), 0.4f},
         {FName("CTRL_expressions_mouthStretchL"), 0.3f}},
        {{FName("jnt_mouth_080"), FVector(0.0f, -0.04f, 0.03f)},
         {FName("jnt_jaw_010"), FVector(0.0f, -0.02f, 0.01f)}});
}
