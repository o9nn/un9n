// FACSToMetaHumanMapping.h
// Complete FACS Action Unit to MetaHuman CTRL_ Morph Target Mapping
// Based on the MetaHuman Facial Description Standard (MH Standards)
// Maps all 70+ FACS Action Units to the 251 MetaHuman control curves
// Includes Rig Logic evaluation pipeline integration points

#pragma once

#include "CoreMinimal.h"
#include "FACSToMetaHumanMapping.generated.h"

/**
 * Single FACS AU to MetaHuman CTRL_ mapping entry
 * Maps one Action Unit to one or more CTRL_ morph targets with weights
 */
USTRUCT(BlueprintType)
struct FFACSToCtrlMapping
{
    GENERATED_BODY()

    /** FACS Action Unit identifier (e.g., "AU1") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FACSActionUnit;

    /** FACS Action Unit description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    /** MetaHuman CTRL_ morph target name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CtrlMorphTarget;

    /** Mapping weight (how much this AU drives this CTRL_) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Weight = 1.0f;

    /** Whether this is a left-side mapping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLeftSide = false;

    /** Whether this is a right-side mapping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRightSide = false;
};

/**
 * Emotion-to-FACS preset mapping
 */
USTRUCT(BlueprintType)
struct FEmotionFACSPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EmotionName;

    /** Map of AU name to intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ActionUnitIntensities;
};

/**
 * Complete FACS to MetaHuman mapping database
 * Provides bidirectional lookup between FACS AUs and MetaHuman CTRL_ targets
 */
UCLASS(BlueprintType)
class DEEPTREEECHO_API UFACSToMetaHumanMapping : public UObject
{
    GENERATED_BODY()

public:
    UFACSToMetaHumanMapping();

    /** Initialize all mappings */
    UFUNCTION(BlueprintCallable, Category = "FACS Mapping")
    void InitializeMappings();

    /** Get all CTRL_ targets for a given FACS AU */
    UFUNCTION(BlueprintPure, Category = "FACS Mapping")
    TArray<FFACSToCtrlMapping> GetCtrlTargetsForAU(FName ActionUnit) const;

    /** Get all FACS AUs that drive a given CTRL_ target */
    UFUNCTION(BlueprintPure, Category = "FACS Mapping")
    TArray<FFACSToCtrlMapping> GetAUsForCtrlTarget(FName CtrlTarget) const;

    /** Convert a set of FACS AU activations to CTRL_ morph target values */
    UFUNCTION(BlueprintCallable, Category = "FACS Mapping")
    TMap<FName, float> ConvertFACSToCtrl(const TMap<FName, float>& FACSActivations) const;

    /** Convert CTRL_ morph target values back to FACS AU activations */
    UFUNCTION(BlueprintCallable, Category = "FACS Mapping")
    TMap<FName, float> ConvertCtrlToFACS(const TMap<FName, float>& CtrlValues) const;

    /** Get emotion preset */
    UFUNCTION(BlueprintPure, Category = "FACS Mapping")
    FEmotionFACSPreset GetEmotionPreset(FName EmotionName) const;

    /** Get all available emotion presets */
    UFUNCTION(BlueprintPure, Category = "FACS Mapping")
    TArray<FName> GetAvailableEmotions() const;

protected:
    /** All FACS-to-CTRL mappings */
    UPROPERTY()
    TArray<FFACSToCtrlMapping> AllMappings;

    /** Emotion presets */
    UPROPERTY()
    TMap<FName, FEmotionFACSPreset> EmotionPresets;

    /** Initialize upper face AU mappings (AU1-AU7) */
    void InitializeUpperFaceMappings();

    /** Initialize lower face AU mappings (AU9-AU28) */
    void InitializeLowerFaceMappings();

    /** Initialize eye and lid AU mappings (AU41-AU46) */
    void InitializeEyeMappings();

    /** Initialize head and neck AU mappings (AU51-AU58) */
    void InitializeHeadMappings();

    /** Initialize miscellaneous AU mappings (AU61-AU64, etc.) */
    void InitializeMiscMappings();

    /** Initialize emotion presets */
    void InitializeEmotionPresets();

    /** Helper to add a mapping */
    void AddMapping(const FName& AU, const FString& Desc, const FName& Ctrl,
                    float Weight = 1.0f, bool bLeft = false, bool bRight = false);
};
