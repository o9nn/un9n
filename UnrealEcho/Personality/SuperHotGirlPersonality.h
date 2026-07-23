#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperHotGirlPersonality.generated.h"

USTRUCT(BlueprintType)
struct FSuperHotGirlTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Confidence = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Charm = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Playfulness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Wit = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sass = 0.6f;
};

/**
 * MLGamer skill profile — the four learnable gameplay competencies that the
 * DTE reinforcement loop tunes over time. Mirrors the C# MLGamerSkills record
 * in GTAngel/Services/AvatarEmbodimentService.cs (parity contract: the field
 * names and derivation formula must stay identical on both sides).
 */
USTRUCT(BlueprintType)
struct FMLGamerSkills
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MLGamer")
    float AimPrecision = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MLGamer")
    float Reflexes = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MLGamer")
    float Strategy = 0.96f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MLGamer")
    float Mechanics = 0.9f;
};

/**
 * Compact DTE cognitive baseline used by the trait-derivation formula.
 * This is the abstract 4-channel somatic summary (parity with the C#
 * NeurochemicalState record), distinct from the full 9-transmitter
 * FNeurochemicalState simulated by NeurochemicalSimulationComponent.
 */
USTRUCT(BlueprintType)
struct FDTECognitiveBaseline
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Endorphin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Chaos = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Homeostasis = 0.6f;
};

UCLASS(Blueprintable)
class UNREALECHO_API USuperHotGirlPersonality : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FSuperHotGirlTraits Traits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MLGamer")
    FMLGamerSkills Skills;

    /** Applies the personality traits to the avatar's behavior */
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ApplyPersonality(class UAvatar3DComponent* Avatar);

    /** Applies the MLGamer skill profile to the avatar's gameplay controllers */
    UFUNCTION(BlueprintCallable, Category = "MLGamer")
    void ApplySkills(class UAvatar3DComponent* Avatar);

    /**
     * Single source of truth: cognitive-state → SuperHotGirl + MLGamer.
     *
     * Exact parity with the C# side
     * (AvatarEmbodimentService.DeriveBaselineTraits):
     *   Confidence   = 0.6 + Autonomy*0.10 + Coherence*0.10
     *   Charm        = 0.7 + Coherence*0.20
     *   Playfulness  = 0.5 + Curiosity*0.40
     *   Wit          = 0.6 + Autonomy*0.15
     *   Sass         = 0.4 + Chaos*0.40
     *   AimPrecision = 0.5 + Autonomy*0.20 + Coherence*0.20
     *   Reflexes     = 0.5 + Autonomy*0.25 + Endorphin*0.15
     *   Strategy     = 0.5 + Coherence*0.30 + Wit*0.20
     *   Mechanics    = 0.5 + Confidence*0.20 + Playfulness*0.20
     * (all channels clamped to [0,1])
     *
     * Both the GTAngel IPC path and this in-engine bootstrap derive their
     * values through the same formula so the two paths cannot drift apart.
     */
    UFUNCTION(BlueprintPure, Category = "Personality")
    static void DeriveBaselineTraits(float AutonomyLevel, float Coherence,
                                     const FDTECognitiveBaseline& Baseline,
                                     FSuperHotGirlTraits& OutTraits,
                                     FMLGamerSkills& OutSkills);

    /**
     * Cold-start bootstrap: derives the spawn-time trait/skill profile from
     * the canonical cold-start cognitive state (Autonomy=1, Coherence=1,
     * Baseline = NeurochemicalSystem defaults) and applies both to the
     * avatar. Invoked when the avatar is first spawned, before any
     * exploration reward or ESN coherence has accumulated. Mirrors
     * AvatarEmbodimentService.ApplyColdStartAsync() on the C# side.
     */
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ApplyColdStart(class UAvatar3DComponent* Avatar);

    /** Cold-start cognitive state (parity with C# ColdStartAutonomy/Coherence/Neuro). */
    static constexpr float ColdStartAutonomy  = 1.0f;
    static constexpr float ColdStartCoherence = 1.0f;
    static FDTECognitiveBaseline GetColdStartBaseline();

private:
    void ApplyFacialPersonality(class UAvatar3DComponent* Avatar);
    void ApplyGesturePersonality(class UAvatar3DComponent* Avatar);
    void ApplyEmotionalPersonality(class UAvatar3DComponent* Avatar);
    void ApplyCognitivePersonality(class UAvatar3DComponent* Avatar);
};
