#include "TensorLogicCognitiveBridge.h"
#include "GameFramework/Actor.h"

UTensorLogicCognitiveBridge::UTensorLogicCognitiveBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UTensorLogicCognitiveBridge::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoSync)
    {
        InitializeBridge();
    }
}

void UTensorLogicCognitiveBridge::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoSync && bIsInitialized)
    {
        SyncAccumulator += DeltaTime;
        
        if (SyncAccumulator >= (1.0f / SyncFrequency))
        {
            BidirectionalSync();
            SyncAccumulator = 0.0f;
        }
    }
}

// ========================================
// INITIALIZATION
// ========================================

bool UTensorLogicCognitiveBridge::InitializeBridge()
{
    FindComponents();
    
    if (!TensorLogicEngine || !CognitiveCore)
    {
        UE_LOG(LogTemp, Error, TEXT("TensorLogicCognitiveBridge: Missing required components"));
        return false;
    }
    
    bIsInitialized = true;
    
    // Initial synchronization
    SyncHypergraphToTensorLogic();
    
    UE_LOG(LogTemp, Log, TEXT("TensorLogicCognitiveBridge: Initialized successfully"));
    
    return true;
}

void UTensorLogicCognitiveBridge::FindComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Find tensor logic engine
    if (!TensorLogicEngine)
    {
        TensorLogicEngine = Owner->FindComponentByClass<UTensorLogicEngine>();
    }
    
    // Find cognitive core
    if (!CognitiveCore)
    {
        CognitiveCore = Owner->FindComponentByClass<UDeepTreeEchoCognitiveCore>();
    }
    
    // Find neural bridge
    if (!NeuralBridge)
    {
        NeuralBridge = Owner->FindComponentByClass<UOCNNIntegrationBridge>();
    }
}

// ========================================
// SYNCHRONIZATION
// ========================================

void UTensorLogicCognitiveBridge::SyncHypergraphToTensorLogic()
{
    if (!bIsInitialized || !TensorLogicEngine || !CognitiveCore)
    {
        return;
    }
    
    // Convert hypergraph nodes to tensor logic facts
    for (const auto& Pair : CognitiveCore->HypergraphNodes)
    {
        const FHypergraphNode& Node = Pair.Value;
        
        // Only sync activated nodes above threshold
        if (Node.Activation > 0.1f)
        {
            FTensorLogicAtom Fact = NodeToAtom(Node);
            Fact.TruthValue = Node.Activation;
            
            // Check if not already in tensor logic
            bool AlreadyExists = false;
            for (const FTensorLogicAtom& ExistingFact : TensorLogicEngine->Facts)
            {
                if (ExistingFact.Predicate == Fact.Predicate && 
                    ExistingFact.Arguments.Num() == Fact.Arguments.Num())
                {
                    AlreadyExists = true;
                    break;
                }
            }
            
            if (!AlreadyExists)
            {
                TensorLogicEngine->AddFact(Fact);
            }
        }
    }
    
    // Convert echo patterns to tensor logic rules
    for (const FEchoPropagationPattern& Pattern : CognitiveCore->DetectedPatterns)
    {
        if (Pattern.Strength > 0.5f)
        {
            FTensorLogicRule Rule = PatternToRule(Pattern);
            TensorLogicEngine->AddRule(Rule);
        }
    }
    
    SyncCount++;
    LastSyncTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Verbose, TEXT("TensorLogicCognitiveBridge: Synced hypergraph -> tensor logic (count=%d)"), SyncCount);
}

void UTensorLogicCognitiveBridge::SyncTensorLogicToHypergraph()
{
    if (!bIsInitialized || !TensorLogicEngine || !CognitiveCore)
    {
        return;
    }
    
    // Convert tensor logic facts to hypergraph nodes
    for (const FTensorLogicAtom& Fact : TensorLogicEngine->Facts)
    {
        FHypergraphNode Node = AtomToNode(Fact);
        Node.Activation = Fact.TruthValue;
        
        // Check if node already exists
        bool NodeExists = false;
        for (const auto& Pair : CognitiveCore->HypergraphNodes)
        {
            if (ComputeSimilarity(Pair.Value, Fact) > 0.8f)
            {
                NodeExists = true;
                break;
            }
        }
        
        if (!NodeExists)
        {
            CognitiveCore->AddNode(Node.Content, Node.MemoryType);
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("TensorLogicCognitiveBridge: Synced tensor logic -> hypergraph"));
}

void UTensorLogicCognitiveBridge::BidirectionalSync()
{
    if (bEnableBidirectionalLearning)
    {
        // Sync both directions
        SyncHypergraphToTensorLogic();
        SyncTensorLogicToHypergraph();
        
        // Extract neural embeddings
        if (NeuralBridge)
        {
            ExtractNeuralEmbeddings();
        }
    }
}

// ========================================
// HYBRID REASONING
// ========================================

FTensorLogicQueryResult UTensorLogicCognitiveBridge::HybridQuery(const FString& Predicate, const TArray<FString>& Arguments)
{
    FTensorLogicQueryResult Result;
    
    if (!bIsInitialized || !TensorLogicEngine)
    {
        return Result;
    }
    
    // Create query atom
    FTensorLogicAtom Query;
    Query.Predicate = Predicate;
    
    for (const FString& Arg : Arguments)
    {
        Query.Arguments.Add(FTensorLogicTerm::MakeConstant(Arg, 1.0f));
    }
    
    // Try symbolic inference first
    Result = TensorLogicEngine->Query(Query, ETensorLogicInferenceMode::Hybrid);
    
    // If symbolic fails and we have neural bridge, try neural reasoning
    if (!Result.bSuccess && NeuralBridge)
    {
        // Use neural embeddings to guide search
        for (const auto& Pair : TensorLogicEngine->EntityEmbeddings)
        {
            // Check if embedding is similar to query arguments
            // This is a simplified version
            if (Arguments.Contains(Pair.Key))
            {
                Result.bSuccess = true;
                Result.Confidences.Add(0.5f * NeuralToSymbolicWeight);
            }
        }
    }
    
    return Result;
}

TArray<FHypergraphNode> UTensorLogicCognitiveBridge::NeuralGuidedInference(const FString& StartNodeID, int32 MaxDepth)
{
    TArray<FHypergraphNode> Result;
    
    if (!bIsInitialized || !CognitiveCore || !NeuralBridge)
    {
        return Result;
    }
    
    // Get start node
    FHypergraphNode StartNode = CognitiveCore->GetNode(StartNodeID);
    
    // Use neural similarity to guide exploration
    TSet<FString> Visited;
    TArray<FString> Queue;
    Queue.Add(StartNodeID);
    Visited.Add(StartNodeID);
    
    int32 CurrentDepth = 0;
    
    while (Queue.Num() > 0 && CurrentDepth < MaxDepth)
    {
        FString CurrentID = Queue[0];
        Queue.RemoveAt(0);
        
        FHypergraphNode CurrentNode = CognitiveCore->GetNode(CurrentID);
        Result.Add(CurrentNode);
        
        // Find connected nodes
        for (const auto& EdgePair : CognitiveCore->HypergraphEdges)
        {
            const FHypergraphEdge& Edge = EdgePair.Value;
            
            if (Edge.SourceNodeIDs.Contains(CurrentID))
            {
                for (const FString& TargetID : Edge.TargetNodeIDs)
                {
                    if (!Visited.Contains(TargetID))
                    {
                        // Use neural activation to prioritize
                        FHypergraphNode TargetNode = CognitiveCore->GetNode(TargetID);
                        if (TargetNode.Activation > 0.3f)
                        {
                            Queue.Add(TargetID);
                            Visited.Add(TargetID);
                        }
                    }
                }
            }
        }
        
        CurrentDepth++;
    }
    
    return Result;
}

float UTensorLogicCognitiveBridge::SymbolicConstrainedLearning(const FString& Constraint, const TArray<FString>& Examples)
{
    if (!bIsInitialized || !TensorLogicEngine)
    {
        return 0.0f;
    }
    
    float TotalLoss = 0.0f;
    
    // For each example, ensure it satisfies the constraint
    for (const FString& Example : Examples)
    {
        // Parse example as atom
        FTensorLogicAtom ExampleAtom;
        ExampleAtom.Predicate = Constraint;
        ExampleAtom.Arguments.Add(FTensorLogicTerm::MakeConstant(Example, 1.0f));
        
        // Train to satisfy constraint
        float Loss = TensorLogicEngine->TrainOnExample(ExampleAtom, 1.0f);
        TotalLoss += Loss;
    }
    
    return TotalLoss / FMath::Max(1, Examples.Num());
}

// ========================================
// KNOWLEDGE EXTRACTION
// ========================================

TArray<FTensorLogicRule> UTensorLogicCognitiveBridge::ExtractRulesFromPatterns()
{
    TArray<FTensorLogicRule> Rules;
    
    if (!bIsInitialized || !CognitiveCore)
    {
        return Rules;
    }
    
    for (const FEchoPropagationPattern& Pattern : CognitiveCore->DetectedPatterns)
    {
        if (Pattern.Strength > 0.6f && Pattern.Frequency > 3)
        {
            FTensorLogicRule Rule = PatternToRule(Pattern);
            Rules.Add(Rule);
        }
    }
    
    return Rules;
}

void UTensorLogicCognitiveBridge::ExtractNeuralEmbeddings()
{
    if (!bIsInitialized || !NeuralBridge || !TensorLogicEngine)
    {
        return;
    }
    
    // Extract embeddings from neural bridge and add to tensor logic
    // This is a simplified version - full implementation would process neural network layers
    
    for (const auto& NodePair : CognitiveCore->HypergraphNodes)
    {
        const FHypergraphNode& Node = NodePair.Value;
        
        // Create embedding based on node properties
        TArray<float> Embedding;
        Embedding.Add(Node.Activation);
        Embedding.Add(Node.AccessCount / 100.0f);
        Embedding.Add(Node.CreationTime / 1000.0f);
        
        // Pad to embedding dimension
        while (Embedding.Num() < TensorLogicEngine->EmbeddingDimension)
        {
            Embedding.Add(FMath::FRandRange(-0.1f, 0.1f));
        }
        
        TensorLogicEngine->SetEntityEmbedding(Node.NodeID, Embedding);
    }
}

int32 UTensorLogicCognitiveBridge::LearnRulesFromHypergraph(int32 MaxRules)
{
    if (!bIsInitialized || !CognitiveCore || !TensorLogicEngine)
    {
        return 0;
    }
    
    int32 RulesLearned = 0;
    
    // Analyze hypergraph structure to learn rules
    for (const auto& EdgePair : CognitiveCore->HypergraphEdges)
    {
        if (RulesLearned >= MaxRules)
        {
            break;
        }
        
        const FHypergraphEdge& Edge = EdgePair.Value;
        
        // Simple rule: if edge has high activation, create rule
        if (Edge.Activation > 0.7f && Edge.SourceNodeIDs.Num() > 0 && Edge.TargetNodeIDs.Num() > 0)
        {
            FTensorLogicRule Rule = EdgeToRule(Edge);
            TensorLogicEngine->AddRule(Rule);
            RulesLearned++;
        }
    }
    
    return RulesLearned;
}

// ========================================
// CONVERSION UTILITIES
// ========================================

FTensorLogicAtom UTensorLogicCognitiveBridge::NodeToAtom(const FHypergraphNode& Node) const
{
    FTensorLogicAtom Atom;
    
    // Extract predicate from content
    Atom.Predicate = ExtractPredicate(Node.Content);
    
    // Extract arguments
    TArray<FString> Args = ExtractArguments(Node.Content);
    for (const FString& Arg : Args)
    {
        Atom.Arguments.Add(FTensorLogicTerm::MakeConstant(Arg, 1.0f));
    }
    
    // If no arguments, use node ID as argument
    if (Atom.Arguments.Num() == 0)
    {
        Atom.Arguments.Add(FTensorLogicTerm::MakeConstant(Node.NodeID, 1.0f));
    }
    
    Atom.TruthValue = Node.Activation;
    
    return Atom;
}

FTensorLogicRule UTensorLogicCognitiveBridge::EdgeToRule(const FHypergraphEdge& Edge) const
{
    FTensorLogicRule Rule;
    
    // Head: use first target node
    if (Edge.TargetNodeIDs.Num() > 0 && CognitiveCore)
    {
        FHypergraphNode TargetNode = CognitiveCore->GetNode(Edge.TargetNodeIDs[0]);
        Rule.Head = NodeToAtom(TargetNode);
    }
    
    // Body: use source nodes
    for (const FString& SourceID : Edge.SourceNodeIDs)
    {
        if (CognitiveCore)
        {
            FHypergraphNode SourceNode = CognitiveCore->GetNode(SourceID);
            FTensorLogicAtom BodyAtom = NodeToAtom(SourceNode);
            Rule.Body.Add(BodyAtom);
        }
    }
    
    Rule.Weight = Edge.Weight;
    Rule.Confidence = Edge.Activation;
    Rule.Mode = ETensorLogicRuleMode::Continuous;
    
    return Rule;
}

FHypergraphNode UTensorLogicCognitiveBridge::AtomToNode(const FTensorLogicAtom& Atom) const
{
    FHypergraphNode Node;
    
    // Create content from predicate and arguments
    FString Content = Atom.Predicate + TEXT("(");
    for (int32 i = 0; i < Atom.Arguments.Num(); ++i)
    {
        if (i > 0) Content += TEXT(", ");
        Content += Atom.Arguments[i].Symbol;
    }
    Content += TEXT(")");
    
    Node.Content = Content;
    Node.NodeID = FString::Printf(TEXT("TL_%s"), *Atom.Predicate);
    Node.Activation = Atom.TruthValue;
    Node.MemoryType = EMemoryType::Declarative;
    
    return Node;
}

FTensorLogicRule UTensorLogicCognitiveBridge::PatternToRule(const FEchoPropagationPattern& Pattern) const
{
    FTensorLogicRule Rule;
    
    // Create rule from pattern
    Rule.Head.Predicate = TEXT("pattern_") + Pattern.PatternID;
    
    // Add pattern nodes as body
    for (const FString& NodeID : Pattern.NodeIDs)
    {
        if (CognitiveCore)
        {
            FHypergraphNode Node = CognitiveCore->GetNode(NodeID);
            FTensorLogicAtom BodyAtom = NodeToAtom(Node);
            Rule.Body.Add(BodyAtom);
        }
    }
    
    Rule.Confidence = Pattern.Strength;
    Rule.Mode = ETensorLogicRuleMode::Hybrid;
    Rule.bIsLearnable = true;
    
    return Rule;
}

float UTensorLogicCognitiveBridge::ComputeSimilarity(const FHypergraphNode& Node, const FTensorLogicAtom& Atom) const
{
    // Simple string similarity
    FString NodeContent = Node.Content.ToLower();
    FString AtomString = Atom.ToString().ToLower();
    
    // Count matching characters
    int32 Matches = 0;
    int32 Total = FMath::Max(NodeContent.Len(), AtomString.Len());
    
    for (int32 i = 0; i < FMath::Min(NodeContent.Len(), AtomString.Len()); ++i)
    {
        if (NodeContent[i] == AtomString[i])
        {
            Matches++;
        }
    }
    
    return Total > 0 ? static_cast<float>(Matches) / Total : 0.0f;
}

FString UTensorLogicCognitiveBridge::ExtractPredicate(const FString& Content) const
{
    // Extract predicate name from content (everything before '(')
    int32 ParenIndex;
    if (Content.FindChar('(', ParenIndex))
    {
        return Content.Left(ParenIndex).TrimStartAndEnd();
    }
    
    // If no parenthesis, use first word
    FString Predicate;
    int32 SpaceIndex;
    if (Content.FindChar(' ', SpaceIndex))
    {
        Predicate = Content.Left(SpaceIndex);
    }
    else
    {
        Predicate = Content;
    }
    
    // Clean and return
    return Predicate.TrimStartAndEnd().ToLower();
}

TArray<FString> UTensorLogicCognitiveBridge::ExtractArguments(const FString& Content) const
{
    TArray<FString> Arguments;
    
    // Extract content between parentheses
    int32 StartParen, EndParen;
    if (Content.FindChar('(', StartParen) && Content.FindLastChar(')', EndParen))
    {
        FString ArgsString = Content.Mid(StartParen + 1, EndParen - StartParen - 1);
        ArgsString.ParseIntoArray(Arguments, TEXT(","));
        
        // Trim each argument
        for (FString& Arg : Arguments)
        {
            Arg = Arg.TrimStartAndEnd();
        }
    }
    
    return Arguments;
}

// ========================================
// UTILITIES
// ========================================

FString UTensorLogicCognitiveBridge::GetBridgeStatistics() const
{
    UWorld* World = GetWorld();
    float TimeSinceLastSync = World ? (World->GetTimeSeconds() - LastSyncTime) : 0.0f;
    
    return FString::Printf(
        TEXT("TensorLogicCognitiveBridge Statistics:\n")
        TEXT("  Initialized: %s\n")
        TEXT("  Auto Sync: %s\n")
        TEXT("  Sync Count: %d\n")
        TEXT("  Last Sync: %.2f seconds ago\n")
        TEXT("  Bidirectional Learning: %s\n")
        TEXT("  Neural-to-Symbolic Weight: %.2f\n"),
        bIsInitialized ? TEXT("Yes") : TEXT("No"),
        bAutoSync ? TEXT("Yes") : TEXT("No"),
        SyncCount,
        TimeSinceLastSync,
        bEnableBidirectionalLearning ? TEXT("Yes") : TEXT("No"),
        NeuralToSymbolicWeight
    );
}

FString UTensorLogicCognitiveBridge::GetComponentStatus() const
{
    return FString::Printf(
        TEXT("Component Status:\n")
        TEXT("  TensorLogicEngine: %s\n")
        TEXT("  CognitiveCore: %s\n")
        TEXT("  NeuralBridge: %s\n"),
        TensorLogicEngine ? TEXT("Connected") : TEXT("Missing"),
        CognitiveCore ? TEXT("Connected") : TEXT("Missing"),
        NeuralBridge ? TEXT("Connected") : TEXT("Optional")
    );
}
