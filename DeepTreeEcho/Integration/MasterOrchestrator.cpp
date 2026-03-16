#include "MasterOrchestrator.h"
#include "OpenCogBridge.h"
#include "UnrealAIBridge.h"
#include "NPUBridge.h"
#include "HyperSkillTensor.h"

UMasterOrchestrator::UMasterOrchestrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;

    CurrentPhase = EEchobeatPhase::Sense;
    PhaseTimer = 0.0f;
    PhaseProgress = 0.0f;
    TotalCycles = 0;
    CycleAccumulator = 0.0f;

    OpenCogBridge = nullptr;
    UnrealAIBridge = nullptr;
    NPUBridgeRef = nullptr;
    HyperSkillTensor = nullptr;
}

void UMasterOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoConnectBridges) ConnectAllBridges();
}

void UMasterOrchestrator::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float PhaseDuration = CyclePeriod / static_cast<float>(EEchobeatPhase::COUNT);
    PhaseTimer += DeltaTime;
    PhaseProgress = FMath::Clamp(PhaseTimer / PhaseDuration, 0.0f, 1.0f);

    // Execute current phase
    switch (CurrentPhase)
    {
    case EEchobeatPhase::Sense:     ExecutePhase_Sense(DeltaTime);     break;
    case EEchobeatPhase::Filter:    ExecutePhase_Filter(DeltaTime);    break;
    case EEchobeatPhase::Resonate:  ExecutePhase_Resonate(DeltaTime);  break;
    case EEchobeatPhase::Echo:      ExecutePhase_Echo(DeltaTime);      break;
    case EEchobeatPhase::Amplify:   ExecutePhase_Amplify(DeltaTime);   break;
    case EEchobeatPhase::Modulate:  ExecutePhase_Modulate(DeltaTime);  break;
    case EEchobeatPhase::Express:   ExecutePhase_Express(DeltaTime);   break;
    case EEchobeatPhase::Reflect:   ExecutePhase_Reflect(DeltaTime);   break;
    case EEchobeatPhase::Evolve:    ExecutePhase_Evolve(DeltaTime);    break;
    default: break;
    }

    // Advance phase when timer expires
    if (PhaseTimer >= PhaseDuration)
    {
        AdvancePhase();
    }

    // Always compute final expression
    ComputeFinalExpression();
    Update4ECognition(DeltaTime);
}

// ============================================================================
// Echobeats Cycle
// ============================================================================
EEchobeatPhase UMasterOrchestrator::GetCurrentPhase() const { return CurrentPhase; }

void UMasterOrchestrator::ForcePhase(EEchobeatPhase Phase)
{
    CurrentPhase = Phase;
    PhaseTimer = 0.0f;
    PhaseProgress = 0.0f;
}

float UMasterOrchestrator::GetPhaseProgress() const { return PhaseProgress; }

void UMasterOrchestrator::AdvancePhase()
{
    PhaseTimer = 0.0f;
    int32 Next = (static_cast<int32>(CurrentPhase) + 1) % static_cast<int32>(EEchobeatPhase::COUNT);
    CurrentPhase = static_cast<EEchobeatPhase>(Next);

    if (CurrentPhase == EEchobeatPhase::Sense)
    {
        TotalCycles++;
        CycleAccumulator = 0.0f;
    }
}

// ============================================================================
// Phase Implementations
// ============================================================================
void UMasterOrchestrator::ExecutePhase_Sense(float DeltaTime)
{
    // Step 1: Perceive environment via EQS (UnrealAI)
    if (UnrealAIBridge)
    {
        FEnvironmentalQuery Env = UnrealAIBridge->QueryEnvironment();
        CognitionState4E.Embedded = FMath::Lerp(CognitionState4E.Embedded,
            (Env.SocialDensity + Env.NoveltyLevel + Env.AestheticValue) / 3.0f,
            DeltaTime * 2.0f);
    }
}

void UMasterOrchestrator::ExecutePhase_Filter(float DeltaTime)
{
    // Step 2: Attention allocation via ECAN (OpenCog)
    if (OpenCogBridge)
    {
        float Attention = OpenCogBridge->GetOverallCognitiveCoherence();
        CognitionState4E.Enacted = FMath::Lerp(CognitionState4E.Enacted,
            Attention, DeltaTime * 2.0f);
    }
}

void UMasterOrchestrator::ExecutePhase_Resonate(float DeltaTime)
{
    // Step 3: Emotional response via Endocrine (OpenCog)
    if (OpenCogBridge)
    {
        float V, A, D;
        OpenCogBridge->GetValenceArousalDominance(V, A, D);
        CognitionState4E.Embodied = FMath::Lerp(CognitionState4E.Embodied,
            (FMath::Abs(V) + FMath::Abs(A)) * 0.5f, DeltaTime * 2.0f);
    }
}

void UMasterOrchestrator::ExecutePhase_Echo(float DeltaTime)
{
    // Step 4: Memory retrieval via AtomSpace (OpenCog)
    if (OpenCogBridge)
    {
        TArray<int32> Focus = OpenCogBridge->GetAttentionalFocus(5);
        // Memory retrieval modulates extended cognition
        CognitionState4E.Extended = FMath::Lerp(CognitionState4E.Extended,
            FMath::Min(static_cast<float>(Focus.Num()) * 0.2f, 1.0f),
            DeltaTime * 2.0f);
    }
}

void UMasterOrchestrator::ExecutePhase_Amplify(float DeltaTime)
{
    // Step 5: Pattern amplification via NPU-253
    if (NPUBridgeRef && OpenCogBridge)
    {
        float V, A, D;
        OpenCogBridge->GetValenceArousalDominance(V, A, D);
        int32 PatternId = NPUBridgeRef->FindCognitivePattern(V, A, D);
        // Pattern found → amplify through tensor
        if (HyperSkillTensor && PatternId > 0)
        {
            // Map pattern to tensor activation
            int32 HotIdx = PatternId % 7;
            int32 AngelIdx = (PatternId / 7) % 7;
            int32 MadIdx = (PatternId / 49) % 7;
            HyperSkillTensor->ActivateCell(
                static_cast<ESuperHotMode>(HotIdx),
                static_cast<EEchoAngelMode>(AngelIdx),
                static_cast<EMadnessMode>(MadIdx),
                0.3f);
        }
    }
}

void UMasterOrchestrator::ExecutePhase_Modulate(float DeltaTime)
{
    // Step 6: Cognitive mode selection via StateTree (UnrealAI)
    if (UnrealAIBridge && OpenCogBridge)
    {
        EOpenCogCognitiveMode CogMode = OpenCogBridge->GetCurrentCognitiveMode();
        // Map cognitive mode to state tree transition
        switch (CogMode)
        {
        case EOpenCogCognitiveMode::Focused:
            UnrealAIBridge->RequestStateTransition(ECognitiveState::Reasoning);
            break;
        case EOpenCogCognitiveMode::Creative:
            UnrealAIBridge->RequestStateTransition(ECognitiveState::CreativeState);
            break;
        case EOpenCogCognitiveMode::Social:
            UnrealAIBridge->RequestStateTransition(ECognitiveState::Expressing);
            break;
        case EOpenCogCognitiveMode::Flow:
            UnrealAIBridge->RequestStateTransition(ECognitiveState::FlowState);
            break;
        case EOpenCogCognitiveMode::Transcendent:
            UnrealAIBridge->RequestStateTransition(ECognitiveState::TranscendentState);
            break;
        default:
            break;
        }
    }
}

void UMasterOrchestrator::ExecutePhase_Express(float DeltaTime)
{
    // Step 7: Avatar expression (computed in ComputeFinalExpression)
    // This phase is the output phase - all bridges contribute AUs
}

void UMasterOrchestrator::ExecutePhase_Reflect(float DeltaTime)
{
    // Step 8: Self-monitoring (Autognosis)
    if (HyperSkillTensor)
    {
        float SA = HyperSkillTensor->GetSelfAwarenessLevel();
        // Self-awareness feeds back into the system
        if (OpenCogBridge)
        {
            OpenCogBridge->InjectCognitiveEvent(TEXT("Flow"), SA * 0.1f);
        }
    }
}

void UMasterOrchestrator::ExecutePhase_Evolve(float DeltaTime)
{
    // Step 9: Ontogenetic growth
    if (HyperSkillTensor)
    {
        // Trigger subtle madness transformation each cycle
        float EvolutionIntensity = 0.01f * (1.0f + TotalCycles * 0.001f);
        HyperSkillTensor->TriggerMadnessTransformation(
            FMath::Clamp(EvolutionIntensity, 0.0f, 0.1f));
    }
}

// ============================================================================
// Expression Computation
// ============================================================================
void UMasterOrchestrator::ComputeFinalExpression()
{
    TMap<FName, float> NewAUs;

    // Collect AUs from all bridges
    if (OpenCogBridge)
    {
        TMap<FName, float> CogAUs = OpenCogBridge->ExportCognitiveStateAsAUs();
        TMap<FName, float> HormAUs = OpenCogBridge->ExportHormoneStateAsAUs();
        for (const auto& P : CogAUs)
            NewAUs.FindOrAdd(P.Key) += P.Value * 0.25f;
        for (const auto& P : HormAUs)
            NewAUs.FindOrAdd(P.Key) += P.Value * 0.25f;
    }

    if (UnrealAIBridge)
    {
        TMap<FName, float> StateAUs = UnrealAIBridge->ExportCognitiveStateAsAUs();
        for (const auto& P : StateAUs)
            NewAUs.FindOrAdd(P.Key) += P.Value * 0.2f;
    }

    if (NPUBridgeRef)
    {
        TMap<FName, float> NPUAUs = NPUBridgeRef->ExportNPUStateAsAUs();
        for (const auto& P : NPUAUs)
            NewAUs.FindOrAdd(P.Key) += P.Value * 0.1f;
    }

    if (HyperSkillTensor)
    {
        TMap<FName, float> TensorAUs = HyperSkillTensor->ExportTensorAsAUs();
        for (const auto& P : TensorAUs)
            NewAUs.FindOrAdd(P.Key) += P.Value * 0.2f;
    }

    // Clamp all values
    for (auto& P : NewAUs)
    {
        P.Value = FMath::Clamp(P.Value, 0.0f, 1.0f);
    }

    // Smooth transition
    for (auto& P : NewAUs)
    {
        float* Old = FinalAUs.Find(P.Key);
        if (Old)
        {
            P.Value = FMath::Lerp(*Old, P.Value, 1.0f - ExpressionSmoothing);
        }
    }

    FinalAUs = NewAUs;
}

// ============================================================================
// 4E Cognition
// ============================================================================
F4ECognitionState UMasterOrchestrator::Get4ECognitionState() const { return CognitionState4E; }

void UMasterOrchestrator::Set4ECognitionState(const F4ECognitionState& State)
{
    CognitionState4E = State;
}

void UMasterOrchestrator::Update4ECognition(float DeltaTime)
{
    // 4E dimensions influence each other
    // Embodied ↔ Enacted (body-action coupling)
    float BodyAction = (CognitionState4E.Embodied + CognitionState4E.Enacted) * 0.5f;
    CognitionState4E.Embodied = FMath::Lerp(CognitionState4E.Embodied, BodyAction, DeltaTime * 0.5f);
    CognitionState4E.Enacted = FMath::Lerp(CognitionState4E.Enacted, BodyAction, DeltaTime * 0.5f);

    // Embedded ↔ Extended (environment-tool coupling)
    float EnvTool = (CognitionState4E.Embedded + CognitionState4E.Extended) * 0.5f;
    CognitionState4E.Embedded = FMath::Lerp(CognitionState4E.Embedded, EnvTool, DeltaTime * 0.3f);
    CognitionState4E.Extended = FMath::Lerp(CognitionState4E.Extended, EnvTool, DeltaTime * 0.3f);
}

// ============================================================================
// Expression Output
// ============================================================================
TMap<FName, float> UMasterOrchestrator::GetFinalExpressionAUs() const { return FinalAUs; }

float UMasterOrchestrator::GetExpressionIntensity() const
{
    float Total = 0.0f;
    for (const auto& P : FinalAUs) Total += P.Value;
    return FinalAUs.Num() > 0 ? Total / FinalAUs.Num() : 0.0f;
}

// ============================================================================
// Telemetry
// ============================================================================
FOrchestratorTelemetry UMasterOrchestrator::GetTelemetry() const
{
    FOrchestratorTelemetry T;
    T.CurrentPhase = CurrentPhase;
    T.TotalCycles = TotalCycles;
    T.CycleFrequencyHz = CyclePeriod > 0.0f ? 1.0f / CyclePeriod : 0.0f;
    T.SelfAwareness = HyperSkillTensor ? HyperSkillTensor->GetSelfAwarenessLevel() : 0.0f;
    T.CognitionState = CognitionState4E;
    T.ActiveBridges = GetActiveBridgeCount();
    T.ActiveAUs = FinalAUs.Num();
    T.OverallCoherence = OpenCogBridge ? OpenCogBridge->GetOverallCognitiveCoherence() : 0.0f;
    return T;
}

// ============================================================================
// Bridge Management
// ============================================================================
void UMasterOrchestrator::ConnectAllBridges()
{
    if (AActor* Owner = GetOwner())
    {
        OpenCogBridge = Owner->FindComponentByClass<UOpenCogBridge>();
        UnrealAIBridge = Owner->FindComponentByClass<UUnrealAIBridge>();
        NPUBridgeRef = Owner->FindComponentByClass<UNPUBridge>();
        HyperSkillTensor = Owner->FindComponentByClass<UHyperSkillTensor>();

        if (HyperSkillTensor)
        {
            HyperSkillTensor->ConnectBridges(OpenCogBridge, UnrealAIBridge, NPUBridgeRef);
        }
    }
}

int32 UMasterOrchestrator::GetActiveBridgeCount() const
{
    int32 Count = 0;
    if (OpenCogBridge) Count++;
    if (UnrealAIBridge) Count++;
    if (NPUBridgeRef) Count++;
    if (HyperSkillTensor) Count++;
    return Count;
}
