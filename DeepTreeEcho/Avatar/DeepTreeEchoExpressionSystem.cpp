// DeepTreeEchoExpressionSystem.cpp
// Master Expression Orchestrator Implementation
// Evaluates the complete expression pipeline each frame

#include "DeepTreeEchoExpressionSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/DateTime.h"

UDeepTreeEchoExpressionSystem::UDeepTreeEchoExpressionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    // Tick after all other cognitive components have updated
    PrimaryComponentTick.TickInterval = 0.0f; // Every frame
}

void UDeepTreeEchoExpressionSystem::BeginPlay()
{
    Super::BeginPlay();
    FindComponents();

    // Initialize FACS mapping database
    FACSMapping = NewObject<UFACSToMetaHumanMapping>(this);
    FACSMapping->InitializeMappings();
}

void UDeepTreeEchoExpressionSystem::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnabled) return;

    double StartTime = FPlatformTime::Seconds();

    // Handle forced expression
    if (bForcedExpression)
    {
        ForcedExpressionTimer += DeltaTime;
        if (ForcedExpressionTimer >= ForcedExpressionDuration)
        {
            ClearForcedExpression();
        }
        else
        {
            TMap<FName, float> CtrlValues = ConvertToCtrl(ForcedFACSValues);
            SmoothAndApply(CtrlValues);
            UpdateTelemetry();
            Telemetry.EvaluationTimeMs = (float)((FPlatformTime::Seconds() - StartTime) * 1000.0);
            OnExpressionEvaluated.Broadcast(Telemetry);
            return;
        }
    }

    // ====================================================================
    // FULL EXPRESSION PIPELINE
    // ====================================================================

    // Step 1: Cognitive base expression
    TMap<FName, float> CognitiveExpr = CollectCognitiveExpression();

    // Step 2: Endocrine overlay
    TMap<FName, float> EndocrineExpr;
    if (bEnableEndocrine)
    {
        EndocrineExpr = CollectEndocrineOverlay();
    }

    // Step 3: Chaotic micro-expressions
    TMap<FName, float> ChaoticExpr;
    if (bEnableChaotic)
    {
        ChaoticExpr = GenerateChaoticMicroExpression(DeltaTime);
    }

    // Step 4: Aesthetic overlay
    TMap<FName, float> AestheticExpr;
    if (bEnableAesthetics)
    {
        AestheticExpr = GenerateAestheticOverlay();
    }

    // Step 5: Blend all sources
    FinalFACSValues = BlendAllSources(CognitiveExpr, EndocrineExpr, ChaoticExpr, AestheticExpr);

    // Apply global intensity
    for (auto& Pair : FinalFACSValues)
    {
        Pair.Value *= GlobalIntensityMultiplier;
        Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }

    // Step 6: Convert FACS → CTRL_
    TMap<FName, float> CtrlValues = ConvertToCtrl(FinalFACSValues);

    // Step 7: Apply evolution limits
    if (bEnableEvolutionLimits)
    {
        CtrlValues = ApplyEvolutionLimits(CtrlValues);
    }

    // Step 8: Smooth and apply to mesh
    SmoothAndApply(CtrlValues);

    // Step 9: Update telemetry
    UpdateTelemetry();
    Telemetry.EvaluationTimeMs = (float)((FPlatformTime::Seconds() - StartTime) * 1000.0);

    OnExpressionEvaluated.Broadcast(Telemetry);
}

// ============================================================================
// COMPONENT DISCOVERY
// ============================================================================

void UDeepTreeEchoExpressionSystem::FindComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    DNABridge = Owner->FindComponentByClass<UMetaHumanDNACognitiveBridge>();
    EndocrinePipeline = Owner->FindComponentByClass<UEndocrineExpressionPipeline>();
    DesignStudio = Owner->FindComponentByClass<UAvatarDesignStudio>();
    ExpressiveSystem = Owner->FindComponentByClass<UExpressiveAnimationSystem>();
    EvolutionSystem = Owner->FindComponentByClass<UEvolutionSystem>();
    SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
}

// ============================================================================
// PIPELINE STEPS
// ============================================================================

TMap<FName, float> UDeepTreeEchoExpressionSystem::CollectCognitiveExpression()
{
    TMap<FName, float> Result;

    if (DNABridge)
    {
        // Collect current AU activations from the DNA bridge
        for (const FMetaHumanActionUnit& AU : DNABridge->ActionUnits)
        {
            if (AU.Activation > 0.01f)
            {
                Result.Add(AU.ActionUnitName, AU.Activation);
            }
        }
    }

    return Result;
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::CollectEndocrineOverlay()
{
    if (!EndocrinePipeline) return TMap<FName, float>();
    return EndocrinePipeline->GetHormoneDriverAUValues();
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::GenerateChaoticMicroExpression(float DeltaTime)
{
    TMap<FName, float> Result;

    if (!DNABridge || !DNABridge->bEnableChaoticAttractor) return Result;

    // Use Lorenz attractor state for micro-expression generation
    FVector LState = DNABridge->LorenzState;
    float ChaosIntensity = DNABridge->ChaoticDynamics.ChaosIntensity;
    float MicroFreq = DNABridge->Aesthetics.MicroExpressionFrequency;

    // Scale by micro-expression frequency and chaos intensity
    float MicroScale = ChaosIntensity * 0.15f; // Keep micro-expressions subtle

    // Map Lorenz state to subtle AU modulations
    float Lx = FMath::Frac(FMath::Abs(LState.X * 0.05f));
    float Ly = FMath::Frac(FMath::Abs(LState.Y * 0.05f));
    float Lz = FMath::Frac(FMath::Abs(LState.Z * 0.03f));

    // Subtle brow micro-movements
    Result.Add(FName("AU1"), Lx * MicroScale * 0.3f);
    Result.Add(FName("AU2"), Ly * MicroScale * 0.2f);
    Result.Add(FName("AU4"), (1.0f - Lx) * MicroScale * 0.15f);

    // Subtle eye micro-movements
    Result.Add(FName("AU5"), Lz * MicroScale * 0.2f);
    Result.Add(FName("AU7"), (1.0f - Lz) * MicroScale * 0.15f);

    // Subtle mouth micro-movements
    Result.Add(FName("AU12"), Ly * MicroScale * 0.25f);
    Result.Add(FName("AU14"), Lx * MicroScale * 0.1f);
    Result.Add(FName("AU25"), Lz * MicroScale * 0.15f);

    return Result;
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::GenerateAestheticOverlay()
{
    TMap<FName, float> Result;

    if (!DNABridge) return Result;

    const FSuperHotGirlAesthetics& Aesthetics = DNABridge->Aesthetics;

    // Confidence posture → subtle smile and brow position
    if (Aesthetics.ConfidencePosture > 0.5f)
    {
        float ConfidenceEffect = (Aesthetics.ConfidencePosture - 0.5f) * 2.0f;
        Result.Add(FName("AU12"), ConfidenceEffect * 0.15f); // Subtle smile
        Result.Add(FName("AU2"), ConfidenceEffect * 0.1f);   // Slight brow raise
    }

    // Eye sparkle → slight lid raise
    if (Aesthetics.EyeSparkle > 0.5f)
    {
        float SparkleEffect = (Aesthetics.EyeSparkle - 0.5f) * 2.0f;
        Result.Add(FName("AU5"), SparkleEffect * 0.1f);
    }

    // Idle elegance → smoothing of all expressions (reduces jitter)
    // This is handled in the smoothing step

    return Result;
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::BlendAllSources(
    const TMap<FName, float>& Cognitive,
    const TMap<FName, float>& Endocrine,
    const TMap<FName, float>& Chaotic,
    const TMap<FName, float>& Aesthetic)
{
    TMap<FName, float> Blended;

    // Collect all unique AU names
    TSet<FName> AllAUs;
    for (const auto& P : Cognitive) AllAUs.Add(P.Key);
    for (const auto& P : Endocrine) AllAUs.Add(P.Key);
    for (const auto& P : Chaotic) AllAUs.Add(P.Key);
    for (const auto& P : Aesthetic) AllAUs.Add(P.Key);

    // Blend weights per source
    const float CognitiveWeight = 1.0f;    // Base layer
    const float EndocrineWeight = 0.5f;    // Additive overlay
    const float ChaoticWeight = 0.3f;      // Subtle additive
    const float AestheticWeight = 0.2f;    // Subtle additive

    for (const FName& AU : AllAUs)
    {
        float Value = 0.0f;

        // Cognitive is the base
        const float* CogVal = Cognitive.Find(AU);
        if (CogVal) Value += *CogVal * CognitiveWeight;

        // Endocrine is additive
        const float* EndoVal = Endocrine.Find(AU);
        if (EndoVal) Value += *EndoVal * EndocrineWeight;

        // Chaotic is additive (subtle)
        const float* ChaosVal = Chaotic.Find(AU);
        if (ChaosVal) Value += *ChaosVal * ChaoticWeight;

        // Aesthetic is additive (subtle)
        const float* AesthVal = Aesthetic.Find(AU);
        if (AesthVal) Value += *AesthVal * AestheticWeight;

        Blended.Add(AU, FMath::Clamp(Value, 0.0f, 1.0f));
    }

    return Blended;
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::ConvertToCtrl(
    const TMap<FName, float>& FACSValues)
{
    if (FACSMapping)
    {
        return FACSMapping->ConvertFACSToCtrl(FACSValues);
    }
    return TMap<FName, float>();
}

TMap<FName, float> UDeepTreeEchoExpressionSystem::ApplyEvolutionLimits(
    const TMap<FName, float>& CtrlValues)
{
    if (!EvolutionSystem) return CtrlValues;

    int32 MaxAUs = GetMaxAUCountForStage();
    if (CtrlValues.Num() <= MaxAUs) return CtrlValues;

    // Sort by value and keep only the top MaxAUs
    TArray<TPair<FName, float>> Sorted;
    for (const auto& Pair : CtrlValues)
    {
        Sorted.Add(TPair<FName, float>(Pair.Key, Pair.Value));
    }

    Sorted.Sort([](const TPair<FName, float>& A, const TPair<FName, float>& B) {
        return A.Value > B.Value;
    });

    TMap<FName, float> Limited;
    for (int32 i = 0; i < FMath::Min(MaxAUs, Sorted.Num()); ++i)
    {
        Limited.Add(Sorted[i].Key, Sorted[i].Value);
    }

    return Limited;
}

void UDeepTreeEchoExpressionSystem::SmoothAndApply(const TMap<FName, float>& CtrlValues)
{
    FinalCtrlValues.Empty();

    // Adaptive smoothing based on elegance factor
    float EffectiveSmoothingFactor = SmoothingFactor;
    if (DNABridge)
    {
        // Higher elegance = more smoothing = more graceful transitions
        EffectiveSmoothingFactor = FMath::Lerp(SmoothingFactor,
            FMath::Min(0.95f, SmoothingFactor + 0.3f),
            DNABridge->Aesthetics.IdleElegance);
    }

    // Smooth each CTRL_ value
    for (const auto& Pair : CtrlValues)
    {
        float PrevValue = 0.0f;
        const float* PrevPtr = PreviousCtrlValues.Find(Pair.Key);
        if (PrevPtr) PrevValue = *PrevPtr;

        float SmoothedValue = FMath::Lerp(Pair.Value, PrevValue, EffectiveSmoothingFactor);
        FinalCtrlValues.Add(Pair.Key, SmoothedValue);
    }

    // Decay previous values not in current frame
    for (const auto& Pair : PreviousCtrlValues)
    {
        if (!CtrlValues.Contains(Pair.Key))
        {
            float DecayedValue = Pair.Value * EffectiveSmoothingFactor;
            if (DecayedValue > 0.01f)
            {
                FinalCtrlValues.Add(Pair.Key, DecayedValue);
            }
        }
    }

    PreviousCtrlValues = FinalCtrlValues;

    // Apply to skeletal mesh morph targets
    if (SkeletalMesh)
    {
        for (const auto& Pair : FinalCtrlValues)
        {
            SkeletalMesh->SetMorphTarget(Pair.Key, Pair.Value);
        }
    }
}

void UDeepTreeEchoExpressionSystem::UpdateTelemetry()
{
    Telemetry.ActiveAUCount = FinalFACSValues.Num();
    Telemetry.ActiveCtrlCount = FinalCtrlValues.Num();

    if (EndocrinePipeline)
    {
        Telemetry.CognitiveMode = EndocrinePipeline->GetCognitiveMode();
        EndocrinePipeline->GetValenceArousal(Telemetry.Valence, Telemetry.Arousal);
    }

    if (EvolutionSystem)
    {
        Telemetry.EvolutionStage = EvolutionSystem->GetCurrentStage();
    }

    if (DNABridge)
    {
        Telemetry.LyapunovExponent = DNABridge->GetLyapunovExponent();
    }

    if (DesignStudio)
    {
        Telemetry.CurrentArchetype = DesignStudio->GetCurrentArchetype();
    }

    // Compute overall intensity
    float TotalIntensity = 0.0f;
    for (const auto& Pair : FinalCtrlValues)
    {
        TotalIntensity += Pair.Value;
    }
    Telemetry.OverallIntensity = (FinalCtrlValues.Num() > 0)
        ? TotalIntensity / FinalCtrlValues.Num()
        : 0.0f;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UDeepTreeEchoExpressionSystem::ForceExpression(const TMap<FName, float>& FACSValues,
                                                      float Duration)
{
    bForcedExpression = true;
    ForcedFACSValues = FACSValues;
    ForcedExpressionTimer = 0.0f;
    ForcedExpressionDuration = Duration;
}

void UDeepTreeEchoExpressionSystem::ClearForcedExpression()
{
    bForcedExpression = false;
    ForcedFACSValues.Empty();
    ForcedExpressionTimer = 0.0f;
}

void UDeepTreeEchoExpressionSystem::SignalEvent(FName EventType, float Intensity)
{
    if (EndocrinePipeline)
    {
        EndocrinePipeline->SignalCognitiveEvent(EventType, Intensity);
    }
}

int32 UDeepTreeEchoExpressionSystem::GetMaxAUCountForStage() const
{
    if (!EvolutionSystem) return 251; // No limit

    switch (EvolutionSystem->GetCurrentStage())
    {
    case EOntogeneticStage::Embryonic:    return 30;   // Basic expressions only
    case EOntogeneticStage::Juvenile:     return 60;   // Core + some nuance
    case EOntogeneticStage::Adolescent:   return 120;  // Most expressions
    case EOntogeneticStage::Adult:        return 200;  // Nearly full range
    case EOntogeneticStage::Transcendent: return 251;  // All 251 CTRL_ targets
    default:                              return 251;
    }
}
