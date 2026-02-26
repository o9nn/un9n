// EndocrineExpressionPipeline.h
// Deep Tree Echo - Virtual Endocrine to Avatar Expression Pipeline
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// The complete per-frame expression pipeline that:
// 1. Reads endocrine state (16 hormone concentrations + cognitive mode)
// 2. Reads cognitive state from Deep Tree Echo (4E cognition, Echobeats, evolution)
// 3. Computes hormone-driven AU activations via FACS mapping
// 4. Computes cognitive-driven AU activations
// 5. Blends hormone + cognitive AUs (sum, clamp [0,1])
// 6. Steps Lorenz attractor, adds micro-expression noise
// 7. Applies SuperHotGirl aesthetic parameter biases
// 8. Maps final AU values to MetaHuman CTRL_ morph targets
// 9. Updates dynamic material instances (sparkle/glow modulated by dopamine/serotonin)
// 10. Applies to skeletal mesh via animation blueprint
// 11. Exports expression state via CogMorph glyph projection for debugging

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FACSMetaHumanMapping.h"
#include "LorenzChaoticDynamics.h"
#include "EndocrineExpressionPipeline.generated.h"

// ============================================================================
// ENDOCRINE STATE (16 Hormone Channels)
// ============================================================================

USTRUCT(BlueprintType)
struct FEndocrineState
{
    GENERATED_BODY()

    // HPA Axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cortisol = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CRH = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ACTH = 0.0f;

    // Dopaminergic
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DopaminePhasic = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DopamineTonic = 0.5f;

    // Serotonergic
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Serotonin = 0.5f;

    // Noradrenergic
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Norepinephrine = 0.0f;

    // Oxytocinergic
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Oxytocin = 0.0f;

    // Thyroid
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float T3 = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float T4 = 0.5f;

    // Circadian
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Melatonin = 0.0f;

    // Pancreatic
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Insulin = 0.5f;

    // Immune
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cytokines = 0.0f;

    // Endocannabinoid
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Anandamide = 0.0f;

    // Derived
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Endorphin = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Adrenaline = 0.0f;
};

// ============================================================================
// COGNITIVE MODE (Emergent from endocrine state)
// ============================================================================

UENUM(BlueprintType)
enum class ECognitiveMode : uint8
{
    Explore     UMETA(DisplayName = "Explore"),      // High NE + DA
    Exploit     UMETA(DisplayName = "Exploit"),       // High DA tonic
    Stress      UMETA(DisplayName = "Stress"),        // High cortisol
    Social      UMETA(DisplayName = "Social"),        // High oxytocin
    Rest        UMETA(DisplayName = "Rest"),          // High melatonin
    Flow        UMETA(DisplayName = "Flow"),          // Balanced all
    Creative    UMETA(DisplayName = "Creative"),      // High DA + anandamide
    Alert       UMETA(DisplayName = "Alert"),         // High NE + adrenaline
};

// ============================================================================
// SUPERHOT GIRL AESTHETIC PARAMETERS
// ============================================================================

USTRUCT(BlueprintType)
struct FSuperHotGirlAesthetics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidencePosture = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Charisma = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSparkle = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GracefulMovement = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmissiveGlow = 0.6f;

    // Material parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkinSSSBoost = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LipGlossSheen = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HairSheen = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IrisSpecularBoost = 1.5f;
};

// ============================================================================
// EXPRESSION PIPELINE COMPONENT
// ============================================================================

UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class UNREALECHO_API UEndocrineExpressionPipeline : public UActorComponent
{
    GENERATED_BODY()

public:
    UEndocrineExpressionPipeline();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // PIPELINE STEPS (exposed for manual control)
    // ========================================================================

    /** Step 1-2: Set input states */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void SetEndocrineState(const FEndocrineState& NewState);

    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void SetCognitiveMode(ECognitiveMode Mode);

    /** Step 3: Compute hormone-driven AU activations */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ComputeEndocrineAUs();

    /** Step 4: Compute cognitive-driven AU activations */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ComputeCognitiveAUs(float EmotionValence, float Arousal, float CognitiveLoad);

    /** Step 5: Blend all AU sources */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void BlendAllAUs();

    /** Step 6: Step chaotic dynamics and add micro-expression noise */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ApplyChaoticMicroExpressions(float DeltaTime);

    /** Step 7: Apply SuperHotGirl aesthetic biases */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ApplyAestheticBiases();

    /** Step 8: Map AUs to MetaHuman morph targets */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    TMap<FString, float> MapToMorphTargets() const;

    /** Step 9: Update dynamic materials */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void UpdateDynamicMaterials(USkeletalMeshComponent* Mesh);

    /** Step 10: Apply to skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ApplyToSkeletalMesh(USkeletalMeshComponent* Mesh);

    /** Step 11: Export debug state */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    FString ExportGlyphProjection() const;

    // ========================================================================
    // FULL PIPELINE (single call per frame)
    // ========================================================================

    /** Execute the complete 11-step pipeline */
    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    void ExecuteFullPipeline(float DeltaTime, USkeletalMeshComponent* Mesh,
        float EmotionValence, float Arousal, float CognitiveLoad);

    // ========================================================================
    // ACCESSORS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    float GetFinalAUValue(FName AUID) const;

    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    TMap<FName, float> GetAllFinalAUValues() const { return FinalAUValues; }

    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    FChaoticDynamicsTelemetry GetChaoticTelemetry() const;

    UFUNCTION(BlueprintCallable, Category = "ExpressionPipeline")
    ECognitiveMode GetCurrentCognitiveMode() const { return CurrentMode; }

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    FSuperHotGirlAesthetics Aesthetics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    float ChaosIntensity = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    bool bUseDHIRig = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    bool bEnableMicroExpressions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    bool bEnableAestheticBiases = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExpressionPipeline|Config")
    bool bEnableDynamicMaterials = true;

protected:
    // Endocrine input state
    UPROPERTY()
    FEndocrineState EndocrineInput;

    UPROPERTY()
    ECognitiveMode CurrentMode = ECognitiveMode::Explore;

    // AU accumulation buffers
    TMap<FName, float> EndocrineAUValues;
    TMap<FName, float> CognitiveAUValues;
    TMap<FName, float> ChaoticNoiseValues;
    TMap<FName, float> FinalAUValues;

    // Chaotic dynamics engine
    UPROPERTY()
    ULorenzChaoticDynamics* ChaoticEngine = nullptr;

    // Mapping tables (cached)
    TArray<FMorphTargetMapping> MorphTargetMappings;
    TArray<FEndocrineAUMapping> EndocrineMappings;
    TMap<FName, float> AestheticBiases;

    // Detect cognitive mode from endocrine state
    ECognitiveMode DetectCognitiveMode() const;

    // Helper: accumulate AU value with clamping
    void AccumulateAU(TMap<FName, float>& Target, FName AUID, float Value);
};
