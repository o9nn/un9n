#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TensorLogic/TensorLogicEngine.h"
#include "TensorLogic/TensorLogicCognitiveBridge.h"
#include "TensorLogicExampleActor.generated.h"

/**
 * Example Actor demonstrating Tensor Logic usage
 * Shows how to use the TensorLogic system for neural-symbolic reasoning
 */
UCLASS()
class ATensorLogicExampleActor : public AActor
{
    GENERATED_BODY()
    
public:    
    ATensorLogicExampleActor();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // ========================================
    // COMPONENTS
    // ========================================

    /** Tensor Logic Engine */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TensorLogic")
    UTensorLogicEngine* TensorLogicEngine;

    /** Cognitive Bridge (optional, for integration with existing systems) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TensorLogic")
    UTensorLogicCognitiveBridge* CognitiveBridge;

    // ========================================
    // EXAMPLE FUNCTIONS (Blueprint Callable)
    // ========================================

    /** Example 1: Simple fact query */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example1_SimpleFact();

    /** Example 2: Rule-based inference (Socrates is mortal) */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example2_RuleInference();

    /** Example 3: Complex multi-rule inference */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example3_ComplexInference();

    /** Example 4: Learning from examples */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example4_Learning();

    /** Example 5: Predicate invention */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example5_PredicateInvention();

    /** Example 6: Hybrid reasoning (symbolic + neural) */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void Example6_HybridReasoning();

    /** Run all examples */
    UFUNCTION(BlueprintCallable, Category = "TensorLogic|Examples")
    void RunAllExamples();

private:
    void LogSeparator(const FString& Title);
};
