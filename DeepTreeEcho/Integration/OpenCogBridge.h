#pragma once
/**
 * @file OpenCogBridge.h
 * @brief Bridge between Modern OpenCog AtomSpace and Deep Tree Echo cognitive architecture
 *
 * This bridge connects the opencog-modern C++23 reimplementation to the DTE
 * cognitive avatar system, enabling:
 * - AtomSpace-backed cognitive state storage
 * - ECAN attention allocation for expression priority
 * - PLN inference for cognitive reasoning about emotional states
 * - Pattern matching for behavioral trigger detection
 * - Endocrine system integration (32 hormone channels)
 * - Entelechy/Cloninger temperament personality modeling
 * - Time crystal temporal scheduling
 * - Nervous system neuroendocrine bridge
 *
 * Composition: opencog-modern ⊗ DeepTreeEcho
 * Tensor product of symbolic reasoning with embodied cognition
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpenCogBridge.generated.h"

// Forward declarations for opencog-modern types
namespace opencog {
    class AtomSpace;
    namespace endo {
        struct EndocrineState;
        struct HormoneState;
        enum class CognitiveMode : uint8_t;
    }
    namespace entelechy {
        struct TemperamentProfile;
        struct DevelopmentalStage;
        enum class AttachmentStyle : uint8_t;
        enum class PolyvagalTier : uint8_t;
    }
    namespace temporal {
        class TimeCrystalBrain;
    }
    namespace nervous {
        class NervousSystem;
    }
}

// ============================================================================
// Hormone Channel IDs (matching opencog-modern endocrine/types.hpp)
// ============================================================================
UENUM(BlueprintType)
enum class EOpenCogHormone : uint8_t
{
    CRH = 0,
    ACTH,
    Cortisol,
    DA_Tonic,
    DA_Phasic,
    Serotonin,
    Norepinephrine,
    Oxytocin,
    T3_T4,
    Melatonin,
    Insulin,
    Glucagon,
    IL6,
    Anandamide,
    NPU_Load,
    CogCoherence,
    Marduk_Load,
    OrgCoherence,
    Touchpad_Load,
    TouchpadCoherence,
    VagalTone,
    SympatheticDrive,
    DorsalVagal,
    CardiacCoherence,
    RespiratoryRhythm,
    GutBrainSignal,
    ImmuneExtended,
    InsularIntegration,
    AllostaticLoad,
    ProprioceptiveTone,
    NociceptiveSignal,
    Thermoregulatory,
    COUNT
};

// ============================================================================
// Cognitive Mode (matching opencog-modern)
// ============================================================================
UENUM(BlueprintType)
enum class EOpenCogCognitiveMode : uint8_t
{
    Resting = 0,
    Focused,
    Stressed,
    Creative,
    Social,
    Alert,
    Flow,
    Contemplative,
    Playful,
    Transcendent,
    COUNT
};

// ============================================================================
// Cloninger Temperament (matching opencog-modern entelechy)
// ============================================================================
USTRUCT(BlueprintType)
struct FClonigerTemperament
{
    GENERATED_BODY()

    // Temperament dimensions (innate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float NoveltySeeking = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float HarmAvoidance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float RewardDependence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Persistence = 0.5f;

    // Character dimensions (developed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float SelfDirectedness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Cooperativeness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float SelfTranscendence = 0.5f;

    // Developmental modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Maturation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Resilience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Plasticity = 1.0f;

    // Attachment (Bowlby)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float AttachmentSecurity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float AttachmentAnxiety = 0.3f;
};

// ============================================================================
// Polyvagal State
// ============================================================================
UENUM(BlueprintType)
enum class EPolyvagalTier : uint8_t
{
    VentralVagal = 0,   // Social engagement
    Sympathetic,        // Fight/flight
    DorsalVagal,        // Freeze/shutdown
};

// ============================================================================
// Interoceptive State
// ============================================================================
USTRUCT(BlueprintType)
struct FInteroceptiveState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeartRateVariability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespiratoryCoherence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GutBrainAxis = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProprioceptiveTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NociceptiveLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThermoregulationBalance = 0.5f;
};

// ============================================================================
// AtomSpace Cognitive State Snapshot
// ============================================================================
USTRUCT(BlueprintType)
struct FAtomSpaceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalAtoms = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ConceptNodes = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 InheritanceLinks = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EvaluationLinks = 0;

    UPROPERTY(BlueprintReadOnly)
    float AttentionalFocus = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PLNConfidence = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActivePatterns = 0;
};

// ============================================================================
// OpenCog Bridge Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UOpenCogBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UOpenCogBridge();

    // --- Lifecycle ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // --- AtomSpace Operations ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|AtomSpace")
    int32 AddConceptNode(const FString& Name, float Strength = 1.0f, float Confidence = 0.9f);

    UFUNCTION(BlueprintCallable, Category="OpenCog|AtomSpace")
    int32 AddInheritanceLink(int32 SubAtomId, int32 SuperAtomId,
                             float Strength = 0.9f, float Confidence = 0.8f);

    UFUNCTION(BlueprintCallable, Category="OpenCog|AtomSpace")
    int32 AddEvaluationLink(const FString& Predicate, const TArray<int32>& Arguments,
                            float Strength = 0.8f, float Confidence = 0.7f);

    UFUNCTION(BlueprintCallable, Category="OpenCog|AtomSpace")
    FAtomSpaceSnapshot GetSnapshot() const;

    // --- ECAN Attention ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Attention")
    void StimulateAtom(int32 AtomId, float Amount);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Attention")
    float GetAttentionValue(int32 AtomId) const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Attention")
    TArray<int32> GetAttentionalFocus(int32 MaxCount = 10) const;

    // --- PLN Inference ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|PLN")
    TArray<int32> ForwardChain(int32 SourceAtomId, int32 MaxSteps = 10);

    UFUNCTION(BlueprintCallable, Category="OpenCog|PLN")
    TArray<int32> BackwardChain(int32 TargetAtomId, int32 MaxSteps = 10);

    // --- Pattern Matching ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Pattern")
    TArray<int32> QueryPattern(int32 PatternAtomId);

    // --- Endocrine Integration (32 channels) ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Endocrine")
    void SetHormoneLevel(EOpenCogHormone Hormone, float Level);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Endocrine")
    float GetHormoneLevel(EOpenCogHormone Hormone) const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Endocrine")
    void InjectCognitiveEvent(const FString& EventType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Endocrine")
    EOpenCogCognitiveMode GetCurrentCognitiveMode() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Endocrine")
    void GetValenceArousalDominance(float& Valence, float& Arousal, float& Dominance) const;

    // --- Entelechy / Temperament ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Entelechy")
    void SetTemperament(const FClonigerTemperament& Temperament);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Entelechy")
    FClonigerTemperament GetTemperament() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Entelechy")
    EPolyvagalTier GetPolyvagalState() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Entelechy")
    FInteroceptiveState GetInteroceptiveState() const;

    // --- Nervous System ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Nervous")
    void SendNerveSignal(const FString& NucleusName, float Intensity);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Nervous")
    float GetNeuroendocrineBridgeOutput(EOpenCogHormone Hormone) const;

    // --- Time Crystal Temporal ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Temporal")
    void TickTimeCrystal(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="OpenCog|Temporal")
    int32 GetCurrentTemporalHierarchyLevel() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Temporal")
    float GetTimeCrystalPhase(int32 Level) const;

    // --- Cognitive State Export (for DTE Expression System) ---
    UFUNCTION(BlueprintCallable, Category="OpenCog|Export")
    TMap<FName, float> ExportCognitiveStateAsAUs() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Export")
    TMap<FName, float> ExportHormoneStateAsAUs() const;

    UFUNCTION(BlueprintCallable, Category="OpenCog|Export")
    float GetOverallCognitiveCoherence() const;

protected:
    // Internal state
    TArray<float> HormoneLevels;
    FClonigerTemperament CurrentTemperament;
    EOpenCogCognitiveMode CurrentMode;
    EPolyvagalTier CurrentPolyvagalTier;
    FInteroceptiveState CurrentInteroceptive;
    FAtomSpaceSnapshot CachedSnapshot;

    // Internal update methods
    void UpdateEndocrineDecay(float DeltaTime);
    void UpdateCognitiveMode();
    void UpdatePolyvagalState();
    void UpdateInteroceptiveState(float DeltaTime);
    void UpdateTimeCrystalHierarchy(float DeltaTime);
    void SyncAtomSpaceWithCognitiveState();

    // Hormone-to-AU mapping (extended from EndocrineExpressionPipeline)
    TMap<FName, float> ComputeHormoneAUs() const;
    TMap<FName, float> ComputeTemperamentAUs() const;
    TMap<FName, float> ComputePolyvagalAUs() const;

    // Time crystal state
    TArray<float> TimeCrystalPhases;   // 12 hierarchy levels
    TArray<float> TimeCrystalPeriods;  // Period per level
    int32 CurrentTemporalLevel = 0;

    // Hormone decay rates (per second)
    TArray<float> HormoneDecayRates;

public:
    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    bool bEnableEndocrine = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    bool bEnableEntelechy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    bool bEnableTimeCrystal = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    bool bEnableNervousSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    float EndocrineTickRate = 0.016f;  // ~60Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCog|Config")
    float TimeCrystalTickRate = 0.008f;  // ~120Hz for temporal precision
};
