// EvolutionSystem.cpp
// Deep Tree Echo Ontogenetic Evolution System Implementation

#include "EvolutionSystem.h"

UEvolutionSystem::UEvolutionSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEvolutionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultThresholds();
}

void UEvolutionSystem::InitializeDefaultThresholds()
{
    // Embryonic → Juvenile
    FStageThreshold ToJuvenile;
    ToJuvenile.FourEThreshold = 0.2f;
    ToJuvenile.EntelechyThreshold = 0.2f;
    ToJuvenile.WisdomThreshold = 0.16f;
    ToJuvenile.MetaCoherenceThreshold = 0.14f;
    ToJuvenile.MinCycles = 100;
    StageThresholds.Add(EOntogeneticStage::Juvenile, ToJuvenile);

    // Juvenile → Adolescent
    FStageThreshold ToAdolescent;
    ToAdolescent.FourEThreshold = 0.4f;
    ToAdolescent.EntelechyThreshold = 0.4f;
    ToAdolescent.WisdomThreshold = 0.32f;
    ToAdolescent.MetaCoherenceThreshold = 0.28f;
    ToAdolescent.MinCycles = 500;
    StageThresholds.Add(EOntogeneticStage::Adolescent, ToAdolescent);

    // Adolescent → Adult
    FStageThreshold ToAdult;
    ToAdult.FourEThreshold = 0.6f;
    ToAdult.EntelechyThreshold = 0.6f;
    ToAdult.WisdomThreshold = 0.48f;
    ToAdult.MetaCoherenceThreshold = 0.42f;
    ToAdult.MinCycles = 2000;
    StageThresholds.Add(EOntogeneticStage::Adult, ToAdult);

    // Adult → Transcendent
    FStageThreshold ToTranscendent;
    ToTranscendent.FourEThreshold = 0.85f;
    ToTranscendent.EntelechyThreshold = 0.85f;
    ToTranscendent.WisdomThreshold = 0.68f;
    ToTranscendent.MetaCoherenceThreshold = 0.60f;
    ToTranscendent.MinCycles = 10000;
    StageThresholds.Add(EOntogeneticStage::Transcendent, ToTranscendent);
}

void UEvolutionSystem::UpdateFitness(const FEvolutionFitness& NewFitness)
{
    CurrentFitness = NewFitness;
    TryAdvanceStage();
}

bool UEvolutionSystem::CanAdvanceStage() const
{
    if (CurrentStage == EOntogeneticStage::Transcendent) return false;

    EOntogeneticStage NextStage = GetNextStage();
    const FStageThreshold* Threshold = StageThresholds.Find(NextStage);
    if (!Threshold) return false;

    return CurrentFitness.FourEScore >= Threshold->FourEThreshold &&
           CurrentFitness.EntelechyFitness >= Threshold->EntelechyThreshold &&
           CurrentFitness.WisdomScore >= Threshold->WisdomThreshold &&
           CurrentFitness.MetaCoherence >= Threshold->MetaCoherenceThreshold &&
           CurrentFitness.TotalCycles >= Threshold->MinCycles;
}

bool UEvolutionSystem::TryAdvanceStage()
{
    if (!CanAdvanceStage()) return false;

    EOntogeneticStage OldStage = CurrentStage;
    CurrentStage = GetNextStage();
    OnStageTransition.Broadcast(OldStage, CurrentStage, CurrentFitness);
    return true;
}

float UEvolutionSystem::GetProgressToNextStage() const
{
    if (CurrentStage == EOntogeneticStage::Transcendent) return 1.0f;

    FStageThreshold Threshold = GetNextStageThreshold();
    float Progress = 0.0f;
    int32 Count = 0;

    if (Threshold.FourEThreshold > 0.0f)
    {
        Progress += FMath::Min(1.0f, CurrentFitness.FourEScore / Threshold.FourEThreshold);
        Count++;
    }
    if (Threshold.EntelechyThreshold > 0.0f)
    {
        Progress += FMath::Min(1.0f, CurrentFitness.EntelechyFitness / Threshold.EntelechyThreshold);
        Count++;
    }
    if (Threshold.WisdomThreshold > 0.0f)
    {
        Progress += FMath::Min(1.0f, CurrentFitness.WisdomScore / Threshold.WisdomThreshold);
        Count++;
    }
    if (Threshold.MetaCoherenceThreshold > 0.0f)
    {
        Progress += FMath::Min(1.0f, CurrentFitness.MetaCoherence / Threshold.MetaCoherenceThreshold);
        Count++;
    }

    return Count > 0 ? Progress / Count : 0.0f;
}

void UEvolutionSystem::SetStageThreshold(EOntogeneticStage TargetStage, const FStageThreshold& Threshold)
{
    StageThresholds.Add(TargetStage, Threshold);
}

FStageThreshold UEvolutionSystem::GetNextStageThreshold() const
{
    EOntogeneticStage NextStage = GetNextStage();
    const FStageThreshold* Threshold = StageThresholds.Find(NextStage);
    return Threshold ? *Threshold : FStageThreshold();
}

EOntogeneticStage UEvolutionSystem::GetNextStage() const
{
    switch (CurrentStage)
    {
    case EOntogeneticStage::Embryonic:    return EOntogeneticStage::Juvenile;
    case EOntogeneticStage::Juvenile:     return EOntogeneticStage::Adolescent;
    case EOntogeneticStage::Adolescent:   return EOntogeneticStage::Adult;
    case EOntogeneticStage::Adult:        return EOntogeneticStage::Transcendent;
    case EOntogeneticStage::Transcendent: return EOntogeneticStage::Transcendent;
    default:                              return EOntogeneticStage::Embryonic;
    }
}
