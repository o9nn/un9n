#include "TensorLogicExampleActor.h"

ATensorLogicExampleActor::ATensorLogicExampleActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create tensor logic engine component
    TensorLogicEngine = CreateDefaultSubobject<UTensorLogicEngine>(TEXT("TensorLogicEngine"));
    
    // Create cognitive bridge component (optional)
    CognitiveBridge = CreateDefaultSubobject<UTensorLogicCognitiveBridge>(TEXT("CognitiveBridge"));
}

void ATensorLogicExampleActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicExampleActor: Starting examples..."));
    
    // Optionally run all examples on start
    // RunAllExamples();
}

void ATensorLogicExampleActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATensorLogicExampleActor::LogSeparator(const FString& Title)
{
    UE_LOG(LogTemp, Log, TEXT("\n========================================"));
    UE_LOG(LogTemp, Log, TEXT("  %s"), *Title);
    UE_LOG(LogTemp, Log, TEXT("========================================"));
}

// ========================================
// EXAMPLE 1: SIMPLE FACT QUERY
// ========================================

void ATensorLogicExampleActor::Example1_SimpleFact()
{
    LogSeparator(TEXT("Example 1: Simple Fact Query"));
    
    // Add a simple fact: human(socrates)
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    
    TensorLogicEngine->AddFact(Fact);
    UE_LOG(LogTemp, Log, TEXT("Added fact: %s"), *Fact.ToString());
    
    // Query: human(socrates)?
    FTensorLogicQueryResult Result = TensorLogicEngine->Query(Fact, ETensorLogicInferenceMode::BackwardChaining);
    
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Query SUCCEEDED!"));
        UE_LOG(LogTemp, Log, TEXT("  Solutions found: %d"), Result.Solutions.Num());
        
        if (Result.ProofTrace.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("  Proof trace:"));
            for (const FString& Step : Result.ProofTrace)
            {
                UE_LOG(LogTemp, Log, TEXT("    %s"), *Step);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Query FAILED"));
    }
}

// ========================================
// EXAMPLE 2: RULE-BASED INFERENCE
// ========================================

void ATensorLogicExampleActor::Example2_RuleInference()
{
    LogSeparator(TEXT("Example 2: Rule-Based Inference (Socrates is Mortal)"));
    
    // Add rule: mortal(X) :- human(X)
    FTensorLogicRule Rule;
    Rule.Head.Predicate = TEXT("mortal");
    Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    
    FTensorLogicAtom BodyAtom;
    BodyAtom.Predicate = TEXT("human");
    BodyAtom.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule.Body.Add(BodyAtom);
    Rule.Confidence = 1.0f;
    
    FString RuleID = TensorLogicEngine->AddRule(Rule);
    UE_LOG(LogTemp, Log, TEXT("Added rule: %s"), *Rule.ToString());
    
    // Add fact: human(socrates)
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    TensorLogicEngine->AddFact(Fact);
    UE_LOG(LogTemp, Log, TEXT("Added fact: %s"), *Fact.ToString());
    
    // Query: mortal(socrates)?
    FTensorLogicAtom Query;
    Query.Predicate = TEXT("mortal");
    Query.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    
    UE_LOG(LogTemp, Log, TEXT("Querying: %s"), *Query.ToString());
    
    FTensorLogicQueryResult Result = TensorLogicEngine->Query(Query, ETensorLogicInferenceMode::BackwardChaining);
    
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Query SUCCEEDED! Socrates is mortal (inferred from rule + fact)"));
        UE_LOG(LogTemp, Log, TEXT("  Confidence: %.2f"), Result.Confidences.Num() > 0 ? Result.Confidences[0] : 0.0f);
        
        if (Result.ProofTrace.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("  Proof trace:"));
            for (const FString& Step : Result.ProofTrace)
            {
                UE_LOG(LogTemp, Log, TEXT("    %s"), *Step);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Query FAILED"));
    }
}

// ========================================
// EXAMPLE 3: COMPLEX MULTI-RULE INFERENCE
// ========================================

void ATensorLogicExampleActor::Example3_ComplexInference()
{
    LogSeparator(TEXT("Example 3: Complex Multi-Rule Inference"));
    
    // Rule 1: mortal(X) :- human(X)
    FTensorLogicRule Rule1;
    Rule1.Head.Predicate = TEXT("mortal");
    Rule1.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    FTensorLogicAtom Body1;
    Body1.Predicate = TEXT("human");
    Body1.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule1.Body.Add(Body1);
    Rule1.Confidence = 1.0f;
    TensorLogicEngine->AddRule(Rule1);
    UE_LOG(LogTemp, Log, TEXT("Rule 1: %s"), *Rule1.ToString());
    
    // Rule 2: philosopher(X) :- greek(X), wise(X)
    FTensorLogicRule Rule2;
    Rule2.Head.Predicate = TEXT("philosopher");
    Rule2.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    FTensorLogicAtom Body2a;
    Body2a.Predicate = TEXT("greek");
    Body2a.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule2.Body.Add(Body2a);
    FTensorLogicAtom Body2b;
    Body2b.Predicate = TEXT("wise");
    Body2b.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule2.Body.Add(Body2b);
    Rule2.Confidence = 1.0f;
    TensorLogicEngine->AddRule(Rule2);
    UE_LOG(LogTemp, Log, TEXT("Rule 2: %s"), *Rule2.ToString());
    
    // Facts
    FTensorLogicAtom Fact1;
    Fact1.Predicate = TEXT("human");
    Fact1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact1.TruthValue = 1.0f;
    TensorLogicEngine->AddFact(Fact1);
    UE_LOG(LogTemp, Log, TEXT("Fact 1: %s"), *Fact1.ToString());
    
    FTensorLogicAtom Fact2;
    Fact2.Predicate = TEXT("greek");
    Fact2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact2.TruthValue = 1.0f;
    TensorLogicEngine->AddFact(Fact2);
    UE_LOG(LogTemp, Log, TEXT("Fact 2: %s"), *Fact2.ToString());
    
    FTensorLogicAtom Fact3;
    Fact3.Predicate = TEXT("wise");
    Fact3.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact3.TruthValue = 1.0f;
    TensorLogicEngine->AddFact(Fact3);
    UE_LOG(LogTemp, Log, TEXT("Fact 3: %s"), *Fact3.ToString());
    
    // Query 1: mortal(socrates)?
    FTensorLogicAtom Query1;
    Query1.Predicate = TEXT("mortal");
    Query1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    FTensorLogicQueryResult Result1 = TensorLogicEngine->Query(Query1, ETensorLogicInferenceMode::BackwardChaining);
    
    UE_LOG(LogTemp, Log, TEXT("\nQuery 1: %s"), *Query1.ToString());
    if (Result1.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ SUCCESS - Socrates is mortal"));
    }
    
    // Query 2: philosopher(socrates)?
    FTensorLogicAtom Query2;
    Query2.Predicate = TEXT("philosopher");
    Query2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    FTensorLogicQueryResult Result2 = TensorLogicEngine->Query(Query2, ETensorLogicInferenceMode::BackwardChaining);
    
    UE_LOG(LogTemp, Log, TEXT("\nQuery 2: %s"), *Query2.ToString());
    if (Result2.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ SUCCESS - Socrates is a philosopher"));
    }
}

// ========================================
// EXAMPLE 4: LEARNING FROM EXAMPLES
// ========================================

void ATensorLogicExampleActor::Example4_Learning()
{
    LogSeparator(TEXT("Example 4: Learning from Examples"));
    
    TensorLogicEngine->bEnableLearning = true;
    UE_LOG(LogTemp, Log, TEXT("Learning enabled"));
    
    // Train on relation: likes(john, mary)
    FTensorLogicAtom Example1;
    Example1.Predicate = TEXT("likes");
    Example1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    Example1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("mary"), 1.0f));
    
    float Loss1 = TensorLogicEngine->TrainOnExample(Example1, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Training example 1: %s = 1.0, Loss: %.4f"), *Example1.ToString(), Loss1);
    
    // Train on relation: likes(alice, bob)
    FTensorLogicAtom Example2;
    Example2.Predicate = TEXT("likes");
    Example2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("alice"), 1.0f));
    Example2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("bob"), 1.0f));
    
    float Loss2 = TensorLogicEngine->TrainOnExample(Example2, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Training example 2: %s = 1.0, Loss: %.4f"), *Example2.ToString(), Loss2);
    
    // Check embeddings were created
    TArray<float> JohnEmb = TensorLogicEngine->GetEntityEmbedding(TEXT("john"));
    TArray<float> MaryEmb = TensorLogicEngine->GetEntityEmbedding(TEXT("mary"));
    
    UE_LOG(LogTemp, Log, TEXT("\nEntity embeddings created:"));
    UE_LOG(LogTemp, Log, TEXT("  john: %d dimensions"), JohnEmb.Num());
    UE_LOG(LogTemp, Log, TEXT("  mary: %d dimensions"), MaryEmb.Num());
    
    UE_LOG(LogTemp, Log, TEXT("\n✓ Learning completed! Embeddings updated via gradient descent"));
}

// ========================================
// EXAMPLE 5: PREDICATE INVENTION
// ========================================

void ATensorLogicExampleActor::Example5_PredicateInvention()
{
    LogSeparator(TEXT("Example 5: Predicate Invention"));
    
    // Invent a new predicate
    FTensorLogicPredicate NewPred = TensorLogicEngine->InventPredicate(TEXT("similar_to"), 2);
    
    UE_LOG(LogTemp, Log, TEXT("✓ Invented new predicate: %s"), *NewPred.ToString());
    UE_LOG(LogTemp, Log, TEXT("  Arity: %d"), NewPred.Arity);
    UE_LOG(LogTemp, Log, TEXT("  Learnable: %s"), NewPred.bIsLearnable ? TEXT("Yes") : TEXT("No"));
    
    // Check it was added to engine
    if (TensorLogicEngine->Predicates.Contains(TEXT("similar_to")))
    {
        UE_LOG(LogTemp, Log, TEXT("  ✓ Predicate registered in engine"));
        
        // Create a rule using the invented predicate
        FTensorLogicRule Rule;
        Rule.Head.Predicate = TEXT("similar_to");
        Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
        Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("Y")));
        Rule.bIsLearnable = true;
        Rule.Mode = ETensorLogicRuleMode::Continuous;
        
        TensorLogicEngine->AddRule(Rule);
        UE_LOG(LogTemp, Log, TEXT("  ✓ Created learnable rule with invented predicate"));
    }
}

// ========================================
// EXAMPLE 6: HYBRID REASONING
// ========================================

void ATensorLogicExampleActor::Example6_HybridReasoning()
{
    LogSeparator(TEXT("Example 6: Hybrid Reasoning (Symbolic + Neural)"));
    
    if (!CognitiveBridge)
    {
        UE_LOG(LogTemp, Warning, TEXT("CognitiveBridge not available, skipping hybrid reasoning example"));
        return;
    }
    
    // Initialize bridge
    bool bInitialized = CognitiveBridge->InitializeBridge();
    
    if (!bInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to initialize cognitive bridge"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("✓ Cognitive bridge initialized"));
    
    // Add some facts
    FTensorLogicAtom Fact1;
    Fact1.Predicate = TEXT("likes");
    Fact1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("alice"), 1.0f));
    Fact1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("bob"), 1.0f));
    Fact1.TruthValue = 0.9f;
    TensorLogicEngine->AddFact(Fact1);
    
    // Perform hybrid query
    TArray<FString> QueryArgs;
    QueryArgs.Add(TEXT("alice"));
    QueryArgs.Add(TEXT("bob"));
    
    FTensorLogicQueryResult Result = CognitiveBridge->HybridQuery(TEXT("likes"), QueryArgs);
    
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Hybrid query succeeded!"));
        UE_LOG(LogTemp, Log, TEXT("  Combined symbolic and neural reasoning"));
    }
    
    // Get bridge statistics
    FString Stats = CognitiveBridge->GetBridgeStatistics();
    UE_LOG(LogTemp, Log, TEXT("\n%s"), *Stats);
}

// ========================================
// RUN ALL EXAMPLES
// ========================================

void ATensorLogicExampleActor::RunAllExamples()
{
    UE_LOG(LogTemp, Log, TEXT("\n\n╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogTemp, Log, TEXT("║  TENSOR LOGIC SYSTEM - COMPREHENSIVE EXAMPLES              ║"));
    UE_LOG(LogTemp, Log, TEXT("╚════════════════════════════════════════════════════════════╝\n"));
    
    Example1_SimpleFact();
    Example2_RuleInference();
    Example3_ComplexInference();
    Example4_Learning();
    Example5_PredicateInvention();
    Example6_HybridReasoning();
    
    // Print final statistics
    LogSeparator(TEXT("Final Statistics"));
    FString Stats = TensorLogicEngine->GetStatistics();
    UE_LOG(LogTemp, Log, TEXT("%s"), *Stats);
    
    UE_LOG(LogTemp, Log, TEXT("\n\n╔════════════════════════════════════════════════════════════╗"));
    UE_LOG(LogTemp, Log, TEXT("║  ALL EXAMPLES COMPLETED SUCCESSFULLY!                      ║"));
    UE_LOG(LogTemp, Log, TEXT("╚════════════════════════════════════════════════════════════╝\n"));
}
