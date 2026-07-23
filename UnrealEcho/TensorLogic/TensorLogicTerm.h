#pragma once

#include "CoreMinimal.h"
#include "TensorLogicTerm.generated.h"

/**
 * Tensor Logic Term
 * Represents a term in tensor logic - can be a constant, variable, or compound term
 * Based on: https://tensor-logic.org/ and Pedro Domingos' "Tensor Logic: The Language of AI"
 */

/**
 * Term Type
 */
UENUM(BlueprintType)
enum class ETensorLogicTermType : uint8
{
    /** Constant value */
    Constant UMETA(DisplayName = "Constant"),
    
    /** Variable to be bound */
    Variable UMETA(DisplayName = "Variable"),
    
    /** Compound term (predicate with arguments) */
    Compound UMETA(DisplayName = "Compound"),
    
    /** Tensor embedding (learned representation) */
    Embedding UMETA(DisplayName = "Embedding")
};

/**
 * Tensor Logic Term
 * Core data structure for tensor logic expressions
 */
USTRUCT(BlueprintType)
struct FTensorLogicTerm
{
    GENERATED_BODY()

    /** Term type */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    ETensorLogicTermType TermType = ETensorLogicTermType::Constant;

    /** Term name/symbol */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString Symbol;

    /** Constant value (for constant terms) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    float ConstantValue = 0.0f;

    /** Tensor embedding (for learned representations) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<float> EmbeddingVector;

    /** Arguments (for compound terms) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FTensorLogicTerm> Arguments;

    /** Tensor dimensions (for tensor terms) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<int32> Dimensions;

    FTensorLogicTerm()
        : TermType(ETensorLogicTermType::Constant)
        , ConstantValue(0.0f)
    {
    }

    /** Create constant term */
    static FTensorLogicTerm MakeConstant(const FString& Symbol, float Value)
    {
        FTensorLogicTerm Term;
        Term.TermType = ETensorLogicTermType::Constant;
        Term.Symbol = Symbol;
        Term.ConstantValue = Value;
        return Term;
    }

    /** Create variable term */
    static FTensorLogicTerm MakeVariable(const FString& Symbol)
    {
        FTensorLogicTerm Term;
        Term.TermType = ETensorLogicTermType::Variable;
        Term.Symbol = Symbol;
        return Term;
    }

    /** Create compound term */
    static FTensorLogicTerm MakeCompound(const FString& Predicate, const TArray<FTensorLogicTerm>& Args)
    {
        FTensorLogicTerm Term;
        Term.TermType = ETensorLogicTermType::Compound;
        Term.Symbol = Predicate;
        Term.Arguments = Args;
        return Term;
    }

    /** Create embedding term */
    static FTensorLogicTerm MakeEmbedding(const FString& Symbol, const TArray<float>& Embedding)
    {
        FTensorLogicTerm Term;
        Term.TermType = ETensorLogicTermType::Embedding;
        Term.Symbol = Symbol;
        Term.EmbeddingVector = Embedding;
        return Term;
    }

    /** Check if term is ground (no variables) */
    bool IsGround() const
    {
        if (TermType == ETensorLogicTermType::Variable)
        {
            return false;
        }
        
        for (const FTensorLogicTerm& Arg : Arguments)
        {
            if (!Arg.IsGround())
            {
                return false;
            }
        }
        
        return true;
    }

    /** Get string representation */
    FString ToString() const
    {
        switch (TermType)
        {
        case ETensorLogicTermType::Constant:
            return FString::Printf(TEXT("%s(%f)"), *Symbol, ConstantValue);
        case ETensorLogicTermType::Variable:
            return FString::Printf(TEXT("?%s"), *Symbol);
        case ETensorLogicTermType::Embedding:
            return FString::Printf(TEXT("Emb[%s](%d)"), *Symbol, EmbeddingVector.Num());
        case ETensorLogicTermType::Compound:
            {
                FString ArgStr;
                for (int32 i = 0; i < Arguments.Num(); ++i)
                {
                    if (i > 0) ArgStr += TEXT(", ");
                    ArgStr += Arguments[i].ToString();
                }
                return FString::Printf(TEXT("%s(%s)"), *Symbol, *ArgStr);
            }
        default:
            return TEXT("Unknown");
        }
    }
};

/**
 * Tensor Logic Predicate
 * Represents a logical predicate with arity and type information
 */
USTRUCT(BlueprintType)
struct FTensorLogicPredicate
{
    GENERATED_BODY()

    /** Predicate name */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString Name;

    /** Arity (number of arguments) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    int32 Arity = 0;

    /** Is this a learnable predicate? */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    bool bIsLearnable = false;

    /** Predicate tensor representation (for learned predicates) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<float> TensorRepresentation;

    /** Tensor shape [d1, d2, ...] where di is embedding dimension for arg i */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<int32> TensorShape;

    FTensorLogicPredicate()
        : Arity(0)
        , bIsLearnable(false)
    {
    }

    /** Get string representation */
    FString ToString() const
    {
        return FString::Printf(TEXT("%s/%d%s"), *Name, Arity, bIsLearnable ? TEXT(" [learnable]") : TEXT(""));
    }
};

/**
 * Tensor Logic Atom
 * Represents an atomic formula (predicate applied to terms)
 */
USTRUCT(BlueprintType)
struct FTensorLogicAtom
{
    GENERATED_BODY()

    /** Predicate name */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString Predicate;

    /** Arguments */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FTensorLogicTerm> Arguments;

    /** Truth value (for ground atoms) - range [0, 1] */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    float TruthValue = 0.0f;

    /** Is this atom negated? */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    bool bIsNegated = false;

    FTensorLogicAtom()
        : TruthValue(0.0f)
        , bIsNegated(false)
    {
    }

    /** Get string representation */
    FString ToString() const
    {
        FString ArgStr;
        for (int32 i = 0; i < Arguments.Num(); ++i)
        {
            if (i > 0) ArgStr += TEXT(", ");
            ArgStr += Arguments[i].ToString();
        }
        
        FString Result = FString::Printf(TEXT("%s(%s)"), *Predicate, *ArgStr);
        if (bIsNegated)
        {
            Result = TEXT("~") + Result;
        }
        
        return Result;
    }

    /** Check if atom is ground */
    bool IsGround() const
    {
        for (const FTensorLogicTerm& Arg : Arguments)
        {
            if (!Arg.IsGround())
            {
                return false;
            }
        }
        return true;
    }
};
