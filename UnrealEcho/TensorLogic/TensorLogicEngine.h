#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TensorLogicTerm.h"
#include "TensorLogicRule.h"
#include "TensorLogicEngine.generated.h"

/**
 * Tensor Logic Engine
 * Core inference and learning engine for tensor logic
 * Implements: https://tensor-logic.org/ tensor logic semantics
 * Based on: Pedro Domingos' "Tensor Logic: The Language of AI"
 * 
 * Key Features:
 * - Logical rules as tensor equations
 * - Forward/backward chaining via tensor operations
 * - Boolean (deductive) and continuous (learnable) modes
 * - Predicate invention via tensor decomposition
 * - Einstein summation (einsum) for tensor operations
 * - Gradient-based learning for continuous reasoning
 * - Transparent reasoning traces for auditing
 */

/**
 * Inference Mode
 */
UENUM(BlueprintType)
enum class ETensorLogicInferenceMode : uint8
{
    /** Forward chaining (from facts to conclusions) */
    ForwardChaining UMETA(DisplayName = "Forward Chaining"),
    
    /** Backward chaining (from goals to facts) */
    BackwardChaining UMETA(DisplayName = "Backward Chaining"),
    
    /** Hybrid (combines both) */
    Hybrid UMETA(DisplayName = "Hybrid")
};

/**
 * Learning Algorithm
 */
UENUM(BlueprintType)
enum class ETensorLogicLearningAlgorithm : uint8
{
    /** Gradient descent */
    GradientDescent UMETA(DisplayName = "Gradient Descent"),
    
    /** RESCAL tensor decomposition */
    RESCAL UMETA(DisplayName = "RESCAL"),
    
    /** Tucker decomposition */
    Tucker UMETA(DisplayName = "Tucker"),
    
    /** CP/PARAFAC decomposition */
    CP UMETA(DisplayName = "CP/PARAFAC")
};

/**
 * Tensor Logic Engine Component
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UTensorLogicEngine : public UActorComponent
{
    GENERATED_BODY()

public:
    UTensorLogicEngine();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================
    // CONFIGURATION
    // ========================================

    /** Default inference mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config")
    ETensorLogicInferenceMode DefaultInferenceMode = ETensorLogicInferenceMode::ForwardChaining;

    /** Maximum inference depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxInferenceDepth = 10;

    /** Enable learning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config")
    bool bEnableLearning = true;

    /** Learning rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config", meta = (ClampMin = "0.0001", ClampMax = "1.0"))
    float LearningRate = 0.01f;

    /** Learning algorithm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config")
    ETensorLogicLearningAlgorithm LearningAlgorithm = ETensorLogicLearningAlgorithm::GradientDescent;

    /** Embedding dimension */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config", meta = (ClampMin = "8", ClampMax = "512"))
    int32 EmbeddingDimension = 64;

    /** Enable proof tracing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config")
    bool bEnableProofTracing = true;

    /** Minimum confidence threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorLogic|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinConfidenceThreshold = 0.5f;

    // ========================================
    // KNOWLEDGE BASE
    // ========================================

    /** All rules in the knowledge base */
    UPROPERTY(BlueprintReadOnly, Category = "TensorLogic|KB")
    TMap<FString, FTensorLogicRule> Rules;

    /** All facts (ground atoms) */
    UPROPERTY(BlueprintReadOnly, Category = "TensorLogic|KB")
    TArray<FTensorLogicAtom> Facts;

    /** All predicates */
    UPROPERTY(BlueprintReadOnly, Category = "TensorLogic|KB")
    TMap<FString, FTensorLogicPredicate> Predicates;

    /** Entity embeddings (for continuous reasoning) */
    UPROPERTY(BlueprintReadOnly, Category = "TensorLogic|KB")
    TMap<FString, TArray<float>> EntityEmbeddings;

    /** Relation embeddings (for continuous reasoning) */
    UPROPERTY(BlueprintReadOnly, Category = "TensorLogic|KB")
    TMap<FString, TArray<float>> RelationEmbeddings;

    // ========================================
    // PUBLIC API - KNOWLEDGE BASE
    // ========================================

    /** Add rule to knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    FString AddRule(const FTensorLogicRule& Rule);

    /** Add fact to knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    void AddFact(const FTensorLogicAtom& Fact);

    /** Add predicate definition */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    void AddPredicate(const FTensorLogicPredicate& Predicate);

    /** Remove rule from knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    bool RemoveRule(const FString& RuleID);

    /** Get rule by ID */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    FTensorLogicRule GetRule(const FString& RuleID) const;

    /** Get all rules matching head predicate */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    TArray<FTensorLogicRule> GetRulesByHeadPredicate(const FString& Predicate) const;

    /** Get all facts matching predicate */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    TArray<FTensorLogicAtom> GetFactsByPredicate(const FString& Predicate) const;

    /** Clear knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|KB")
    void ClearKnowledgeBase();

    // ========================================
    // PUBLIC API - INFERENCE
    // ========================================

    /** Query knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Inference")
    FTensorLogicQueryResult Query(const FTensorLogicAtom& Goal, ETensorLogicInferenceMode Mode);

    /** Forward chaining inference */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Inference")
    TArray<FTensorLogicAtom> ForwardChain(int32 MaxIterations = 10);

    /** Backward chaining inference */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Inference")
    FTensorLogicQueryResult BackwardChain(const FTensorLogicAtom& Goal);

    /** Check if atom is entailed by knowledge base */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Inference")
    bool IsEntailed(const FTensorLogicAtom& Atom);

    /** Compute atom confidence score */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Inference")
    float ComputeConfidence(const FTensorLogicAtom& Atom);

    // ========================================
    // PUBLIC API - LEARNING
    // ========================================

    /** Train on example (atom with truth value) */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    float TrainOnExample(const FTensorLogicAtom& Example, float TruthValue);

    /** Invent new predicate via tensor decomposition */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    FTensorLogicPredicate InventPredicate(const FString& Name, int32 Arity);

    /** Learn rule from examples */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    FTensorLogicRule LearnRule(const TArray<FTensorLogicAtom>& PositiveExamples, const TArray<FTensorLogicAtom>& NegativeExamples);

    /** Update embeddings via gradient descent */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    void UpdateEmbeddings(float LearningStep);

    /** Get entity embedding */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    TArray<float> GetEntityEmbedding(const FString& Entity) const;

    /** Set entity embedding */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Learning")
    void SetEntityEmbedding(const FString& Entity, const TArray<float>& Embedding);

    // ========================================
    // PUBLIC API - TENSOR OPERATIONS
    // ========================================

    /** Einstein summation (einsum) operation */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Tensor")
    TArray<float> Einsum(const FString& Equation, const TArray<TArray<float>>& Tensors);

    /** Tensor dot product */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Tensor")
    float TensorDot(const TArray<float>& A, const TArray<float>& B);

    /** Tensor matrix multiplication */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Tensor")
    TArray<float> TensorMatMul(const TArray<float>& A, const TArray<float>& B, int32 M, int32 N, int32 K);

    /** Apply activation function */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Tensor")
    TArray<float> ApplyActivation(const TArray<float>& Input, const FString& ActivationType);

    // ========================================
    // PUBLIC API - UTILITIES
    // ========================================

    /** Get knowledge base statistics */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic")
    FString GetStatistics() const;

    /** Export knowledge base to string */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic")
    FString ExportKnowledgeBase() const;

    /** Import knowledge base from string */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic")
    bool ImportKnowledgeBase(const FString& Data);

    /** Save knowledge base to file */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic")
    bool SaveKnowledgeBase(const FString& FilePath);

    /** Load knowledge base from file */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic")
    bool LoadKnowledgeBase(const FString& FilePath);

protected:
    virtual void BeginPlay() override;

private:
    // ========================================
    // INTERNAL STATE
    // ========================================

    /** Rule ID counter */
    int32 RuleIDCounter = 0;

    /** Inference depth (current) */
    int32 CurrentInferenceDepth = 0;

    /** Proof trace (current query) */
    TArray<FString> CurrentProofTrace;

    /** Is engine initialized? */
    bool bIsInitialized = false;

    // ========================================
    // INTERNAL METHODS - INFERENCE
    // ========================================

    /** Unify two atoms */
    bool Unify(const FTensorLogicAtom& A, const FTensorLogicAtom& B, FTensorLogicSubstitution& OutSubstitution) const;

    /** Apply substitution to atom */
    FTensorLogicAtom ApplySubstitution(const FTensorLogicAtom& Atom, const FTensorLogicSubstitution& Substitution) const;

    /** Apply substitution to term */
    FTensorLogicTerm ApplySubstitutionToTerm(const FTensorLogicTerm& Term, const FTensorLogicSubstitution& Substitution) const;

    /** Find matching rules */
    TArray<FTensorLogicRule> FindMatchingRules(const FTensorLogicAtom& Goal) const;

    /** Prove goal recursively */
    bool ProveGoal(const FTensorLogicAtom& Goal, FTensorLogicSubstitution& Substitution, float& Confidence);

    /** Add to proof trace */
    void AddProofStep(const FString& Step);

    // ========================================
    // INTERNAL METHODS - LEARNING
    // ========================================

    /** Initialize entity embedding */
    void InitializeEntityEmbedding(const FString& Entity);

    /** Initialize relation embedding */
    void InitializeRelationEmbedding(const FString& Relation, int32 Arity);

    /** Compute embedding gradient */
    TArray<float> ComputeEmbeddingGradient(const FTensorLogicAtom& Atom, float TruthValue, float Prediction);

    /** RESCAL score function */
    float RESCALScore(const FString& Entity1, const FString& Relation, const FString& Entity2);

    // ========================================
    // INTERNAL METHODS - TENSOR OPS
    // ========================================

    /** Initialize tensor with random values */
    void InitializeTensor(TArray<float>& Tensor, int32 Size);

    /** Compute tensor norm */
    float ComputeTensorNorm(const TArray<float>& Tensor) const;

    /** Normalize tensor */
    void NormalizeTensor(TArray<float>& Tensor);

    /** Generate unique rule ID */
    FString GenerateRuleID();
};
