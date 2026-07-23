#pragma once
/**
 * @file NPUBridge.h
 * @brief Bridge between Neural Processing Unit virtual hardware and Deep Tree Echo
 *
 * Integrates the NPU virtual hardware abstractions into DTE:
 * - CogMorph: 5-projection cognitive state transforms (Hardware→Library→Static→Network→Glyph)
 * - NPU-253: Pattern language coprocessor (253 patterns from A Pattern Language)
 * - Llama GGUF Coprocessor: LLM inference pipeline for cognitive reasoning
 * - ASSD: AtomSpace State Drive for persistent cognitive state
 * - Grassmannian: Manifold operations for cognitive state geometry
 *
 * Composition: npu ⊗ DeepTreeEcho
 * Tensor product of virtual hardware with embodied cognition
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPUBridge.generated.h"

// ============================================================================
// CogMorph Projection Types (matching npu/cogmorph)
// ============================================================================
UENUM(BlueprintType)
enum class ECogMorphProjection : uint8_t
{
    Hardware = 0,   // VirtualPCB, registers, DMA, interrupts
    Library,        // APIs, function calls, objects
    Static,         // CGGUF, frozen weights, serialized state
    Network,        // Wire protocol, distributed
    Glyph,          // Font-based storage, visual encoding
    COUNT
};

// ============================================================================
// NPU-253 Pattern Record (matching npu/fhp/npu253)
// ============================================================================
USTRUCT(BlueprintType)
struct FPatternRecord253
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PatternId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProblemSummary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Solution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 EvidenceLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 Category = 0;  // 0=none, 1=towns, 2=buildings, 3=construction

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> PrecedingPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> FollowingPatterns;
};

// ============================================================================
// NPU-253 Domain Transformation Mode
// ============================================================================
UENUM(BlueprintType)
enum class EPatternDomainMode : uint8_t
{
    None = 0,
    Physical,
    Social,
    Conceptual,
    Psychic,
    Cognitive,      // DTE extension
    Emotional,      // DTE extension
    Aesthetic,       // DTE extension
    COUNT
};

// ============================================================================
// Llama Coprocessor Configuration
// ============================================================================
USTRUCT(BlueprintType)
struct FLlamaCoprocessorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModelPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModelName = TEXT("DeepTreeEcho-7B");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ContextLength = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NumThreads = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GPULayers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BatchSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLowVRAM = false;
};

// ============================================================================
// Llama Inference Telemetry
// ============================================================================
USTRUCT(BlueprintType)
struct FLlamaTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TokensPerSecond = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int64 TotalTokensGenerated = 0;

    UPROPERTY(BlueprintReadOnly)
    int64 TotalPrompts = 0;

    UPROPERTY(BlueprintReadOnly)
    int64 LastPromptTokens = 0;

    UPROPERTY(BlueprintReadOnly)
    int64 LastCompletionTokens = 0;
};

// ============================================================================
// CogMorph State Snapshot
// ============================================================================
USTRUCT(BlueprintType)
struct FCogMorphState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ECogMorphProjection CurrentProjection = ECogMorphProjection::Library;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveComponents = 0;

    UPROPERTY(BlueprintReadOnly)
    float CoherenceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GrassmannianDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 MatulaNumber = 0;
};

// ============================================================================
// ASSD (AtomSpace State Drive) Status
// ============================================================================
USTRUCT(BlueprintType)
struct FASSDStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int64 TotalBlocks = 0;

    UPROPERTY(BlueprintReadOnly)
    int64 UsedBlocks = 0;

    UPROPERTY(BlueprintReadOnly)
    int64 AtomsStored = 0;

    UPROPERTY(BlueprintReadOnly)
    float ReadLatencyMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float WriteLatencyMs = 0.0f;
};

// ============================================================================
// NPU Bridge Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UNPUBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPUBridge();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- CogMorph Projection Transforms ---
    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    void SetProjection(ECogMorphProjection Projection);

    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    ECogMorphProjection GetCurrentProjection() const;

    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    FCogMorphState GetCogMorphState() const;

    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    bool TransformProjection(ECogMorphProjection From, ECogMorphProjection To);

    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    float ComputeGrassmannianDistance(ECogMorphProjection A, ECogMorphProjection B) const;

    UFUNCTION(BlueprintCallable, Category="NPU|CogMorph")
    int32 ComputeMatulaNumber() const;

    // --- NPU-253 Pattern Coprocessor ---
    UFUNCTION(BlueprintCallable, Category="NPU|Pattern253")
    FPatternRecord253 QueryPattern(int32 PatternId) const;

    UFUNCTION(BlueprintCallable, Category="NPU|Pattern253")
    TArray<FPatternRecord253> SearchPatterns(const FString& Query, int32 MaxResults = 10) const;

    UFUNCTION(BlueprintCallable, Category="NPU|Pattern253")
    FPatternRecord253 TransformPatternDomain(int32 PatternId, EPatternDomainMode TargetDomain) const;

    UFUNCTION(BlueprintCallable, Category="NPU|Pattern253")
    TArray<int32> GetPatternSequence(int32 StartPatternId) const;

    UFUNCTION(BlueprintCallable, Category="NPU|Pattern253")
    int32 FindCognitivePattern(float Valence, float Arousal, float Dominance) const;

    // --- Llama GGUF Coprocessor ---
    UFUNCTION(BlueprintCallable, Category="NPU|Llama")
    void ConfigureLlama(const FLlamaCoprocessorConfig& Config);

    UFUNCTION(BlueprintCallable, Category="NPU|Llama")
    FString InferCognitiveResponse(const FString& Prompt, int32 MaxTokens = 128);

    UFUNCTION(BlueprintCallable, Category="NPU|Llama")
    FString GenerateInternalMonologue(const FString& CognitiveContext);

    UFUNCTION(BlueprintCallable, Category="NPU|Llama")
    FLlamaTelemetry GetLlamaTelemetry() const;

    // --- ASSD (AtomSpace State Drive) ---
    UFUNCTION(BlueprintCallable, Category="NPU|ASSD")
    bool WriteAtomToASSD(int32 AtomId, const TArray<uint8>& Data);

    UFUNCTION(BlueprintCallable, Category="NPU|ASSD")
    TArray<uint8> ReadAtomFromASSD(int32 AtomId) const;

    UFUNCTION(BlueprintCallable, Category="NPU|ASSD")
    FASSDStatus GetASSDStatus() const;

    // --- Cognitive State Export (for DTE Expression System) ---
    UFUNCTION(BlueprintCallable, Category="NPU|Export")
    TMap<FName, float> ExportNPUStateAsAUs() const;

    UFUNCTION(BlueprintCallable, Category="NPU|Export")
    float GetNPULoad() const;

    UFUNCTION(BlueprintCallable, Category="NPU|Export")
    float GetInferenceConfidence() const;

protected:
    // State
    ECogMorphProjection CurrentProjection;
    FLlamaCoprocessorConfig LlamaConfig;
    FLlamaTelemetry LlamaTelemetryData;
    FASSDStatus ASSDStatusData;
    float NPULoadLevel;
    float InferenceConfidenceLevel;

    // Pattern database (subset for cognitive patterns)
    TMap<int32, FPatternRecord253> PatternDatabase;

    // Internal methods
    void InitializeCognitivePatterns();
    void UpdateNPULoad(float DeltaTime);
    TMap<FName, float> MapProjectionToAUs(ECogMorphProjection Proj) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPU|Config")
    bool bEnableLlamaCoprocessor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPU|Config")
    bool bEnableASSD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPU|Config")
    bool bEnablePattern253 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPU|Config")
    float NPUTickRate = 0.033f;  // ~30Hz
};
