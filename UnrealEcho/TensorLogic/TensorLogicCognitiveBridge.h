#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TensorLogicEngine.h"
#include "Cognitive/DeepTreeEchoCognitiveCore.h"
#include "NeuralNetwork/OCNNIntegrationBridge.h"
#include "TensorLogicCognitiveBridge.generated.h"

/**
 * Tensor Logic Cognitive Bridge
 * Bridges tensor logic with existing cognitive architectures
 * 
 * Integrates:
 * - TensorLogicEngine (neural-symbolic reasoning)
 * - DeepTreeEchoCognitiveCore (hypergraph memory)
 * - OCNNIntegrationBridge (neural processing)
 * 
 * Enables:
 * - Symbolic reasoning over neural representations
 * - Neural learning guided by logical constraints
 * - Hybrid inference (symbolic + subsymbolic)
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UTensorLogicCognitiveBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UTensorLogicCognitiveBridge();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================
    // COMPONENT REFERENCES
    // ========================================

    /** Tensor logic engine */
    UPROPERTY(BlueprintReadOnly, Category = "Cognitive|Components")
    UTensorLogicEngine* TensorLogicEngine;

    /** Deep Tree Echo cognitive core */
    UPROPERTY(BlueprintReadOnly, Category = "Cognitive|Components")
    UDeepTreeEchoCognitiveCore* CognitiveCore;

    /** OCNN integration bridge */
    UPROPERTY(BlueprintReadOnly, Category = "Cognitive|Components")
    UOCNNIntegrationBridge* NeuralBridge;

    // ========================================
    // CONFIGURATION
    // ========================================

    /** Enable automatic synchronization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Config")
    bool bAutoSync = true;

    /** Sync frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Config", meta = (ClampMin = "0.1", ClampMax = "60"))
    float SyncFrequency = 1.0f;

    /** Enable bidirectional learning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Config")
    bool bEnableBidirectionalLearning = true;

    /** Neural-to-symbolic weight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NeuralToSymbolicWeight = 0.5f;

    // ========================================
    // PUBLIC API - SYNCHRONIZATION
    // ========================================

    /** Initialize components */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    bool InitializeBridge();

    /** Synchronize hypergraph to tensor logic */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    void SyncHypergraphToTensorLogic();

    /** Synchronize tensor logic to hypergraph */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    void SyncTensorLogicToHypergraph();

    /** Bidirectional synchronization */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    void BidirectionalSync();

    // ========================================
    // PUBLIC API - HYBRID REASONING
    // ========================================

    /** Hybrid query (symbolic + neural) */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    FTensorLogicQueryResult HybridQuery(const FString& Predicate, const TArray<FString>& Arguments);

    /** Neural-guided symbolic inference */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    TArray<FHypergraphNode> NeuralGuidedInference(const FString& StartNodeID, int32 MaxDepth);

    /** Symbolic-constrained neural learning */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    float SymbolicConstrainedLearning(const FString& Constraint, const TArray<FString>& Examples);

    // ========================================
    // PUBLIC API - KNOWLEDGE EXTRACTION
    // ========================================

    /** Extract rules from hypergraph patterns */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    TArray<FTensorLogicRule> ExtractRulesFromPatterns();

    /** Extract embeddings from neural bridge */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    void ExtractNeuralEmbeddings();

    /** Learn tensor logic rules from hypergraph */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    int32 LearnRulesFromHypergraph(int32 MaxRules);

    // ========================================
    // PUBLIC API - UTILITIES
    // ========================================

    /** Get bridge statistics */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    FString GetBridgeStatistics() const;

    /** Get component status */
    UFUNCTION(BlueprintCallable, Category = "Cognitive|Bridge")
    FString GetComponentStatus() const;

protected:
    virtual void BeginPlay() override;

private:
    // ========================================
    // INTERNAL STATE
    // ========================================

    /** Is bridge initialized? */
    bool bIsInitialized = false;

    /** Sync timer accumulator */
    float SyncAccumulator = 0.0f;

    /** Last sync timestamp */
    float LastSyncTime = 0.0f;

    /** Synchronization count */
    int32 SyncCount = 0;

    // ========================================
    // INTERNAL METHODS
    // ========================================

    /** Find component references */
    void FindComponents();

    /** Convert hypergraph node to tensor logic atom */
    FTensorLogicAtom NodeToAtom(const FHypergraphNode& Node) const;

    /** Convert hypergraph edge to tensor logic rule */
    FTensorLogicRule EdgeToRule(const FHypergraphEdge& Edge) const;

    /** Convert tensor logic atom to hypergraph node */
    FHypergraphNode AtomToNode(const FTensorLogicAtom& Atom) const;

    /** Convert echo pattern to tensor logic rule */
    FTensorLogicRule PatternToRule(const FEchoPropagationPattern& Pattern) const;

    /** Compute similarity between node and atom */
    float ComputeSimilarity(const FHypergraphNode& Node, const FTensorLogicAtom& Atom) const;

    /** Extract predicate from node content */
    FString ExtractPredicate(const FString& Content) const;

    /** Extract arguments from node content */
    TArray<FString> ExtractArguments(const FString& Content) const;
};
