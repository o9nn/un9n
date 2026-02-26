#pragma once

#include "CoreMinimal.h"
#include "TensorLogicTerm.h"
#include "TensorLogicRule.generated.h"

/**
 * Tensor Logic Rule
 * Represents a logical rule as a tensor equation
 * Format: Head :- Body1, Body2, ..., BodyN
 * Implements: https://tensor-logic.org/ tensor equation semantics
 */

/**
 * Rule Mode
 */
UENUM(BlueprintType)
enum class ETensorLogicRuleMode : uint8
{
    /** Boolean/deductive reasoning (no hallucination) */
    Boolean UMETA(DisplayName = "Boolean"),
    
    /** Continuous/learnable reasoning (gradient-based) */
    Continuous UMETA(DisplayName = "Continuous"),
    
    /** Hybrid mode (combines both) */
    Hybrid UMETA(DisplayName = "Hybrid")
};

/**
 * Tensor Logic Rule
 * Core data structure for logical rules
 */
USTRUCT(BlueprintType)
struct FTensorLogicRule
{
    GENERATED_BODY()

    /** Rule ID */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString RuleID;

    /** Head atom (conclusion) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FTensorLogicAtom Head;

    /** Body atoms (premises) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FTensorLogicAtom> Body;

    /** Rule mode */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    ETensorLogicRuleMode Mode = ETensorLogicRuleMode::Boolean;

    /** Rule weight (for weighted rules) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    float Weight = 1.0f;

    /** Is this rule learnable? */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    bool bIsLearnable = false;

    /** Tensor parameters (for learnable rules) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TMap<FString, TArray<float>> Parameters;

    /** Confidence score [0, 1] */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    float Confidence = 1.0f;

    /** Application count (how many times this rule has been used) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    int32 ApplicationCount = 0;

    FTensorLogicRule()
        : Mode(ETensorLogicRuleMode::Boolean)
        , Weight(1.0f)
        , bIsLearnable(false)
        , Confidence(1.0f)
        , ApplicationCount(0)
    {
    }

    /** Get string representation */
    FString ToString() const
    {
        FString Result = Head.ToString();
        
        if (Body.Num() > 0)
        {
            Result += TEXT(" :- ");
            for (int32 i = 0; i < Body.Num(); ++i)
            {
                if (i > 0) Result += TEXT(", ");
                Result += Body[i].ToString();
            }
        }
        
        Result += FString::Printf(TEXT(" [w=%.2f, conf=%.2f]"), Weight, Confidence);
        
        return Result;
    }

    /** Check if rule is ground (all atoms are ground) */
    bool IsGround() const
    {
        if (!Head.IsGround())
        {
            return false;
        }
        
        for (const FTensorLogicAtom& Atom : Body)
        {
            if (!Atom.IsGround())
            {
                return false;
            }
        }
        
        return true;
    }

    /** Get all variables in this rule */
    TSet<FString> GetVariables() const
    {
        TSet<FString> Variables;
        
        // Head variables
        for (const FTensorLogicTerm& Arg : Head.Arguments)
        {
            if (Arg.TermType == ETensorLogicTermType::Variable)
            {
                Variables.Add(Arg.Symbol);
            }
        }
        
        // Body variables
        for (const FTensorLogicAtom& Atom : Body)
        {
            for (const FTensorLogicTerm& Arg : Atom.Arguments)
            {
                if (Arg.TermType == ETensorLogicTermType::Variable)
                {
                    Variables.Add(Arg.Symbol);
                }
            }
        }
        
        return Variables;
    }
};

/**
 * Tensor Logic Substitution
 * Maps variables to terms
 */
USTRUCT(BlueprintType)
struct FTensorLogicSubstitution
{
    GENERATED_BODY()

    /** Variable bindings */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TMap<FString, FTensorLogicTerm> Bindings;

    FTensorLogicSubstitution()
    {
    }

    /** Add binding */
    void AddBinding(const FString& Variable, const FTensorLogicTerm& Term)
    {
        Bindings.Add(Variable, Term);
    }

    /** Get binding */
    bool GetBinding(const FString& Variable, FTensorLogicTerm& OutTerm) const
    {
        const FTensorLogicTerm* Found = Bindings.Find(Variable);
        if (Found)
        {
            OutTerm = *Found;
            return true;
        }
        return false;
    }

    /** Check if variable is bound */
    bool IsBound(const FString& Variable) const
    {
        return Bindings.Contains(Variable);
    }

    /** Merge with another substitution */
    bool Merge(const FTensorLogicSubstitution& Other)
    {
        for (const auto& Pair : Other.Bindings)
        {
            if (Bindings.Contains(Pair.Key))
            {
                // Check compatibility
                const FTensorLogicTerm& Existing = Bindings[Pair.Key];
                if (Existing.Symbol != Pair.Value.Symbol)
                {
                    return false; // Conflict
                }
            }
            else
            {
                Bindings.Add(Pair.Key, Pair.Value);
            }
        }
        return true;
    }

    /** Get string representation */
    FString ToString() const
    {
        FString Result = TEXT("{");
        int32 Count = 0;
        for (const auto& Pair : Bindings)
        {
            if (Count > 0) Result += TEXT(", ");
            Result += FString::Printf(TEXT("?%s -> %s"), *Pair.Key, *Pair.Value.ToString());
            Count++;
        }
        Result += TEXT("}");
        return Result;
    }
};

/**
 * Tensor Logic Query Result
 * Represents a query result with substitutions and confidence
 */
USTRUCT(BlueprintType)
struct FTensorLogicQueryResult
{
    GENERATED_BODY()

    /** Was the query successful? */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    bool bSuccess = false;

    /** Substitutions for variables */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FTensorLogicSubstitution> Solutions;

    /** Confidence scores for each solution */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<float> Confidences;

    /** Proof trace (for debugging/auditing) */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FString> ProofTrace;

    FTensorLogicQueryResult()
        : bSuccess(false)
    {
    }

    /** Get string representation */
    FString ToString() const
    {
        if (!bSuccess)
        {
            return TEXT("Query failed");
        }
        
        FString Result = FString::Printf(TEXT("Query succeeded with %d solution(s):\n"), Solutions.Num());
        for (int32 i = 0; i < Solutions.Num(); ++i)
        {
            Result += FString::Printf(TEXT("  [%d] %s (conf=%.2f)\n"), 
                i + 1, *Solutions[i].ToString(), 
                i < Confidences.Num() ? Confidences[i] : 1.0f);
        }
        return Result;
    }
};

/**
 * Einsum Operation
 * Represents an Einstein summation operation for tensor logic
 */
USTRUCT(BlueprintType)
struct FTensorLogicEinsum
{
    GENERATED_BODY()

    /** Einsum equation (e.g., "ij,jk->ik") */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString Equation;

    /** Input tensor names */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    TArray<FString> InputTensors;

    /** Output tensor name */
    UPROPERTY(BlueprintReadWrite, Category = "TensorLogic")
    FString OutputTensor;

    FTensorLogicEinsum()
    {
    }

    /** Parse einsum equation */
    bool ParseEquation(const FString& EqString)
    {
        Equation = EqString;
        
        // Split on "->"
        FString LeftPart, RightPart;
        if (!EqString.Split(TEXT("->"), &LeftPart, &RightPart))
        {
            return false;
        }
        
        // Parse input indices (split on comma)
        TArray<FString> InputSpecs;
        LeftPart.ParseIntoArray(InputSpecs, TEXT(","));
        
        return true;
    }

    /** Get string representation */
    FString ToString() const
    {
        return FString::Printf(TEXT("einsum(%s)"), *Equation);
    }
};
