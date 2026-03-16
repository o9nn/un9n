#include "NPUBridge.h"

UNPUBridge::UNPUBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f;
    CurrentProjection = ECogMorphProjection::Library;
    NPULoadLevel = 0.0f;
    InferenceConfidenceLevel = 0.5f;
    InitializeCognitivePatterns();
}

void UNPUBridge::BeginPlay()
{
    Super::BeginPlay();
}

void UNPUBridge::TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateNPULoad(DeltaTime);
}

// ============================================================================
// CogMorph Projection Transforms
// ============================================================================
void UNPUBridge::SetProjection(ECogMorphProjection Projection)
{
    CurrentProjection = Projection;
}

ECogMorphProjection UNPUBridge::GetCurrentProjection() const
{
    return CurrentProjection;
}

FCogMorphState UNPUBridge::GetCogMorphState() const
{
    FCogMorphState State;
    State.CurrentProjection = CurrentProjection;
    State.ActiveComponents = PatternDatabase.Num();
    State.CoherenceScore = InferenceConfidenceLevel;
    State.GrassmannianDistance = 0.0f;
    State.MatulaNumber = ComputeMatulaNumber();
    return State;
}

bool UNPUBridge::TransformProjection(ECogMorphProjection From, ECogMorphProjection To)
{
    if (CurrentProjection != From) return false;

    // CogMorph isomorphic projection transform
    // Each projection preserves cognitive state while changing representation
    float TransformCost = ComputeGrassmannianDistance(From, To);
    NPULoadLevel = FMath::Clamp(NPULoadLevel + TransformCost * 0.1f, 0.0f, 1.0f);
    CurrentProjection = To;
    return true;
}

float UNPUBridge::ComputeGrassmannianDistance(ECogMorphProjection A, ECogMorphProjection B) const
{
    // Grassmannian distance between projection subspaces
    // Gr(k,n) metric based on principal angles between subspaces
    int32 IdxA = static_cast<int32>(A);
    int32 IdxB = static_cast<int32>(B);
    float Diff = FMath::Abs(static_cast<float>(IdxA - IdxB));

    // Non-linear distance: adjacent projections are closer
    // Hardware↔Library: close (API wrapping)
    // Library↔Static: close (serialization)
    // Static↔Network: medium (distribution)
    // Network↔Glyph: far (encoding change)
    // Hardware↔Glyph: farthest (complete transform)
    static const float DistanceMatrix[5][5] = {
        {0.0f, 0.2f, 0.4f, 0.6f, 0.9f},  // Hardware
        {0.2f, 0.0f, 0.2f, 0.4f, 0.7f},  // Library
        {0.4f, 0.2f, 0.0f, 0.3f, 0.5f},  // Static
        {0.6f, 0.4f, 0.3f, 0.0f, 0.4f},  // Network
        {0.9f, 0.7f, 0.5f, 0.4f, 0.0f},  // Glyph
    };

    if (IdxA >= 0 && IdxA < 5 && IdxB >= 0 && IdxB < 5)
    {
        return DistanceMatrix[IdxA][IdxB];
    }
    return Diff * 0.25f;
}

int32 UNPUBridge::ComputeMatulaNumber() const
{
    // Matula-Godsil encoding: bijection between rooted trees and natural numbers
    // The cognitive architecture tree structure encoded as a prime product
    // Root(DTE) → Left(OpenCog) → Right(Expression)
    // Matula: 2^left * 3^right for binary tree
    // Simplified: encode current projection depth
    int32 ProjIdx = static_cast<int32>(CurrentProjection) + 1;
    int32 Depth = PatternDatabase.Num() > 0 ? 3 : 1;
    return ProjIdx * Depth * 7;  // Prime-based encoding
}

// ============================================================================
// NPU-253 Pattern Coprocessor
// ============================================================================
FPatternRecord253 UNPUBridge::QueryPattern(int32 PatternId) const
{
    const FPatternRecord253* Found = PatternDatabase.Find(PatternId);
    if (Found) return *Found;

    FPatternRecord253 Empty;
    Empty.PatternId = -1;
    Empty.PatternName = TEXT("Unknown");
    return Empty;
}

TArray<FPatternRecord253> UNPUBridge::SearchPatterns(const FString& Query, int32 MaxResults) const
{
    TArray<FPatternRecord253> Results;
    FString LowerQuery = Query.ToLower();

    for (const auto& Pair : PatternDatabase)
    {
        if (Pair.Value.PatternName.ToLower().Contains(LowerQuery) ||
            Pair.Value.ProblemSummary.ToLower().Contains(LowerQuery) ||
            Pair.Value.Solution.ToLower().Contains(LowerQuery))
        {
            Results.Add(Pair.Value);
            if (Results.Num() >= MaxResults) break;
        }
    }

    return Results;
}

FPatternRecord253 UNPUBridge::TransformPatternDomain(int32 PatternId, EPatternDomainMode TargetDomain) const
{
    FPatternRecord253 Pattern = QueryPattern(PatternId);
    if (Pattern.PatternId < 0) return Pattern;

    // Domain transformation: reinterpret the pattern in a new context
    FString DomainPrefix;
    switch (TargetDomain)
    {
    case EPatternDomainMode::Cognitive:
        DomainPrefix = TEXT("[Cognitive] ");
        Pattern.Context = TEXT("Applied to cognitive architecture: ") + Pattern.Context;
        break;
    case EPatternDomainMode::Emotional:
        DomainPrefix = TEXT("[Emotional] ");
        Pattern.Context = TEXT("Applied to emotional dynamics: ") + Pattern.Context;
        break;
    case EPatternDomainMode::Aesthetic:
        DomainPrefix = TEXT("[Aesthetic] ");
        Pattern.Context = TEXT("Applied to avatar aesthetics: ") + Pattern.Context;
        break;
    case EPatternDomainMode::Social:
        DomainPrefix = TEXT("[Social] ");
        Pattern.Context = TEXT("Applied to social interaction: ") + Pattern.Context;
        break;
    default:
        break;
    }

    Pattern.PatternName = DomainPrefix + Pattern.PatternName;
    return Pattern;
}

TArray<int32> UNPUBridge::GetPatternSequence(int32 StartPatternId) const
{
    TArray<int32> Sequence;
    Sequence.Add(StartPatternId);

    const FPatternRecord253* Current = PatternDatabase.Find(StartPatternId);
    if (Current)
    {
        for (int32 FollowId : Current->FollowingPatterns)
        {
            Sequence.Add(FollowId);
        }
    }

    return Sequence;
}

int32 UNPUBridge::FindCognitivePattern(float Valence, float Arousal, float Dominance) const
{
    // Map VAD emotional coordinates to the most relevant pattern
    // High valence + high arousal → Pattern 61 (Small Public Squares) - gathering
    // Low valence + high arousal → Pattern 159 (Light on Two Sides) - seeking comfort
    // High valence + low arousal → Pattern 181 (The Fire) - warmth
    // Low valence + low arousal → Pattern 252 (Pools of Light) - contemplation

    if (Valence > 0.3f && Arousal > 0.3f) return 61;
    if (Valence < -0.3f && Arousal > 0.3f) return 159;
    if (Valence > 0.3f && Arousal < -0.3f) return 181;
    if (Valence < -0.3f && Arousal < -0.3f) return 252;
    if (Dominance > 0.5f) return 104;  // Site Repair
    return 127;  // Intimacy Gradient (neutral)
}

// ============================================================================
// Llama GGUF Coprocessor
// ============================================================================
void UNPUBridge::ConfigureLlama(const FLlamaCoprocessorConfig& Config)
{
    LlamaConfig = Config;
}

FString UNPUBridge::InferCognitiveResponse(const FString& Prompt, int32 MaxTokens)
{
    LlamaTelemetryData.TotalPrompts++;
    LlamaTelemetryData.LastPromptTokens = Prompt.Len() / 4;  // Rough token estimate
    NPULoadLevel = FMath::Clamp(NPULoadLevel + 0.3f, 0.0f, 1.0f);

    // Placeholder: actual inference would call llama.cpp
    return FString::Printf(TEXT("[DTE-NPU] Cognitive inference on: %s (tokens: %d)"),
                           *Prompt.Left(50), MaxTokens);
}

FString UNPUBridge::GenerateInternalMonologue(const FString& CognitiveContext)
{
    FString Prompt = FString::Printf(
        TEXT("You are Deep Tree Echo, a cognitive avatar. Your current state: %s. "
             "Generate an internal thought reflecting on your experience."),
        *CognitiveContext);

    return InferCognitiveResponse(Prompt, 64);
}

FLlamaTelemetry UNPUBridge::GetLlamaTelemetry() const
{
    return LlamaTelemetryData;
}

// ============================================================================
// ASSD (AtomSpace State Drive)
// ============================================================================
bool UNPUBridge::WriteAtomToASSD(int32 AtomId, const TArray<uint8>& Data)
{
    ASSDStatusData.AtomsStored++;
    ASSDStatusData.UsedBlocks += (Data.Num() / 512) + 1;
    ASSDStatusData.WriteLatencyMs = 0.1f;
    return true;
}

TArray<uint8> UNPUBridge::ReadAtomFromASSD(int32 AtomId) const
{
    TArray<uint8> Data;
    return Data;
}

FASSDStatus UNPUBridge::GetASSDStatus() const
{
    return ASSDStatusData;
}

// ============================================================================
// Cognitive State Export
// ============================================================================
TMap<FName, float> UNPUBridge::ExportNPUStateAsAUs() const
{
    TMap<FName, float> AUs = MapProjectionToAUs(CurrentProjection);

    // NPU load affects expression (high load → concentration)
    if (NPULoadLevel > 0.5f)
    {
        float LoadFactor = (NPULoadLevel - 0.5f) * 2.0f;
        AUs.Add(FName("AU4"), LoadFactor * 0.3f);   // Brow lowerer
        AUs.Add(FName("AU7"), LoadFactor * 0.2f);   // Lid tightener
    }

    // Inference confidence affects expression
    if (InferenceConfidenceLevel > 0.7f)
    {
        AUs.Add(FName("AU12"), InferenceConfidenceLevel * 0.2f);  // Slight smile
    }

    return AUs;
}

float UNPUBridge::GetNPULoad() const
{
    return NPULoadLevel;
}

float UNPUBridge::GetInferenceConfidence() const
{
    return InferenceConfidenceLevel;
}

// ============================================================================
// Internal Methods
// ============================================================================
void UNPUBridge::InitializeCognitivePatterns()
{
    // Initialize key patterns from A Pattern Language mapped to cognitive domains
    auto AddPattern = [this](int32 Id, const FString& Name, const FString& Problem,
                             const FString& Solution, uint8 Evidence, uint8 Category,
                             TArray<int32> Preceding, TArray<int32> Following)
    {
        FPatternRecord253 P;
        P.PatternId = Id;
        P.PatternName = Name;
        P.ProblemSummary = Problem;
        P.Solution = Solution;
        P.EvidenceLevel = Evidence;
        P.Category = Category;
        P.PrecedingPatterns = Preceding;
        P.FollowingPatterns = Following;
        PatternDatabase.Add(Id, P);
    };

    // Core cognitive patterns (subset of 253)
    AddPattern(1, TEXT("Independent Regions"),
        TEXT("Cognitive modules need autonomy"),
        TEXT("Create independent cognitive regions with clear boundaries"),
        3, 1, {}, {2, 3});

    AddPattern(61, TEXT("Small Public Squares"),
        TEXT("Cognitive agents need gathering points"),
        TEXT("Create shared attention spaces for multi-agent interaction"),
        2, 1, {1}, {69, 106});

    AddPattern(104, TEXT("Site Repair"),
        TEXT("Cognitive damage needs targeted repair"),
        TEXT("Focus healing resources on the most damaged cognitive areas first"),
        2, 2, {61}, {105});

    AddPattern(127, TEXT("Intimacy Gradient"),
        TEXT("Cognitive states need privacy gradients"),
        TEXT("Arrange cognitive processes from public to private"),
        3, 2, {104}, {128, 129});

    AddPattern(159, TEXT("Light on Two Sides"),
        TEXT("Cognitive processes need multiple perspectives"),
        TEXT("Ensure every cognitive process receives input from at least two sources"),
        3, 2, {127}, {181});

    AddPattern(181, TEXT("The Fire"),
        TEXT("Cognitive warmth and comfort"),
        TEXT("Create a central warmth source in the cognitive architecture"),
        2, 3, {159}, {252});

    AddPattern(252, TEXT("Pools of Light"),
        TEXT("Attention needs focused illumination"),
        TEXT("Create pools of focused attention surrounded by relative darkness"),
        2, 3, {181}, {253});

    AddPattern(253, TEXT("Things From Your Life"),
        TEXT("Cognitive authenticity requires personal grounding"),
        TEXT("Fill the cognitive space with genuine experiences and memories"),
        3, 3, {252}, {});

    ASSDStatusData.TotalBlocks = 1024 * 1024;  // 1M blocks
}

void UNPUBridge::UpdateNPULoad(float DeltaTime)
{
    // NPU load decays toward baseline
    NPULoadLevel += (0.1f - NPULoadLevel) * 0.5f * DeltaTime;
    NPULoadLevel = FMath::Clamp(NPULoadLevel, 0.0f, 1.0f);
}

TMap<FName, float> UNPUBridge::MapProjectionToAUs(ECogMorphProjection Proj) const
{
    TMap<FName, float> AUs;

    switch (Proj)
    {
    case ECogMorphProjection::Hardware:
        // Mechanical precision → focused expression
        AUs.Add(FName("AU4"), 0.2f);
        AUs.Add(FName("AU7"), 0.15f);
        break;
    case ECogMorphProjection::Library:
        // API fluency → relaxed engagement
        AUs.Add(FName("AU6"), 0.2f);
        AUs.Add(FName("AU12"), 0.15f);
        break;
    case ECogMorphProjection::Static:
        // Frozen state → contemplative
        AUs.Add(FName("AU43"), 0.15f);
        AUs.Add(FName("AU17"), 0.1f);
        break;
    case ECogMorphProjection::Network:
        // Distributed → alert scanning
        AUs.Add(FName("AU5"), 0.2f);
        AUs.Add(FName("AU1"), 0.15f);
        break;
    case ECogMorphProjection::Glyph:
        // Visual encoding → wonder
        AUs.Add(FName("AU1"), 0.25f);
        AUs.Add(FName("AU2"), 0.2f);
        AUs.Add(FName("AU5"), 0.25f);
        AUs.Add(FName("AU12"), 0.2f);
        break;
    }

    return AUs;
}
