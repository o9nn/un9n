// ReinforcementLearningBridge.cpp
// Implementation of RL Bridge connecting game training with Deep Tree Echo

#include "ReinforcementLearningBridge.h"
#include "../Learning/OnlineLearningSystem.h"
#include "../4ECognition/EmbodiedCognitionComponent.h"
#include "../Core/DeepTreeEchoCore.h"

// ============================================================================
// UReinforcementLearningBridge Implementation
// ============================================================================

UReinforcementLearningBridge::UReinforcementLearningBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UReinforcementLearningBridge::BeginPlay()
{
    Super::BeginPlay();

    FindComponentReferences();
    InitializeActionNames();
    InitializeQTable();
}

void UReinforcementLearningBridge::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsTraining)
    {
        return;
    }

    // Update cognitive modulation from DTE state
    if (bUseCognitiveModulation)
    {
        UpdateCognitiveModulation();
    }

    // Periodic learning updates
    StepCounter++;
    if (StepCounter % UpdateFrequency == 0 && ReplayBuffer.Num() >= BatchSize)
    {
        PerformBatchUpdate();
    }
}

void UReinforcementLearningBridge::FindComponentReferences()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ControllerInterface = Owner->FindComponentByClass<UGameControllerInterface>();
    TrainingEnvironment = Owner->FindComponentByClass<UGameTrainingEnvironment>();
    SkillSystem = Owner->FindComponentByClass<UGameSkillTrainingSystem>();
    LearningSystem = Owner->FindComponentByClass<UOnlineLearningSystem>();
    EmbodimentComponent = Owner->FindComponentByClass<UEmbodiedCognitionComponent>();
    // DTECore = Owner->FindComponentByClass<UDeepTreeEchoCore>();
}

void UReinforcementLearningBridge::InitializeActionNames()
{
    if (ActionNames.Num() == 0)
    {
        // Default action names for common game actions
        ActionNames = {
            TEXT("Idle"),
            TEXT("MoveForward"),
            TEXT("MoveBackward"),
            TEXT("MoveLeft"),
            TEXT("MoveRight"),
            TEXT("Jump"),
            TEXT("Crouch"),
            TEXT("Attack"),
            TEXT("Block"),
            TEXT("Dodge"),
            TEXT("Interact"),
            TEXT("Special1"),
            TEXT("Special2"),
            TEXT("LookLeft"),
            TEXT("LookRight"),
            TEXT("LookUp")
        };
        NumActions = ActionNames.Num();
    }
}

void UReinforcementLearningBridge::InitializeQTable()
{
    // Q-table is lazily initialized as states are encountered
    QTable.Empty();
}

FString UReinforcementLearningBridge::StateToKey(const TArray<float>& State) const
{
    // Discretize state for tabular Q-learning. Built with a single pre-reserved buffer and
    // AppendInt/AppendChar - the previous per-element FString::Printf allocated a transient
    // string per bucket (~30 heap allocations per call on this hot path).
    FString Key;
    Key.Reserve(State.Num() * 4 + 8);
    for (int32 i = 0; i < FMath::Min(State.Num(), 20); ++i)
    {
        Key.AppendInt(FMath::RoundToInt(State[i] * 10));
        Key.AppendChar(TEXT('_'));
    }

    // Fold the tail (entity slots and, with bUseReservoirIntegration, the appended echo memory)
    // into coarse pooled buckets. Truncating at 20 elements made states differing only in their
    // reservoir tail collapse onto one key, so the echo memory had no effect on the tabular
    // policy at all. Pooling keeps the key space bounded while letting the tail differentiate.
    for (int32 i = 20; i < State.Num(); i += 10)
    {
        float Sum = 0.0f;
        const int32 End = FMath::Min(i + 10, State.Num());
        for (int32 j = i; j < End; ++j)
        {
            Sum += State[j];
        }
        Key.AppendChar(TEXT('p'));
        Key.AppendInt(FMath::RoundToInt(Sum * 2.0f));
        Key.AppendChar(TEXT('_'));
    }

    return Key;
}

TArray<float>& UReinforcementLearningBridge::GetOrCreateQValues(const FString& StateKey)
{
    TArray<float>& Values = QTable.FindOrAdd(StateKey);
    if (Values.Num() == 0)
    {
        // Small optimistic initialization (same as before the by-reference conversion)
        Values.Init(0.1f, NumActions);
    }
    return Values;
}

float UReinforcementLearningBridge::GetMaxQValueForKey(const FString& StateKey) const
{
    const TArray<float>* Values = QTable.Find(StateKey);
    if (!Values || Values->Num() == 0)
    {
        return 0.0f;
    }
    float MaxQ = (*Values)[0];
    for (float V : *Values)
    {
        MaxQ = FMath::Max(MaxQ, V);
    }
    return MaxQ;
}

const FString& UReinforcementLearningBridge::ResolveStateKey(const FTransition& Transition, FString& Scratch) const
{
    if (!Transition.StateKey.IsEmpty())
    {
        return Transition.StateKey;
    }
    Scratch = StateToKey(Transition.State);
    return Scratch;
}

const FString& UReinforcementLearningBridge::ResolveNextStateKey(const FTransition& Transition, FString& Scratch) const
{
    if (!Transition.NextStateKey.IsEmpty())
    {
        return Transition.NextStateKey;
    }
    Scratch = StateToKey(Transition.NextState);
    return Scratch;
}


// ============================================================================
// Action Selection
// ============================================================================

FRLAction UReinforcementLearningBridge::SelectAction(const TArray<float>& State)
{
    FRLAction Action;

    switch (SelectionPolicy)
    {
        case EActionSelectionPolicy::EpsilonGreedy:
            Action = SelectEpsilonGreedy(State);
            break;
        case EActionSelectionPolicy::Softmax:
            Action = SelectSoftmax(State);
            break;
        case EActionSelectionPolicy::UCB:
            Action = SelectUCB(State);
            break;
        case EActionSelectionPolicy::Thompson:
            Action = SelectThompson(State);
            break;
        case EActionSelectionPolicy::ActiveInference:
            Action = SelectActionActiveInference(State);
            break;
        default:
            Action = SelectEpsilonGreedy(State);
            break;
    }

    // Populate the continuous action vector every selection path leaves empty. The documented
    // training loop (README.md) passes Action.ContinuousAction into
    // UGameTrainingEnvironment::Step -> FControllerInputState::FromActionVector, which returns
    // an all-zero/no-op state for any vector shorter than 22 elements - so without this, no
    // selected action ever actually reached the controller.
    Action.ContinuousAction = ActionToControllerOutput(Action).DesiredState.ToActionVector();

    // Store for learning
    LastState = State;
    LastAction = Action;

    OnActionSelected.Broadcast(Action, UEnum::GetValueAsString(SelectionPolicy));

    return Action;
}

FRLAction UReinforcementLearningBridge::SelectActionFromObservation(const FGameStateObservation& Observation)
{
    TArray<float> State = Observation.ToNormalizedVector();

    // Enhance with reservoir state if enabled
    if (bUseReservoirIntegration)
    {
        TArray<float> EnhancedState = GetReservoirState(State);
        return SelectAction(EnhancedState);
    }

    return SelectAction(State);
}

TArray<float> UReinforcementLearningBridge::GetQValues(const TArray<float>& State) const
{
    FString StateKey = StateToKey(State);
    if (const TArray<float>* Values = QTable.Find(StateKey))
    {
        return *Values;
    }

    // Return default values
    TArray<float> DefaultValues;
    DefaultValues.SetNumZeroed(NumActions);
    return DefaultValues;
}

TArray<float> UReinforcementLearningBridge::GetActionProbabilities(const TArray<float>& State) const
{
    TArray<float> QValues = GetQValues(State);
    TArray<float> Probs;
    Probs.SetNum(NumActions);

    float MaxQ = QValues[0];
    for (float Q : QValues)
    {
        MaxQ = FMath::Max(MaxQ, Q);
    }

    float Sum = 0.0f;
    for (int32 i = 0; i < NumActions; ++i)
    {
        Probs[i] = FMath::Exp((QValues[i] - MaxQ) / SoftmaxTemperature);
        Sum += Probs[i];
    }

    if (Sum > 0.0f)
    {
        for (int32 i = 0; i < NumActions; ++i)
        {
            Probs[i] /= Sum;
        }
    }

    return Probs;
}

FControllerOutputCommand UReinforcementLearningBridge::ActionToControllerOutput(const FRLAction& Action) const
{
    FControllerOutputCommand Output;
    Output.ActionName = Action.ActionName;

    // Map action index to controller state
    switch (Action.ActionIndex)
    {
        case 0:  // Idle
            break;
        case 1:  // MoveForward
            Output.DesiredState.LeftStickY = 1.0f;
            Output.Category = EGameActionCategory::Movement;
            break;
        case 2:  // MoveBackward
            Output.DesiredState.LeftStickY = -1.0f;
            Output.Category = EGameActionCategory::Movement;
            break;
        case 3:  // MoveLeft
            Output.DesiredState.LeftStickX = -1.0f;
            Output.Category = EGameActionCategory::Movement;
            break;
        case 4:  // MoveRight
            Output.DesiredState.LeftStickX = 1.0f;
            Output.Category = EGameActionCategory::Movement;
            break;
        case 5:  // Jump
            Output.DesiredState.Press(EGamepadButton::FaceBottom);
            Output.Category = EGameActionCategory::Movement;
            break;
        case 6:  // Crouch
            Output.DesiredState.Press(EGamepadButton::LeftThumb);
            Output.Category = EGameActionCategory::Movement;
            break;
        case 7:  // Attack
            Output.DesiredState.Press(EGamepadButton::FaceRight);
            Output.Category = EGameActionCategory::Attack;
            break;
        case 8:  // Block
            Output.DesiredState.LeftTrigger = 1.0f;
            Output.Category = EGameActionCategory::Defense;
            break;
        case 9:  // Dodge
            Output.DesiredState.Press(EGamepadButton::FaceLeft);
            Output.Category = EGameActionCategory::Defense;
            break;
        case 10: // Interact
            Output.DesiredState.Press(EGamepadButton::FaceTop);
            Output.Category = EGameActionCategory::Interaction;
            break;
        case 11: // Special1
            Output.DesiredState.Press(EGamepadButton::RightShoulder);
            Output.Category = EGameActionCategory::Special;
            break;
        case 12: // Special2
            Output.DesiredState.Press(EGamepadButton::LeftShoulder);
            Output.Category = EGameActionCategory::Special;
            break;
        case 13: // LookLeft
            Output.DesiredState.RightStickX = -1.0f;
            Output.Category = EGameActionCategory::Camera;
            break;
        case 14: // LookRight
            Output.DesiredState.RightStickX = 1.0f;
            Output.Category = EGameActionCategory::Camera;
            break;
        case 15: // LookUp
            Output.DesiredState.RightStickY = 1.0f;
            Output.Category = EGameActionCategory::Camera;
            break;
    }

    Output.Duration = 0.1f;
    Output.Priority = 1.0f;
    Output.Confidence = Action.Probability;

    return Output;
}

FRLAction UReinforcementLearningBridge::GetGreedyAction(const TArray<float>& State) const
{
    TArray<float> QValues = GetQValues(State);

    FRLAction Action;
    Action.QValue = QValues[0];
    Action.ActionIndex = 0;

    for (int32 i = 1; i < NumActions; ++i)
    {
        if (QValues[i] > Action.QValue)
        {
            Action.QValue = QValues[i];
            Action.ActionIndex = i;
        }
    }

    if (ActionNames.IsValidIndex(Action.ActionIndex))
    {
        Action.ActionName = ActionNames[Action.ActionIndex];
    }
    Action.Probability = 1.0f;

    return Action;
}

FRLAction UReinforcementLearningBridge::SelectEpsilonGreedy(const TArray<float>& State)
{
    float EffectiveEpsilon = GetEffectiveExplorationRate();

    if (FMath::FRand() < EffectiveEpsilon)
    {
        // Explore: random action
        FRLAction Action;
        Action.ActionIndex = FMath::RandRange(0, NumActions - 1);
        if (ActionNames.IsValidIndex(Action.ActionIndex))
        {
            Action.ActionName = ActionNames[Action.ActionIndex];
        }
        Action.Probability = 1.0f / NumActions;
        return Action;
    }
    else
    {
        // Exploit: greedy action
        return GetGreedyAction(State);
    }
}

FRLAction UReinforcementLearningBridge::SelectSoftmax(const TArray<float>& State)
{
    TArray<float> Probs = GetActionProbabilities(State);

    // Sample from distribution
    float Roll = FMath::FRand();
    float Cumulative = 0.0f;

    FRLAction Action;
    for (int32 i = 0; i < NumActions; ++i)
    {
        Cumulative += Probs[i];
        // The final iteration always selects: float rounding can leave the cumulative sum
        // slightly below Roll (FRand can return exactly 1.0), and an all-zero Probs array
        // (Sum==0 guard) would otherwise fall through to a default-constructed action.
        if (Roll <= Cumulative || i == NumActions - 1)
        {
            Action.ActionIndex = i;
            Action.Probability = Probs[i];
            break;
        }
    }

    if (ActionNames.IsValidIndex(Action.ActionIndex))
    {
        Action.ActionName = ActionNames[Action.ActionIndex];
    }

    TArray<float> QValues = GetQValues(State);
    Action.QValue = QValues.IsValidIndex(Action.ActionIndex) ? QValues[Action.ActionIndex] : 0.0f;

    return Action;
}

FRLAction UReinforcementLearningBridge::SelectUCB(const TArray<float>& State)
{
    TArray<float> QValues = GetQValues(State);

    // UCB1 formula: Q(a) + c * sqrt(ln(t) / N(a))
    float c = 2.0f;  // Exploration constant
    float LogT = FMath::Loge(static_cast<float>(TotalSteps + 1));

    FRLAction BestAction;
    float BestUCB = -FLT_MAX;

    for (int32 i = 0; i < NumActions; ++i)
    {
        // Simplified: assume equal visitation for now
        float VisitCount = FMath::Max(1.0f, static_cast<float>(TotalSteps / NumActions));
        float UCBValue = QValues[i] + c * FMath::Sqrt(LogT / VisitCount);

        if (UCBValue > BestUCB)
        {
            BestUCB = UCBValue;
            BestAction.ActionIndex = i;
            BestAction.QValue = QValues[i];
        }
    }

    if (ActionNames.IsValidIndex(BestAction.ActionIndex))
    {
        BestAction.ActionName = ActionNames[BestAction.ActionIndex];
    }
    BestAction.Probability = 1.0f;

    return BestAction;
}

FRLAction UReinforcementLearningBridge::SelectThompson(const TArray<float>& State)
{
    TArray<float> QValues = GetQValues(State);

    // Thompson sampling: sample from posterior
    // Simplified: use Q-value + Gaussian noise scaled by uncertainty
    FRLAction BestAction;
    float BestSample = -FLT_MAX;

    for (int32 i = 0; i < NumActions; ++i)
    {
        float Uncertainty = 1.0f / FMath::Sqrt(static_cast<float>(TotalSteps / NumActions + 1));
        float Sample = QValues[i] + FMath::FRandRange(-1.0f, 1.0f) * Uncertainty;

        if (Sample > BestSample)
        {
            BestSample = Sample;
            BestAction.ActionIndex = i;
            BestAction.QValue = QValues[i];
        }
    }

    if (ActionNames.IsValidIndex(BestAction.ActionIndex))
    {
        BestAction.ActionName = ActionNames[BestAction.ActionIndex];
    }

    return BestAction;
}

// ============================================================================
// Learning
// ============================================================================

void UReinforcementLearningBridge::RecordTransition(const TArray<float>& State, const FRLAction& Action,
                                                     float Reward, const TArray<float>& NextState, bool bTerminal)
{
    FTransition Transition;
    Transition.State = State;
    Transition.Action = Action;
    Transition.Reward = Reward;
    Transition.NextState = NextState;
    Transition.bTerminal = bTerminal;
    Transition.Timestamp = GetWorld()->GetTimeSeconds();

    // Compute both Q-table keys exactly once here; every downstream consumer (immediate update,
    // batch replays, cognitive sync) reuses them instead of re-discretizing the state vectors.
    Transition.StateKey = StateToKey(State);
    Transition.NextStateKey = StateToKey(NextState);

    AddToReplayBuffer(Transition);

    // Apply immediate learning based on algorithm
    if (Algorithm == ELearningAlgorithm::QLearning)
    {
        ApplyQLearningUpdate(Transition);
    }
    else if (Algorithm == ELearningAlgorithm::SARSA)
    {
        ApplySARSAUpdate(Transition);
    }

    // Update statistics
    TotalSteps++;
    TotalReward += Reward;
    RewardCount++;
    if (bTerminal)
    {
        TotalEpisodes++;
    }

    // Sync with cognitive learning system (pass the transition directly - the previous
    // ReplayBuffer.Last() read breaks under ring-buffer eviction ordering)
    SyncWithCognitiveSystem(Transition);

    OnTransitionRecorded.Broadcast(Transition);
}

void UReinforcementLearningBridge::RecordGameTransition(const FGameStateObservation& State,
                                                         const FControllerInputState& Input,
                                                         float Reward,
                                                         const FGameStateObservation& NextState,
                                                         bool bTerminal)
{
    TArray<float> StateVec = State.ToNormalizedVector();
    TArray<float> NextStateVec = NextState.ToNormalizedVector();

    FRLAction Action;
    Action.ContinuousAction = Input.ToActionVector();
    // Map input to discrete action (simplified)
    Action.ActionIndex = 0;  // Default to idle
    if (FMath::Abs(Input.LeftStickY) > 0.5f)
    {
        Action.ActionIndex = Input.LeftStickY > 0 ? 1 : 2;  // Forward/Backward
    }
    else if (FMath::Abs(Input.LeftStickX) > 0.5f)
    {
        Action.ActionIndex = Input.LeftStickX > 0 ? 4 : 3;  // Right/Left
    }
    else if (Input.IsPressed(EGamepadButton::FaceBottom))
    {
        Action.ActionIndex = 5;  // Jump
    }
    else if (Input.IsPressed(EGamepadButton::FaceRight))
    {
        Action.ActionIndex = 7;  // Attack
    }

    if (ActionNames.IsValidIndex(Action.ActionIndex))
    {
        Action.ActionName = ActionNames[Action.ActionIndex];
    }

    RecordTransition(StateVec, Action, Reward, NextStateVec, bTerminal);
}

float UReinforcementLearningBridge::PerformLearningUpdate()
{
    if (ReplayBuffer.Num() == 0)
    {
        return 0.0f;
    }

    return PerformBatchUpdate();
}

float UReinforcementLearningBridge::PerformBatchUpdate()
{
    if (ReplayBuffer.Num() < BatchSize)
    {
        return 0.0f;
    }

    const TArray<int32> BatchIndices = SampleIndicesFromReplayBuffer(BatchSize);

    float TotalLoss = 0.0f;

    for (const int32 Idx : BatchIndices)
    {
        const FTransition& Transition = ReplayBuffer[Idx];

        if (Algorithm == ELearningAlgorithm::QLearning || Algorithm == ELearningAlgorithm::DQN)
        {
            // The update returns its own pre-update TD error (computed against the same
            // intrinsic-augmented target it applies), so the loss no longer duplicates the
            // full target computation - and no longer measures it post-update.
            const float TDError = ApplyQLearningUpdate(Transition);
            TotalLoss += TDError * TDError;
        }
        else if (Algorithm == ELearningAlgorithm::SARSA)
        {
            ApplySARSAUpdate(Transition);
        }
    }

    float AvgLoss = BatchIndices.Num() > 0 ? TotalLoss / BatchIndices.Num() : 0.0f;
    float AvgReward = GetAverageReward();

    OnLearningUpdate.Broadcast(AvgLoss, AvgReward);

    return AvgLoss;
}

float UReinforcementLearningBridge::ApplyQLearningUpdate(const FTransition& Transition)
{
    // RecordTransition is BlueprintCallable with an arbitrary caller-supplied FRLAction, so the
    // index must be validated up front - and BEFORE GetOrCreateQValues, which inserts a fresh
    // optimistically-initialized row into the Q-table as a side effect. A rejected transition
    // must be a complete no-op, not a table-polluting one.
    if (Transition.Action.ActionIndex < 0 || Transition.Action.ActionIndex >= NumActions)
    {
        return 0.0f;
    }

    // Use the keys cached at RecordTransition; only recompute for transitions that never went
    // through it (StateToKey previously ran up to 7x per recorded transition).
    FString StateScratch, NextScratch;
    const FString& StateKey = ResolveStateKey(Transition, StateScratch);
    const FString& NextKey = ResolveNextStateKey(Transition, NextScratch);

    // Order matters for reference validity: FindOrAdd (may rehash) FIRST, then only read-only
    // lookups (GetMaxQValueForKey / ComputeIntrinsicRewardForKey use Find) while the reference
    // is held, then mutate in place - no by-value copy, no QTable.Add writeback.
    TArray<float>& QValues = GetOrCreateQValues(StateKey);

    float CurrentQ = QValues[Transition.Action.ActionIndex];
    float MaxNextQ = Transition.bTerminal ? 0.0f : GetMaxQValueForKey(NextKey);

    // Add intrinsic reward (curiosity bonus)
    float IntrinsicReward = ComputeIntrinsicRewardForKey(StateKey);
    float AugmentedReward = Transition.Reward + CurrentModulation.Curiosity * IntrinsicReward;

    // TD target
    float Target = AugmentedReward + DiscountFactor * MaxNextQ;
    float TDError = Target - CurrentQ;

    // Q-learning update
    float EffectiveLR = GetEffectiveLearningRate();
    QValues[Transition.Action.ActionIndex] = CurrentQ + EffectiveLR * TDError;

    return TDError;
}

float UReinforcementLearningBridge::ApplySARSAUpdate(const FTransition& Transition)
{
    // Same BlueprintCallable-supplied index hazard as ApplyQLearningUpdate; checked before
    // GetOrCreateQValues so rejected transitions don't insert phantom Q-table rows.
    if (Transition.Action.ActionIndex < 0 || Transition.Action.ActionIndex >= NumActions)
    {
        return 0.0f;
    }

    FString StateScratch, NextScratch;
    const FString& StateKey = ResolveStateKey(Transition, StateScratch);
    const FString& NextKey = ResolveNextStateKey(Transition, NextScratch);

    // SARSA uses the action actually taken in next state (on-policy)
    // For simplicity, use epsilon-greedy policy. Selected BEFORE taking the QValues reference:
    // SelectEpsilonGreedy -> GetGreedyAction -> GetQValues only reads, but keeping all
    // non-mutating work ahead of the FindOrAdd reference is the safer pattern.
    float NextQ = 0.0f;
    if (!Transition.bTerminal)
    {
        FRLAction NextAction = SelectEpsilonGreedy(Transition.NextState);
        const TArray<float>* NextValues = QTable.Find(NextKey);
        if (NextValues && NextValues->IsValidIndex(NextAction.ActionIndex))
        {
            NextQ = (*NextValues)[NextAction.ActionIndex];
        }
    }

    TArray<float>& QValues = GetOrCreateQValues(StateKey);
    float CurrentQ = QValues[Transition.Action.ActionIndex];

    // SARSA update
    float Target = Transition.Reward + DiscountFactor * NextQ;
    float TDError = Target - CurrentQ;
    float EffectiveLR = GetEffectiveLearningRate();
    QValues[Transition.Action.ActionIndex] = CurrentQ + EffectiveLR * TDError;

    return TDError;
}

void UReinforcementLearningBridge::UpdateQValue(const TArray<float>& State, int32 ActionIndex, float Target)
{
    if (ActionIndex < 0 || ActionIndex >= NumActions)
    {
        return;
    }

    FString StateKey = StateToKey(State);
    TArray<float>& QValues = GetOrCreateQValues(StateKey);

    if (QValues.IsValidIndex(ActionIndex))
    {
        float EffectiveLR = GetEffectiveLearningRate();
        QValues[ActionIndex] += EffectiveLR * (Target - QValues[ActionIndex]);
    }
}

void UReinforcementLearningBridge::ClearReplayBuffer()
{
    ReplayBuffer.Empty();
    ReplayWriteIndex = 0;
}

void UReinforcementLearningBridge::AddToReplayBuffer(const FTransition& Transition)
{
    // Ring-buffer write: RemoveAt(0) eviction memmoved the entire full buffer (~1.2MB at the
    // default 10k capacity) on every recorded transition. Overwriting the oldest slot in place
    // is O(1) and reuses the evicted element's array allocations.
    if (ReplayBuffer.Num() < ReplayBufferSize)
    {
        ReplayBuffer.Add(Transition);
    }
    else
    {
        if (ReplayWriteIndex >= ReplayBuffer.Num())
        {
            // ReplayBufferSize shrank at runtime (EditAnywhere) - re-wrap defensively.
            ReplayWriteIndex = 0;
        }
        ReplayBuffer[ReplayWriteIndex] = Transition;
        ReplayWriteIndex = (ReplayWriteIndex + 1) % ReplayBuffer.Num();
    }
}

TArray<int32> UReinforcementLearningBridge::SampleIndicesFromReplayBuffer(int32 Count) const
{
    // Indices only - callers iterate ReplayBuffer by const reference. The previous by-value
    // sample deep-copied Count full transitions (two 60+ float arrays each) per batch update.
    TArray<int32> Sample;
    Sample.Reserve(Count);

    for (int32 i = 0; i < Count && ReplayBuffer.Num() > 0; ++i)
    {
        Sample.Add(FMath::RandRange(0, ReplayBuffer.Num() - 1));
    }

    return Sample;
}

float UReinforcementLearningBridge::ComputeIntrinsicReward(const TArray<float>& State, int32 ActionIndex) const
{
    return ComputeIntrinsicRewardForKey(StateToKey(State));
}

float UReinforcementLearningBridge::ComputeIntrinsicRewardForKey(const FString& StateKey) const
{
    // Curiosity-driven intrinsic reward: higher for less-visited states. Key-based so hot
    // callers that already hold the discretized key skip the StateToKey recomputation.
    // Read-only (Contains) - safe to call while a GetOrCreateQValues reference is held.
    if (!QTable.Contains(StateKey))
    {
        return 1.0f;  // Novel state bonus
    }

    // Simplified curiosity: inverse of state visitation
    return 0.1f / FMath::Sqrt(static_cast<float>(TotalSteps + 1));
}

float UReinforcementLearningBridge::ComputeCuriosityBonus(const TArray<float>& State) const
{
    // Simplified curiosity: inverse of state visitation
    // In full implementation, would use prediction error or count-based exploration
    return 0.1f / FMath::Sqrt(static_cast<float>(TotalSteps + 1));
}

// ============================================================================
// Cognitive Integration
// ============================================================================

FCognitiveModulation UReinforcementLearningBridge::GetCognitiveModulation() const
{
    return CurrentModulation;
}

void UReinforcementLearningBridge::SetCognitiveModulation(const FCognitiveModulation& Modulation)
{
    CurrentModulation = Modulation;
    OnCognitiveModulationChanged.Broadcast(CurrentModulation);
}

void UReinforcementLearningBridge::UpdateCognitiveModulation()
{
    // Update from embodiment component
    if (EmbodimentComponent)
    {
        CurrentModulation.Attention = EmbodimentComponent->Get4EIntegrationScore();
    }

    // Compute arousal from recent performance
    float RecentReward = GetAverageReward();
    CurrentModulation.Arousal = FMath::Clamp(0.5f + RecentReward * 0.1f, 0.0f, 1.0f);

    // Compute confidence from success rate
    if (TotalSteps > 100)
    {
        // Simplified: confidence based on reward trend
        CurrentModulation.Confidence = FMath::Clamp(RecentReward / 10.0f + 0.5f, 0.0f, 1.0f);
    }

    // Compute frustration from consecutive failures
    // (Simplified - would track actual failure streaks)
    CurrentModulation.Frustration = FMath::Max(0.0f, -RecentReward * 0.05f);

    // Compute flow state
    float SuccessRate = RecentReward > 0 ? 0.5f : 0.3f;
    CurrentModulation.FlowState = 1.0f - FMath::Abs(SuccessRate - 0.6f) * 2.0f;
}

float UReinforcementLearningBridge::GetEffectiveLearningRate() const
{
    float Rate = LearningRate;

    if (bUseCognitiveModulation)
    {
        // Higher attention = better learning
        Rate *= (0.5f + CurrentModulation.Attention * 0.5f);

        // Moderate arousal is optimal (Yerkes-Dodson)
        float ArousalMod = 1.0f - FMath::Abs(CurrentModulation.Arousal - 0.6f) * 0.5f;
        Rate *= ArousalMod;

        // Reduce learning when frustrated (prevent bad habits)
        Rate *= (1.0f - CurrentModulation.Frustration * 0.5f);
    }

    return Rate;
}

float UReinforcementLearningBridge::GetEffectiveExplorationRate() const
{
    float Rate = ExplorationRate;

    if (bUseCognitiveModulation)
    {
        // Higher curiosity = more exploration
        Rate *= (0.5f + CurrentModulation.Curiosity);

        // Low confidence = more exploration
        Rate *= (1.5f - CurrentModulation.Confidence * 0.5f);

        // High frustration = more exploration (try new things)
        Rate *= (1.0f + CurrentModulation.Frustration);
    }

    return FMath::Clamp(Rate, MinExplorationRate, 1.0f);
}

void UReinforcementLearningBridge::SyncWithCognitiveSystem(const FTransition& Latest)
{
    if (!LearningSystem)
    {
        return;
    }

    // Sync exploration rate
    LearningSystem->SetExplorationRate(GetEffectiveExplorationRate());

    // Record experience in cognitive system, reusing the keys cached on the transition
    // (this function previously recomputed StateToKey four more times per transition).
    FString StateScratch, NextScratch;
    const FString& StateKey = ResolveStateKey(Latest, StateScratch);
    const FString& NextKey = ResolveNextStateKey(Latest, NextScratch);

    TArray<FString> Tags;
    Tags.Reserve(2);
    Tags.Add(TEXT("GameTraining"));
    Tags.Add(Latest.Action.ActionName);

    LearningSystem->RecordExperience(
        StateKey,
        Latest.Action.ActionName,
        NextKey,
        Latest.Reward,
        Tags,
        Latest.bTerminal
    );

    // Update Q-value in cognitive system
    LearningSystem->UpdateQValue(
        StateKey,
        Latest.Action.ActionName,
        Latest.Reward,
        NextKey
    );
}

// ============================================================================
// Reservoir Integration
// ============================================================================

TArray<float> UReinforcementLearningBridge::GetReservoirState(const TArray<float>& Input)
{
    if (!bUseReservoirIntegration)
    {
        return Input;
    }

    // Update reservoir with input
    UpdateReservoir(Input);

    // Concatenate input with echo memory for richer state representation
    TArray<float> EnhancedState = Input;
    EnhancedState.Append(ReservoirState.EchoMemory);

    return EnhancedState;
}

void UReinforcementLearningBridge::UpdateReservoir(const TArray<float>& State)
{
    // Add to temporal buffer
    ReservoirState.TemporalBuffer.Add(State);
    while (ReservoirState.TemporalBuffer.Num() > 30)
    {
        ReservoirState.TemporalBuffer.RemoveAt(0);
    }

    // Simplified echo state: exponential moving average
    if (ReservoirState.EchoMemory.Num() != State.Num())
    {
        ReservoirState.EchoMemory.SetNumZeroed(State.Num());
    }

    float LeakRate = 0.3f;
    for (int32 i = 0; i < State.Num(); ++i)
    {
        ReservoirState.EchoMemory[i] = (1.0f - LeakRate) * ReservoirState.EchoMemory[i] +
                                        LeakRate * State[i];
    }
}

TArray<FString> UReinforcementLearningBridge::GetDetectedPatterns() const
{
    return ReservoirState.DetectedPatterns;
}

TArray<float> UReinforcementLearningBridge::GetEchoMemory() const
{
    return ReservoirState.EchoMemory;
}

// ============================================================================
// Active Inference Integration
// ============================================================================

FRLAction UReinforcementLearningBridge::SelectActionActiveInference(const TArray<float>& State)
{
    // Active inference: minimize expected free energy
    FRLAction BestAction;
    float LowestEFE = FLT_MAX;

    for (int32 i = 0; i < NumActions; ++i)
    {
        float EFE = ComputeExpectedFreeEnergy(State, i);
        if (EFE < LowestEFE)
        {
            LowestEFE = EFE;
            BestAction.ActionIndex = i;
            BestAction.ExpectedFreeEnergy = EFE;
        }
    }

    if (ActionNames.IsValidIndex(BestAction.ActionIndex))
    {
        BestAction.ActionName = ActionNames[BestAction.ActionIndex];
    }

    // Convert EFE to probability (softmax over negative EFE)
    TArray<float> EFEs;
    float MinEFE = FLT_MAX;
    for (int32 i = 0; i < NumActions; ++i)
    {
        float EFE = ComputeExpectedFreeEnergy(State, i);
        EFEs.Add(EFE);
        MinEFE = FMath::Min(MinEFE, EFE);
    }

    float Sum = 0.0f;
    for (int32 i = 0; i < NumActions; ++i)
    {
        Sum += FMath::Exp(-(EFEs[i] - MinEFE) / SoftmaxTemperature);
    }

    BestAction.Probability = FMath::Exp(-(LowestEFE - MinEFE) / SoftmaxTemperature) / Sum;

    return BestAction;
}

float UReinforcementLearningBridge::ComputeExpectedFreeEnergy(const TArray<float>& State, int32 ActionIndex) const
{
    // EFE = Risk + Ambiguity - Epistemic Value - Pragmatic Value

    TArray<float> QValues = GetQValues(State);
    float Q = QValues.IsValidIndex(ActionIndex) ? QValues[ActionIndex] : 0.0f;

    // Risk: divergence from preferred outcomes (use negative Q as proxy)
    float Risk = -Q;

    // Ambiguity: uncertainty about outcomes (higher for less-visited states)
    float Ambiguity = 1.0f / FMath::Sqrt(static_cast<float>(TotalSteps + 1));

    // Epistemic value: information gain (higher for novel actions)
    float EpistemicValue = ComputeCuriosityBonus(State);

    // Pragmatic value: expected reward
    float PragmaticValue = Q;

    // Modulate by cognitive state
    float CuriosityMod = bUseCognitiveModulation ? CurrentModulation.Curiosity : 0.5f;

    return Risk + Ambiguity - CuriosityMod * EpistemicValue - PragmaticValue;
}

void UReinforcementLearningBridge::UpdateBeliefs(const TArray<float>& Observation)
{
    // Update internal model based on observation
    // This integrates with the reservoir for temporal belief updates
    UpdateReservoir(Observation);
}

// ============================================================================
// Training Control
// ============================================================================

void UReinforcementLearningBridge::StartTraining()
{
    bIsTraining = true;
    StepCounter = 0;
}

void UReinforcementLearningBridge::StopTraining()
{
    bIsTraining = false;
}

bool UReinforcementLearningBridge::IsTraining() const
{
    return bIsTraining;
}

void UReinforcementLearningBridge::DecayExploration()
{
    ExplorationRate = FMath::Max(ExplorationRate * ExplorationDecay, MinExplorationRate);

    if (LearningSystem)
    {
        LearningSystem->SetExplorationRate(GetEffectiveExplorationRate());
    }
}

TArray<float> UReinforcementLearningBridge::SavePolicy() const
{
    TArray<float> PolicyData;

    // Serialize Q-table (simplified)
    for (const auto& Pair : QTable)
    {
        PolicyData.Append(Pair.Value);
    }

    return PolicyData;
}

void UReinforcementLearningBridge::LoadPolicy(const TArray<float>& PolicyData)
{
    // In practice, would deserialize properly
    // This is a placeholder for the interface
}

void UReinforcementLearningBridge::ResetLearning()
{
    QTable.Empty();
    ClearReplayBuffer();
    TotalSteps = 0;
    TotalEpisodes = 0;
    TotalReward = 0.0f;
    RewardCount = 0;
    ExplorationRate = 0.3f;
    CurrentModulation = FCognitiveModulation();
}

// ============================================================================
// Statistics
// ============================================================================

float UReinforcementLearningBridge::GetAverageReward() const
{
    return RewardCount > 0 ? TotalReward / RewardCount : 0.0f;
}

int32 UReinforcementLearningBridge::GetTotalSteps() const
{
    return TotalSteps;
}

int32 UReinforcementLearningBridge::GetTotalEpisodes() const
{
    return TotalEpisodes;
}

int32 UReinforcementLearningBridge::GetReplayBufferCount() const
{
    return ReplayBuffer.Num();
}
