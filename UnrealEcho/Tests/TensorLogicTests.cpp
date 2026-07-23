#include "CoreMinimal.h"
#include "TensorLogic/TensorLogicEngine.h"
#include "TensorLogic/TensorLogicTerm.h"
#include "TensorLogic/TensorLogicRule.h"
#include "Misc/AutomationTest.h"

/**
 * Tensor Logic Unit Tests
 * Comprehensive testing for all tensor logic functionality
 */

// ========================================
// TERM TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTermConstantTest, "TensorLogic.Term.Constant", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTermConstantTest::RunTest(const FString& Parameters)
{
    // Test constant term creation
    FTensorLogicTerm Term = FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f);
    
    TestEqual(TEXT("Term type is Constant"), Term.TermType, ETensorLogicTermType::Constant);
    TestEqual(TEXT("Symbol is 'john'"), Term.Symbol, FString(TEXT("john")));
    TestEqual(TEXT("ConstantValue is 1.0"), Term.ConstantValue, 1.0f);
    TestTrue(TEXT("Term is ground"), Term.IsGround());
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTermVariableTest, "TensorLogic.Term.Variable", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTermVariableTest::RunTest(const FString& Parameters)
{
    // Test variable term creation
    FTensorLogicTerm Term = FTensorLogicTerm::MakeVariable(TEXT("X"));
    
    TestEqual(TEXT("Term type is Variable"), Term.TermType, ETensorLogicTermType::Variable);
    TestEqual(TEXT("Symbol is 'X'"), Term.Symbol, FString(TEXT("X")));
    TestFalse(TEXT("Term is not ground"), Term.IsGround());
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTermCompoundTest, "TensorLogic.Term.Compound", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTermCompoundTest::RunTest(const FString& Parameters)
{
    // Test compound term creation
    TArray<FTensorLogicTerm> Args;
    Args.Add(FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    Args.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    
    FTensorLogicTerm Term = FTensorLogicTerm::MakeCompound(TEXT("likes"), Args);
    
    TestEqual(TEXT("Term type is Compound"), Term.TermType, ETensorLogicTermType::Compound);
    TestEqual(TEXT("Symbol is 'likes'"), Term.Symbol, FString(TEXT("likes")));
    TestEqual(TEXT("Has 2 arguments"), Term.Arguments.Num(), 2);
    TestFalse(TEXT("Term is not ground (has variable)"), Term.IsGround());
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTermEmbeddingTest, "TensorLogic.Term.Embedding", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTermEmbeddingTest::RunTest(const FString& Parameters)
{
    // Test embedding term creation
    TArray<float> Embedding = {0.1f, 0.2f, 0.3f, 0.4f};
    FTensorLogicTerm Term = FTensorLogicTerm::MakeEmbedding(TEXT("entity1"), Embedding);
    
    TestEqual(TEXT("Term type is Embedding"), Term.TermType, ETensorLogicTermType::Embedding);
    TestEqual(TEXT("Symbol is 'entity1'"), Term.Symbol, FString(TEXT("entity1")));
    TestEqual(TEXT("Embedding size is 4"), Term.EmbeddingVector.Num(), 4);
    TestTrue(TEXT("Term is ground"), Term.IsGround());
    
    return true;
}

// ========================================
// ATOM TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicAtomCreationTest, "TensorLogic.Atom.Creation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicAtomCreationTest::RunTest(const FString& Parameters)
{
    // Test atom creation
    FTensorLogicAtom Atom;
    Atom.Predicate = TEXT("likes");
    Atom.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    Atom.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("mary"), 1.0f));
    Atom.TruthValue = 0.9f;
    
    TestEqual(TEXT("Predicate is 'likes'"), Atom.Predicate, FString(TEXT("likes")));
    TestEqual(TEXT("Has 2 arguments"), Atom.Arguments.Num(), 2);
    TestEqual(TEXT("Truth value is 0.9"), Atom.TruthValue, 0.9f);
    TestTrue(TEXT("Atom is ground"), Atom.IsGround());
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicAtomNegationTest, "TensorLogic.Atom.Negation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicAtomNegationTest::RunTest(const FString& Parameters)
{
    // Test negated atom
    FTensorLogicAtom Atom;
    Atom.Predicate = TEXT("mortal");
    Atom.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Atom.bIsNegated = true;
    
    TestTrue(TEXT("Atom is negated"), Atom.bIsNegated);
    TestTrue(TEXT("ToString contains negation"), Atom.ToString().Contains(TEXT("~")));
    
    return true;
}

// ========================================
// RULE TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicRuleCreationTest, "TensorLogic.Rule.Creation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicRuleCreationTest::RunTest(const FString& Parameters)
{
    // Test rule creation: mortal(X) :- human(X)
    FTensorLogicRule Rule;
    
    // Head: mortal(X)
    Rule.Head.Predicate = TEXT("mortal");
    Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    
    // Body: human(X)
    FTensorLogicAtom BodyAtom;
    BodyAtom.Predicate = TEXT("human");
    BodyAtom.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule.Body.Add(BodyAtom);
    
    Rule.Mode = ETensorLogicRuleMode::Boolean;
    Rule.Confidence = 1.0f;
    
    TestEqual(TEXT("Head predicate is 'mortal'"), Rule.Head.Predicate, FString(TEXT("mortal")));
    TestEqual(TEXT("Body has 1 atom"), Rule.Body.Num(), 1);
    TestFalse(TEXT("Rule is not ground"), Rule.IsGround());
    TestEqual(TEXT("Has 1 variable"), Rule.GetVariables().Num(), 1);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicRuleLearnableTest, "TensorLogic.Rule.Learnable", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicRuleLearnableTest::RunTest(const FString& Parameters)
{
    // Test learnable rule
    FTensorLogicRule Rule;
    Rule.bIsLearnable = true;
    Rule.Mode = ETensorLogicRuleMode::Continuous;
    Rule.Confidence = 0.75f;
    Rule.Weight = 0.5f;
    
    TestTrue(TEXT("Rule is learnable"), Rule.bIsLearnable);
    TestEqual(TEXT("Mode is Continuous"), Rule.Mode, ETensorLogicRuleMode::Continuous);
    
    return true;
}

// ========================================
// SUBSTITUTION TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicSubstitutionTest, "TensorLogic.Substitution.Basic", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicSubstitutionTest::RunTest(const FString& Parameters)
{
    // Test substitution creation and binding
    FTensorLogicSubstitution Sub;
    
    Sub.AddBinding(TEXT("X"), FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    Sub.AddBinding(TEXT("Y"), FTensorLogicTerm::MakeConstant(TEXT("mary"), 1.0f));
    
    TestTrue(TEXT("X is bound"), Sub.IsBound(TEXT("X")));
    TestTrue(TEXT("Y is bound"), Sub.IsBound(TEXT("Y")));
    TestFalse(TEXT("Z is not bound"), Sub.IsBound(TEXT("Z")));
    
    FTensorLogicTerm BoundTerm;
    TestTrue(TEXT("Can retrieve X binding"), Sub.GetBinding(TEXT("X"), BoundTerm));
    TestEqual(TEXT("X bound to 'john'"), BoundTerm.Symbol, FString(TEXT("john")));
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicSubstitutionMergeTest, "TensorLogic.Substitution.Merge", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicSubstitutionMergeTest::RunTest(const FString& Parameters)
{
    // Test substitution merging
    FTensorLogicSubstitution Sub1;
    Sub1.AddBinding(TEXT("X"), FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    
    FTensorLogicSubstitution Sub2;
    Sub2.AddBinding(TEXT("Y"), FTensorLogicTerm::MakeConstant(TEXT("mary"), 1.0f));
    
    TestTrue(TEXT("Merge succeeds"), Sub1.Merge(Sub2));
    TestTrue(TEXT("X still bound"), Sub1.IsBound(TEXT("X")));
    TestTrue(TEXT("Y now bound"), Sub1.IsBound(TEXT("Y")));
    
    return true;
}

// ========================================
// ENGINE TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineInitTest, "TensorLogic.Engine.Init", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineInitTest::RunTest(const FString& Parameters)
{
    // Test engine initialization
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    TestNotNull(TEXT("Engine created"), Engine);
    TestEqual(TEXT("No rules initially"), Engine->Rules.Num(), 0);
    TestEqual(TEXT("No facts initially"), Engine->Facts.Num(), 0);
    TestEqual(TEXT("No predicates initially"), Engine->Predicates.Num(), 0);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineAddFactTest, "TensorLogic.Engine.AddFact", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineAddFactTest::RunTest(const FString& Parameters)
{
    // Test adding facts
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    
    Engine->AddFact(Fact);
    
    TestEqual(TEXT("Has 1 fact"), Engine->Facts.Num(), 1);
    TestEqual(TEXT("Predicate registered"), Engine->Predicates.Num(), 1);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineAddRuleTest, "TensorLogic.Engine.AddRule", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineAddRuleTest::RunTest(const FString& Parameters)
{
    // Test adding rules
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    FTensorLogicRule Rule;
    Rule.Head.Predicate = TEXT("mortal");
    Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    
    FTensorLogicAtom BodyAtom;
    BodyAtom.Predicate = TEXT("human");
    BodyAtom.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule.Body.Add(BodyAtom);
    
    FString RuleID = Engine->AddRule(Rule);
    
    TestEqual(TEXT("Has 1 rule"), Engine->Rules.Num(), 1);
    TestTrue(TEXT("Rule ID not empty"), !RuleID.IsEmpty());
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineSimpleQueryTest, "TensorLogic.Engine.Query.Simple", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineSimpleQueryTest::RunTest(const FString& Parameters)
{
    // Test simple fact query
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // Add fact: human(socrates)
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    Engine->AddFact(Fact);
    
    // Query: human(socrates)?
    FTensorLogicAtom Query = Fact;
    FTensorLogicQueryResult Result = Engine->Query(Query, ETensorLogicInferenceMode::BackwardChaining);
    
    TestTrue(TEXT("Query succeeded"), Result.bSuccess);
    TestEqual(TEXT("Has 1 solution"), Result.Solutions.Num(), 1);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineInferenceTest, "TensorLogic.Engine.Query.Inference", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineInferenceTest::RunTest(const FString& Parameters)
{
    // Test inference with rules
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // Add rule: mortal(X) :- human(X)
    FTensorLogicRule Rule;
    Rule.Head.Predicate = TEXT("mortal");
    Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    
    FTensorLogicAtom BodyAtom;
    BodyAtom.Predicate = TEXT("human");
    BodyAtom.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule.Body.Add(BodyAtom);
    Rule.Confidence = 1.0f;
    
    Engine->AddRule(Rule);
    
    // Add fact: human(socrates)
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    Engine->AddFact(Fact);
    
    // Query: mortal(socrates)?
    FTensorLogicAtom Query;
    Query.Predicate = TEXT("mortal");
    Query.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    
    FTensorLogicQueryResult Result = Engine->Query(Query, ETensorLogicInferenceMode::BackwardChaining);
    
    TestTrue(TEXT("Query succeeded"), Result.bSuccess);
    TestTrue(TEXT("Proof trace generated"), Result.ProofTrace.Num() > 0);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEngineForwardChainingTest, "TensorLogic.Engine.ForwardChaining", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEngineForwardChainingTest::RunTest(const FString& Parameters)
{
    // Test forward chaining
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // Add ground rule: mortal(socrates) :- human(socrates)
    FTensorLogicRule Rule;
    Rule.Head.Predicate = TEXT("mortal");
    Rule.Head.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    
    FTensorLogicAtom BodyAtom;
    BodyAtom.Predicate = TEXT("human");
    BodyAtom.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Rule.Body.Add(BodyAtom);
    Rule.Confidence = 1.0f;
    
    Engine->AddRule(Rule);
    
    // Add fact: human(socrates)
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("human");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact.TruthValue = 1.0f;
    Engine->AddFact(Fact);
    
    // Forward chain
    TArray<FTensorLogicAtom> NewFacts = Engine->ForwardChain(10);
    
    TestTrue(TEXT("New facts derived"), NewFacts.Num() > 0);
    
    return true;
}

// ========================================
// TENSOR OPERATION TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTensorDotTest, "TensorLogic.Tensor.Dot", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTensorDotTest::RunTest(const FString& Parameters)
{
    // Test tensor dot product
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    TArray<float> A = {1.0f, 2.0f, 3.0f};
    TArray<float> B = {4.0f, 5.0f, 6.0f};
    
    float Result = Engine->TensorDot(A, B);
    float Expected = 1*4 + 2*5 + 3*6; // = 32
    
    TestEqual(TEXT("Dot product correct"), Result, Expected);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTensorMatMulTest, "TensorLogic.Tensor.MatMul", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTensorMatMulTest::RunTest(const FString& Parameters)
{
    // Test tensor matrix multiplication
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // 2x2 * 2x2 = 2x2
    TArray<float> A = {1.0f, 2.0f, 3.0f, 4.0f};
    TArray<float> B = {5.0f, 6.0f, 7.0f, 8.0f};
    
    TArray<float> Result = Engine->TensorMatMul(A, B, 2, 2, 2);
    
    TestEqual(TEXT("Result size correct"), Result.Num(), 4);
    TestEqual(TEXT("Result[0] correct"), Result[0], 1*5 + 2*7); // = 19
    TestEqual(TEXT("Result[1] correct"), Result[1], 1*6 + 2*8); // = 22
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicActivationTest, "TensorLogic.Tensor.Activation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicActivationTest::RunTest(const FString& Parameters)
{
    // Test activation functions
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    TArray<float> Input = {-1.0f, 0.0f, 1.0f};
    
    // ReLU
    TArray<float> ReLU = Engine->ApplyActivation(Input, TEXT("relu"));
    TestEqual(TEXT("ReLU(-1) = 0"), ReLU[0], 0.0f);
    TestEqual(TEXT("ReLU(0) = 0"), ReLU[1], 0.0f);
    TestEqual(TEXT("ReLU(1) = 1"), ReLU[2], 1.0f);
    
    // Sigmoid
    TArray<float> Sigmoid = Engine->ApplyActivation(Input, TEXT("sigmoid"));
    TestTrue(TEXT("Sigmoid(0) ~ 0.5"), FMath::IsNearlyEqual(Sigmoid[1], 0.5f, 0.01f));
    
    return true;
}

// ========================================
// LEARNING TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicEmbeddingTest, "TensorLogic.Learning.Embedding", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicEmbeddingTest::RunTest(const FString& Parameters)
{
    // Test entity embedding management
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    TArray<float> Embedding = {0.1f, 0.2f, 0.3f, 0.4f};
    Engine->SetEntityEmbedding(TEXT("entity1"), Embedding);
    
    TArray<float> Retrieved = Engine->GetEntityEmbedding(TEXT("entity1"));
    
    TestEqual(TEXT("Embedding size correct"), Retrieved.Num(), 4);
    TestEqual(TEXT("Embedding value correct"), Retrieved[0], 0.1f);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicPredicateInventionTest, "TensorLogic.Learning.PredicateInvention", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicPredicateInventionTest::RunTest(const FString& Parameters)
{
    // Test predicate invention
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    FTensorLogicPredicate NewPred = Engine->InventPredicate(TEXT("invented_relation"), 2);
    
    TestEqual(TEXT("Predicate name correct"), NewPred.Name, FString(TEXT("invented_relation")));
    TestEqual(TEXT("Predicate arity correct"), NewPred.Arity, 2);
    TestTrue(TEXT("Predicate is learnable"), NewPred.bIsLearnable);
    TestTrue(TEXT("Predicate registered"), Engine->Predicates.Contains(TEXT("invented_relation")));
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicTrainingTest, "TensorLogic.Learning.Training", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicTrainingTest::RunTest(const FString& Parameters)
{
    // Test training on examples
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    Engine->bEnableLearning = true;
    
    FTensorLogicAtom Example;
    Example.Predicate = TEXT("likes");
    Example.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("john"), 1.0f));
    Example.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("mary"), 1.0f));
    
    float Loss = Engine->TrainOnExample(Example, 1.0f);
    
    TestTrue(TEXT("Loss computed"), Loss >= 0.0f);
    
    return true;
}

// ========================================
// INTEGRATION TESTS
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicComplexInferenceTest, "TensorLogic.Integration.ComplexInference", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicComplexInferenceTest::RunTest(const FString& Parameters)
{
    // Test complex inference chain
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // Rule 1: mortal(X) :- human(X)
    FTensorLogicRule Rule1;
    Rule1.Head.Predicate = TEXT("mortal");
    Rule1.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    FTensorLogicAtom Body1;
    Body1.Predicate = TEXT("human");
    Body1.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
    Rule1.Body.Add(Body1);
    Rule1.Confidence = 1.0f;
    Engine->AddRule(Rule1);
    
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
    Engine->AddRule(Rule2);
    
    // Facts
    FTensorLogicAtom Fact1;
    Fact1.Predicate = TEXT("human");
    Fact1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact1.TruthValue = 1.0f;
    Engine->AddFact(Fact1);
    
    FTensorLogicAtom Fact2;
    Fact2.Predicate = TEXT("greek");
    Fact2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact2.TruthValue = 1.0f;
    Engine->AddFact(Fact2);
    
    FTensorLogicAtom Fact3;
    Fact3.Predicate = TEXT("wise");
    Fact3.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    Fact3.TruthValue = 1.0f;
    Engine->AddFact(Fact3);
    
    // Query: mortal(socrates)?
    FTensorLogicAtom Query1;
    Query1.Predicate = TEXT("mortal");
    Query1.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    FTensorLogicQueryResult Result1 = Engine->Query(Query1, ETensorLogicInferenceMode::BackwardChaining);
    TestTrue(TEXT("Mortal query succeeded"), Result1.bSuccess);
    
    // Query: philosopher(socrates)?
    FTensorLogicAtom Query2;
    Query2.Predicate = TEXT("philosopher");
    Query2.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
    FTensorLogicQueryResult Result2 = Engine->Query(Query2, ETensorLogicInferenceMode::BackwardChaining);
    TestTrue(TEXT("Philosopher query succeeded"), Result2.bSuccess);
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicExportImportTest, "TensorLogic.Integration.ExportImport", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicExportImportTest::RunTest(const FString& Parameters)
{
    // Test knowledge base export/import
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    // Add some data
    FTensorLogicAtom Fact;
    Fact.Predicate = TEXT("test");
    Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("data"), 1.0f));
    Fact.TruthValue = 1.0f;
    Engine->AddFact(Fact);
    
    FString Exported = Engine->ExportKnowledgeBase();
    
    TestTrue(TEXT("Export not empty"), !Exported.IsEmpty());
    TestTrue(TEXT("Export contains fact"), Exported.Contains(TEXT("test")));
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorLogicStatisticsTest, "TensorLogic.Integration.Statistics", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTensorLogicStatisticsTest::RunTest(const FString& Parameters)
{
    // Test statistics generation
    UTensorLogicEngine* Engine = NewObject<UTensorLogicEngine>();
    
    FString Stats = Engine->GetStatistics();
    
    TestTrue(TEXT("Statistics not empty"), !Stats.IsEmpty());
    TestTrue(TEXT("Statistics contains 'Rules'"), Stats.Contains(TEXT("Rules")));
    TestTrue(TEXT("Statistics contains 'Facts'"), Stats.Contains(TEXT("Facts")));
    
    return true;
}
