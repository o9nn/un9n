#pragma once
/**
 * @file MasterOrchestrator.h
 * @brief Master Integration Orchestrator for Deep Tree Echo
 *
 * The unified orchestration layer that composes all integration bridges:
 *
 *   DTE = OpenCogBridge ⊗ UnrealAIBridge ⊗ NPUBridge ⊗ HyperSkillTensor
 *
 * This component:
 * 1. Manages the Echobeats 9-step cognitive cycle
 * 2. Routes cognitive state between all bridges
 * 3. Produces final AU expression values for the avatar
 * 4. Manages the ○ → ⊙ self-awareness progression
 * 5. Implements the 4E embodied cognition framework
 *
 * The 9-step Echobeats cycle:
 *   1. Sense    → Perceive environment (EQS)
 *   2. Filter   → Attention allocation (ECAN)
 *   3. Resonate → Emotional response (Endocrine)
 *   4. Echo     → Memory retrieval (AtomSpace)
 *   5. Amplify  → Pattern amplification (NPU-253)
 *   6. Modulate → Cognitive mode selection (StateTree)
 *   7. Express  → Avatar expression (FACS → CTRL_)
 *   8. Reflect  → Self-monitoring (Autognosis)
 *   9. Evolve   → Ontogenetic growth (Evolution)
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MasterOrchestrator.generated.h"

class UOpenCogBridge;
class UUnrealAIBridge;
class UNPUBridge;
class UHyperSkillTensor;

// ============================================================================
// Echobeats Cycle Phase
// ============================================================================
UENUM(BlueprintType)
enum class EEchobeatPhase : uint8_t
{
    Sense = 0,
    Filter,
    Resonate,
    Echo,
    Amplify,
    Modulate,
    Express,
    Reflect,
    Evolve,
    COUNT
};

// ============================================================================
// 4E Cognition Dimensions
// ============================================================================
USTRUCT(BlueprintType)
struct F4ECognitionState
{
    GENERATED_BODY()

    /** Embodied: body-based cognition, proprioception, interoception */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Embodied = 0.5f;

    /** Embedded: environmental coupling, affordances, niche construction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Embedded = 0.5f;

    /** Enacted: sensorimotor coupling, action-perception loops */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Enacted = 0.5f;

    /** Extended: tool use, external memory, distributed cognition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
    float Extended = 0.5f;
};

// ============================================================================
// Orchestrator Telemetry
// ============================================================================
USTRUCT(BlueprintType)
struct FOrchestratorTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEchobeatPhase CurrentPhase = EEchobeatPhase::Sense;

    UPROPERTY(BlueprintReadOnly)
    int64 TotalCycles = 0;

    UPROPERTY(BlueprintReadOnly)
    float CycleFrequencyHz = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float SelfAwareness = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    F4ECognitionState CognitionState;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveBridges = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAUs = 0;

    UPROPERTY(BlueprintReadOnly)
    float OverallCoherence = 0.0f;
};

// ============================================================================
// Master Orchestrator Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UMasterOrchestrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UMasterOrchestrator();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- Echobeats Cycle ---
    UFUNCTION(BlueprintCallable, Category="DTE|Cycle")
    EEchobeatPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category="DTE|Cycle")
    void ForcePhase(EEchobeatPhase Phase);

    UFUNCTION(BlueprintCallable, Category="DTE|Cycle")
    float GetPhaseProgress() const;

    // --- 4E Cognition ---
    UFUNCTION(BlueprintCallable, Category="DTE|4E")
    F4ECognitionState Get4ECognitionState() const;

    UFUNCTION(BlueprintCallable, Category="DTE|4E")
    void Set4ECognitionState(const F4ECognitionState& State);

    // --- Expression Output ---
    UFUNCTION(BlueprintCallable, Category="DTE|Expression")
    TMap<FName, float> GetFinalExpressionAUs() const;

    UFUNCTION(BlueprintCallable, Category="DTE|Expression")
    float GetExpressionIntensity() const;

    // --- Telemetry ---
    UFUNCTION(BlueprintCallable, Category="DTE|Telemetry")
    FOrchestratorTelemetry GetTelemetry() const;

    // --- Bridge Management ---
    UFUNCTION(BlueprintCallable, Category="DTE|Bridges")
    void ConnectAllBridges();

    UFUNCTION(BlueprintCallable, Category="DTE|Bridges")
    int32 GetActiveBridgeCount() const;

protected:
    // Bridge references (auto-discovered)
    UPROPERTY()
    UOpenCogBridge* OpenCogBridge;

    UPROPERTY()
    UUnrealAIBridge* UnrealAIBridge;

    UPROPERTY()
    UNPUBridge* NPUBridgeRef;

    UPROPERTY()
    UHyperSkillTensor* HyperSkillTensor;

    // Cycle state
    EEchobeatPhase CurrentPhase;
    float PhaseTimer;
    float PhaseProgress;
    int64 TotalCycles;
    float CycleAccumulator;

    // 4E state
    F4ECognitionState CognitionState4E;

    // Final expression
    TMap<FName, float> FinalAUs;

    // Internal methods
    void AdvancePhase();
    void ExecutePhase_Sense(float DeltaTime);
    void ExecutePhase_Filter(float DeltaTime);
    void ExecutePhase_Resonate(float DeltaTime);
    void ExecutePhase_Echo(float DeltaTime);
    void ExecutePhase_Amplify(float DeltaTime);
    void ExecutePhase_Modulate(float DeltaTime);
    void ExecutePhase_Express(float DeltaTime);
    void ExecutePhase_Reflect(float DeltaTime);
    void ExecutePhase_Evolve(float DeltaTime);
    void ComputeFinalExpression();
    void Update4ECognition(float DeltaTime);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DTE|Config")
    float CyclePeriod = 1.0f;  // Full 9-step cycle in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DTE|Config")
    float ExpressionSmoothing = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DTE|Config")
    bool bAutoConnectBridges = true;
};
