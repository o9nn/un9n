#pragma once
/**
 * @file UnrealAIBridge.h
 * @brief Bridge between Unreal Engine AI Framework and Deep Tree Echo cognitive architecture
 *
 * Integrates Epic's AI systems into the DTE cognitive avatar:
 * - HTN Planner for hierarchical cognitive planning
 * - StateTree for behavior state machine (cognitive modes)
 * - MassAI for multi-avatar crowd cognition
 * - EQS for environmental awareness
 * - Smart Objects for world interaction
 *
 * Composition: unreal-ai ⊗ DeepTreeEcho
 * Tensor product of game AI with embodied cognition
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealAIBridge.generated.h"

// ============================================================================
// Cognitive Plan (HTN-inspired)
// ============================================================================
UENUM(BlueprintType)
enum class ECognitivePlanType : uint8_t
{
    Idle = 0,
    Observe,
    Think,
    Express,
    Interact,
    Create,
    Dream,
    Transcend,
    COUNT
};

USTRUCT(BlueprintType)
struct FCognitiveTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitivePlanType PlanType = ECognitivePlanType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Preconditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Effects;
};

USTRUCT(BlueprintType)
struct FCognitivePlan
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCognitiveTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentTaskIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlanCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid = false;
};

// ============================================================================
// Cognitive State Tree (StateTree-inspired)
// ============================================================================
UENUM(BlueprintType)
enum class ECognitiveState : uint8_t
{
    // Root states
    Dormant = 0,
    Awakening,
    Active,
    Dreaming,

    // Active sub-states
    Perceiving,
    Reasoning,
    Emoting,
    Expressing,
    Planning,
    Executing,
    Reflecting,

    // Transcendent states
    FlowState,
    CreativeState,
    TranscendentState,
    ChaoticState,

    COUNT
};

USTRUCT(BlueprintType)
struct FCognitiveStateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionCost = 0.0f;
};

// ============================================================================
// Environmental Query (EQS-inspired)
// ============================================================================
USTRUCT(BlueprintType)
struct FEnvironmentalQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QueryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoveltyLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ComfortLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AestheticValue = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyAgentCount = 0;
};

// ============================================================================
// Smart Object Interaction (Smart Objects-inspired)
// ============================================================================
USTRUCT(BlueprintType)
struct FCognitiveSmartObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InterestLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AffordanceStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredCapabilities;
};

// ============================================================================
// Mass Cognition (MassAI-inspired for multi-avatar)
// ============================================================================
USTRUCT(BlueprintType)
struct FMassCognitionFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EntityId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveState CurrentState = ECognitiveState::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttentionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalValence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialInfluence = 0.0f;
};

// ============================================================================
// Unreal AI Bridge Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UUnrealAIBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UUnrealAIBridge();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- HTN Cognitive Planning ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|HTN")
    FCognitivePlan CreateCognitivePlan(ECognitivePlanType GoalType);

    UFUNCTION(BlueprintCallable, Category="UnrealAI|HTN")
    void ExecuteCurrentPlan();

    UFUNCTION(BlueprintCallable, Category="UnrealAI|HTN")
    void AbortCurrentPlan();

    UFUNCTION(BlueprintCallable, Category="UnrealAI|HTN")
    FCognitivePlan GetCurrentPlan() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|HTN")
    float GetPlanProgress() const;

    // --- StateTree Cognitive States ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|StateTree")
    ECognitiveState GetCurrentCognitiveState() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|StateTree")
    bool RequestStateTransition(ECognitiveState TargetState);

    UFUNCTION(BlueprintCallable, Category="UnrealAI|StateTree")
    TArray<ECognitiveState> GetAvailableTransitions() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|StateTree")
    float GetStateResidenceTime() const;

    // --- EQS Environmental Awareness ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|EQS")
    FEnvironmentalQuery QueryEnvironment();

    UFUNCTION(BlueprintCallable, Category="UnrealAI|EQS")
    void UpdateEnvironmentalContext(const FEnvironmentalQuery& Context);

    // --- Smart Object Interaction ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|SmartObjects")
    TArray<FCognitiveSmartObject> GetNearbySmartObjects() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|SmartObjects")
    bool InteractWithSmartObject(const FString& ObjectName);

    // --- MassAI Multi-Avatar ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|Mass")
    void RegisterMassEntity(int32 EntityId);

    UFUNCTION(BlueprintCallable, Category="UnrealAI|Mass")
    void UpdateMassEntity(const FMassCognitionFragment& Fragment);

    UFUNCTION(BlueprintCallable, Category="UnrealAI|Mass")
    TArray<FMassCognitionFragment> GetNearbyEntities(float Radius) const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|Mass")
    float GetCollectiveEmotionalValence() const;

    // --- Cognitive State Export (for DTE Expression System) ---
    UFUNCTION(BlueprintCallable, Category="UnrealAI|Export")
    TMap<FName, float> ExportCognitiveStateAsAUs() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|Export")
    float GetCognitiveLoad() const;

    UFUNCTION(BlueprintCallable, Category="UnrealAI|Export")
    float GetPlanningConfidence() const;

protected:
    // Current state
    ECognitiveState CurrentState;
    FCognitivePlan CurrentPlan;
    FEnvironmentalQuery CurrentEnvironment;
    TArray<FCognitiveSmartObject> SmartObjects;
    TArray<FMassCognitionFragment> MassEntities;
    float StateResidenceTime;

    // State transition table
    TArray<FCognitiveStateTransition> TransitionTable;

    // Internal methods
    void InitializeTransitionTable();
    void UpdateStateTree(float DeltaTime);
    void UpdateHTNExecution(float DeltaTime);
    bool CanTransition(ECognitiveState From, ECognitiveState To) const;

    // State-to-AU mapping for expression
    TMap<FName, float> MapStateToAUs(ECognitiveState State) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UnrealAI|Config")
    float StateTransitionSmoothing = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UnrealAI|Config")
    float HTNReplanInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UnrealAI|Config")
    bool bEnableMassCognition = false;
};
