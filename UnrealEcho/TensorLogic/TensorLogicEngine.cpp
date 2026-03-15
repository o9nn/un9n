#include "TensorLogicEngine.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

UTensorLogicEngine::UTensorLogicEngine()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz
}

void UTensorLogicEngine::BeginPlay()
{
    Super::BeginPlay();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Initialized"));
}

void UTensorLogicEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic updates if needed
}

// ========================================
// KNOWLEDGE BASE OPERATIONS
// ========================================

FString UTensorLogicEngine::AddRule(const FTensorLogicRule& Rule)
{
    FString RuleID = Rule.RuleID.IsEmpty() ? GenerateRuleID() : Rule.RuleID;
    
    FTensorLogicRule NewRule = Rule;
    NewRule.RuleID = RuleID;
    
    Rules.Add(RuleID, NewRule);
    
    // Register predicate if not exists
    if (!Predicates.Contains(NewRule.Head.Predicate))
    {
        FTensorLogicPredicate Pred;
        Pred.Name = NewRule.Head.Predicate;
        Pred.Arity = NewRule.Head.Arguments.Num();
        Pred.bIsLearnable = NewRule.bIsLearnable;
        AddPredicate(Pred);
    }
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Added rule %s: %s"), *RuleID, *NewRule.ToString());
    
    return RuleID;
}

void UTensorLogicEngine::AddFact(const FTensorLogicAtom& Fact)
{
    if (!Fact.IsGround())
    {
        UE_LOG(LogTemp, Warning, TEXT("TensorLogicEngine: Cannot add non-ground fact: %s"), *Fact.ToString());
        return;
    }
    
    Facts.Add(Fact);
    
    // Register predicate if not exists
    if (!Predicates.Contains(Fact.Predicate))
    {
        FTensorLogicPredicate Pred;
        Pred.Name = Fact.Predicate;
        Pred.Arity = Fact.Arguments.Num();
        AddPredicate(Pred);
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("TensorLogicEngine: Added fact: %s"), *Fact.ToString());
}

void UTensorLogicEngine::AddPredicate(const FTensorLogicPredicate& Predicate)
{
    Predicates.Add(Predicate.Name, Predicate);
    
    // Initialize relation embedding if learnable
    if (Predicate.bIsLearnable)
    {
        InitializeRelationEmbedding(Predicate.Name, Predicate.Arity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Added predicate: %s"), *Predicate.ToString());
}

bool UTensorLogicEngine::RemoveRule(const FString& RuleID)
{
    return Rules.Remove(RuleID) > 0;
}

FTensorLogicRule UTensorLogicEngine::GetRule(const FString& RuleID) const
{
    const FTensorLogicRule* Found = Rules.Find(RuleID);
    return Found ? *Found : FTensorLogicRule();
}

TArray<FTensorLogicRule> UTensorLogicEngine::GetRulesByHeadPredicate(const FString& Predicate) const
{
    TArray<FTensorLogicRule> Result;
    
    for (const auto& Pair : Rules)
    {
        if (Pair.Value.Head.Predicate == Predicate)
        {
            Result.Add(Pair.Value);
        }
    }
    
    return Result;
}

TArray<FTensorLogicAtom> UTensorLogicEngine::GetFactsByPredicate(const FString& Predicate) const
{
    TArray<FTensorLogicAtom> Result;
    
    for (const FTensorLogicAtom& Fact : Facts)
    {
        if (Fact.Predicate == Predicate)
        {
            Result.Add(Fact);
        }
    }
    
    return Result;
}

void UTensorLogicEngine::ClearKnowledgeBase()
{
    Rules.Empty();
    Facts.Empty();
    Predicates.Empty();
    EntityEmbeddings.Empty();
    RelationEmbeddings.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Knowledge base cleared"));
}

// ========================================
// INFERENCE OPERATIONS
// ========================================

FTensorLogicQueryResult UTensorLogicEngine::Query(const FTensorLogicAtom& Goal, ETensorLogicInferenceMode Mode)
{
    FTensorLogicQueryResult Result;
    CurrentInferenceDepth = 0;
    CurrentProofTrace.Empty();
    
    if (bEnableProofTracing)
    {
        AddProofStep(FString::Printf(TEXT("Query: %s"), *Goal.ToString()));
    }
    
    if (Mode == ETensorLogicInferenceMode::BackwardChaining || Mode == ETensorLogicInferenceMode::Hybrid)
    {
        Result = BackwardChain(Goal);
    }
    else
    {
        // Forward chaining for simple queries
        for (const FTensorLogicAtom& Fact : Facts)
        {
            FTensorLogicSubstitution Sub;
            if (Unify(Goal, Fact, Sub))
            {
                Result.bSuccess = true;
                Result.Solutions.Add(Sub);
                Result.Confidences.Add(Fact.TruthValue);
            }
        }
    }
    
    Result.ProofTrace = CurrentProofTrace;
    
    return Result;
}

FTensorLogicQueryResult UTensorLogicEngine::BackwardChain(const FTensorLogicAtom& Goal)
{
    FTensorLogicQueryResult Result;
    FTensorLogicSubstitution Substitution;
    float Confidence = 0.0f;
    
    if (ProveGoal(Goal, Substitution, Confidence))
    {
        Result.bSuccess = true;
        Result.Solutions.Add(Substitution);
        Result.Confidences.Add(Confidence);
    }
    
    return Result;
}

bool UTensorLogicEngine::ProveGoal(const FTensorLogicAtom& Goal, FTensorLogicSubstitution& Substitution, float& Confidence)
{
    if (CurrentInferenceDepth >= MaxInferenceDepth)
    {
        UE_LOG(LogTemp, Warning, TEXT("TensorLogicEngine: Max inference depth reached"));
        return false;
    }
    
    CurrentInferenceDepth++;
    
    if (bEnableProofTracing)
    {
        AddProofStep(FString::Printf(TEXT("  [%d] Proving: %s"), CurrentInferenceDepth, *Goal.ToString()));
    }
    
    // Check facts first
    for (const FTensorLogicAtom& Fact : Facts)
    {
        FTensorLogicSubstitution TempSub;
        if (Unify(Goal, Fact, TempSub))
        {
            Substitution = TempSub;
            Confidence = Fact.TruthValue;
            
            if (bEnableProofTracing)
            {
                AddProofStep(FString::Printf(TEXT("  [%d] Matched fact: %s"), CurrentInferenceDepth, *Fact.ToString()));
            }
            
            CurrentInferenceDepth--;
            return true;
        }
    }
    
    // Try rules
    TArray<FTensorLogicRule> MatchingRules = GetRulesByHeadPredicate(Goal.Predicate);
    
    for (const FTensorLogicRule& Rule : MatchingRules)
    {
        FTensorLogicSubstitution HeadSub;
        if (!Unify(Goal, Rule.Head, HeadSub))
        {
            continue;
        }
        
        if (bEnableProofTracing)
        {
            AddProofStep(FString::Printf(TEXT("  [%d] Trying rule: %s"), CurrentInferenceDepth, *Rule.ToString()));
        }
        
        // Try to prove all body atoms
        bool AllProved = true;
        float BodyConfidence = 1.0f;
        FTensorLogicSubstitution BodySub = HeadSub;
        
        for (const FTensorLogicAtom& BodyAtom : Rule.Body)
        {
            FTensorLogicAtom InstantiatedBody = ApplySubstitution(BodyAtom, BodySub);
            FTensorLogicSubstitution TempSub;
            float TempConf = 0.0f;
            
            if (!ProveGoal(InstantiatedBody, TempSub, TempConf))
            {
                AllProved = false;
                break;
            }
            
            // Merge substitutions
            if (!BodySub.Merge(TempSub))
            {
                AllProved = false;
                break;
            }
            
            // Multiply confidences (probabilistic conjunction)
            BodyConfidence *= TempConf;
        }
        
        if (AllProved)
        {
            Substitution = BodySub;
            Confidence = BodyConfidence * Rule.Confidence;
            
            if (bEnableProofTracing)
            {
                AddProofStep(FString::Printf(TEXT("  [%d] Proved with confidence %.2f"), CurrentInferenceDepth, Confidence));
            }
            
            CurrentInferenceDepth--;
            return true;
        }
    }
    
    CurrentInferenceDepth--;
    return false;
}

TArray<FTensorLogicAtom> UTensorLogicEngine::ForwardChain(int32 MaxIterations)
{
    TArray<FTensorLogicAtom> NewFacts;
    
    for (int32 Iteration = 0; Iteration < MaxIterations; ++Iteration)
    {
        TArray<FTensorLogicAtom> IterationFacts;
        
        for (const auto& Pair : Rules)
        {
            const FTensorLogicRule& Rule = Pair.Value;
            
            // Try to instantiate rule with current facts
            // This is a simplified version - full implementation would need more sophisticated matching
            bool CanFire = true;
            
            for (const FTensorLogicAtom& BodyAtom : Rule.Body)
            {
                bool BodySatisfied = false;
                
                for (const FTensorLogicAtom& Fact : Facts)
                {
                    FTensorLogicSubstitution Sub;
                    if (Unify(BodyAtom, Fact, Sub))
                    {
                        BodySatisfied = true;
                        break;
                    }
                }
                
                if (!BodySatisfied)
                {
                    CanFire = false;
                    break;
                }
            }
            
            if (CanFire && Rule.Head.IsGround())
            {
                // Check if we already have this fact
                bool AlreadyExists = false;
                for (const FTensorLogicAtom& Fact : Facts)
                {
                    if (Fact.Predicate == Rule.Head.Predicate && Fact.Arguments.Num() == Rule.Head.Arguments.Num())
                    {
                        // Simple equality check
                        AlreadyExists = true;
                        break;
                    }
                }
                
                if (!AlreadyExists)
                {
                    FTensorLogicAtom NewFact = Rule.Head;
                    NewFact.TruthValue = Rule.Confidence;
                    IterationFacts.Add(NewFact);
                }
            }
        }
        
        if (IterationFacts.Num() == 0)
        {
            break; // Fixed point reached
        }
        
        // Add new facts
        for (const FTensorLogicAtom& NewFact : IterationFacts)
        {
            Facts.Add(NewFact);
            NewFacts.Add(NewFact);
        }
    }
    
    return NewFacts;
}

bool UTensorLogicEngine::IsEntailed(const FTensorLogicAtom& Atom)
{
    FTensorLogicQueryResult Result = Query(Atom, DefaultInferenceMode);
    return Result.bSuccess;
}

float UTensorLogicEngine::ComputeConfidence(const FTensorLogicAtom& Atom)
{
    FTensorLogicQueryResult Result = Query(Atom, DefaultInferenceMode);
    
    if (Result.bSuccess && Result.Confidences.Num() > 0)
    {
        return Result.Confidences[0];
    }
    
    return 0.0f;
}

// ========================================
// UNIFICATION
// ========================================

bool UTensorLogicEngine::Unify(const FTensorLogicAtom& A, const FTensorLogicAtom& B, FTensorLogicSubstitution& OutSubstitution) const
{
    if (A.Predicate != B.Predicate)
    {
        return false;
    }
    
    if (A.Arguments.Num() != B.Arguments.Num())
    {
        return false;
    }
    
    FTensorLogicSubstitution Sub;
    
    for (int32 i = 0; i < A.Arguments.Num(); ++i)
    {
        const FTensorLogicTerm& TermA = A.Arguments[i];
        const FTensorLogicTerm& TermB = B.Arguments[i];
        
        if (TermA.TermType == ETensorLogicTermType::Variable)
        {
            Sub.AddBinding(TermA.Symbol, TermB);
        }
        else if (TermB.TermType == ETensorLogicTermType::Variable)
        {
            Sub.AddBinding(TermB.Symbol, TermA);
        }
        else if (TermA.TermType == ETensorLogicTermType::Constant && TermB.TermType == ETensorLogicTermType::Constant)
        {
            if (TermA.Symbol != TermB.Symbol)
            {
                return false;
            }
        }
        else
        {
            // More complex unification needed for compound terms
            return false;
        }
    }
    
    OutSubstitution = Sub;
    return true;
}

FTensorLogicAtom UTensorLogicEngine::ApplySubstitution(const FTensorLogicAtom& Atom, const FTensorLogicSubstitution& Substitution) const
{
    FTensorLogicAtom Result = Atom;
    
    for (int32 i = 0; i < Result.Arguments.Num(); ++i)
    {
        Result.Arguments[i] = ApplySubstitutionToTerm(Result.Arguments[i], Substitution);
    }
    
    return Result;
}

FTensorLogicTerm UTensorLogicEngine::ApplySubstitutionToTerm(const FTensorLogicTerm& Term, const FTensorLogicSubstitution& Substitution) const
{
    if (Term.TermType == ETensorLogicTermType::Variable)
    {
        FTensorLogicTerm Bound;
        if (Substitution.GetBinding(Term.Symbol, Bound))
        {
            return Bound;
        }
    }
    
    return Term;
}

TArray<FTensorLogicRule> UTensorLogicEngine::FindMatchingRules(const FTensorLogicAtom& Goal) const
{
    return GetRulesByHeadPredicate(Goal.Predicate);
}

void UTensorLogicEngine::AddProofStep(const FString& Step)
{
    if (bEnableProofTracing)
    {
        CurrentProofTrace.Add(Step);
    }
}

// ========================================
// LEARNING OPERATIONS
// ========================================

float UTensorLogicEngine::TrainOnExample(const FTensorLogicAtom& Example, float TruthValue)
{
    if (!bEnableLearning)
    {
        return 0.0f;
    }
    
    // Compute current prediction
    float Prediction = ComputeConfidence(Example);
    
    // Compute loss (MSE)
    float Loss = FMath::Square(TruthValue - Prediction);
    
    // Compute and apply gradients
    if (Example.Arguments.Num() == 2 && Example.Arguments[0].TermType == ETensorLogicTermType::Constant && Example.Arguments[1].TermType == ETensorLogicTermType::Constant)
    {
        // Binary relation - use RESCAL-style learning
        TArray<float> Gradient = ComputeEmbeddingGradient(Example, TruthValue, Prediction);
        
        // Update embeddings
        FString Entity1 = Example.Arguments[0].Symbol;
        FString Entity2 = Example.Arguments[1].Symbol;
        
        TArray<float>& Emb1 = EntityEmbeddings.FindOrAdd(Entity1);
        TArray<float>& Emb2 = EntityEmbeddings.FindOrAdd(Entity2);
        
        if (Emb1.Num() == 0) InitializeEntityEmbedding(Entity1);
        if (Emb2.Num() == 0) InitializeEntityEmbedding(Entity2);
        
        // Simple gradient descent step
        float LearningStep = LearningRate * (TruthValue - Prediction);
        
        for (int32 i = 0; i < FMath::Min(Emb1.Num(), Emb2.Num()); ++i)
        {
            Emb1[i] += LearningStep * Emb2[i];
            Emb2[i] += LearningStep * Emb1[i];
        }
    }
    
    return Loss;
}

FTensorLogicPredicate UTensorLogicEngine::InventPredicate(const FString& Name, int32 Arity)
{
    FTensorLogicPredicate NewPred;
    NewPred.Name = Name;
    NewPred.Arity = Arity;
    NewPred.bIsLearnable = true;
    
    // Initialize with random tensor
    InitializeRelationEmbedding(Name, Arity);
    
    AddPredicate(NewPred);
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Invented predicate: %s"), *NewPred.ToString());
    
    return NewPred;
}

FTensorLogicRule UTensorLogicEngine::LearnRule(const TArray<FTensorLogicAtom>& PositiveExamples, const TArray<FTensorLogicAtom>& NegativeExamples)
{
    // Simplified rule learning - just create a rule template
    FTensorLogicRule NewRule;
    
    if (PositiveExamples.Num() > 0)
    {
        NewRule.Head = PositiveExamples[0];
        NewRule.bIsLearnable = true;
        NewRule.Confidence = 0.5f; // Start with medium confidence
        
        // Learn confidence from examples
        int32 TotalExamples = PositiveExamples.Num() + NegativeExamples.Num();
        if (TotalExamples > 0)
        {
            NewRule.Confidence = static_cast<float>(PositiveExamples.Num()) / TotalExamples;
        }
    }
    
    return NewRule;
}

void UTensorLogicEngine::UpdateEmbeddings(float LearningStep)
{
    // Apply accumulated gradients
    for (auto& Pair : EntityEmbeddings)
    {
        NormalizeTensor(Pair.Value);
    }
    
    for (auto& Pair : RelationEmbeddings)
    {
        NormalizeTensor(Pair.Value);
    }
}

TArray<float> UTensorLogicEngine::GetEntityEmbedding(const FString& Entity) const
{
    const TArray<float>* Found = EntityEmbeddings.Find(Entity);
    return Found ? *Found : TArray<float>();
}

void UTensorLogicEngine::SetEntityEmbedding(const FString& Entity, const TArray<float>& Embedding)
{
    EntityEmbeddings.Add(Entity, Embedding);
}

void UTensorLogicEngine::InitializeEntityEmbedding(const FString& Entity)
{
    TArray<float> Embedding;
    InitializeTensor(Embedding, EmbeddingDimension);
    EntityEmbeddings.Add(Entity, Embedding);
}

void UTensorLogicEngine::InitializeRelationEmbedding(const FString& Relation, int32 Arity)
{
    TArray<float> Embedding;
    // Compute tensor size using loop to avoid FMath::Pow with integers
    int32 TensorSize = 1;
    for (int32 i = 0; i < Arity; ++i)
    {
        TensorSize *= EmbeddingDimension;
    }
    InitializeTensor(Embedding, TensorSize);
    RelationEmbeddings.Add(Relation, Embedding);
}

TArray<float> UTensorLogicEngine::ComputeEmbeddingGradient(const FTensorLogicAtom& Atom, float TruthValue, float Prediction)
{
    TArray<float> Gradient;
    
    // Simplified gradient computation
    float Error = TruthValue - Prediction;
    
    for (int32 i = 0; i < EmbeddingDimension; ++i)
    {
        Gradient.Add(Error * 0.01f); // Simplified
    }
    
    return Gradient;
}

float UTensorLogicEngine::RESCALScore(const FString& Entity1, const FString& Relation, const FString& Entity2)
{
    const TArray<float>* Emb1 = EntityEmbeddings.Find(Entity1);
    const TArray<float>* Emb2 = EntityEmbeddings.Find(Entity2);
    const TArray<float>* RelEmb = RelationEmbeddings.Find(Relation);
    
    if (!Emb1 || !Emb2 || !RelEmb)
    {
        return 0.0f;
    }
    
    // RESCAL: score = e1^T * M_r * e2
    // Simplified: just dot product for now
    return TensorDot(*Emb1, *Emb2);
}

// ========================================
// TENSOR OPERATIONS
// ========================================

TArray<float> UTensorLogicEngine::Einsum(const FString& Equation, const TArray<TArray<float>>& Tensors)
{
    // Einsum tensor contraction — supports common patterns:
    //   "ij,jk->ik" (matmul), "ij->i" (row sum), "ij->ji" (transpose),
    //   "i,i->" (dot), "ij,ij->" (Frobenius), "i,j->ij" (outer product)

    TArray<float> Result;
    if (Tensors.Num() == 0) return Result;

    // Parse equation: split on "->"
    FString InputSpec, OutputSpec;
    if (!Equation.Split(TEXT("->"), &InputSpec, &OutputSpec))
    {
        InputSpec = Equation;
        OutputSpec = TEXT("");
    }

    TArray<FString> InputSpecs;
    InputSpec.ParseIntoArray(InputSpecs, TEXT(","), true);
    if (InputSpecs.Num() != Tensors.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("TensorLogicEngine::Einsum: spec/tensor count mismatch in '%s'"), *Equation);
        return Result;
    }

    // --- Single-tensor operations ---
    if (Tensors.Num() == 1)
    {
        const TArray<float>& A = Tensors[0];
        if (OutputSpec.IsEmpty())
        {
            float Sum = 0.0f;
            for (float V : A) Sum += V;
            Result.Add(Sum);
            return Result;
        }
        if (InputSpecs[0] == TEXT("ij") && OutputSpec == TEXT("i"))
        {
            int32 N = FMath::CeilToInt(FMath::Sqrt((float)A.Num()));
            Result.SetNum(N);
            for (int32 i = 0; i < N; ++i)
            {
                float S = 0.0f;
                for (int32 j = 0; j < N && i * N + j < A.Num(); ++j)
                    S += A[i * N + j];
                Result[i] = S;
            }
            return Result;
        }
        if (InputSpecs[0] == TEXT("ij") && OutputSpec == TEXT("ji"))
        {
            int32 N = FMath::CeilToInt(FMath::Sqrt((float)A.Num()));
            Result.SetNum(A.Num());
            for (int32 i = 0; i < N; ++i)
                for (int32 j = 0; j < N && i * N + j < A.Num(); ++j)
                    Result[j * N + i] = A[i * N + j];
            return Result;
        }
        Result = A;
        return Result;
    }

    // --- Two-tensor operations ---
    if (Tensors.Num() == 2)
    {
        const TArray<float>& A = Tensors[0];
        const TArray<float>& B = Tensors[1];

        // Dot product: "i,i->"
        if (InputSpecs[0] == TEXT("i") && InputSpecs[1] == TEXT("i"))
        {
            float Dot = 0.0f;
            for (int32 i = 0; i < FMath::Min(A.Num(), B.Num()); ++i)
                Dot += A[i] * B[i];
            Result.Add(Dot);
            return Result;
        }

        // Matrix multiply: "ij,jk->ik"
        if (InputSpecs[0] == TEXT("ij") && InputSpecs[1] == TEXT("jk") && OutputSpec == TEXT("ik"))
        {
            int32 M = FMath::CeilToInt(FMath::Sqrt((float)A.Num()));
            int32 N = M;
            int32 K = B.Num() > 0 ? B.Num() / N : 0;
            Result.SetNum(M * K);
            for (int32 i = 0; i < M; ++i)
                for (int32 k = 0; k < K; ++k)
                {
                    float S = 0.0f;
                    for (int32 j = 0; j < N; ++j)
                    {
                        int32 AI = i * N + j, BI = j * K + k;
                        if (AI < A.Num() && BI < B.Num()) S += A[AI] * B[BI];
                    }
                    Result[i * K + k] = S;
                }
            return Result;
        }

        // Element-wise / Frobenius: "ij,ij->"
        if (InputSpecs[0] == TEXT("ij") && InputSpecs[1] == TEXT("ij"))
        {
            if (OutputSpec.IsEmpty())
            {
                float S = 0.0f;
                for (int32 i = 0; i < FMath::Min(A.Num(), B.Num()); ++i) S += A[i] * B[i];
                Result.Add(S);
                return Result;
            }
            Result.SetNum(FMath::Min(A.Num(), B.Num()));
            for (int32 i = 0; i < Result.Num(); ++i) Result[i] = A[i] * B[i];
            return Result;
        }

        // Outer product: "i,j->ij"
        if (InputSpecs[0] == TEXT("i") && InputSpecs[1] == TEXT("j") && OutputSpec == TEXT("ij"))
        {
            Result.SetNum(A.Num() * B.Num());
            for (int32 i = 0; i < A.Num(); ++i)
                for (int32 j = 0; j < B.Num(); ++j)
                    Result[i * B.Num() + j] = A[i] * B[j];
            return Result;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("TensorLogicEngine::Einsum: unsupported pattern '%s'"), *Equation);
    return Result;
}

float UTensorLogicEngine::TensorDot(const TArray<float>& A, const TArray<float>& B)
{
    if (A.Num() != B.Num())
    {
        return 0.0f;
    }
    
    float Result = 0.0f;
    for (int32 i = 0; i < A.Num(); ++i)
    {
        Result += A[i] * B[i];
    }
    
    return Result;
}

TArray<float> UTensorLogicEngine::TensorMatMul(const TArray<float>& A, const TArray<float>& B, int32 M, int32 N, int32 K)
{
    TArray<float> Result;
    Result.SetNum(M * K);
    
    for (int32 i = 0; i < M; ++i)
    {
        for (int32 j = 0; j < K; ++j)
        {
            float Sum = 0.0f;
            for (int32 k = 0; k < N; ++k)
            {
                Sum += A[i * N + k] * B[k * K + j];
            }
            Result[i * K + j] = Sum;
        }
    }
    
    return Result;
}

TArray<float> UTensorLogicEngine::ApplyActivation(const TArray<float>& Input, const FString& ActivationType)
{
    TArray<float> Result = Input;
    
    if (ActivationType == TEXT("sigmoid"))
    {
        for (float& Val : Result)
        {
            Val = 1.0f / (1.0f + FMath::Exp(-Val));
        }
    }
    else if (ActivationType == TEXT("tanh"))
    {
        for (float& Val : Result)
        {
            Val = FMath::Tanh(Val);
        }
    }
    else if (ActivationType == TEXT("relu"))
    {
        for (float& Val : Result)
        {
            Val = FMath::Max(0.0f, Val);
        }
    }
    
    return Result;
}

void UTensorLogicEngine::InitializeTensor(TArray<float>& Tensor, int32 Size)
{
    Tensor.SetNum(Size);
    
    // Xavier initialization
    float Scale = FMath::Sqrt(2.0f / Size);
    
    for (int32 i = 0; i < Size; ++i)
    {
        Tensor[i] = FMath::FRandRange(-Scale, Scale);
    }
}

float UTensorLogicEngine::ComputeTensorNorm(const TArray<float>& Tensor) const
{
    float Sum = 0.0f;
    for (float Val : Tensor)
    {
        Sum += Val * Val;
    }
    return FMath::Sqrt(Sum);
}

void UTensorLogicEngine::NormalizeTensor(TArray<float>& Tensor)
{
    float Norm = ComputeTensorNorm(Tensor);
    
    if (Norm > SMALL_NUMBER)
    {
        for (float& Val : Tensor)
        {
            Val /= Norm;
        }
    }
}

// ========================================
// UTILITIES
// ========================================

FString UTensorLogicEngine::GenerateRuleID()
{
    return FString::Printf(TEXT("R%d"), ++RuleIDCounter);
}

FString UTensorLogicEngine::GetStatistics() const
{
    return FString::Printf(
        TEXT("TensorLogic Statistics:\n")
        TEXT("  Rules: %d\n")
        TEXT("  Facts: %d\n")
        TEXT("  Predicates: %d\n")
        TEXT("  Entity Embeddings: %d\n")
        TEXT("  Relation Embeddings: %d\n")
        TEXT("  Embedding Dimension: %d\n"),
        Rules.Num(),
        Facts.Num(),
        Predicates.Num(),
        EntityEmbeddings.Num(),
        RelationEmbeddings.Num(),
        EmbeddingDimension
    );
}

FString UTensorLogicEngine::ExportKnowledgeBase() const
{
    FString Result;
    
    Result += TEXT("# TensorLogic Knowledge Base\n\n");
    
    Result += TEXT("## Facts\n");
    for (const FTensorLogicAtom& Fact : Facts)
    {
        Result += Fact.ToString() + TEXT(".\n");
    }
    
    Result += TEXT("\n## Rules\n");
    for (const auto& Pair : Rules)
    {
        Result += Pair.Value.ToString() + TEXT(".\n");
    }
    
    return Result;
}

bool UTensorLogicEngine::ImportKnowledgeBase(const FString& Data)
{
    // Simplified parser - just clear and log
    ClearKnowledgeBase();
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicEngine: Import not fully implemented"));
    
    return true;
}

bool UTensorLogicEngine::SaveKnowledgeBase(const FString& FilePath)
{
    FString Data = ExportKnowledgeBase();
    return FFileHelper::SaveStringToFile(Data, *FilePath);
}

bool UTensorLogicEngine::LoadKnowledgeBase(const FString& FilePath)
{
    FString Data;
    if (!FFileHelper::LoadFileToString(Data, *FilePath))
    {
        return false;
    }
    
    return ImportKnowledgeBase(Data);
}
