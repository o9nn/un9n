// GameControllerInterface.cpp
// Implementation of Controller Input/Output Interface for Deep Tree Echo Game Training

#include "GameControllerInterface.h"
#include "../Learning/OnlineLearningSystem.h"
#include "../4ECognition/EmbodiedCognitionComponent.h"
#include "../Core/DeepTreeEchoCore.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

// ============================================================================
// FControllerInputState Implementation
// ============================================================================

TArray<float> FControllerInputState::ToActionVector() const
{
    TArray<float> Vector;
    Vector.Reserve(22);

    // Analog axes (6 values)
    Vector.Add(LeftStickX);
    Vector.Add(LeftStickY);
    Vector.Add(RightStickX);
    Vector.Add(RightStickY);
    Vector.Add(LeftTrigger);
    Vector.Add(RightTrigger);

    // Button states (16 values, 0 or 1)
    Vector.Add(IsPressed(EGamepadButton::FaceBottom) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::FaceRight) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::FaceLeft) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::FaceTop) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::LeftShoulder) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::RightShoulder) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::LeftThumb) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::RightThumb) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::DPadUp) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::DPadDown) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::DPadLeft) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::DPadRight) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::Start) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::Select) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::LeftTrigger) ? 1.0f : 0.0f);
    Vector.Add(IsPressed(EGamepadButton::RightTrigger) ? 1.0f : 0.0f);

    return Vector;
}

FControllerInputState FControllerInputState::FromActionVector(const TArray<float>& Vector)
{
    FControllerInputState State;
    if (Vector.Num() < 22) return State;

    State.LeftStickX = FMath::Clamp(Vector[0], -1.0f, 1.0f);
    State.LeftStickY = FMath::Clamp(Vector[1], -1.0f, 1.0f);
    State.RightStickX = FMath::Clamp(Vector[2], -1.0f, 1.0f);
    State.RightStickY = FMath::Clamp(Vector[3], -1.0f, 1.0f);
    State.LeftTrigger = FMath::Clamp(Vector[4], 0.0f, 1.0f);
    State.RightTrigger = FMath::Clamp(Vector[5], 0.0f, 1.0f);

    // Button states
    if (Vector[6] > 0.5f) State.Press(EGamepadButton::FaceBottom);
    if (Vector[7] > 0.5f) State.Press(EGamepadButton::FaceRight);
    if (Vector[8] > 0.5f) State.Press(EGamepadButton::FaceLeft);
    if (Vector[9] > 0.5f) State.Press(EGamepadButton::FaceTop);
    if (Vector[10] > 0.5f) State.Press(EGamepadButton::LeftShoulder);
    if (Vector[11] > 0.5f) State.Press(EGamepadButton::RightShoulder);
    if (Vector[12] > 0.5f) State.Press(EGamepadButton::LeftThumb);
    if (Vector[13] > 0.5f) State.Press(EGamepadButton::RightThumb);
    if (Vector[14] > 0.5f) State.Press(EGamepadButton::DPadUp);
    if (Vector[15] > 0.5f) State.Press(EGamepadButton::DPadDown);
    if (Vector[16] > 0.5f) State.Press(EGamepadButton::DPadLeft);
    if (Vector[17] > 0.5f) State.Press(EGamepadButton::DPadRight);
    if (Vector[18] > 0.5f) State.Press(EGamepadButton::Start);
    if (Vector[19] > 0.5f) State.Press(EGamepadButton::Select);
    if (Vector[20] > 0.5f) State.Press(EGamepadButton::LeftTrigger);
    if (Vector[21] > 0.5f) State.Press(EGamepadButton::RightTrigger);

    return State;
}

FString FControllerInputState::ToActionString() const
{
    // Create discretized state for Q-learning
    FString ActionStr;

    // Discretize stick positions to 9 regions each
    int32 LeftStickRegion = (FMath::RoundToInt(LeftStickX) + 1) * 3 + (FMath::RoundToInt(LeftStickY) + 1);
    int32 RightStickRegion = (FMath::RoundToInt(RightStickX) + 1) * 3 + (FMath::RoundToInt(RightStickY) + 1);

    ActionStr = FString::Printf(TEXT("L%d_R%d"), LeftStickRegion, RightStickRegion);

    // Add active buttons
    if (IsPressed(EGamepadButton::FaceBottom)) ActionStr += TEXT("_A");
    if (IsPressed(EGamepadButton::FaceRight)) ActionStr += TEXT("_B");
    if (IsPressed(EGamepadButton::FaceLeft)) ActionStr += TEXT("_X");
    if (IsPressed(EGamepadButton::FaceTop)) ActionStr += TEXT("_Y");
    if (IsPressed(EGamepadButton::LeftShoulder)) ActionStr += TEXT("_LB");
    if (IsPressed(EGamepadButton::RightShoulder)) ActionStr += TEXT("_RB");
    if (LeftTrigger > 0.5f) ActionStr += TEXT("_LT");
    if (RightTrigger > 0.5f) ActionStr += TEXT("_RT");

    return ActionStr;
}

bool FControllerInputState::HasSameActionSignature(const FControllerInputState& Other) const
{
    // Mirrors exactly what ToActionString encodes, without building any strings.
    if (FMath::RoundToInt(LeftStickX) != FMath::RoundToInt(Other.LeftStickX) ||
        FMath::RoundToInt(LeftStickY) != FMath::RoundToInt(Other.LeftStickY) ||
        FMath::RoundToInt(RightStickX) != FMath::RoundToInt(Other.RightStickX) ||
        FMath::RoundToInt(RightStickY) != FMath::RoundToInt(Other.RightStickY))
    {
        return false;
    }

    if ((LeftTrigger > 0.5f) != (Other.LeftTrigger > 0.5f) ||
        (RightTrigger > 0.5f) != (Other.RightTrigger > 0.5f))
    {
        return false;
    }

    // Only the buttons ToActionString encodes participate in the signature.
    constexpr int32 SignatureMask =
        (1 << static_cast<uint8>(EGamepadButton::FaceBottom)) |
        (1 << static_cast<uint8>(EGamepadButton::FaceRight)) |
        (1 << static_cast<uint8>(EGamepadButton::FaceLeft)) |
        (1 << static_cast<uint8>(EGamepadButton::FaceTop)) |
        (1 << static_cast<uint8>(EGamepadButton::LeftShoulder)) |
        (1 << static_cast<uint8>(EGamepadButton::RightShoulder));

    return (ButtonMask & SignatureMask) == (Other.ButtonMask & SignatureMask);
}

// ============================================================================
// UGameControllerInterface Implementation
// ============================================================================

UGameControllerInterface::UGameControllerInterface()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UGameControllerInterface::BeginPlay()
{
    Super::BeginPlay();

    FindComponentReferences();
    InitializeDefaultMappings();

    InputBuffer.Reserve(InputBufferSize);
}

void UGameControllerInterface::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Store previous state
    PreviousState = CurrentState;

    // Poll input
    PollControllerInput();

    // Apply queued AI output BEFORE edge detection: ExecuteQueuedOutput writes the command's
    // DesiredState into CurrentState, and press edges (which feed ButtonHoldStartTimes and
    // therefore hold-gated actions) are only observable if that write happens before
    // ProcessButtonEvents compares CurrentState against PreviousState. Running it at the end of
    // the tick meant AI-injected presses were copied into PreviousState before ever being
    // edge-detected, so hold-gated actions could never fire in AI output mode.
    if (bAIOutputMode)
    {
        ProcessOutputQueue(DeltaTime);
    }

    // Update buffer
    UpdateInputBuffer(DeltaTime);

    // Process events
    ProcessButtonEvents(DeltaTime);
    ProcessAxisEvents();

    // Detect and broadcast actions
    DetectAndBroadcastActions();

    // Broadcast state change - allocation-free signature compare; the strings themselves are
    // only built inside BroadcastCognitiveState on the (rare) ticks the signature changed.
    if (!CurrentState.HasSameActionSignature(PreviousState))
    {
        OnInputStateChanged.Broadcast(CurrentState);
        BroadcastCognitiveState();
    }
}

void UGameControllerInterface::FindComponentReferences()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    LearningSystem = Owner->FindComponentByClass<UOnlineLearningSystem>();
    EmbodimentComponent = Owner->FindComponentByClass<UEmbodiedCognitionComponent>();
    // DTECore = Owner->FindComponentByClass<UDeepTreeEchoCore>();
}

void UGameControllerInterface::InitializeDefaultMappings()
{
    // Default action mappings for common game actions
    FGameActionMapping MoveForward;
    MoveForward.ActionName = TEXT("MoveForward");
    MoveForward.Category = EGameActionCategory::Movement;
    MoveForward.PrimaryAxis = EGamepadAxis::LeftStickY;
    MoveForward.AxisThreshold = 0.3f;
    MoveForward.CognitiveMapping = TEXT("Locomotion.Forward");
    ActionMappings.Add(MoveForward);

    FGameActionMapping MoveStrafe;
    MoveStrafe.ActionName = TEXT("MoveStrafe");
    MoveStrafe.Category = EGameActionCategory::Movement;
    MoveStrafe.PrimaryAxis = EGamepadAxis::LeftStickX;
    MoveStrafe.AxisThreshold = 0.3f;
    MoveStrafe.CognitiveMapping = TEXT("Locomotion.Strafe");
    ActionMappings.Add(MoveStrafe);

    FGameActionMapping LookHorizontal;
    LookHorizontal.ActionName = TEXT("LookHorizontal");
    LookHorizontal.Category = EGameActionCategory::Camera;
    LookHorizontal.PrimaryAxis = EGamepadAxis::RightStickX;
    LookHorizontal.AxisThreshold = 0.2f;
    LookHorizontal.CognitiveMapping = TEXT("Orientation.Yaw");
    ActionMappings.Add(LookHorizontal);

    FGameActionMapping LookVertical;
    LookVertical.ActionName = TEXT("LookVertical");
    LookVertical.Category = EGameActionCategory::Camera;
    LookVertical.PrimaryAxis = EGamepadAxis::RightStickY;
    LookVertical.AxisThreshold = 0.2f;
    LookVertical.CognitiveMapping = TEXT("Orientation.Pitch");
    ActionMappings.Add(LookVertical);

    FGameActionMapping Jump;
    Jump.ActionName = TEXT("Jump");
    Jump.Category = EGameActionCategory::Movement;
    Jump.PrimaryButton = EGamepadButton::FaceBottom;
    Jump.CognitiveMapping = TEXT("Locomotion.Jump");
    ActionMappings.Add(Jump);

    FGameActionMapping Attack;
    Attack.ActionName = TEXT("Attack");
    Attack.Category = EGameActionCategory::Attack;
    Attack.PrimaryButton = EGamepadButton::FaceRight;
    Attack.CognitiveMapping = TEXT("Combat.Attack");
    ActionMappings.Add(Attack);

    FGameActionMapping Block;
    Block.ActionName = TEXT("Block");
    Block.Category = EGameActionCategory::Defense;
    Block.PrimaryButton = EGamepadButton::LeftTrigger;
    Block.bRequiresHold = true;
    Block.CognitiveMapping = TEXT("Combat.Block");
    ActionMappings.Add(Block);

    FGameActionMapping Interact;
    Interact.ActionName = TEXT("Interact");
    Interact.Category = EGameActionCategory::Interaction;
    Interact.PrimaryButton = EGamepadButton::FaceLeft;
    Interact.CognitiveMapping = TEXT("Interaction.Use");
    ActionMappings.Add(Interact);

    FGameActionMapping Sprint;
    Sprint.ActionName = TEXT("Sprint");
    Sprint.Category = EGameActionCategory::Movement;
    Sprint.PrimaryButton = EGamepadButton::LeftThumb;
    Sprint.bRequiresHold = true;
    Sprint.CognitiveMapping = TEXT("Locomotion.Sprint");
    ActionMappings.Add(Sprint);

    FGameActionMapping Special;
    Special.ActionName = TEXT("Special");
    Special.Category = EGameActionCategory::Special;
    Special.PrimaryButton = EGamepadButton::RightShoulder;
    Special.CognitiveMapping = TEXT("Ability.Special");
    ActionMappings.Add(Special);
}

void UGameControllerInterface::PollControllerInput()
{
    // ========================================
    // CONTROLLER INPUT POLLING
    // ========================================
    // Polls actual gamepad state via Unreal's input subsystem.
    // Supports both physical controller input and AI-generated input
    // for training scenarios. Falls back to AI generation when no
    // physical controller is detected.

    CurrentState.Timestamp = GetWorld()->GetTimeSeconds();

    // Try to get input from the player controller
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->IsInputKeyDown(EKeys::Gamepad_RightTrigger))
    {
        // Physical controller detected - read actual input
        CurrentState.LeftStick.X = PC->GetInputAxisValue(TEXT("MoveRight"));
        CurrentState.LeftStick.Y = PC->GetInputAxisValue(TEXT("MoveForward"));
        CurrentState.RightStick.X = PC->GetInputAxisValue(TEXT("LookRight"));
        CurrentState.RightStick.Y = PC->GetInputAxisValue(TEXT("LookUp"));
        CurrentState.LeftTrigger = PC->GetInputAxisValue(TEXT("LeftTrigger"));
        CurrentState.RightTrigger = PC->GetInputAxisValue(TEXT("RightTrigger"));

        // Poll button states
        CurrentState.PressedButtons.Empty();
        if (PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Bottom))
            CurrentState.PressedButtons.Add(EGamepadButton::FaceBottom);
        if (PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Right))
            CurrentState.PressedButtons.Add(EGamepadButton::FaceRight);
        if (PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left))
            CurrentState.PressedButtons.Add(EGamepadButton::FaceLeft);
        if (PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Top))
            CurrentState.PressedButtons.Add(EGamepadButton::FaceTop);
        if (PC->IsInputKeyDown(EKeys::Gamepad_LeftShoulder))
            CurrentState.PressedButtons.Add(EGamepadButton::LeftShoulder);
        if (PC->IsInputKeyDown(EKeys::Gamepad_RightShoulder))
            CurrentState.PressedButtons.Add(EGamepadButton::RightShoulder);

        CurrentState.bIsAIGenerated = false;
    }
    else
    {
        // No physical controller - use AI-generated input for training
        // The cognitive system can set input externally via SetAIInput()
        CurrentState.bIsAIGenerated = true;
    }
}

void UGameControllerInterface::UpdateInputBuffer(float DeltaTime)
{
    InputBuffer.Insert(CurrentState, 0);

    if (InputBuffer.Num() > InputBufferSize)
    {
        InputBuffer.SetNum(InputBufferSize);
    }
}

void UGameControllerInterface::ProcessButtonEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Diff the button bitmasks directly; both loops below only visit set bits.
    const int32 PressedNow = CurrentState.ButtonMask & ~PreviousState.ButtonMask;
    const int32 ReleasedNow = PreviousState.ButtonMask & ~CurrentState.ButtonMask;
    if (PressedNow == 0 && ReleasedNow == 0)
    {
        return;
    }

    for (int32 i = 1; i <= static_cast<int32>(EGamepadButton::Select); ++i)
    {
        const int32 Bit = 1 << i;
        const EGamepadButton Button = static_cast<EGamepadButton>(i);

        if (PressedNow & Bit)
        {
            ButtonHoldStartTimes.Add(Button, CurrentTime);
            OnButtonPressed.Broadcast(Button, CurrentTime);
        }

        if (ReleasedNow & Bit)
        {
            float HeldDuration = 0.0f;
            if (float* StartTime = ButtonHoldStartTimes.Find(Button))
            {
                HeldDuration = CurrentTime - *StartTime;
                ButtonHoldStartTimes.Remove(Button);
            }
            OnButtonReleased.Broadcast(Button, HeldDuration);
        }
    }
}

void UGameControllerInterface::ProcessAxisEvents()
{
    const float Epsilon = 0.01f;

    if (FMath::Abs(CurrentState.LeftStickX - PreviousState.LeftStickX) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::LeftStickX, CurrentState.LeftStickX,
                                CurrentState.LeftStickX - PreviousState.LeftStickX);
    }

    if (FMath::Abs(CurrentState.LeftStickY - PreviousState.LeftStickY) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::LeftStickY, CurrentState.LeftStickY,
                                CurrentState.LeftStickY - PreviousState.LeftStickY);
    }

    if (FMath::Abs(CurrentState.RightStickX - PreviousState.RightStickX) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::RightStickX, CurrentState.RightStickX,
                                CurrentState.RightStickX - PreviousState.RightStickX);
    }

    if (FMath::Abs(CurrentState.RightStickY - PreviousState.RightStickY) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::RightStickY, CurrentState.RightStickY,
                                CurrentState.RightStickY - PreviousState.RightStickY);
    }

    if (FMath::Abs(CurrentState.LeftTrigger - PreviousState.LeftTrigger) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::LeftTrigger, CurrentState.LeftTrigger,
                                CurrentState.LeftTrigger - PreviousState.LeftTrigger);
    }

    if (FMath::Abs(CurrentState.RightTrigger - PreviousState.RightTrigger) > Epsilon)
    {
        OnAxisChanged.Broadcast(EGamepadAxis::RightTrigger, CurrentState.RightTrigger,
                                CurrentState.RightTrigger - PreviousState.RightTrigger);
    }
}

void UGameControllerInterface::DetectAndBroadcastActions()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<FString> DetectedActions = DetectCurrentActions();

    for (const FString& Action : DetectedActions)
    {
        OnActionDetected.Broadcast(Action);

        // Track for combo detection, RISING EDGES ONLY: DetectCurrentActions is level-based
        // (an action stays "detected" every tick its input is held), so unconditional appends
        // would record one entry per tick of a held press - falsely matching repeated-action
        // combos against a single press and burying real sequences in duplicates.
        if (!PreviouslyActiveActions.Contains(Action))
        {
            RecentActions.Add(TPair<FString, float>(Action, CurrentTime));
            LastActionTime = CurrentTime;
        }
    }

    PreviouslyActiveActions.Reset();
    for (const FString& Action : DetectedActions)
    {
        PreviouslyActiveActions.Add(Action);
    }

    // Drop entries older than the window, regardless of whether new actions are still arriving.
    RecentActions.RemoveAll([CurrentTime](const TPair<FString, float>& Entry)
    {
        return CurrentTime - Entry.Value > RecentActionWindow;
    });

    // Detect combos
    TArray<FString> Combos = DetectCombos();
    for (const FString& Combo : Combos)
    {
        OnComboDetected.Broadcast(Combo);
    }
    if (Combos.Num() > 0)
    {
        // Consume the history on a match, or the same combo would re-fire every tick until its
        // entries age out of the window.
        RecentActions.Empty();
    }

    // Record for imitation learning
    if (bLearnFromHumanInput && DetectedActions.Num() > 0)
    {
        FString Context = FString::Join(DetectedActions, TEXT(","));
        RecordInputForImitation(CurrentState, Context);
    }
}

float UGameControllerInterface::ApplyDeadzone(float Value, float Deadzone) const
{
    if (FMath::Abs(Value) < Deadzone)
    {
        return 0.0f;
    }

    float Sign = FMath::Sign(Value);
    return Sign * (FMath::Abs(Value) - Deadzone) / (1.0f - Deadzone);
}

FVector2D UGameControllerInterface::ApplyRadialDeadzone(const FVector2D& Stick, float Deadzone) const
{
    float Magnitude = Stick.Size();
    if (Magnitude < Deadzone)
    {
        return FVector2D::ZeroVector;
    }

    FVector2D Normalized = Stick / Magnitude;
    float AdjustedMagnitude = (Magnitude - Deadzone) / (1.0f - Deadzone);
    return Normalized * FMath::Min(AdjustedMagnitude, 1.0f);
}

// ============================================================================
// Input Query Methods
// ============================================================================

FControllerInputState UGameControllerInterface::GetCurrentInputState() const
{
    return CurrentState;
}

FControllerInputState UGameControllerInterface::GetInputStateFromBuffer(int32 FramesAgo) const
{
    if (FramesAgo >= 0 && FramesAgo < InputBuffer.Num())
    {
        return InputBuffer[FramesAgo];
    }
    return FControllerInputState();
}

TArray<FControllerInputState> UGameControllerInterface::GetInputBuffer() const
{
    return InputBuffer;
}

bool UGameControllerInterface::IsButtonPressed(EGamepadButton Button) const
{
    return CurrentState.IsPressed(Button);
}

bool UGameControllerInterface::WasButtonJustPressed(EGamepadButton Button) const
{
    return CurrentState.IsPressed(Button) &&
           !PreviousState.IsPressed(Button);
}

bool UGameControllerInterface::WasButtonJustReleased(EGamepadButton Button) const
{
    return !CurrentState.IsPressed(Button) &&
           PreviousState.IsPressed(Button);
}

float UGameControllerInterface::GetAxisValue(EGamepadAxis Axis) const
{
    switch (Axis)
    {
        case EGamepadAxis::LeftStickX: return CurrentState.LeftStickX;
        case EGamepadAxis::LeftStickY: return CurrentState.LeftStickY;
        case EGamepadAxis::RightStickX: return CurrentState.RightStickX;
        case EGamepadAxis::RightStickY: return CurrentState.RightStickY;
        case EGamepadAxis::LeftTrigger: return CurrentState.LeftTrigger;
        case EGamepadAxis::RightTrigger: return CurrentState.RightTrigger;
        default: return 0.0f;
    }
}

FVector2D UGameControllerInterface::GetLeftStick() const
{
    return ApplyRadialDeadzone(FVector2D(CurrentState.LeftStickX, CurrentState.LeftStickY), StickDeadzone);
}

FVector2D UGameControllerInterface::GetRightStick() const
{
    return ApplyRadialDeadzone(FVector2D(CurrentState.RightStickX, CurrentState.RightStickY), StickDeadzone);
}

// ============================================================================
// Action Detection
// ============================================================================

TArray<FString> UGameControllerInterface::DetectCurrentActions() const
{
    TArray<FString> Actions;

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (const FGameActionMapping& Mapping : ActionMappings)
    {
        // Evaluate button and axis conditions independently, then combine, rather than letting
        // one silently overwrite the other when a mapping configures both.
        bool bButtonOk = true;
        bool bHasButtonCondition = false;
        if (Mapping.PrimaryButton != EGamepadButton::None)
        {
            bHasButtonCondition = true;
            bButtonOk = CurrentState.IsPressed(Mapping.PrimaryButton);
        }

        bool bAxisOk = true;
        bool bHasAxisCondition = false;
        if (Mapping.PrimaryAxis != EGamepadAxis::None)
        {
            bHasAxisCondition = true;
            float AxisValue = GetAxisValue(Mapping.PrimaryAxis);
            bAxisOk = FMath::Abs(AxisValue) >= Mapping.AxisThreshold;
        }

        bool bActionActive = (bHasButtonCondition || bHasAxisCondition) &&
                              (!bHasButtonCondition || bButtonOk) &&
                              (!bHasAxisCondition || bAxisOk);

        // Check secondary button
        if (Mapping.SecondaryButton != EGamepadButton::None && bActionActive)
        {
            bActionActive = CurrentState.IsPressed(Mapping.SecondaryButton);
        }

        // Hold-gated actions only activate once the trigger has been held for HoldDuration.
        if (bActionActive && Mapping.bRequiresHold)
        {
            EGamepadButton HoldButton = Mapping.PrimaryButton != EGamepadButton::None ?
                Mapping.PrimaryButton : Mapping.SecondaryButton;

            const float* StartTime = HoldButton != EGamepadButton::None ?
                ButtonHoldStartTimes.Find(HoldButton) : nullptr;

            if (StartTime)
            {
                bActionActive = (CurrentTime - *StartTime) >= Mapping.HoldDuration;
            }
            else if (HoldButton == EGamepadButton::None)
            {
                // Axis-only hold (e.g. trigger axis): no discrete button press is tracked,
                // so a hold requirement without a button cannot be timed - treat as satisfied.
                bActionActive = true;
            }
            else
            {
                // Button not currently tracked as held (shouldn't happen if bActionActive is true).
                bActionActive = false;
            }
        }

        if (bActionActive)
        {
            Actions.Add(Mapping.ActionName);
        }
    }

    return Actions;
}

bool UGameControllerInterface::IsActionActive(const FString& ActionName) const
{
    TArray<FString> CurrentActions = DetectCurrentActions();
    return CurrentActions.Contains(ActionName);
}

void UGameControllerInterface::RegisterComboSequence(const FString& ComboName,
                                                      const TArray<FString>& ActionSequence,
                                                      float MaxTimeBetween)
{
    FInputSequence Combo;
    Combo.SequenceName = ComboName;
    Combo.ActionNames = ActionSequence;
    Combo.MaxTimeBetweenInputs = MaxTimeBetween;

    RegisteredCombos.Add(ComboName, Combo);
}

TArray<FString> UGameControllerInterface::DetectCombos() const
{
    TArray<FString> DetectedCombos;

    if (RecentActions.Num() == 0)
    {
        return DetectedCombos;
    }

    for (const auto& ComboPair : RegisteredCombos)
    {
        const FInputSequence& Combo = ComboPair.Value;
        const int32 Len = Combo.ActionNames.Num();
        if (Len == 0 || RecentActions.Num() < Len)
        {
            continue;
        }

        // Ordered-subsequence scan: the combo's actions must appear in order, each within
        // MaxTimeBetweenInputs of the previously matched one, but OTHER concurrently active
        // actions (movement, camera - active almost constantly in real play) may be interleaved
        // between them. A strict "last Len entries" suffix match would be broken by any such
        // interleaving.
        int32 MatchIdx = 0;
        float LastMatchTime = 0.0f;
        for (const TPair<FString, float>& Entry : RecentActions)
        {
            if (Entry.Key != Combo.ActionNames[MatchIdx])
            {
                continue;
            }
            if (MatchIdx > 0 && Entry.Value - LastMatchTime > Combo.MaxTimeBetweenInputs)
            {
                // Chain broken by too large a gap - this entry can still start a new attempt.
                MatchIdx = 0;
                if (Entry.Key != Combo.ActionNames[0])
                {
                    continue;
                }
            }
            LastMatchTime = Entry.Value;
            ++MatchIdx;
            if (MatchIdx == Len)
            {
                DetectedCombos.Add(Combo.SequenceName);
                break;
            }
        }
    }

    return DetectedCombos;
}

// ============================================================================
// Output Generation (AI Mode)
// ============================================================================

FControllerOutputCommand UGameControllerInterface::GenerateOutputFromCognition()
{
    FControllerOutputCommand Command;

    if (!DTECore)
    {
        return Command;
    }

    // Get cognitive output and convert to controller command
    // TArray<float> CognitiveOutput = DTECore->GenerateActionOutput();
    // Command = CognitiveOutputToCommand(CognitiveOutput);

    Command.ActionName = TEXT("CognitiveAction");
    OnOutputCommandGenerated.Broadcast(Command);

    return Command;
}

FControllerOutputCommand UGameControllerInterface::GenerateOutputForAction(const FString& ActionName)
{
    FControllerOutputCommand Command;
    Command.ActionName = ActionName;

    // Find mapping for action
    for (const FGameActionMapping& Mapping : ActionMappings)
    {
        if (Mapping.ActionName == ActionName)
        {
            Command.Category = Mapping.Category;

            if (Mapping.PrimaryButton != EGamepadButton::None)
            {
                Command.DesiredState.Press(Mapping.PrimaryButton);
            }

            break;
        }
    }

    return Command;
}

FControllerOutputCommand UGameControllerInterface::GenerateMovementOutput(const FVector2D& Direction, float Speed)
{
    FControllerOutputCommand Command;
    Command.Category = EGameActionCategory::Movement;
    Command.ActionName = TEXT("Movement");

    FVector2D ClampedDir = Direction.GetSafeNormal() * FMath::Clamp(Speed, 0.0f, 1.0f);
    Command.DesiredState.LeftStickX = ClampedDir.X;
    Command.DesiredState.LeftStickY = ClampedDir.Y;

    return Command;
}

FControllerOutputCommand UGameControllerInterface::GenerateCameraOutput(const FVector2D& Direction, float Speed)
{
    FControllerOutputCommand Command;
    Command.Category = EGameActionCategory::Camera;
    Command.ActionName = TEXT("Camera");

    FVector2D ClampedDir = Direction.GetSafeNormal() * FMath::Clamp(Speed, 0.0f, 1.0f);
    Command.DesiredState.RightStickX = ClampedDir.X;
    Command.DesiredState.RightStickY = ClampedDir.Y;

    return Command;
}

void UGameControllerInterface::QueueOutputCommand(const FControllerOutputCommand& Command)
{
    OutputQueue.Add(Command);

    // Sort by priority
    OutputQueue.Sort([](const FControllerOutputCommand& A, const FControllerOutputCommand& B)
    {
        return A.Priority > B.Priority;
    });
}

FControllerOutputCommand UGameControllerInterface::GetCurrentOutputCommand() const
{
    if (OutputQueue.Num() > 0)
    {
        return OutputQueue[0];
    }
    return FControllerOutputCommand();
}

void UGameControllerInterface::ExecuteQueuedOutput()
{
    if (OutputQueue.Num() == 0)
    {
        return;
    }

    FControllerOutputCommand& Command = OutputQueue[0];

    // In AI output mode, the desired state becomes the current state
    // This simulates controller input for the game
    CurrentState = Command.DesiredState;
    CurrentState.Timestamp = GetWorld()->GetTimeSeconds();
}

void UGameControllerInterface::ProcessOutputQueue(float DeltaTime)
{
    // Process and execute outputs
    ExecuteQueuedOutput();

    // Only the head command is actually executing each tick; only its duration should burn down.
    // Commands still waiting behind it must be left untouched.
    if (OutputQueue.Num() > 0)
    {
        OutputQueue[0].Duration -= DeltaTime;
        if (OutputQueue[0].Duration <= 0.0f)
        {
            OutputQueue.RemoveAt(0);
        }
    }
}

// ============================================================================
// Cognitive Integration
// ============================================================================

TArray<float> UGameControllerInterface::InputToSensoryVector(const FControllerInputState& Input) const
{
    return Input.ToActionVector();
}

FControllerOutputCommand UGameControllerInterface::CognitiveOutputToCommand(const TArray<float>& CognitiveOutput) const
{
    FControllerOutputCommand Command;

    if (CognitiveOutput.Num() >= 22)
    {
        Command.DesiredState = FControllerInputState::FromActionVector(CognitiveOutput);
    }

    return Command;
}

FString UGameControllerInterface::GetStateString(const FControllerInputState& Input) const
{
    return Input.ToActionString();
}

void UGameControllerInterface::RecordInputForImitation(const FControllerInputState& Input, const FString& Context)
{
    // Ring-buffer write: RemoveAt(0) eviction memmoved the whole buffer (~65KB) every tick at
    // steady state. Once full, overwrite the oldest slot in place instead - O(1), and the
    // evicted slot's heap blocks are reused by the assignment.
    if (ImitationBuffer.Num() < MaxImitationBufferSize)
    {
        ImitationBuffer.Emplace(Input, Context);
    }
    else
    {
        ImitationBuffer[ImitationWriteIndex] = TPair<FControllerInputState, FString>(Input, Context);
        ImitationWriteIndex = (ImitationWriteIndex + 1) % MaxImitationBufferSize;
    }

    // Record to learning system
    if (LearningSystem)
    {
        // Build each state string exactly once (the input string is used for both the action
        // and next-state slots).
        const FString InputStr = GetStateString(Input);

        TArray<FString> Tags;
        Tags.Reserve(2);
        Tags.Add(TEXT("Imitation"));
        Tags.Add(Context);

        LearningSystem->RecordExperience(
            GetStateString(PreviousState),
            InputStr,
            InputStr,
            0.0f,  // Reward assigned later
            Tags,
            false
        );
    }
}

void UGameControllerInterface::BroadcastCognitiveState()
{
    if (!EmbodimentComponent)
    {
        return;
    }

    // Update sensorimotor contingencies based on controller state
    FString ActionStr = CurrentState.ToActionString();
    FString PreviousActionStr = PreviousState.ToActionString();

    if (ActionStr != PreviousActionStr)
    {
        // Learn action-outcome relationships
        EmbodimentComponent->LearnContingency(
            PreviousActionStr,
            ActionStr,  // Expected (same as actual for now)
            ActionStr   // Actual
        );
    }
}

// ============================================================================
// Mapping Management
// ============================================================================

void UGameControllerInterface::AddActionMapping(const FGameActionMapping& Mapping)
{
    // Remove existing mapping with same name
    RemoveActionMapping(Mapping.ActionName);
    ActionMappings.Add(Mapping);
}

void UGameControllerInterface::RemoveActionMapping(const FString& ActionName)
{
    ActionMappings.RemoveAll([&ActionName](const FGameActionMapping& M)
    {
        return M.ActionName == ActionName;
    });
}

FGameActionMapping UGameControllerInterface::GetActionMapping(const FString& ActionName) const
{
    for (const FGameActionMapping& Mapping : ActionMappings)
    {
        if (Mapping.ActionName == ActionName)
        {
            return Mapping;
        }
    }
    return FGameActionMapping();
}

void UGameControllerInterface::LoadPresetMappings(const FString& GameType)
{
    ActionMappings.Empty();

    if (GameType == TEXT("FPS"))
    {
        // First-person shooter mappings
        InitializeDefaultMappings();

        FGameActionMapping Aim;
        Aim.ActionName = TEXT("Aim");
        Aim.Category = EGameActionCategory::Attack;
        Aim.PrimaryButton = EGamepadButton::LeftTrigger;
        Aim.bRequiresHold = true;
        ActionMappings.Add(Aim);

        FGameActionMapping Fire;
        Fire.ActionName = TEXT("Fire");
        Fire.Category = EGameActionCategory::Attack;
        Fire.PrimaryButton = EGamepadButton::RightTrigger;
        ActionMappings.Add(Fire);

        FGameActionMapping Reload;
        Reload.ActionName = TEXT("Reload");
        Reload.Category = EGameActionCategory::Special;
        Reload.PrimaryButton = EGamepadButton::FaceLeft;
        ActionMappings.Add(Reload);
    }
    else if (GameType == TEXT("Fighting"))
    {
        // Fighting game mappings
        FGameActionMapping LightPunch;
        LightPunch.ActionName = TEXT("LightPunch");
        LightPunch.Category = EGameActionCategory::Attack;
        LightPunch.PrimaryButton = EGamepadButton::FaceLeft;
        ActionMappings.Add(LightPunch);

        FGameActionMapping HeavyPunch;
        HeavyPunch.ActionName = TEXT("HeavyPunch");
        HeavyPunch.Category = EGameActionCategory::Attack;
        HeavyPunch.PrimaryButton = EGamepadButton::FaceTop;
        ActionMappings.Add(HeavyPunch);

        FGameActionMapping LightKick;
        LightKick.ActionName = TEXT("LightKick");
        LightKick.Category = EGameActionCategory::Attack;
        LightKick.PrimaryButton = EGamepadButton::FaceBottom;
        ActionMappings.Add(LightKick);

        FGameActionMapping HeavyKick;
        HeavyKick.ActionName = TEXT("HeavyKick");
        HeavyKick.Category = EGameActionCategory::Attack;
        HeavyKick.PrimaryButton = EGamepadButton::FaceRight;
        ActionMappings.Add(HeavyKick);
    }
    else if (GameType == TEXT("Racing"))
    {
        // Racing game mappings
        FGameActionMapping Accelerate;
        Accelerate.ActionName = TEXT("Accelerate");
        Accelerate.Category = EGameActionCategory::Movement;
        Accelerate.PrimaryAxis = EGamepadAxis::RightTrigger;
        Accelerate.AxisThreshold = 0.1f;
        ActionMappings.Add(Accelerate);

        FGameActionMapping Brake;
        Brake.ActionName = TEXT("Brake");
        Brake.Category = EGameActionCategory::Movement;
        Brake.PrimaryAxis = EGamepadAxis::LeftTrigger;
        Brake.AxisThreshold = 0.1f;
        ActionMappings.Add(Brake);

        FGameActionMapping Steer;
        Steer.ActionName = TEXT("Steer");
        Steer.Category = EGameActionCategory::Movement;
        Steer.PrimaryAxis = EGamepadAxis::LeftStickX;
        Steer.AxisThreshold = 0.1f;
        ActionMappings.Add(Steer);

        FGameActionMapping Boost;
        Boost.ActionName = TEXT("Boost");
        Boost.Category = EGameActionCategory::Special;
        Boost.PrimaryButton = EGamepadButton::FaceBottom;
        ActionMappings.Add(Boost);
    }
    else
    {
        // Default mappings
        InitializeDefaultMappings();
    }
}

// ============================================================================
// Serialization
// ============================================================================

TArray<uint8> UGameControllerInterface::SerializeInputState(const FControllerInputState& State) const
{
    TArray<uint8> Data;
    FMemoryWriter Ar(Data);

    // Write axes
    Ar << const_cast<float&>(State.LeftStickX);
    Ar << const_cast<float&>(State.LeftStickY);
    Ar << const_cast<float&>(State.RightStickX);
    Ar << const_cast<float&>(State.RightStickY);
    Ar << const_cast<float&>(State.LeftTrigger);
    Ar << const_cast<float&>(State.RightTrigger);

    // Buttons are already a bitmask (bit index = enum value) - same wire format as before
    uint32 ButtonMask = static_cast<uint32>(State.ButtonMask);
    Ar << ButtonMask;

    Ar << const_cast<float&>(State.Timestamp);

    return Data;
}

FControllerInputState UGameControllerInterface::DeserializeInputState(const TArray<uint8>& Data) const
{
    FControllerInputState State;
    FMemoryReader Ar(Data);

    Ar << State.LeftStickX;
    Ar << State.LeftStickY;
    Ar << State.RightStickX;
    Ar << State.RightStickY;
    Ar << State.LeftTrigger;
    Ar << State.RightTrigger;

    uint32 ButtonMask;
    Ar << ButtonMask;

    // Same wire format as before (bit index = enum value, incl. Select at bit 16); the struct
    // stores exactly this representation now, so no per-bit expansion is needed.
    State.ButtonMask = static_cast<int32>(ButtonMask);

    Ar << State.Timestamp;

    return State;
}
