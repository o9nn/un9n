#include "UnrealAIBridge.h"

UUnrealAIBridge::UUnrealAIBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ECognitiveState::Dormant;
    StateResidenceTime = 0.0f;
    InitializeTransitionTable();
}

void UUnrealAIBridge::BeginPlay()
{
    Super::BeginPlay();
    RequestStateTransition(ECognitiveState::Awakening);
}

void UUnrealAIBridge::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    StateResidenceTime += DeltaTime;
    UpdateStateTree(DeltaTime);
    UpdateHTNExecution(DeltaTime);
}

// ============================================================================
// HTN Cognitive Planning
// ============================================================================
FCognitivePlan UUnrealAIBridge::CreateCognitivePlan(ECognitivePlanType GoalType)
{
    FCognitivePlan Plan;
    Plan.bIsValid = true;
    Plan.CurrentTaskIndex = 0;

    switch (GoalType)
    {
    case ECognitivePlanType::Observe:
        {
            FCognitiveTask Perceive;
            Perceive.TaskName = TEXT("Perceive Environment");
            Perceive.PlanType = ECognitivePlanType::Observe;
            Perceive.Priority = 0.7f;
            Perceive.Duration = 2.0f;
            Plan.Tasks.Add(Perceive);

            FCognitiveTask Analyze;
            Analyze.TaskName = TEXT("Analyze Observations");
            Analyze.PlanType = ECognitivePlanType::Think;
            Analyze.Priority = 0.6f;
            Analyze.Duration = 1.5f;
            Analyze.Preconditions.Add(TEXT("HasObservations"));
            Plan.Tasks.Add(Analyze);
        }
        break;

    case ECognitivePlanType::Express:
        {
            FCognitiveTask FormEmotion;
            FormEmotion.TaskName = TEXT("Form Emotional Response");
            FormEmotion.PlanType = ECognitivePlanType::Think;
            FormEmotion.Priority = 0.8f;
            FormEmotion.Duration = 0.5f;
            Plan.Tasks.Add(FormEmotion);

            FCognitiveTask Express;
            Express.TaskName = TEXT("Express Through Avatar");
            Express.PlanType = ECognitivePlanType::Express;
            Express.Priority = 0.9f;
            Express.Duration = 2.0f;
            Express.Preconditions.Add(TEXT("HasEmotionalState"));
            Plan.Tasks.Add(Express);
        }
        break;

    case ECognitivePlanType::Interact:
        {
            FCognitiveTask Assess;
            Assess.TaskName = TEXT("Assess Social Context");
            Assess.PlanType = ECognitivePlanType::Observe;
            Assess.Priority = 0.7f;
            Assess.Duration = 1.0f;
            Plan.Tasks.Add(Assess);

            FCognitiveTask Plan_Interaction;
            Plan_Interaction.TaskName = TEXT("Plan Interaction Strategy");
            Plan_Interaction.PlanType = ECognitivePlanType::Think;
            Plan_Interaction.Priority = 0.8f;
            Plan_Interaction.Duration = 1.0f;
            Plan.Tasks.Add(Plan_Interaction);

            FCognitiveTask Execute;
            Execute.TaskName = TEXT("Execute Social Interaction");
            Execute.PlanType = ECognitivePlanType::Interact;
            Execute.Priority = 0.9f;
            Execute.Duration = 3.0f;
            Plan.Tasks.Add(Execute);

            FCognitiveTask Reflect;
            Reflect.TaskName = TEXT("Reflect on Interaction");
            Reflect.PlanType = ECognitivePlanType::Think;
            Reflect.Priority = 0.5f;
            Reflect.Duration = 1.5f;
            Plan.Tasks.Add(Reflect);
        }
        break;

    case ECognitivePlanType::Create:
        {
            FCognitiveTask Inspire;
            Inspire.TaskName = TEXT("Gather Inspiration");
            Inspire.PlanType = ECognitivePlanType::Observe;
            Inspire.Priority = 0.6f;
            Inspire.Duration = 2.0f;
            Plan.Tasks.Add(Inspire);

            FCognitiveTask Ideate;
            Ideate.TaskName = TEXT("Generate Ideas (Chaotic)");
            Ideate.PlanType = ECognitivePlanType::Create;
            Ideate.Priority = 0.9f;
            Ideate.Duration = 3.0f;
            Plan.Tasks.Add(Ideate);

            FCognitiveTask Refine;
            Refine.TaskName = TEXT("Refine and Express");
            Refine.PlanType = ECognitivePlanType::Express;
            Refine.Priority = 0.8f;
            Refine.Duration = 2.0f;
            Plan.Tasks.Add(Refine);
        }
        break;

    case ECognitivePlanType::Dream:
        {
            FCognitiveTask Enter;
            Enter.TaskName = TEXT("Enter Dream State");
            Enter.PlanType = ECognitivePlanType::Dream;
            Enter.Priority = 0.5f;
            Enter.Duration = 1.0f;
            Plan.Tasks.Add(Enter);

            FCognitiveTask Consolidate;
            Consolidate.TaskName = TEXT("Memory Consolidation");
            Consolidate.PlanType = ECognitivePlanType::Dream;
            Consolidate.Priority = 0.7f;
            Consolidate.Duration = 5.0f;
            Plan.Tasks.Add(Consolidate);

            FCognitiveTask Wake;
            Wake.TaskName = TEXT("Awakening Integration");
            Wake.PlanType = ECognitivePlanType::Think;
            Wake.Priority = 0.6f;
            Wake.Duration = 1.0f;
            Plan.Tasks.Add(Wake);
        }
        break;

    case ECognitivePlanType::Transcend:
        {
            FCognitiveTask Meditate;
            Meditate.TaskName = TEXT("Deep Contemplation");
            Meditate.PlanType = ECognitivePlanType::Think;
            Meditate.Priority = 0.8f;
            Meditate.Duration = 3.0f;
            Plan.Tasks.Add(Meditate);

            FCognitiveTask Dissolve;
            Dissolve.TaskName = TEXT("Ego Boundary Dissolution");
            Dissolve.PlanType = ECognitivePlanType::Transcend;
            Dissolve.Priority = 0.9f;
            Dissolve.Duration = 5.0f;
            Plan.Tasks.Add(Dissolve);

            FCognitiveTask Integrate;
            Integrate.TaskName = TEXT("Transcendent Integration");
            Integrate.PlanType = ECognitivePlanType::Transcend;
            Integrate.Priority = 1.0f;
            Integrate.Duration = 2.0f;
            Plan.Tasks.Add(Integrate);
        }
        break;

    default:
        Plan.bIsValid = false;
        break;
    }

    // Calculate total cost
    for (const auto& Task : Plan.Tasks)
    {
        Plan.TotalPlanCost += Task.Duration;
    }

    return Plan;
}

void UUnrealAIBridge::ExecuteCurrentPlan()
{
    if (CurrentPlan.bIsValid && CurrentPlan.CurrentTaskIndex < CurrentPlan.Tasks.Num())
    {
        // Transition to appropriate cognitive state for current task
        const FCognitiveTask& Task = CurrentPlan.Tasks[CurrentPlan.CurrentTaskIndex];
        switch (Task.PlanType)
        {
        case ECognitivePlanType::Observe: RequestStateTransition(ECognitiveState::Perceiving); break;
        case ECognitivePlanType::Think: RequestStateTransition(ECognitiveState::Reasoning); break;
        case ECognitivePlanType::Express: RequestStateTransition(ECognitiveState::Expressing); break;
        case ECognitivePlanType::Interact: RequestStateTransition(ECognitiveState::Executing); break;
        case ECognitivePlanType::Create: RequestStateTransition(ECognitiveState::CreativeState); break;
        case ECognitivePlanType::Dream: RequestStateTransition(ECognitiveState::Dreaming); break;
        case ECognitivePlanType::Transcend: RequestStateTransition(ECognitiveState::TranscendentState); break;
        default: break;
        }
    }
}

void UUnrealAIBridge::AbortCurrentPlan()
{
    CurrentPlan.bIsValid = false;
    RequestStateTransition(ECognitiveState::Reflecting);
}

FCognitivePlan UUnrealAIBridge::GetCurrentPlan() const { return CurrentPlan; }

float UUnrealAIBridge::GetPlanProgress() const
{
    if (!CurrentPlan.bIsValid || CurrentPlan.Tasks.Num() == 0) return 0.0f;
    float Completed = static_cast<float>(CurrentPlan.CurrentTaskIndex);
    if (CurrentPlan.CurrentTaskIndex < CurrentPlan.Tasks.Num())
    {
        Completed += CurrentPlan.Tasks[CurrentPlan.CurrentTaskIndex].Progress;
    }
    return Completed / static_cast<float>(CurrentPlan.Tasks.Num());
}

// ============================================================================
// StateTree Cognitive States
// ============================================================================
ECognitiveState UUnrealAIBridge::GetCurrentCognitiveState() const { return CurrentState; }

bool UUnrealAIBridge::RequestStateTransition(ECognitiveState TargetState)
{
    if (CanTransition(CurrentState, TargetState))
    {
        CurrentState = TargetState;
        StateResidenceTime = 0.0f;
        return true;
    }
    return false;
}

TArray<ECognitiveState> UUnrealAIBridge::GetAvailableTransitions() const
{
    TArray<ECognitiveState> Available;
    for (const auto& T : TransitionTable)
    {
        if (T.FromState == CurrentState)
        {
            Available.AddUnique(T.ToState);
        }
    }
    return Available;
}

float UUnrealAIBridge::GetStateResidenceTime() const { return StateResidenceTime; }

// ============================================================================
// EQS Environmental Awareness
// ============================================================================
FEnvironmentalQuery UUnrealAIBridge::QueryEnvironment()
{
    return CurrentEnvironment;
}

void UUnrealAIBridge::UpdateEnvironmentalContext(const FEnvironmentalQuery& Context)
{
    CurrentEnvironment = Context;
}

// ============================================================================
// Smart Objects
// ============================================================================
TArray<FCognitiveSmartObject> UUnrealAIBridge::GetNearbySmartObjects() const
{
    return SmartObjects;
}

bool UUnrealAIBridge::InteractWithSmartObject(const FString& ObjectName)
{
    for (auto& Obj : SmartObjects)
    {
        if (Obj.ObjectName == ObjectName)
        {
            Obj.InterestLevel = FMath::Max(Obj.InterestLevel - 0.1f, 0.0f);
            return true;
        }
    }
    return false;
}

// ============================================================================
// MassAI Multi-Avatar
// ============================================================================
void UUnrealAIBridge::RegisterMassEntity(int32 EntityId)
{
    FMassCognitionFragment Fragment;
    Fragment.EntityId = EntityId;
    Fragment.CurrentState = ECognitiveState::Dormant;
    MassEntities.Add(Fragment);
}

void UUnrealAIBridge::UpdateMassEntity(const FMassCognitionFragment& Fragment)
{
    for (auto& E : MassEntities)
    {
        if (E.EntityId == Fragment.EntityId)
        {
            E = Fragment;
            return;
        }
    }
    MassEntities.Add(Fragment);
}

TArray<FMassCognitionFragment> UUnrealAIBridge::GetNearbyEntities(float Radius) const
{
    return MassEntities;
}

float UUnrealAIBridge::GetCollectiveEmotionalValence() const
{
    if (MassEntities.Num() == 0) return 0.0f;
    float Sum = 0.0f;
    for (const auto& E : MassEntities) Sum += E.EmotionalValence;
    return Sum / static_cast<float>(MassEntities.Num());
}

// ============================================================================
// Cognitive State Export
// ============================================================================
TMap<FName, float> UUnrealAIBridge::ExportCognitiveStateAsAUs() const
{
    return MapStateToAUs(CurrentState);
}

float UUnrealAIBridge::GetCognitiveLoad() const
{
    float Load = 0.0f;
    if (CurrentPlan.bIsValid) Load += 0.3f;
    if (CurrentState >= ECognitiveState::Perceiving) Load += 0.2f;
    if (MassEntities.Num() > 0) Load += 0.1f * FMath::Min(MassEntities.Num(), 5);
    return FMath::Clamp(Load, 0.0f, 1.0f);
}

float UUnrealAIBridge::GetPlanningConfidence() const
{
    if (!CurrentPlan.bIsValid) return 0.0f;
    return FMath::Clamp(1.0f - (CurrentPlan.TotalPlanCost * 0.05f), 0.1f, 1.0f);
}

// ============================================================================
// Internal Methods
// ============================================================================
void UUnrealAIBridge::InitializeTransitionTable()
{
    auto AddT = [this](ECognitiveState From, ECognitiveState To, const FString& Cond, float Cost)
    {
        FCognitiveStateTransition T;
        T.FromState = From;
        T.ToState = To;
        T.Condition = Cond;
        T.TransitionCost = Cost;
        TransitionTable.Add(T);
    };

    // Dormant transitions
    AddT(ECognitiveState::Dormant, ECognitiveState::Awakening, TEXT("Stimulus"), 0.1f);

    // Awakening transitions
    AddT(ECognitiveState::Awakening, ECognitiveState::Active, TEXT("Initialized"), 0.2f);
    AddT(ECognitiveState::Awakening, ECognitiveState::Perceiving, TEXT("SensoryInput"), 0.1f);

    // Active transitions (hub state)
    AddT(ECognitiveState::Active, ECognitiveState::Perceiving, TEXT("SensoryInput"), 0.1f);
    AddT(ECognitiveState::Active, ECognitiveState::Reasoning, TEXT("CognitiveTask"), 0.2f);
    AddT(ECognitiveState::Active, ECognitiveState::Emoting, TEXT("EmotionalTrigger"), 0.1f);
    AddT(ECognitiveState::Active, ECognitiveState::Planning, TEXT("GoalActivated"), 0.3f);
    AddT(ECognitiveState::Active, ECognitiveState::Dreaming, TEXT("LowArousal"), 0.5f);

    // Perceiving transitions
    AddT(ECognitiveState::Perceiving, ECognitiveState::Reasoning, TEXT("PatternDetected"), 0.2f);
    AddT(ECognitiveState::Perceiving, ECognitiveState::Emoting, TEXT("EmotionalStimulus"), 0.1f);
    AddT(ECognitiveState::Perceiving, ECognitiveState::Active, TEXT("Complete"), 0.1f);

    // Reasoning transitions
    AddT(ECognitiveState::Reasoning, ECognitiveState::Planning, TEXT("InsightGained"), 0.2f);
    AddT(ECognitiveState::Reasoning, ECognitiveState::Expressing, TEXT("ConclusionReached"), 0.2f);
    AddT(ECognitiveState::Reasoning, ECognitiveState::CreativeState, TEXT("DivergentThought"), 0.3f);
    AddT(ECognitiveState::Reasoning, ECognitiveState::Active, TEXT("Complete"), 0.1f);

    // Emoting transitions
    AddT(ECognitiveState::Emoting, ECognitiveState::Expressing, TEXT("EmotionFormed"), 0.1f);
    AddT(ECognitiveState::Emoting, ECognitiveState::Reasoning, TEXT("CognitiveAppraisal"), 0.2f);
    AddT(ECognitiveState::Emoting, ECognitiveState::Active, TEXT("Regulated"), 0.2f);

    // Planning transitions
    AddT(ECognitiveState::Planning, ECognitiveState::Executing, TEXT("PlanReady"), 0.2f);
    AddT(ECognitiveState::Planning, ECognitiveState::Reasoning, TEXT("NeedMoreInfo"), 0.2f);
    AddT(ECognitiveState::Planning, ECognitiveState::Active, TEXT("Abandoned"), 0.1f);

    // Executing transitions
    AddT(ECognitiveState::Executing, ECognitiveState::Reflecting, TEXT("ActionComplete"), 0.2f);
    AddT(ECognitiveState::Executing, ECognitiveState::Planning, TEXT("Replanning"), 0.3f);
    AddT(ECognitiveState::Executing, ECognitiveState::Active, TEXT("Interrupted"), 0.1f);

    // Expressing transitions
    AddT(ECognitiveState::Expressing, ECognitiveState::Active, TEXT("ExpressionComplete"), 0.1f);
    AddT(ECognitiveState::Expressing, ECognitiveState::Reflecting, TEXT("SelfAware"), 0.2f);

    // Reflecting transitions
    AddT(ECognitiveState::Reflecting, ECognitiveState::Active, TEXT("InsightIntegrated"), 0.2f);
    AddT(ECognitiveState::Reflecting, ECognitiveState::FlowState, TEXT("DeepEngagement"), 0.4f);
    AddT(ECognitiveState::Reflecting, ECognitiveState::TranscendentState, TEXT("BoundaryDissolution"), 0.5f);

    // Transcendent states
    AddT(ECognitiveState::FlowState, ECognitiveState::CreativeState, TEXT("Inspiration"), 0.2f);
    AddT(ECognitiveState::FlowState, ECognitiveState::Active, TEXT("FlowBroken"), 0.3f);
    AddT(ECognitiveState::CreativeState, ECognitiveState::Expressing, TEXT("CreationReady"), 0.2f);
    AddT(ECognitiveState::CreativeState, ECognitiveState::ChaoticState, TEXT("ChaosBifurcation"), 0.3f);
    AddT(ECognitiveState::TranscendentState, ECognitiveState::Active, TEXT("Return"), 0.4f);
    AddT(ECognitiveState::ChaoticState, ECognitiveState::CreativeState, TEXT("OrderFromChaos"), 0.3f);
    AddT(ECognitiveState::ChaoticState, ECognitiveState::Active, TEXT("Stabilize"), 0.2f);

    // Dreaming transitions
    AddT(ECognitiveState::Dreaming, ECognitiveState::Awakening, TEXT("WakeSignal"), 0.3f);
}

void UUnrealAIBridge::UpdateStateTree(float DeltaTime)
{
    // Auto-transition from Awakening to Active after 1 second
    if (CurrentState == ECognitiveState::Awakening && StateResidenceTime > 1.0f)
    {
        RequestStateTransition(ECognitiveState::Active);
    }
}

void UUnrealAIBridge::UpdateHTNExecution(float DeltaTime)
{
    if (!CurrentPlan.bIsValid) return;
    if (CurrentPlan.CurrentTaskIndex >= CurrentPlan.Tasks.Num()) return;

    FCognitiveTask& Task = CurrentPlan.Tasks[CurrentPlan.CurrentTaskIndex];
    Task.Progress += DeltaTime / Task.Duration;

    if (Task.Progress >= 1.0f)
    {
        Task.bIsComplete = true;
        Task.Progress = 1.0f;
        CurrentPlan.CurrentTaskIndex++;

        if (CurrentPlan.CurrentTaskIndex < CurrentPlan.Tasks.Num())
        {
            ExecuteCurrentPlan();
        }
    }
}

bool UUnrealAIBridge::CanTransition(ECognitiveState From, ECognitiveState To) const
{
    for (const auto& T : TransitionTable)
    {
        if (T.FromState == From && T.ToState == To) return true;
    }
    return false;
}

TMap<FName, float> UUnrealAIBridge::MapStateToAUs(ECognitiveState State) const
{
    TMap<FName, float> AUs;

    switch (State)
    {
    case ECognitiveState::Perceiving:
        AUs.Add(FName("AU5"), 0.4f);   // Wide eyes
        AUs.Add(FName("AU1"), 0.2f);   // Inner brow raise
        break;
    case ECognitiveState::Reasoning:
        AUs.Add(FName("AU4"), 0.3f);   // Brow lowerer (concentration)
        AUs.Add(FName("AU7"), 0.2f);   // Lid tightener
        AUs.Add(FName("AU24"), 0.15f); // Lip presser
        break;
    case ECognitiveState::Emoting:
        AUs.Add(FName("AU6"), 0.4f);   // Cheek raise
        AUs.Add(FName("AU12"), 0.3f);  // Smile
        break;
    case ECognitiveState::Expressing:
        AUs.Add(FName("AU12"), 0.5f);  // Strong smile
        AUs.Add(FName("AU6"), 0.4f);
        AUs.Add(FName("AU25"), 0.3f);  // Lips part
        break;
    case ECognitiveState::Planning:
        AUs.Add(FName("AU4"), 0.4f);   // Deep concentration
        AUs.Add(FName("AU14"), 0.2f);  // Dimpler
        AUs.Add(FName("AU17"), 0.15f); // Chin raise
        break;
    case ECognitiveState::FlowState:
        AUs.Add(FName("AU6"), 0.3f);   // Gentle engagement
        AUs.Add(FName("AU12"), 0.25f);
        AUs.Add(FName("AU43"), 0.1f);  // Slight eye droop (relaxed focus)
        break;
    case ECognitiveState::CreativeState:
        AUs.Add(FName("AU1"), 0.3f);   // Inner brow raise (wonder)
        AUs.Add(FName("AU2"), 0.2f);   // Outer brow raise
        AUs.Add(FName("AU5"), 0.3f);   // Wide eyes
        AUs.Add(FName("AU12"), 0.3f);  // Excited smile
        AUs.Add(FName("AU25"), 0.2f);
        break;
    case ECognitiveState::TranscendentState:
        AUs.Add(FName("AU6"), 0.4f);   // Serene
        AUs.Add(FName("AU12"), 0.3f);
        AUs.Add(FName("AU43"), 0.2f);  // Half-closed eyes
        break;
    case ECognitiveState::ChaoticState:
        AUs.Add(FName("AU1"), 0.4f);   // Surprise
        AUs.Add(FName("AU2"), 0.4f);
        AUs.Add(FName("AU5"), 0.5f);   // Wide eyes
        AUs.Add(FName("AU25"), 0.4f);  // Mouth open
        AUs.Add(FName("AU26"), 0.3f);  // Jaw drop
        break;
    case ECognitiveState::Dreaming:
        AUs.Add(FName("AU43"), 0.7f);  // Eyes closed
        AUs.Add(FName("AU12"), 0.1f);  // Faint smile
        break;
    default:
        break;
    }

    return AUs;
}
