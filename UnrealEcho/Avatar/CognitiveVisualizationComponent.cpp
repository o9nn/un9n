#include "CognitiveVisualizationComponent.h"
#include "DrawDebugHelpers.h"

UCognitiveVisualizationComponent::UCognitiveVisualizationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    CurrentActivity = ECognitiveActivityType::Idle;
    ActivityIntensity = 0.0f;

    bHasAttentionFocus = false;
    AttentionFocusIntensity = 0.0f;

    NeuralActivityTime = 0.0f;
    CognitivePatternTime = 0.0f;

    bDecisionVisualizationActive = false;
    SelectedDecisionIndex = -1;
    DecisionVisualizationTimer = 0.0f;
}

void UCognitiveVisualizationComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeNeuralNetwork();

    UE_LOG(LogTemp, Log, TEXT("CognitiveVisualizationComponent initialized with %d neural nodes"), NeuralNodes.Num());
}

void UCognitiveVisualizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bVisualizationEnabled)
        return;

    UpdateNeuralActivity(DeltaTime);
    UpdateThoughtParticles(DeltaTime);
    UpdateAttentionVisualization(DeltaTime);
    UpdateCognitivePatterns(DeltaTime);
}

void UCognitiveVisualizationComponent::InitializeNeuralNetwork()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    FVector BasePosition = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);

    // Create a multi-layer neural network visualization
    const int32 LayerCount = 5;
    const TArray<int32> NodesPerLayer = {8, 12, 16, 12, 8}; // Deep-tree-echo architecture

    int32 NodeIndex = 0;

    for (int32 Layer = 0; Layer < LayerCount; Layer++)
    {
        int32 NodeCount = NodesPerLayer[Layer];
        float LayerZ = Layer * 40.0f;

        for (int32 Node = 0; Node < NodeCount; Node++)
        {
            FNeuralNode NewNode;
            
            // Arrange nodes in a circle for each layer
            float Angle = (2.0f * PI * Node) / NodeCount;
            float Radius = 50.0f + Layer * 10.0f;
            
            NewNode.Position = BasePosition + FVector(
                FMath::Cos(Angle) * Radius,
                FMath::Sin(Angle) * Radius,
                LayerZ
            );
            
            NewNode.Activation = 0.0f;
            NewNode.Color = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
            NewNode.Size = 5.0f;
            NewNode.Layer = Layer;

            NeuralNodes.Add(NewNode);
            NodeIndex++;
        }
    }

    // Create connections between layers
    int32 CurrentNodeIndex = 0;

    for (int32 Layer = 0; Layer < LayerCount - 1; Layer++)
    {
        int32 CurrentLayerNodes = NodesPerLayer[Layer];
        int32 NextLayerNodes = NodesPerLayer[Layer + 1];
        int32 NextLayerStartIndex = CurrentNodeIndex + CurrentLayerNodes;

        for (int32 i = 0; i < CurrentLayerNodes; i++)
        {
            for (int32 j = 0; j < NextLayerNodes; j++)
            {
                // Create connections with some randomness
                if (FMath::FRand() > 0.3f) // 70% connection probability
                {
                    FNeuralConnection Connection;
                    Connection.FromNodeIndex = CurrentNodeIndex + i;
                    Connection.ToNodeIndex = NextLayerStartIndex + j;
                    Connection.Weight = FMath::FRandRange(0.3f, 1.0f);
                    Connection.ActivityLevel = 0.0f;

                    NeuralConnections.Add(Connection);
                }
            }
        }

        CurrentNodeIndex += CurrentLayerNodes;
    }

    UE_LOG(LogTemp, Log, TEXT("Neural network initialized: %d nodes, %d connections"), 
        NeuralNodes.Num(), NeuralConnections.Num());
}

void UCognitiveVisualizationComponent::SetCognitiveActivity(ECognitiveActivityType Activity, float Intensity)
{
    CurrentActivity = Activity;
    ActivityIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);

    // Trigger neural activation based on activity type
    int32 StartNodeIndex = 0;
    float ActivationStrength = ActivityIntensity;

    switch (Activity)
    {
        case ECognitiveActivityType::Thinking:
            StartNodeIndex = FMath::RandRange(0, 7); // Input layer
            ActivationStrength *= 0.8f;
            break;
        case ECognitiveActivityType::Learning:
            StartNodeIndex = FMath::RandRange(8, 19); // Second layer
            ActivationStrength *= 1.2f;
            break;
        case ECognitiveActivityType::Remembering:
            StartNodeIndex = FMath::RandRange(20, 35); // Middle layer
            ActivationStrength *= 1.0f;
            break;
        case ECognitiveActivityType::Creating:
            StartNodeIndex = FMath::RandRange(36, 47); // Fourth layer
            ActivationStrength *= 1.5f;
            break;
        case ECognitiveActivityType::Analyzing:
            StartNodeIndex = FMath::RandRange(0, 19); // First two layers
            ActivationStrength *= 1.1f;
            break;
        case ECognitiveActivityType::Deciding:
            StartNodeIndex = FMath::RandRange(48, 55); // Output layer
            ActivationStrength *= 1.3f;
            break;
        default:
            ActivationStrength = 0.1f;
            break;
    }

    PropagateNeuralActivation(StartNodeIndex, ActivationStrength);

    UE_LOG(LogTemp, Log, TEXT("Cognitive activity set: %d (intensity: %.2f)"), (int32)Activity, Intensity);
}

void UCognitiveVisualizationComponent::TriggerThoughtProcess(const FVector& StartPosition, const FVector& EndPosition)
{
    // Generate a stream of thought particles
    FLinearColor ThoughtColor;

    switch (CurrentActivity)
    {
        case ECognitiveActivityType::Thinking:
            ThoughtColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
            break;
        case ECognitiveActivityType::Learning:
            ThoughtColor = FLinearColor(0.3f, 1.0f, 0.5f, 1.0f);
            break;
        case ECognitiveActivityType::Creating:
            ThoughtColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            break;
        case ECognitiveActivityType::Emotional:
            ThoughtColor = FLinearColor(1.0f, 0.4f, 0.7f, 1.0f);
            break;
        default:
            ThoughtColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            break;
    }

    GenerateThoughtParticleStream(StartPosition, EndPosition, ThoughtColor);
}

void UCognitiveVisualizationComponent::VisualizeMemoryAccess(int32 MemoryRegion, float AccessIntensity)
{
    MemoryAccessIndicators.Add(MemoryRegion, AccessIntensity);

    // Trigger neural activation in memory-related nodes
    if (NeuralNodes.IsValidIndex(20 + (MemoryRegion % 16)))
    {
        PropagateNeuralActivation(20 + (MemoryRegion % 16), AccessIntensity);
    }

    UE_LOG(LogTemp, Log, TEXT("Memory access visualized: region %d, intensity %.2f"), MemoryRegion, AccessIntensity);
}

void UCognitiveVisualizationComponent::VisualizeDecisionMaking(const TArray<FString>& Options, int32 SelectedIndex)
{
    DecisionOptions = Options;
    SelectedDecisionIndex = SelectedIndex;
    bDecisionVisualizationActive = true;
    DecisionVisualizationTimer = 0.0f;

    // Activate output layer nodes
    for (int32 i = 48; i < 56 && i < NeuralNodes.Num(); i++)
    {
        if (i - 48 == SelectedIndex)
        {
            NeuralNodes[i].Activation = 1.0f;
            NeuralNodes[i].Color = FLinearColor(0.3f, 1.0f, 0.3f, 1.0f);
        }
        else
        {
            NeuralNodes[i].Activation = 0.3f;
            NeuralNodes[i].Color = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Decision visualization: %d options, selected %d"), Options.Num(), SelectedIndex);
}

void UCognitiveVisualizationComponent::SetAttentionFocus(const FVector& FocusPoint, float FocusIntensity)
{
    AttentionFocusPoint = FocusPoint;
    AttentionFocusIntensity = FocusIntensity;
    bHasAttentionFocus = true;
}

void UCognitiveVisualizationComponent::SetVisualizationEnabled(bool bEnabled)
{
    bVisualizationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Cognitive visualization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCognitiveVisualizationComponent::UpdateNeuralActivity(float DeltaTime)
{
    NeuralActivityTime += DeltaTime;

    // Decay neural activation
    for (FNeuralNode& Node : NeuralNodes)
    {
        Node.Activation *= FMath::Exp(-DeltaTime * 2.0f); // Exponential decay

        // Update node color based on activation
        float ActivationLevel = FMath::Clamp(Node.Activation, 0.0f, 1.0f);
        Node.Color = FLinearColor::LerpUsingHSV(
            FLinearColor(0.3f, 0.5f, 0.8f, 1.0f),
            FLinearColor(1.0f, 0.8f, 0.3f, 1.0f),
            ActivationLevel
        );

        Node.Size = 5.0f + ActivationLevel * 5.0f;
    }

    // Decay connection activity
    for (FNeuralConnection& Connection : NeuralConnections)
    {
        Connection.ActivityLevel *= FMath::Exp(-DeltaTime * 3.0f);
    }

    // Decay memory access indicators
    for (auto& Pair : MemoryAccessIndicators)
    {
        Pair.Value *= FMath::Exp(-DeltaTime * 1.5f);
    }
}

void UCognitiveVisualizationComponent::UpdateThoughtParticles(float DeltaTime)
{
    // Update existing thought particles
    for (int32 i = ThoughtParticles.Num() - 1; i >= 0; i--)
    {
        FThoughtParticle& Particle = ThoughtParticles[i];
        
        Particle.Lifetime -= DeltaTime;

        if (Particle.Lifetime <= 0.0f)
        {
            ThoughtParticles.RemoveAt(i);
        }
        else
        {
            // Update particle position
            Particle.Position += Particle.Velocity * DeltaTime;

            // Apply some turbulence
            Particle.Velocity += FVector(
                FMath::FRandRange(-10.0f, 10.0f),
                FMath::FRandRange(-10.0f, 10.0f),
                FMath::FRandRange(-5.0f, 5.0f)
            ) * DeltaTime;

            // Fade out particle
            float LifetimeRatio = Particle.Lifetime / 2.0f;
            Particle.Color.A = LifetimeRatio;
        }
    }

    // Limit particle count
    if (ThoughtParticles.Num() > 1000)
    {
        ThoughtParticles.RemoveAt(0, ThoughtParticles.Num() - 1000);
    }
}

void UCognitiveVisualizationComponent::UpdateAttentionVisualization(float DeltaTime)
{
    if (!bHasAttentionFocus)
        return;

    // Decay attention focus
    AttentionFocusIntensity *= FMath::Exp(-DeltaTime * 0.5f);

    if (AttentionFocusIntensity < 0.1f)
    {
        bHasAttentionFocus = false;
    }
}

void UCognitiveVisualizationComponent::GenerateThoughtParticleStream(const FVector& Start, const FVector& End, const FLinearColor& Color)
{
    int32 ParticleCount = 20;
    FVector Direction = (End - Start).GetSafeNormal();
    float Distance = FVector::Dist(Start, End);

    for (int32 i = 0; i < ParticleCount; i++)
    {
        FThoughtParticle Particle;
        
        float Progress = (float)i / ParticleCount;
        Particle.Position = FMath::Lerp(Start, End, Progress);
        
        // Add some randomness to position
        Particle.Position += FVector(
            FMath::FRandRange(-10.0f, 10.0f),
            FMath::FRandRange(-10.0f, 10.0f),
            FMath::FRandRange(-5.0f, 5.0f)
        );

        Particle.Velocity = Direction * (Distance / 2.0f) + FVector(
            FMath::FRandRange(-20.0f, 20.0f),
            FMath::FRandRange(-20.0f, 20.0f),
            FMath::FRandRange(-10.0f, 10.0f)
        );

        Particle.Color = Color;
        Particle.Lifetime = FMath::FRandRange(1.5f, 2.5f);
        Particle.Size = FMath::FRandRange(2.0f, 5.0f);

        ThoughtParticles.Add(Particle);
    }
}

void UCognitiveVisualizationComponent::PropagateNeuralActivation(int32 StartNodeIndex, float ActivationStrength)
{
    if (!NeuralNodes.IsValidIndex(StartNodeIndex))
        return;

    // Set initial activation
    NeuralNodes[StartNodeIndex].Activation = ActivationStrength;

    // Propagate activation through connections
    for (FNeuralConnection& Connection : NeuralConnections)
    {
        if (Connection.FromNodeIndex == StartNodeIndex)
        {
            int32 ToIndex = Connection.ToNodeIndex;
            if (NeuralNodes.IsValidIndex(ToIndex))
            {
                float PropagatedActivation = ActivationStrength * Connection.Weight * 0.7f;
                NeuralNodes[ToIndex].Activation = FMath::Max(NeuralNodes[ToIndex].Activation, PropagatedActivation);
                Connection.ActivityLevel = PropagatedActivation;

                // Recursively propagate (limited depth)
                if (PropagatedActivation > 0.2f)
                {
                    PropagateNeuralActivation(ToIndex, PropagatedActivation);
                }
            }
        }
    }
}

void UCognitiveVisualizationComponent::UpdateCognitivePatterns(float DeltaTime)
{
    CognitivePatternTime += DeltaTime;

    // Generate spontaneous neural activity based on current cognitive state
    if (CurrentActivity != ECognitiveActivityType::Idle && FMath::FRand() < ActivityIntensity * DeltaTime)
    {
        int32 RandomNodeIndex = FMath::RandRange(0, NeuralNodes.Num() - 1);
        PropagateNeuralActivation(RandomNodeIndex, ActivityIntensity * 0.5f);
    }

    // Update decision visualization
    if (bDecisionVisualizationActive)
    {
        DecisionVisualizationTimer += DeltaTime;

        if (DecisionVisualizationTimer >= 3.0f)
        {
            bDecisionVisualizationActive = false;
            DecisionOptions.Empty();
            SelectedDecisionIndex = -1;
        }
    }
}
