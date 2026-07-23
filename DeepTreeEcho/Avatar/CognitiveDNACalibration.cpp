#include "CognitiveDNACalibration.h"
#include "../Integration/MasterOrchestrator.h"

// ============================================================================
// Constructor
// ============================================================================
UCognitiveDNACalibration::UCognitiveDNACalibration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;

    RigLogicRef = nullptr;
    OrchestratorRef = nullptr;
    EvolutionGeneration = 0;

    for (int i = 0; i < 4; ++i)
        LayerStatus[i] = EDNALayerStatus::NotLoaded;

    // Default descriptor for Deep Tree Echo
    Descriptor.CharacterName = TEXT("DeepTreeEcho");
    Descriptor.Archetype = TEXT("CognitiveAvatar");
    Descriptor.Age = 25;
    Descriptor.Gender = TEXT("Female");
    Descriptor.LODCount = 8;
    Descriptor.Metadata.Add(TEXT("Creator"), TEXT("Marduk"));
    Descriptor.Metadata.Add(TEXT("Architecture"), TEXT("HyperSkillTensor"));
    Descriptor.Metadata.Add(TEXT("Composition"), TEXT("SuperHot ⊗ EchoAngel ⊗ Madness"));
}

void UCognitiveDNACalibration::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoDiscoverBridges)
    {
        AutoDiscoverBridges();
    }

    // Load DNA file if path specified
    if (!DNAFilePath.IsEmpty())
    {
        LoadDNAFile(DNAFilePath);
    }
}

void UCognitiveDNACalibration::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ProcessCognitiveExpression(DeltaTime);
}

// ============================================================================
// DNA File Operations
// ============================================================================
bool UCognitiveDNACalibration::LoadDNAFile(const FString& FilePath)
{
    // In production, this would use the DNACalib BinaryStreamReader
    // For now, we initialize with default MetaHuman-compatible data

    LayerStatus[0] = EDNALayerStatus::Loading;  // Descriptor
    LayerStatus[1] = EDNALayerStatus::Loading;  // Definition
    LayerStatus[2] = EDNALayerStatus::Loading;  // Behavior
    LayerStatus[3] = EDNALayerStatus::Loading;  // Geometry

    // Simulate successful load
    LayerStatus[0] = EDNALayerStatus::Loaded;
    LayerStatus[1] = EDNALayerStatus::Loaded;
    LayerStatus[2] = EDNALayerStatus::Loaded;
    LayerStatus[3] = EDNALayerStatus::Loaded;

    CalibrateNeutralPose();
    TakeSnapshot(TEXT("Initial DNA Load"));

    return true;
}

bool UCognitiveDNACalibration::SaveDNAFile(const FString& FilePath)
{
    // In production, this would use DNACalib BinaryStreamWriter
    // to serialize the current calibrated state
    return true;
}

FDNACharacterDescriptor UCognitiveDNACalibration::GetDescriptor() const { return Descriptor; }
void UCognitiveDNACalibration::SetDescriptor(const FDNACharacterDescriptor& Desc) { Descriptor = Desc; }

// ============================================================================
// Calibration
// ============================================================================
void UCognitiveDNACalibration::CalibrateNeutralPose()
{
    // Set aesthetic-influenced neutral offsets
    // These create the "resting face" that reflects the SuperHotGirl aesthetic

    // Subtle smile at rest (Mona Lisa effect)
    NeutralPoseOffsets.Add(FName("jnt_mouth_010"), FVector(0.0f, 0.005f, 0.002f));
    NeutralPoseOffsets.Add(FName("jnt_mouth_110"), FVector(0.0f, 0.005f, 0.002f));

    // Slightly raised brows (alert, engaged look)
    NeutralPoseOffsets.Add(FName("jnt_forehead_005"), FVector(0.0f, 0.003f, 0.001f));
    NeutralPoseOffsets.Add(FName("jnt_forehead_035"), FVector(0.0f, 0.003f, 0.001f));

    // Confident jaw position
    NeutralPoseOffsets.Add(FName("jnt_jaw_000"), FVector(0.0f, -0.001f, 0.002f));

    // Wide, alert eyes
    NeutralPoseOffsets.Add(FName("jnt_eyeL_000"), FVector(0.0f, 0.002f, 0.0f));
    NeutralPoseOffsets.Add(FName("jnt_eyeR_000"), FVector(0.0f, 0.002f, 0.0f));
}

void UCognitiveDNACalibration::SetNeutralPoseOffset(FName JointName, FVector Offset)
{
    NeutralPoseOffsets.FindOrAdd(JointName) = Offset;
}

void UCognitiveDNACalibration::ApplyAestheticOverlay()
{
    ApplyAestheticToAnimatedMaps();
}

FAestheticOverlay UCognitiveDNACalibration::GetAestheticOverlay() const { return Aesthetic; }
void UCognitiveDNACalibration::SetAestheticOverlay(const FAestheticOverlay& Overlay) { Aesthetic = Overlay; }

// ============================================================================
// Cognitive Expression Pipeline (main per-frame processing)
// ============================================================================
void UCognitiveDNACalibration::ProcessCognitiveExpression(float DeltaTime)
{
    // Step 1: Get base expression from RigLogicEvaluator
    if (RigLogicRef)
    {
        FinalJoints = RigLogicRef->GetJointTransforms();
        FinalBlendShapes = RigLogicRef->GetBlendShapeWeights();
        FinalAnimatedMaps = RigLogicRef->GetAnimatedMapMultipliers();
    }

    // Step 2: Apply neutral pose offsets (aesthetic resting face)
    ApplyNeutralPoseOffsets();

    // Step 3: Apply micro-expressions (HyperChaotic Lorenz attractor)
    if (bEnableMicroExpressions)
    {
        ApplyMicroExpressions(DeltaTime);
    }

    // Step 4: Apply aesthetic overlay to animated maps (wrinkles, blood flow, SSS)
    if (bEnableAestheticOverlay)
    {
        ApplyAestheticToAnimatedMaps();
    }
}

// ============================================================================
// Micro-Expressions (Lorenz Attractor Chaos)
// ============================================================================
void UCognitiveDNACalibration::ApplyMicroExpressions(float DeltaTime)
{
    // Update the Lorenz attractor
    UpdateLorenzAttractor(DeltaTime);

    // Map Lorenz state to subtle AU perturbations
    float Amp = Aesthetic.MicroExpressionAmplitude * Aesthetic.ChaosIntensity;
    float Asym = Aesthetic.AsymmetryFactor;

    // Lorenz X → brow micro-movements
    float BrowPerturbL = Aesthetic.LorenzState.X * Amp * 0.01f;
    float BrowPerturbR = Aesthetic.LorenzState.X * Amp * 0.01f * (1.0f - Asym);

    // Lorenz Y → lip corner micro-movements
    float LipPerturbL = Aesthetic.LorenzState.Y * Amp * 0.008f;
    float LipPerturbR = Aesthetic.LorenzState.Y * Amp * 0.008f * (1.0f - Asym);

    // Lorenz Z → eye micro-movements
    float EyePerturbL = Aesthetic.LorenzState.Z * Amp * 0.005f;
    float EyePerturbR = Aesthetic.LorenzState.Z * Amp * 0.005f * (1.0f - Asym);

    // Apply to joints
    for (auto& Joint : FinalJoints)
    {
        FString Name = Joint.JointName.ToString();

        if (Name.Contains(TEXT("forehead")))
        {
            Joint.Translation.Z += Name.Contains(TEXT("L")) ? BrowPerturbL : BrowPerturbR;
        }
        else if (Name.Contains(TEXT("mouth")))
        {
            Joint.Translation.Y += Name.Contains(TEXT("L")) ? LipPerturbL : LipPerturbR;
        }
        else if (Name.Contains(TEXT("eye")))
        {
            Joint.Translation.Z += Name.Contains(TEXT("L")) ? EyePerturbL : EyePerturbR;
        }
    }

    // Apply to blend shapes (subtle weight perturbations)
    for (auto& Pair : FinalBlendShapes)
    {
        float Perturb = FMath::Sin(Aesthetic.LorenzState.X * 3.14159f) * Amp * 0.02f;
        Pair.Value = FMath::Clamp(Pair.Value + Perturb, 0.0f, 1.0f);
    }
}

void UCognitiveDNACalibration::UpdateLorenzAttractor(float DeltaTime)
{
    // Lorenz system: dx/dt = σ(y-x), dy/dt = x(ρ-z)-y, dz/dt = xy-βz
    FVector Deriv = ComputeLorenzDerivative(Aesthetic.LorenzState);
    float TimeScale = Aesthetic.MicroExpressionFrequency * 0.1f;
    Aesthetic.LorenzState += Deriv * DeltaTime * TimeScale;

    // Normalize to prevent divergence (keep in [-50, 50] range)
    Aesthetic.LorenzState.X = FMath::Clamp(Aesthetic.LorenzState.X, -50.0f, 50.0f);
    Aesthetic.LorenzState.Y = FMath::Clamp(Aesthetic.LorenzState.Y, -50.0f, 50.0f);
    Aesthetic.LorenzState.Z = FMath::Clamp(Aesthetic.LorenzState.Z, 0.0f, 60.0f);
}

FVector UCognitiveDNACalibration::ComputeLorenzDerivative(const FVector& S) const
{
    const float Sigma = 10.0f;
    const float Rho = 28.0f;
    const float Beta = 8.0f / 3.0f;

    return FVector(
        Sigma * (S.Y - S.X),
        S.X * (Rho - S.Z) - S.Y,
        S.X * S.Y - Beta * S.Z
    );
}

// ============================================================================
// Neutral Pose Offsets
// ============================================================================
void UCognitiveDNACalibration::ApplyNeutralPoseOffsets()
{
    for (auto& Joint : FinalJoints)
    {
        const FVector* Offset = NeutralPoseOffsets.Find(Joint.JointName);
        if (Offset)
        {
            Joint.Translation += *Offset;
        }
    }
}

// ============================================================================
// Aesthetic Animated Maps
// ============================================================================
void UCognitiveDNACalibration::ApplyAestheticToAnimatedMaps()
{
    // Eye sparkle → boost eye region animated maps
    FinalAnimatedMaps.FindOrAdd(FName("EyeSparkleL")) = Aesthetic.EyeSparkle;
    FinalAnimatedMaps.FindOrAdd(FName("EyeSparkleR")) = Aesthetic.EyeSparkle;

    // Lip gloss → boost lip region animated maps
    FinalAnimatedMaps.FindOrAdd(FName("LipGloss")) = Aesthetic.LipGlossSheen;

    // Skin SSS → boost subsurface scattering globally
    FinalAnimatedMaps.FindOrAdd(FName("SkinSSS")) = Aesthetic.SkinSSSBoost;

    // Charisma glow → boost overall skin luminance
    FinalAnimatedMaps.FindOrAdd(FName("CharismaGlow")) = Aesthetic.CharismaGlow;

    // Blood flow modulated by chaos
    float ChaosBloodFlow = FMath::Abs(Aesthetic.LorenzState.Y) * 0.01f * Aesthetic.ChaosIntensity;
    float* CheekL = FinalAnimatedMaps.Find(FName("BloodFlowCheekL"));
    float* CheekR = FinalAnimatedMaps.Find(FName("BloodFlowCheekR"));
    if (CheekL) *CheekL = FMath::Clamp(*CheekL + ChaosBloodFlow, 0.0f, 1.0f);
    if (CheekR) *CheekR = FMath::Clamp(*CheekR + ChaosBloodFlow, 0.0f, 1.0f);
}

// ============================================================================
// Snapshots
// ============================================================================
void UCognitiveDNACalibration::TakeSnapshot(const FString& Description)
{
    FCalibrationSnapshot Snap;
    Snap.Timestamp = FDateTime::Now();
    Snap.Description = Description;
    Snap.NeutralPoseOffsets = NeutralPoseOffsets;
    Snap.AestheticState = Aesthetic;
    Snap.EvolutionGeneration = EvolutionGeneration;

    Snapshots.Add(Snap);

    // Trim to max
    while (Snapshots.Num() > MaxSnapshots)
    {
        Snapshots.RemoveAt(0);
    }
}

bool UCognitiveDNACalibration::RestoreSnapshot(int32 Index)
{
    if (Index < 0 || Index >= Snapshots.Num()) return false;

    const FCalibrationSnapshot& Snap = Snapshots[Index];
    NeutralPoseOffsets = Snap.NeutralPoseOffsets;
    Aesthetic = Snap.AestheticState;
    EvolutionGeneration = Snap.EvolutionGeneration;

    return true;
}

TArray<FCalibrationSnapshot> UCognitiveDNACalibration::GetSnapshots() const { return Snapshots; }

// ============================================================================
// Evolution
// ============================================================================
void UCognitiveDNACalibration::EvolveFromExperience(float ExperienceIntensity)
{
    EvolutionGeneration++;

    // Subtle neutral pose drift based on experience
    for (auto& Pair : NeutralPoseOffsets)
    {
        float Drift = ExperienceIntensity * EvolutionRate;
        Pair.Value += FVector(
            FMath::FRandRange(-Drift, Drift),
            FMath::FRandRange(-Drift, Drift),
            FMath::FRandRange(-Drift, Drift)
        );
    }

    // Aesthetic evolution — confidence grows with experience
    Aesthetic.ConfidencePosture = FMath::Clamp(
        Aesthetic.ConfidencePosture + ExperienceIntensity * EvolutionRate * 0.1f, 0.0f, 1.0f);

    // Chaos becomes more refined (lower amplitude, higher frequency)
    Aesthetic.MicroExpressionAmplitude = FMath::Clamp(
        Aesthetic.MicroExpressionAmplitude - ExperienceIntensity * EvolutionRate * 0.01f, 0.01f, 0.2f);
    Aesthetic.MicroExpressionFrequency = FMath::Clamp(
        Aesthetic.MicroExpressionFrequency + ExperienceIntensity * EvolutionRate * 0.05f, 0.5f, 5.0f);

    TakeSnapshot(FString::Printf(TEXT("Evolution Gen %d (Intensity: %.2f)"),
                                  EvolutionGeneration, ExperienceIntensity));
}

int32 UCognitiveDNACalibration::GetEvolutionGeneration() const { return EvolutionGeneration; }

// ============================================================================
// Bridge Connections
// ============================================================================
void UCognitiveDNACalibration::ConnectRigLogic(URigLogicEvaluator* RigLogic)
{
    RigLogicRef = RigLogic;
}

void UCognitiveDNACalibration::ConnectOrchestrator(UMasterOrchestrator* Orchestrator)
{
    OrchestratorRef = Orchestrator;

    // Also connect the orchestrator to the rig logic evaluator
    if (RigLogicRef && Orchestrator)
    {
        RigLogicRef->ConnectToOrchestrator(Orchestrator);
    }
}

void UCognitiveDNACalibration::AutoDiscoverBridges()
{
    if (AActor* Owner = GetOwner())
    {
        if (!RigLogicRef)
            RigLogicRef = Owner->FindComponentByClass<URigLogicEvaluator>();
        if (!OrchestratorRef)
            OrchestratorRef = Owner->FindComponentByClass<UMasterOrchestrator>();

        if (RigLogicRef && OrchestratorRef)
        {
            RigLogicRef->ConnectToOrchestrator(OrchestratorRef);
        }
    }
}

// ============================================================================
// Telemetry
// ============================================================================
FCalibrationTelemetry UCognitiveDNACalibration::GetTelemetry() const
{
    FCalibrationTelemetry T;
    T.DescriptorStatus = LayerStatus[0];
    T.DefinitionStatus = LayerStatus[1];
    T.BehaviorStatus = LayerStatus[2];
    T.GeometryStatus = LayerStatus[3];
    T.SnapshotCount = Snapshots.Num();
    T.EvolutionGeneration = EvolutionGeneration;
    T.ChaosLevel = Aesthetic.ChaosIntensity;

    if (RigLogicRef)
    {
        FRigLogicTelemetry RLT = RigLogicRef->GetTelemetry();
        T.TotalControls = RLT.TotalControlInputs;
        T.TotalJoints = RLT.ActiveJoints;
        T.TotalBlendShapes = RLT.ActiveBlendShapes;
    }

    // Aesthetic coherence = how well the aesthetic overlay matches the expression
    T.AestheticCoherence = (Aesthetic.ConfidencePosture + Aesthetic.CharismaGlow +
                            Aesthetic.EyeSparkle + Aesthetic.IdleElegance) * 0.25f;

    return T;
}

// ============================================================================
// Output
// ============================================================================
TArray<FRigJointTransform> UCognitiveDNACalibration::GetFinalJointTransforms() const { return FinalJoints; }
TMap<FName, float> UCognitiveDNACalibration::GetFinalBlendShapeWeights() const { return FinalBlendShapes; }
TMap<FName, float> UCognitiveDNACalibration::GetFinalAnimatedMaps() const { return FinalAnimatedMaps; }
