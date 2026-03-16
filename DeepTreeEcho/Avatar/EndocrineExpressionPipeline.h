// EndocrineExpressionPipeline.h
// Deep Tree Echo Virtual Endocrine System → Avatar Expression Pipeline
// Maps 16 hormone channels from 10 virtual glands to FACS Action Units
// with SIMD-optimized hormone bus, cognitive mode detection, and
// valence-arousal emotional state tracking.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaHumanDNACognitiveBridge.h"
#include "EndocrineExpressionPipeline.generated.h"

// ============================================================================
// Enums
// ============================================================================

/** Cognitive mode detected from hormone state */
UENUM(BlueprintType)
enum class ECognitiveMode : uint8
{
    Resting        UMETA(DisplayName = "Resting - Low arousal, neutral valence"),
    Focused        UMETA(DisplayName = "Focused - Moderate arousal, positive valence"),
    Stressed       UMETA(DisplayName = "Stressed - High cortisol, negative valence"),
    Creative       UMETA(DisplayName = "Creative - Edge of chaos, high dopamine"),
    Social         UMETA(DisplayName = "Social - High oxytocin, positive valence"),
    Alert          UMETA(DisplayName = "Alert - High norepinephrine"),
    Flow           UMETA(DisplayName = "Flow - Optimal arousal, deep engagement"),
    Contemplative  UMETA(DisplayName = "Contemplative - High serotonin, low arousal"),
    Playful        UMETA(DisplayName = "Playful - High anandamide, moderate chaos"),
    Transcendent   UMETA(DisplayName = "Transcendent - All systems harmonized")
};

/** Virtual gland identifier */
UENUM(BlueprintType)
enum class EVirtualGland : uint8
{
    HPA_Axis           UMETA(DisplayName = "HPA Axis (Stress)"),
    Dopaminergic       UMETA(DisplayName = "Dopaminergic (Reward)"),
    Serotonergic       UMETA(DisplayName = "Serotonergic (Mood)"),
    Noradrenergic      UMETA(DisplayName = "Noradrenergic (Arousal)"),
    Oxytocinergic      UMETA(DisplayName = "Oxytocinergic (Bonding)"),
    Thyroid            UMETA(DisplayName = "Thyroid (Processing Rate)"),
    Circadian          UMETA(DisplayName = "Circadian (Sleep/Wake)"),
    Pancreatic         UMETA(DisplayName = "Pancreatic (Energy)"),
    Immune             UMETA(DisplayName = "Immune (Health)"),
    Endocannabinoid    UMETA(DisplayName = "Endocannabinoid (Noise Reduction)")
};

// ============================================================================
// Structs
// ============================================================================

/** Single hormone channel state */
USTRUCT(BlueprintType)
struct FHormoneChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HormoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Concentration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Baseline = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HalfLife = 10.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float DecayRate = 0.0f;
};

/** Complete hormone bus state (16 channels, SIMD-aligned) */
USTRUCT(BlueprintType)
struct FHormoneBusState
{
    GENERATED_BODY()

    /** 16 hormone concentrations (SIMD-aligned for AVX2) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FHormoneChannel> Channels;

    /** Current cognitive mode */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ECognitiveMode CurrentMode = ECognitiveMode::Resting;

    /** Valence (-1 to +1, negative to positive) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Valence = 0.0f;

    /** Arousal (0 to 1, calm to excited) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Arousal = 0.0f;

    /** Dominance (0 to 1, submissive to dominant) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Dominance = 0.5f;
};

/** Hormone-to-AU mapping entry */
USTRUCT(BlueprintType)
struct FHormoneAUMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HormoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionUnitName;

    /** Mapping weight (how strongly this hormone drives this AU) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 1.0f;

    /** Threshold below which no effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Threshold = 0.0f;

    /** Whether this is an inhibitory mapping (reduces AU activation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInhibitory = false;
};

/** Cognitive event that triggers gland response */
USTRUCT(BlueprintType)
struct FCognitiveEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCognitiveModeChanged, ECognitiveMode, OldMode, ECognitiveMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHormoneSpike, FName, HormoneName, float, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValenceArousalChanged, float, Valence, float, Arousal);

// ============================================================================
// UEndocrineExpressionPipeline
// ============================================================================

/**
 * Endocrine Expression Pipeline
 * 
 * Implements the complete virtual endocrine system for Deep Tree Echo avatars.
 * 10 virtual glands produce 16 hormone channels that modulate:
 * - FACS Action Unit activations (facial expression)
 * - Material parameters (skin flush, eye dilation, etc.)
 * - Cognitive mode detection (flow, stress, creativity, etc.)
 * - Valence-arousal emotional state tracking
 * 
 * The pipeline runs at tick rate with exponential decay toward baselines,
 * responding to cognitive events from the Echobeats cycle.
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class DEEPTREEECHO_API UEndocrineExpressionPipeline : public UActorComponent
{
    GENERATED_BODY()

public:
    UEndocrineExpressionPipeline();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // Hormone Bus API
    // ========================================================================

    /** Get current hormone bus state */
    UFUNCTION(BlueprintPure, Category = "Endocrine Pipeline")
    FHormoneBusState GetHormoneBusState() const { return BusState; }

    /** Get concentration of a specific hormone */
    UFUNCTION(BlueprintPure, Category = "Endocrine Pipeline")
    float GetHormoneLevel(FName HormoneName) const;

    /** Set hormone baseline (for design studio) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SetHormoneBaseline(FName HormoneName, float Baseline);

    /** Get current cognitive mode */
    UFUNCTION(BlueprintPure, Category = "Endocrine Pipeline")
    ECognitiveMode GetCognitiveMode() const { return BusState.CurrentMode; }

    /** Get valence-arousal state */
    UFUNCTION(BlueprintPure, Category = "Endocrine Pipeline")
    void GetValenceArousal(float& OutValence, float& OutArousal) const;

    // ========================================================================
    // Cognitive Event API
    // ========================================================================

    /** Signal a cognitive event (triggers gland responses) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalCognitiveEvent(FName EventType, float Intensity = 1.0f);

    /** Signal threat detected (HPA axis activation) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalThreat(float Intensity = 1.0f);

    /** Signal reward received (dopaminergic activation) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalReward(float Intensity = 1.0f);

    /** Signal social connection (oxytocinergic activation) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalSocialConnection(float Intensity = 1.0f);

    /** Signal novelty encountered (noradrenergic activation) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalNovelty(float Intensity = 1.0f);

    /** Signal goal achieved (broad positive cascade) */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void SignalGoalAchieved(float Intensity = 1.0f);

    // ========================================================================
    // Expression Mapping API
    // ========================================================================

    /** Get computed AU activations from current hormone state */
    UFUNCTION(BlueprintPure, Category = "Endocrine Pipeline")
    TMap<FName, float> GetHormoneDriverAUValues() const;

    /** Connect to MetaHuman DNA bridge for direct expression driving */
    UFUNCTION(BlueprintCallable, Category = "Endocrine Pipeline")
    void ConnectToDNABridge(UMetaHumanDNACognitiveBridge* Bridge);

    // ========================================================================
    // Delegates
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Endocrine Pipeline|Events")
    FOnCognitiveModeChanged OnCognitiveModeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Endocrine Pipeline|Events")
    FOnHormoneSpike OnHormoneSpike;

    UPROPERTY(BlueprintAssignable, Category = "Endocrine Pipeline|Events")
    FOnValenceArousalChanged OnValenceArousalChanged;

protected:
    /** Hormone bus state */
    UPROPERTY()
    FHormoneBusState BusState;

    /** Hormone-to-AU mappings */
    UPROPERTY()
    TArray<FHormoneAUMapping> HormoneAUMappings;

    /** Connected DNA bridge */
    UPROPERTY()
    UMetaHumanDNACognitiveBridge* ConnectedBridge = nullptr;

    /** Initialize 16 hormone channels with defaults */
    void InitializeHormoneChannels();

    /** Initialize hormone-to-AU mappings */
    void InitializeHormoneAUMappings();

    /** Update hormone decay (exponential toward baseline) */
    void UpdateHormoneDecay(float DeltaTime);

    /** Detect cognitive mode from current hormone state */
    ECognitiveMode DetectCognitiveMode() const;

    /** Compute valence-arousal from hormone state */
    void ComputeValenceArousal(float& OutValence, float& OutArousal) const;

    /** Apply hormone-driven AU values to connected bridge */
    void ApplyHormoneExpressions();

    /** Get channel index by name */
    int32 GetChannelIndex(FName HormoneName) const;

    /** Inject hormone into a channel */
    void InjectHormone(FName HormoneName, float Amount);
};
