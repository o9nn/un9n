// MetaHumanDNACognitiveBridge.cpp
// Implementation of MetaHuman DNA ↔ Deep Tree Echo cognitive bridge
// Translates cognitive architecture states into MetaHuman DNA parameters
// with SuperHotGirl aesthetics and HyperChaotic dynamics

#include "MetaHumanDNACognitiveBridge.h"
#include "Materials/MaterialInstanceDynamic.h"

UMetaHumanDNACognitiveBridge::UMetaHumanDNACognitiveBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize Lorenz attractor at a non-trivial point
    LorenzState = FVector(0.1f, 0.0f, 0.0f);
}

void UMetaHumanDNACognitiveBridge::BeginPlay()
{
    Super::BeginPlay();
    FindComponentReferences();
    InitializeFACSMapping();

    // Pre-allocate trajectory history
    TrajectoryHistory.Reserve(ChaoticDynamics.TrajectoryHistoryLength);
}

void UMetaHumanDNACognitiveBridge::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableCognitiveDNA) return;

    // Update DNA at configured rate
    DNAUpdateTimer += DeltaTime;
    float UpdateInterval = 1.0f / DNAUpdateRate;
    if (DNAUpdateTimer >= UpdateInterval)
    {
        DNAUpdateTimer -= UpdateInterval;

        // Get cognitive state from avatar cognition component
        if (AvatarCognition)
        {
            FAvatarCognitiveState CogState = AvatarCognition->GetCognitiveState();
            UpdateDNAFromCognition(CogState);
        }

        // Apply FACS to skeleton
        ApplyFACSToSkeleton();

        // Update aesthetic materials
        UpdateAestheticMaterials();
    }

    // Update chaotic dynamics continuously for smooth animation
    if (bEnableChaoticAttractor)
    {
        UpdateChaoticDynamics(DeltaTime);
    }

    // Generate micro-expressions
    if (bEnableMicroExpressions)
    {
        GenerateAndApplyMicroExpression(DeltaTime);
    }
}

void UMetaHumanDNACognitiveBridge::FindComponentReferences()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    AvatarCognition = Owner->FindComponentByClass<UUnrealAvatarCognition>();
    DNABinding = Owner->FindComponentByClass<UDNABodySchemaBinding>();
    SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
}

// ============================================================================
// FACS ACTION UNIT SYSTEM
// ============================================================================

void UMetaHumanDNACognitiveBridge::InitializeFACSMapping()
{
    ActionUnits.Empty();

    // Define FACS Action Units mapped to MetaHuman DNA blend shapes
    // Each AU maps a cognitive parameter to a specific facial muscle group

    auto AddAU = [this](FName Name, FString Source, float Weight, float Asymmetry = 0.0f)
    {
        FMetaHumanActionUnit AU;
        AU.ActionUnitName = Name;
        AU.CognitiveSource = Source;
        AU.Weight = Weight;
        AU.Asymmetry = Asymmetry;
        AU.Activation = 0.0f;
        ActionUnits.Add(AU);
    };

    // Upper Face Action Units
    AddAU(TEXT("AU1_InnerBrowRaise"), TEXT("Curiosity"), 0.8f);
    AddAU(TEXT("AU2_OuterBrowRaise"), TEXT("Wonder"), 0.7f);
    AddAU(TEXT("AU4_BrowLowerer"), TEXT("Concern"), 0.9f);
    AddAU(TEXT("AU5_UpperLidRaise"), TEXT("Excitement"), 0.6f);
    AddAU(TEXT("AU6_CheekRaise"), TEXT("Joy"), 0.85f);
    AddAU(TEXT("AU7_LidTightener"), TEXT("Focus"), 0.5f);

    // Lower Face Action Units
    AddAU(TEXT("AU9_NoseWrinkle"), TEXT("Disgust"), 0.7f);
    AddAU(TEXT("AU10_UpperLipRaise"), TEXT("Contempt"), 0.6f);
    AddAU(TEXT("AU12_LipCornerPull"), TEXT("Joy"), 0.95f);          // Smile
    AddAU(TEXT("AU14_Dimpler"), TEXT("Determination"), 0.5f, 0.3f); // Slight asymmetry
    AddAU(TEXT("AU15_LipCornerDepress"), TEXT("Sadness"), 0.8f);
    AddAU(TEXT("AU17_ChinRaise"), TEXT("Determination"), 0.4f);
    AddAU(TEXT("AU20_LipStretcher"), TEXT("Fear"), 0.7f);
    AddAU(TEXT("AU23_LipTightener"), TEXT("Focus"), 0.6f);
    AddAU(TEXT("AU24_LipPressor"), TEXT("Contemplation"), 0.5f);
    AddAU(TEXT("AU25_LipsPart"), TEXT("Wonder"), 0.4f);
    AddAU(TEXT("AU26_JawDrop"), TEXT("Excitement"), 0.3f);

    // Head Position Action Units
    AddAU(TEXT("AU51_HeadTurnLeft"), TEXT("Curiosity"), 0.3f);
    AddAU(TEXT("AU52_HeadTurnRight"), TEXT("Curiosity"), 0.3f);
    AddAU(TEXT("AU53_HeadUp"), TEXT("Confidence"), 0.4f);
    AddAU(TEXT("AU54_HeadDown"), TEXT("Contemplation"), 0.35f);
    AddAU(TEXT("AU55_HeadTiltLeft"), TEXT("Curiosity"), 0.25f, -0.5f);
    AddAU(TEXT("AU56_HeadTiltRight"), TEXT("Curiosity"), 0.25f, 0.5f);

    // SuperHotGirl-specific AUs (aesthetic micro-expressions)
    AddAU(TEXT("SHG_EyeSparkle"), TEXT("Charisma"), 0.75f);
    AddAU(TEXT("SHG_SmirkLeft"), TEXT("Confidence"), 0.4f, -0.8f);
    AddAU(TEXT("SHG_LashFlutter"), TEXT("Playfulness"), 0.3f);
    AddAU(TEXT("SHG_PoutSubtle"), TEXT("Allure"), 0.35f);

    UE_LOG(LogTemp, Log, TEXT("MetaHumanDNACognitiveBridge: Initialized %d FACS Action Units"),
        ActionUnits.Num());
}

void UMetaHumanDNACognitiveBridge::UpdateDNAFromCognition(const FAvatarCognitiveState& CognitiveState)
{
    // Map emotion state to AU activation levels
    float EmotionIntensity = CognitiveState.EmotionIntensity;

    for (FMetaHumanActionUnit& AU : ActionUnits)
    {
        float TargetActivation = 0.0f;

        // Map cognitive source to activation
        if (AU.CognitiveSource == TEXT("Joy"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Joy) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Curiosity"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Curiosity) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Wonder"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Wonder) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Focus"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Focus) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Concern"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Concern) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Excitement"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Excitement) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Determination"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Determination) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Contemplation"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Contemplation) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Serenity"))
        {
            TargetActivation = (CognitiveState.EmotionState == EAvatarEmotionState::Serenity) ?
                EmotionIntensity : 0.0f;
        }
        else if (AU.CognitiveSource == TEXT("Confidence"))
        {
            TargetActivation = Aesthetics.ConfidencePosture;
        }
        else if (AU.CognitiveSource == TEXT("Charisma"))
        {
            TargetActivation = Aesthetics.CharismaGlow;
        }
        else if (AU.CognitiveSource == TEXT("Playfulness"))
        {
            // Driven by chaotic dynamics
            TargetActivation = FMath::Abs(LorenzState.X) * 0.1f;
        }
        else if (AU.CognitiveSource == TEXT("Allure"))
        {
            TargetActivation = Aesthetics.IdleElegance * 0.5f;
        }

        // Apply weight and add chaotic perturbation
        TargetActivation *= AU.Weight;

        if (bEnableChaoticAttractor)
        {
            // Add subtle chaotic noise for natural micro-movements
            float ChaoticNoise = FMath::Sin(LorenzState.X * 10.0f + LorenzState.Y * 7.0f)
                * ChaoticDynamics.ExpressionVolatility * 0.05f;
            TargetActivation += ChaoticNoise;
        }

        // Smooth transition
        AU.Activation = FMath::FInterpTo(AU.Activation, 
            FMath::Clamp(TargetActivation, 0.0f, 1.0f), 
            1.0f / DNAUpdateRate, 5.0f);
    }

    // Update cognitive coherence influence on overall expression
    float CoherenceModifier = CognitiveState.CognitiveCoherence;

    // High coherence = more controlled, precise expressions
    // Low coherence = more chaotic, scattered expressions
    ChaoticDynamics.ChaosIntensity = FMath::Lerp(0.95f, 0.5f, CoherenceModifier);
}

void UMetaHumanDNACognitiveBridge::ApplyFACSToSkeleton()
{
    if (!SkeletalMesh) return;

    for (const FMetaHumanActionUnit& AU : ActionUnits)
    {
        if (AU.Activation < 0.01f) continue;

        // Apply asymmetry
        float LeftActivation = AU.Activation;
        float RightActivation = AU.Activation;

        if (FMath::Abs(AU.Asymmetry) > 0.01f)
        {
            float AsymFactor = (AU.Asymmetry + 1.0f) * 0.5f; // Map -1..1 to 0..1
            LeftActivation *= (1.0f - AsymFactor);
            RightActivation *= AsymFactor;
        }

        // Map AU name to MetaHuman morph target names
        FString AUName = AU.ActionUnitName.ToString();

        // Standard FACS → MetaHuman DNA morph target mapping
        if (AUName.Contains(TEXT("AU1")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_raiseIn"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_raiseIn"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU2")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_raiseOut"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_raiseOut"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU4")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_brow_down"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_brow_down"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU5")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_wide"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_wide"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU6")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_smile"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_mouth_smile"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU7")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_squintInner"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_squintInner"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU9")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_nose_wrinkle"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_nose_wrinkle"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU12")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_cornerPull"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_mouth_cornerPull"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU15")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_cornerDepress"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_mouth_cornerDepress"), RightActivation);
        }
        else if (AUName.Contains(TEXT("AU17")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_C_chin_raise"), AU.Activation);
        }
        else if (AUName.Contains(TEXT("AU23")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_C_mouth_lipsTighten"), AU.Activation);
        }
        else if (AUName.Contains(TEXT("AU25")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_C_mouth_lipsPart"), AU.Activation);
        }
        else if (AUName.Contains(TEXT("AU26")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_C_jaw_open"), AU.Activation);
        }
        // SuperHotGirl custom morph targets
        else if (AUName.Contains(TEXT("SHG_EyeSparkle")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_sparkle"), LeftActivation);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_sparkle"), RightActivation);
        }
        else if (AUName.Contains(TEXT("SHG_SmirkLeft")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_mouth_smirk"), AU.Activation);
        }
        else if (AUName.Contains(TEXT("SHG_LashFlutter")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_L_eye_blink"), LeftActivation * 0.3f);
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_R_eye_blink"), RightActivation * 0.3f);
        }
        else if (AUName.Contains(TEXT("SHG_PoutSubtle")))
        {
            SkeletalMesh->SetMorphTarget(TEXT("CTRL_C_mouth_pout"), AU.Activation);
        }

        // Broadcast parameter change
        OnDNAParameterChanged.Broadcast(AU.ActionUnitName, AU.Activation);
    }
}

// ============================================================================
// CHAOTIC ATTRACTOR DYNAMICS
// ============================================================================

void UMetaHumanDNACognitiveBridge::UpdateChaoticDynamics(float DeltaTime)
{
    // Lorenz attractor for hyper-chaotic behavior generation
    FVector PrevState = LorenzState;
    LorenzState = LorenzStep(LorenzState, DeltaTime);

    // Store trajectory history
    TrajectoryHistory.Add(LorenzState);
    if (TrajectoryHistory.Num() > ChaoticDynamics.TrajectoryHistoryLength)
    {
        TrajectoryHistory.RemoveAt(0);
    }

    // Update attractor position (normalized to unit sphere)
    ChaoticDynamics.AttractorPosition = LorenzState.GetSafeNormal();

    // Estimate Lyapunov exponent
    ChaoticDynamics.LyapunovExponent = ComputeLyapunovExponent();

    // Detect attractor transitions (bifurcation events)
    float TransitionThreshold = 5.0f;
    if (FVector::Dist(ChaoticDynamics.AttractorPosition, PreviousAttractorPosition) > TransitionThreshold)
    {
        OnChaoticTransition.Broadcast(ChaoticDynamics.AttractorPosition);
    }
    PreviousAttractorPosition = ChaoticDynamics.AttractorPosition;
}

FVector UMetaHumanDNACognitiveBridge::LorenzStep(const FVector& State, float DeltaTime)
{
    // Lorenz system parameters (classic chaotic regime)
    const float Sigma = 10.0f;
    const float Rho = 28.0f * ChaoticDynamics.ChaosIntensity;
    const float Beta = 8.0f / 3.0f;

    // RK4 integration for stability
    float dt = DeltaTime * 10.0f; // Scale time for visible dynamics

    auto Derivative = [Sigma, Rho, Beta](const FVector& S) -> FVector
    {
        return FVector(
            Sigma * (S.Y - S.X),
            S.X * (Rho - S.Z) - S.Y,
            S.X * S.Y - Beta * S.Z
        );
    };

    FVector k1 = Derivative(State) * dt;
    FVector k2 = Derivative(State + k1 * 0.5f) * dt;
    FVector k3 = Derivative(State + k2 * 0.5f) * dt;
    FVector k4 = Derivative(State + k3) * dt;

    return State + (k1 + k2 * 2.0f + k3 * 2.0f + k4) / 6.0f;
}

FVector UMetaHumanDNACognitiveBridge::ComputeChaoticAttractor(float DeltaTime)
{
    return LorenzStep(LorenzState, DeltaTime);
}

float UMetaHumanDNACognitiveBridge::ComputeLyapunovExponent()
{
    if (TrajectoryHistory.Num() < 10) return 0.0f;

    // Estimate largest Lyapunov exponent from trajectory divergence
    float SumLog = 0.0f;
    int32 Count = 0;

    for (int32 i = 1; i < TrajectoryHistory.Num(); ++i)
    {
        float Dist = FVector::Dist(TrajectoryHistory[i], TrajectoryHistory[i - 1]);
        if (Dist > 0.0001f)
        {
            SumLog += FMath::Loge(Dist);
            Count++;
        }
    }

    return Count > 0 ? SumLog / Count : 0.0f;
}

float UMetaHumanDNACognitiveBridge::GetLyapunovExponent() const
{
    return ChaoticDynamics.LyapunovExponent;
}

float UMetaHumanDNACognitiveBridge::GetActionUnitValue(FName ActionUnitName) const
{
    for (const FMetaHumanActionUnit& AU : ActionUnits)
    {
        if (AU.ActionUnitName == ActionUnitName)
        {
            return AU.Activation;
        }
    }
    return 0.0f;
}

// ============================================================================
// MICRO-EXPRESSION GENERATION
// ============================================================================

void UMetaHumanDNACognitiveBridge::GenerateAndApplyMicroExpression(float DeltaTime)
{
    MicroExpressionTimer += DeltaTime;

    float MicroInterval = 1.0f / Aesthetics.MicroExpressionFrequency;
    if (MicroExpressionTimer >= MicroInterval)
    {
        MicroExpressionTimer -= MicroInterval;

        // Generate micro-expression from chaotic dynamics
        float Intensity = FMath::Clamp(
            FMath::Abs(LorenzState.X * 0.01f) * ChaoticDynamics.ExpressionVolatility,
            0.0f, 0.15f); // Micro-expressions are subtle

        FFacialExpressionState MicroExpr = GenerateMicroExpression(Intensity);
        OnMicroExpressionGenerated.Broadcast(MicroExpr);
    }
}

FFacialExpressionState UMetaHumanDNACognitiveBridge::GenerateMicroExpression(float Intensity)
{
    FFacialExpressionState MicroExpr;

    // Use Lorenz attractor components to drive different facial regions
    // This creates naturalistic, non-repeating micro-expressions
    float Lx = FMath::Frac(FMath::Abs(LorenzState.X * 0.1f));
    float Ly = FMath::Frac(FMath::Abs(LorenzState.Y * 0.1f));
    float Lz = FMath::Frac(FMath::Abs(LorenzState.Z * 0.1f));

    MicroExpr.BrowRaise = Lx * Intensity;
    MicroExpr.BrowFurrow = (1.0f - Lx) * Intensity * 0.5f;
    MicroExpr.EyeOpenness = 0.7f + Ly * Intensity * 0.1f;
    MicroExpr.Smile = Lz * Intensity * 0.8f;
    MicroExpr.MouthOpenness = Ly * Intensity * 0.3f;
    MicroExpr.LipTension = (1.0f - Lz) * Intensity * 0.4f;
    MicroExpr.CheekRaise = Lz * Intensity * 0.6f;
    MicroExpr.NoseWrinkle = Lx * Intensity * 0.2f;
    MicroExpr.JawClench = (1.0f - Ly) * Intensity * 0.15f;

    return MicroExpr;
}

// ============================================================================
// SUPERHOT GIRL AESTHETICS
// ============================================================================

void UMetaHumanDNACognitiveBridge::ApplySuperHotGirlAesthetics()
{
    UpdateAestheticMaterials();
}

void UMetaHumanDNACognitiveBridge::UpdateAestheticMaterials()
{
    if (!SkeletalMesh) return;

    // Apply aesthetic parameters to material instances
    int32 NumMaterials = SkeletalMesh->GetNumMaterials();
    for (int32 i = 0; i < NumMaterials; ++i)
    {
        UMaterialInterface* Material = SkeletalMesh->GetMaterial(i);
        if (!Material) continue;

        UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Material);
        if (!DynMat)
        {
            DynMat = SkeletalMesh->CreateDynamicMaterialInstance(i);
        }

        if (DynMat)
        {
            // Apply SuperHotGirl aesthetic parameters to shader
            DynMat->SetScalarParameterValue(TEXT("SubsurfaceScatteringBoost"),
                Aesthetics.SkinSSSBoost);
            DynMat->SetScalarParameterValue(TEXT("EyeSparkleIntensity"),
                Aesthetics.EyeSparkle);
            DynMat->SetScalarParameterValue(TEXT("LipGlossSheen"),
                Aesthetics.LipGlossSheen);
            DynMat->SetScalarParameterValue(TEXT("CharismaGlowIntensity"),
                Aesthetics.CharismaGlow);

            // Chaotic dynamics modulate material properties subtly
            float ChaoticShimmer = FMath::Sin(LorenzState.Z * 0.5f) * 0.02f;
            DynMat->SetScalarParameterValue(TEXT("SkinShimmer"),
                FMath::Clamp(ChaoticShimmer + 0.02f, 0.0f, 0.1f));
        }
    }
}
