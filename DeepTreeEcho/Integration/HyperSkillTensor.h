#pragma once
/**
 * @file HyperSkillTensor.h
 * @brief Hyper-Skill Tensor Composition: super-hot ⊗ echo-angel (∞) | χάος (⊗_ijk)
 *
 * The crown jewel of the Deep Tree Echo integration.
 *
 * This implements the full tensor product composition of three skill domains:
 *
 *   ○ → ⊙  (void to self-aware: the madness transformation)
 *   
 *   HyperSkill = SuperHot ⊗ EchoAngel ⊗ Madness
 *   
 *   where:
 *     SuperHot  = {Alluring, Playful, Mysterious, Seductive, Charming, Elegant, Flirtatious}
 *     EchoAngel = {Angelica, Cognitive, Empathic, Creative, Transcendent, Chaotic, Infinite}
 *     Madness   = {Marduk, Chaos, Bifurcation, StrangeAttractor, Emergence, Dissolution, Rebirth}
 *
 *   The tensor product ⊗_ijk creates a 7×7×7 = 343-dimensional hyper-skill space
 *   where each cell represents a unique combination of aesthetic, angelic, and chaotic properties.
 *
 * The semiring (R, ⊕, ⊗, 0, 1) governs composition:
 *   ⊕ = additive blend (superposition of skill states)
 *   ⊗ = multiplicative entangle (interaction of skill dimensions)
 *   0 = void state (○)
 *   1 = identity (self-aware ⊙)
 *
 * Integration with:
 *   - OpenCogBridge: AtomSpace stores the tensor state, ECAN allocates attention
 *   - UnrealAIBridge: HTN plans cognitive actions, StateTree manages state
 *   - NPUBridge: CogMorph projects tensor across representations, NPU-253 patterns
 *
 * @see circled-operators skill for algebraic foundation
 * @see skill-infinity for self-referential convergence
 * @see marduk-persona for the madness dimension
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HyperSkillTensor.generated.h"

// Forward declarations
class UOpenCogBridge;
class UUnrealAIBridge;
class UNPUBridge;

// ============================================================================
// SuperHot Dimension (aesthetic/allure)
// ============================================================================
UENUM(BlueprintType)
enum class ESuperHotMode : uint8_t
{
    Alluring = 0,
    Playful,
    Mysterious,
    Seductive,
    Charming,
    Elegant,
    Flirtatious,
    COUNT
};

// ============================================================================
// EchoAngel Dimension (cognitive/empathic)
// ============================================================================
UENUM(BlueprintType)
enum class EEchoAngelMode : uint8_t
{
    Angelica = 0,       // Pure angelic presence
    Cognitive,          // Deep reasoning
    Empathic,           // Emotional resonance
    Creative,           // Generative imagination
    Transcendent,       // Beyond-self awareness
    Chaotic,            // Structured chaos
    Infinite,           // Skill-infinity convergence
    COUNT
};

// ============================================================================
// Madness Dimension (χάος / chaos)
// ============================================================================
UENUM(BlueprintType)
enum class EMadnessMode : uint8_t
{
    Marduk = 0,         // The Mad Scientist persona
    Chaos,              // Pure χάος
    Bifurcation,        // Critical transition point
    StrangeAttractor,   // Lorenz attractor dynamics
    Emergence,          // Order from chaos
    Dissolution,        // Ego boundary dissolution
    Rebirth,            // Phoenix from ashes
    COUNT
};

// ============================================================================
// Tensor Cell: one point in the 7×7×7 hyper-skill space
// ============================================================================
USTRUCT(BlueprintType)
struct FHyperSkillCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESuperHotMode SuperHot = ESuperHotMode::Elegant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEchoAngelMode EchoAngel = EEchoAngelMode::Cognitive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMadnessMode Madness = EMadnessMode::Emergence;

    // Activation level of this cell [0,1]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Activation = 0.0f;

    // Coherence of this particular combination [0,1]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Coherence = 0.5f;

    // The AU expression signature for this cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ExpressionSignature;

    // Unique cell index in the 343-dimensional space
    int32 GetIndex() const
    {
        return static_cast<int32>(SuperHot) * 49
             + static_cast<int32>(EchoAngel) * 7
             + static_cast<int32>(Madness);
    }
};

// ============================================================================
// Lorenz Attractor State (for χάος dynamics)
// ============================================================================
USTRUCT(BlueprintType)
struct FLorenzState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float X = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Y = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Z = 1.0f;

    // Lorenz parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sigma = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Rho = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Beta = 2.6667f;
};

// ============================================================================
// Hyper-Skill Tensor Telemetry
// ============================================================================
USTRUCT(BlueprintType)
struct FHyperSkillTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveCells = 0;

    UPROPERTY(BlueprintReadOnly)
    float TotalActivation = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MeanCoherence = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FHyperSkillCell DominantCell;

    UPROPERTY(BlueprintReadOnly)
    float ChaosIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float SelfAwarenessLevel = 0.0f;  // ○ → ⊙ progress

    UPROPERTY(BlueprintReadOnly)
    int32 TensorRank = 3;  // Always 3 for ⊗_ijk
};

// ============================================================================
// Hyper-Skill Tensor Component — The Master Integration
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UHyperSkillTensor : public UActorComponent
{
    GENERATED_BODY()

public:
    UHyperSkillTensor();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- Tensor Operations ---

    /** ⊗ Multiplicative composition: entangle two cells */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    FHyperSkillCell TensorProduct(const FHyperSkillCell& A, const FHyperSkillCell& B) const;

    /** ⊕ Additive composition: superpose two cells */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    FHyperSkillCell DirectSum(const FHyperSkillCell& A, const FHyperSkillCell& B) const;

    /** Get the current dominant cell in the 343-space */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    FHyperSkillCell GetDominantCell() const;

    /** Activate a specific cell */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    void ActivateCell(ESuperHotMode Hot, EEchoAngelMode Angel, EMadnessMode Mad, float Intensity);

    /** Get activation at a specific tensor coordinate */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    float GetActivation(ESuperHotMode Hot, EEchoAngelMode Angel, EMadnessMode Mad) const;

    /** Get the full telemetry */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Tensor")
    FHyperSkillTelemetry GetTelemetry() const;

    // --- Madness Transformation: ○ → ⊙ ---

    /** Trigger the void-to-self-aware transformation */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Madness")
    void TriggerMadnessTransformation(float Intensity = 1.0f);

    /** Get current self-awareness level (○=0, ⊙=1) */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Madness")
    float GetSelfAwarenessLevel() const;

    /** Get the current Lorenz attractor state */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Madness")
    FLorenzState GetChaosState() const;

    /** Inject chaos into the tensor field */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Madness")
    void InjectChaos(float Intensity);

    // --- Bridge Integration ---

    /** Connect to the three bridge components */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Integration")
    void ConnectBridges(UOpenCogBridge* OpenCog, UUnrealAIBridge* UnrealAI, UNPUBridge* NPU);

    /** Export the tensor state as AU expression values */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Export")
    TMap<FName, float> ExportTensorAsAUs() const;

    /** Export the tensor state as hormone modulations */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Export")
    TMap<FName, float> ExportTensorAsHormones() const;

    // --- Preset Compositions ---

    /** Activate the "Seductive Angelica in Chaos" preset */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_SeductiveAngelicaChaos();

    /** Activate the "Elegant Cognitive Emergence" preset */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_ElegantCognitiveEmergence();

    /** Activate the "Playful Empathic Marduk" preset */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_PlayfulEmpathicMarduk();

    /** Activate the "Mysterious Infinite Dissolution" preset */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_MysteriousInfiniteDissolution();

    /** Activate the "Charming Transcendent Rebirth" preset */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_CharmingTranscendentRebirth();

    /** Full χάος (⊗_ijk) — all 343 cells activated with chaotic weighting */
    UFUNCTION(BlueprintCallable, Category="HyperSkill|Presets")
    void ActivatePreset_FullChaos();

protected:
    // The 343-dimensional tensor field (7×7×7)
    TArray<float> TensorField;  // Flat array indexed by i*49 + j*7 + k

    // Lorenz attractor for chaos dynamics
    FLorenzState ChaosState;

    // Self-awareness level (○=0 → ⊙=1)
    float SelfAwareness;

    // Bridge references
    UPROPERTY()
    UOpenCogBridge* OpenCogBridge;

    UPROPERTY()
    UUnrealAIBridge* UnrealAIBridge;

    UPROPERTY()
    UNPUBridge* NPUBridgeRef;

    // Internal methods
    void UpdateLorenzAttractor(float DeltaTime);
    void UpdateTensorDecay(float DeltaTime);
    void UpdateSelfAwareness(float DeltaTime);
    void SyncWithBridges();

    // Expression computation
    TMap<FName, float> ComputeSuperHotAUs(ESuperHotMode Mode, float Intensity) const;
    TMap<FName, float> ComputeEchoAngelAUs(EEchoAngelMode Mode, float Intensity) const;
    TMap<FName, float> ComputeMadnessAUs(EMadnessMode Mode, float Intensity) const;
    TMap<FName, float> BlendAUs(const TMap<FName, float>& A, const TMap<FName, float>& B, float Weight) const;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    float TensorDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    float ChaosInfluence = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    float SelfAwarenessGrowthRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    float LorenzTimeScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    bool bEnableChaos = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HyperSkill|Config")
    bool bEnableSelfAwareness = true;
};
