// DeepTreeEchoExpressionSystem.h
// Master Expression Orchestrator for Deep Tree Echo Cognitive Avatar
// Bridges ALL avatar subsystems into a unified expression pipeline:
// - Echobeats cognitive cycle → base expression state
// - Endocrine pipeline → hormone-modulated overlays
// - Chaotic dynamics → micro-expression generation
// - SuperHotGirl aesthetics → material and posture enhancement
// - FACS-to-MetaHuman mapping → CTRL_ morph target output
// - Evolution system → stage-dependent expression complexity
// - Design Studio → user customization and animation authoring
//
// This is the single entry point for all avatar expression evaluation.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaHumanDNACognitiveBridge.h"
#include "EndocrineExpressionPipeline.h"
#include "AvatarDesignStudio.h"
#include "FACSToMetaHumanMapping.h"
#include "ExpressiveAnimationSystem.h"
#include "../Evolution/EvolutionSystem.h"
#include "DeepTreeEchoExpressionSystem.generated.h"

/** Expression evaluation telemetry for debugging and visualization */
USTRUCT(BlueprintType)
struct FExpressionTelemetry
{
    GENERATED_BODY()

    /** Number of active FACS AUs */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ActiveAUCount = 0;

    /** Number of active CTRL_ morph targets */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ActiveCtrlCount = 0;

    /** Current cognitive mode from endocrine system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EEndocrineCognitiveMode CognitiveMode = EEndocrineCognitiveMode::Resting;

    /** Current ontogenetic stage */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EOntogeneticStage EvolutionStage = EOntogeneticStage::Embryonic;

    /** Valence (-1 to +1) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Valence = 0.0f;

    /** Arousal (0 to 1) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Arousal = 0.0f;

    /** Lyapunov exponent (positive = chaotic) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LyapunovExponent = 0.0f;

    /** Overall expression intensity (0-1) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float OverallIntensity = 0.0f;

    /** Frame evaluation time in milliseconds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float EvaluationTimeMs = 0.0f;

    /** Current archetype */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EAvatarArchetype CurrentArchetype = EAvatarArchetype::DeepTreeEcho;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpressionEvaluated, const FExpressionTelemetry&, Telemetry);

/**
 * Deep Tree Echo Expression System
 * 
 * The master orchestrator that evaluates and blends all expression sources
 * into a final set of MetaHuman CTRL_ morph target values each frame.
 * 
 * Evaluation pipeline (per frame):
 * 1. Collect cognitive state from Echobeats cycle
 * 2. Evaluate endocrine hormone bus state
 * 3. Compute chaotic micro-expression overlay
 * 4. Apply SuperHotGirl aesthetic modulation
 * 5. Blend user-authored animation sequences
 * 6. Convert FACS AUs to MetaHuman CTRL_ targets
 * 7. Apply evolution-stage complexity limits
 * 8. Output final CTRL_ values to skeletal mesh
 * 9. Update telemetry for visualization
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class DEEPTREEECHO_API UDeepTreeEchoExpressionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDeepTreeEchoExpressionSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Enable the full expression pipeline */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config")
    bool bEnabled = true;

    /** Enable endocrine modulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config")
    bool bEnableEndocrine = true;

    /** Enable chaotic micro-expressions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config")
    bool bEnableChaotic = true;

    /** Enable SuperHotGirl aesthetic overlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config")
    bool bEnableAesthetics = true;

    /** Enable evolution-based complexity limits */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config")
    bool bEnableEvolutionLimits = true;

    /** Global expression intensity multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config",
              meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float GlobalIntensityMultiplier = 1.0f;

    /** Expression smoothing factor (0 = no smoothing, 1 = max smoothing) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DTE Expression|Config",
              meta = (ClampMin = "0.0", ClampMax = "0.99"))
    float SmoothingFactor = 0.15f;

    // ========================================================================
    // API
    // ========================================================================

    /** Get current telemetry */
    UFUNCTION(BlueprintPure, Category = "DTE Expression")
    FExpressionTelemetry GetTelemetry() const { return Telemetry; }

    /** Get final CTRL_ morph target values */
    UFUNCTION(BlueprintPure, Category = "DTE Expression")
    TMap<FName, float> GetFinalCtrlValues() const { return FinalCtrlValues; }

    /** Get final FACS AU values (before CTRL_ conversion) */
    UFUNCTION(BlueprintPure, Category = "DTE Expression")
    TMap<FName, float> GetFinalFACSValues() const { return FinalFACSValues; }

    /** Force a specific expression (bypasses pipeline) */
    UFUNCTION(BlueprintCallable, Category = "DTE Expression")
    void ForceExpression(const TMap<FName, float>& FACSValues, float Duration = 1.0f);

    /** Clear forced expression */
    UFUNCTION(BlueprintCallable, Category = "DTE Expression")
    void ClearForcedExpression();

    /** Signal a cognitive event to the endocrine system */
    UFUNCTION(BlueprintCallable, Category = "DTE Expression")
    void SignalEvent(FName EventType, float Intensity = 1.0f);

    /** Get the Design Studio component */
    UFUNCTION(BlueprintPure, Category = "DTE Expression")
    UAvatarDesignStudio* GetDesignStudio() const { return DesignStudio; }

    /** Get the Endocrine Pipeline component */
    UFUNCTION(BlueprintPure, Category = "DTE Expression")
    UEndocrineExpressionPipeline* GetEndocrinePipeline() const { return EndocrinePipeline; }

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "DTE Expression|Events")
    FOnExpressionEvaluated OnExpressionEvaluated;

protected:
    // ========================================================================
    // Component References
    // ========================================================================

    UPROPERTY()
    UMetaHumanDNACognitiveBridge* DNABridge = nullptr;

    UPROPERTY()
    UEndocrineExpressionPipeline* EndocrinePipeline = nullptr;

    UPROPERTY()
    UAvatarDesignStudio* DesignStudio = nullptr;

    UPROPERTY()
    UExpressiveAnimationSystem* ExpressiveSystem = nullptr;

    UPROPERTY()
    UEvolutionSystem* EvolutionSystem = nullptr;

    UPROPERTY()
    UFACSToMetaHumanMapping* FACSMapping = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMesh = nullptr;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Final blended FACS AU values */
    TMap<FName, float> FinalFACSValues;

    /** Final CTRL_ morph target values */
    TMap<FName, float> FinalCtrlValues;

    /** Previous frame CTRL_ values for smoothing */
    TMap<FName, float> PreviousCtrlValues;

    /** Forced expression state */
    bool bForcedExpression = false;
    TMap<FName, float> ForcedFACSValues;
    float ForcedExpressionTimer = 0.0f;
    float ForcedExpressionDuration = 0.0f;

    /** Telemetry */
    FExpressionTelemetry Telemetry;

    // ========================================================================
    // Pipeline Methods
    // ========================================================================

    /** Find all component references on the owning actor */
    void FindComponents();

    /** Step 1: Collect cognitive base expression */
    TMap<FName, float> CollectCognitiveExpression();

    /** Step 2: Collect endocrine overlay */
    TMap<FName, float> CollectEndocrineOverlay();

    /** Step 3: Generate chaotic micro-expression */
    TMap<FName, float> GenerateChaoticMicroExpression(float DeltaTime);

    /** Step 4: Generate aesthetic overlay */
    TMap<FName, float> GenerateAestheticOverlay();

    /** Step 5: Blend all sources */
    TMap<FName, float> BlendAllSources(
        const TMap<FName, float>& Cognitive,
        const TMap<FName, float>& Endocrine,
        const TMap<FName, float>& Chaotic,
        const TMap<FName, float>& Aesthetic);

    /** Step 6: Convert FACS to CTRL_ */
    TMap<FName, float> ConvertToCtrl(const TMap<FName, float>& FACSValues);

    /** Step 7: Apply evolution limits */
    TMap<FName, float> ApplyEvolutionLimits(const TMap<FName, float>& CtrlValues);

    /** Step 8: Smooth and apply to mesh */
    void SmoothAndApply(const TMap<FName, float>& CtrlValues);

    /** Step 9: Update telemetry */
    void UpdateTelemetry();

    /** Get max AU count for current evolution stage */
    int32 GetMaxAUCountForStage() const;
};
