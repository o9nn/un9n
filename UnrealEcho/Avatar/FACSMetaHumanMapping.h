// FACSMetaHumanMapping.h
// Deep Tree Echo - Comprehensive FACS Action Unit to MetaHuman CTRL_ Morph Target Mapping
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// This header provides the complete mapping between FACS Action Units and
// MetaHuman DNA blend shape morph targets for both DHI and MH.4 rigs,
// including endocrine-driven modulation and SuperHotGirl aesthetic biases.

#pragma once

#include "CoreMinimal.h"
#include "FACSMetaHumanMapping.generated.h"

// ============================================================================
// FACS Action Unit Definition
// ============================================================================

USTRUCT(BlueprintType)
struct FFACSActionUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionUnitID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Activation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxValue = 1.0f;

    // Asymmetry: -1.0 = left only, 0.0 = bilateral, 1.0 = right only
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Asymmetry = 0.0f;
};

// ============================================================================
// MetaHuman Morph Target Mapping Entry
// ============================================================================

USTRUCT(BlueprintType)
struct FMorphTargetMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionUnitID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DHI_MorphTarget_Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DHI_MorphTarget_Right;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MH4_MorphTarget_Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MH4_MorphTarget_Right;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeightMultiplier = 1.0f;
};

// ============================================================================
// Endocrine-to-AU Mapping
// ============================================================================

USTRUCT(BlueprintType)
struct FEndocrineAUMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HormoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetAU;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Threshold = 0.1f;

    // Mapping curve: linear, quadratic, sigmoid
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurveType = TEXT("sigmoid");
};

// ============================================================================
// COMPREHENSIVE FACS-TO-METAHUMAN MAPPING TABLE
// ============================================================================

class UNREALECHO_API FFACSMetaHumanMappingTable
{
public:
    static TArray<FMorphTargetMapping> GetCompleteMappingTable()
    {
        TArray<FMorphTargetMapping> Mappings;

        // === UPPER FACE ===

        // AU1 - Inner Brow Raise (Frontalis, pars medialis)
        Mappings.Add({TEXT("AU1"), TEXT("CTRL_L_brow_raiseIn"), TEXT("CTRL_R_brow_raiseIn"),
            TEXT("CTRL_L_brow_raiseIn"), TEXT("CTRL_R_brow_raiseIn"), 1.0f});

        // AU2 - Outer Brow Raise (Frontalis, pars lateralis)
        Mappings.Add({TEXT("AU2"), TEXT("CTRL_L_brow_raiseOut"), TEXT("CTRL_R_brow_raiseOut"),
            TEXT("CTRL_L_brow_raiseOut"), TEXT("CTRL_R_brow_raiseOut"), 1.0f});

        // AU4 - Brow Lowerer (Corrugator supercilii, Depressor supercilii)
        Mappings.Add({TEXT("AU4"), TEXT("CTRL_L_brow_down"), TEXT("CTRL_R_brow_down"),
            TEXT("CTRL_L_brow_down"), TEXT("CTRL_R_brow_down"), 1.0f});

        // AU5 - Upper Lid Raise (Levator palpebrae superioris)
        Mappings.Add({TEXT("AU5"), TEXT("CTRL_L_eye_openUpperLid"), TEXT("CTRL_R_eye_openUpperLid"),
            TEXT("CTRL_L_eye_openUpperLid"), TEXT("CTRL_R_eye_openUpperLid"), 0.9f});

        // AU6 - Cheek Raise (Orbicularis oculi, pars orbitalis)
        Mappings.Add({TEXT("AU6"), TEXT("CTRL_L_eye_cheekRaise"), TEXT("CTRL_R_eye_cheekRaise"),
            TEXT("CTRL_L_eye_cheekRaise"), TEXT("CTRL_R_eye_cheekRaise"), 1.0f});

        // AU7 - Lid Tightener (Orbicularis oculi, pars palpebralis)
        Mappings.Add({TEXT("AU7"), TEXT("CTRL_L_eye_squintInner"), TEXT("CTRL_R_eye_squintInner"),
            TEXT("CTRL_L_eye_squintInner"), TEXT("CTRL_R_eye_squintInner"), 0.8f});

        // AU43 - Eyes Closed
        Mappings.Add({TEXT("AU43"), TEXT("CTRL_L_eye_blink"), TEXT("CTRL_R_eye_blink"),
            TEXT("CTRL_L_eye_blink"), TEXT("CTRL_R_eye_blink"), 1.0f});

        // AU45 - Blink
        Mappings.Add({TEXT("AU45"), TEXT("CTRL_L_eye_blink"), TEXT("CTRL_R_eye_blink"),
            TEXT("CTRL_L_eye_blink"), TEXT("CTRL_R_eye_blink"), 1.0f});

        // AU46 - Wink
        Mappings.Add({TEXT("AU46"), TEXT("CTRL_L_eye_blink"), TEXT(""),
            TEXT("CTRL_L_eye_blink"), TEXT(""), 1.0f});

        // === LOWER FACE ===

        // AU9 - Nose Wrinkle (Levator labii superioris alaeque nasi)
        Mappings.Add({TEXT("AU9"), TEXT("CTRL_L_nose_wrinkleUpper"), TEXT("CTRL_R_nose_wrinkleUpper"),
            TEXT("CTRL_L_nose_wrinkleUpper"), TEXT("CTRL_R_nose_wrinkleUpper"), 0.9f});

        // AU10 - Upper Lip Raise (Levator labii superioris)
        Mappings.Add({TEXT("AU10"), TEXT("CTRL_L_mouth_upperLipRaise"), TEXT("CTRL_R_mouth_upperLipRaise"),
            TEXT("CTRL_L_mouth_upperLipRaise"), TEXT("CTRL_R_mouth_upperLipRaise"), 0.85f});

        // AU11 - Nasolabial Deepener (Zygomaticus minor)
        Mappings.Add({TEXT("AU11"), TEXT("CTRL_L_mouth_nasolabialDeepen"), TEXT("CTRL_R_mouth_nasolabialDeepen"),
            TEXT("CTRL_L_mouth_nasolabialDeepen"), TEXT("CTRL_R_mouth_nasolabialDeepen"), 0.7f});

        // AU12 - Lip Corner Pull / Smile (Zygomaticus major)
        Mappings.Add({TEXT("AU12"), TEXT("CTRL_L_mouth_cornerPull"), TEXT("CTRL_R_mouth_cornerPull"),
            TEXT("CTRL_L_mouth_smile"), TEXT("CTRL_R_mouth_smile"), 1.0f});

        // AU13 - Sharp Lip Puller (Levator anguli oris)
        Mappings.Add({TEXT("AU13"), TEXT("CTRL_L_mouth_cornerSharpPull"), TEXT("CTRL_R_mouth_cornerSharpPull"),
            TEXT("CTRL_L_mouth_cornerSharpPull"), TEXT("CTRL_R_mouth_cornerSharpPull"), 0.75f});

        // AU14 - Dimpler (Buccinator)
        Mappings.Add({TEXT("AU14"), TEXT("CTRL_L_mouth_dimple"), TEXT("CTRL_R_mouth_dimple"),
            TEXT("CTRL_L_mouth_dimple"), TEXT("CTRL_R_mouth_dimple"), 0.6f});

        // AU15 - Lip Corner Depressor (Depressor anguli oris)
        Mappings.Add({TEXT("AU15"), TEXT("CTRL_L_mouth_cornerDepress"), TEXT("CTRL_R_mouth_cornerDepress"),
            TEXT("CTRL_L_mouth_frown"), TEXT("CTRL_R_mouth_frown"), 1.0f});

        // AU16 - Lower Lip Depressor (Depressor labii inferioris)
        Mappings.Add({TEXT("AU16"), TEXT("CTRL_L_mouth_lowerLipDepress"), TEXT("CTRL_R_mouth_lowerLipDepress"),
            TEXT("CTRL_L_mouth_lowerLipDepress"), TEXT("CTRL_R_mouth_lowerLipDepress"), 0.8f});

        // AU17 - Chin Raise (Mentalis)
        Mappings.Add({TEXT("AU17"), TEXT("CTRL_C_mouth_chinRaise"), TEXT("CTRL_C_mouth_chinRaise"),
            TEXT("CTRL_C_mouth_chinRaise"), TEXT("CTRL_C_mouth_chinRaise"), 0.7f});

        // AU18 - Lip Pucker (Incisivii labii, Orbicularis oris)
        Mappings.Add({TEXT("AU18"), TEXT("CTRL_C_mouth_pucker"), TEXT("CTRL_C_mouth_pucker"),
            TEXT("CTRL_C_mouth_pucker"), TEXT("CTRL_C_mouth_pucker"), 0.9f});

        // AU20 - Lip Stretcher (Risorius, Platysma)
        Mappings.Add({TEXT("AU20"), TEXT("CTRL_L_mouth_stretch"), TEXT("CTRL_R_mouth_stretch"),
            TEXT("CTRL_L_mouth_stretch"), TEXT("CTRL_R_mouth_stretch"), 0.8f});

        // AU22 - Lip Funneler (Orbicularis oris)
        Mappings.Add({TEXT("AU22"), TEXT("CTRL_C_mouth_lipsFunnel"), TEXT("CTRL_C_mouth_lipsFunnel"),
            TEXT("CTRL_C_mouth_lipsFunnel"), TEXT("CTRL_C_mouth_lipsFunnel"), 0.75f});

        // AU23 - Lip Tightener (Orbicularis oris)
        Mappings.Add({TEXT("AU23"), TEXT("CTRL_C_mouth_lipsTighten"), TEXT("CTRL_C_mouth_lipsTighten"),
            TEXT("CTRL_C_mouth_lipsTighten"), TEXT("CTRL_C_mouth_lipsTighten"), 0.7f});

        // AU24 - Lip Pressor (Orbicularis oris)
        Mappings.Add({TEXT("AU24"), TEXT("CTRL_C_mouth_lipsPress"), TEXT("CTRL_C_mouth_lipsPress"),
            TEXT("CTRL_C_mouth_lipsPress"), TEXT("CTRL_C_mouth_lipsPress"), 0.65f});

        // AU25 - Lips Part (Depressor labii, Mentalis)
        Mappings.Add({TEXT("AU25"), TEXT("CTRL_C_mouth_lipsPart"), TEXT("CTRL_C_mouth_lipsPart"),
            TEXT("CTRL_C_mouth_lipsPart"), TEXT("CTRL_C_mouth_lipsPart"), 0.8f});

        // AU26 - Jaw Drop (Masseter, Internal Pterygoid, Lateral Pterygoid)
        Mappings.Add({TEXT("AU26"), TEXT("CTRL_C_jaw_open"), TEXT("CTRL_C_jaw_open"),
            TEXT("CTRL_C_jaw_open"), TEXT("CTRL_C_jaw_open"), 0.9f});

        // AU27 - Mouth Stretch (Pterygoids, Digastric)
        Mappings.Add({TEXT("AU27"), TEXT("CTRL_C_jaw_openExtreme"), TEXT("CTRL_C_jaw_openExtreme"),
            TEXT("CTRL_C_jaw_openExtreme"), TEXT("CTRL_C_jaw_openExtreme"), 0.85f});

        // AU28 - Lip Suck (Mentalis)
        Mappings.Add({TEXT("AU28"), TEXT("CTRL_C_mouth_lipsSuck"), TEXT("CTRL_C_mouth_lipsSuck"),
            TEXT("CTRL_C_mouth_lipsSuck"), TEXT("CTRL_C_mouth_lipsSuck"), 0.6f});

        // === HEAD POSITION ===

        // AU51 - Head Turn Left
        Mappings.Add({TEXT("AU51"), TEXT("CTRL_C_head_turnLeft"), TEXT(""),
            TEXT("CTRL_C_head_turnLeft"), TEXT(""), 0.5f});

        // AU52 - Head Turn Right
        Mappings.Add({TEXT("AU52"), TEXT("CTRL_C_head_turnRight"), TEXT(""),
            TEXT("CTRL_C_head_turnRight"), TEXT(""), 0.5f});

        // AU53 - Head Up
        Mappings.Add({TEXT("AU53"), TEXT("CTRL_C_head_tiltUp"), TEXT(""),
            TEXT("CTRL_C_head_tiltUp"), TEXT(""), 0.5f});

        // AU54 - Head Down
        Mappings.Add({TEXT("AU54"), TEXT("CTRL_C_head_tiltDown"), TEXT(""),
            TEXT("CTRL_C_head_tiltDown"), TEXT(""), 0.5f});

        // AU55 - Head Tilt Left
        Mappings.Add({TEXT("AU55"), TEXT("CTRL_C_head_tiltLeft"), TEXT(""),
            TEXT("CTRL_C_head_tiltLeft"), TEXT(""), 0.4f});

        // AU56 - Head Tilt Right
        Mappings.Add({TEXT("AU56"), TEXT("CTRL_C_head_tiltRight"), TEXT(""),
            TEXT("CTRL_C_head_tiltRight"), TEXT(""), 0.4f});

        // === EYE GAZE ===

        // AU61 - Eyes Turn Left
        Mappings.Add({TEXT("AU61"), TEXT("CTRL_L_eye_lookLeft"), TEXT("CTRL_R_eye_lookLeft"),
            TEXT("CTRL_L_eye_lookLeft"), TEXT("CTRL_R_eye_lookLeft"), 0.7f});

        // AU62 - Eyes Turn Right
        Mappings.Add({TEXT("AU62"), TEXT("CTRL_L_eye_lookRight"), TEXT("CTRL_R_eye_lookRight"),
            TEXT("CTRL_L_eye_lookRight"), TEXT("CTRL_R_eye_lookRight"), 0.7f});

        // AU63 - Eyes Up
        Mappings.Add({TEXT("AU63"), TEXT("CTRL_L_eye_lookUp"), TEXT("CTRL_R_eye_lookUp"),
            TEXT("CTRL_L_eye_lookUp"), TEXT("CTRL_R_eye_lookUp"), 0.6f});

        // AU64 - Eyes Down
        Mappings.Add({TEXT("AU64"), TEXT("CTRL_L_eye_lookDown"), TEXT("CTRL_R_eye_lookDown"),
            TEXT("CTRL_L_eye_lookDown"), TEXT("CTRL_R_eye_lookDown"), 0.6f});

        return Mappings;
    }

    // ========================================================================
    // ENDOCRINE-TO-FACS MAPPING TABLE
    // Maps virtual endocrine hormone concentrations to FACS Action Units
    // ========================================================================

    static TArray<FEndocrineAUMapping> GetEndocrineToFACSMappings()
    {
        TArray<FEndocrineAUMapping> Mappings;

        // --- Cortisol (HPA Axis - Stress) ---
        Mappings.Add({TEXT("Cortisol"), TEXT("AU4"), 0.9f, 0.2f, TEXT("sigmoid")});    // Brow lowerer
        Mappings.Add({TEXT("Cortisol"), TEXT("AU1"), 0.6f, 0.3f, TEXT("linear")});     // Inner brow raise
        Mappings.Add({TEXT("Cortisol"), TEXT("AU15"), 0.7f, 0.25f, TEXT("sigmoid")});  // Lip corner depress
        Mappings.Add({TEXT("Cortisol"), TEXT("AU7"), 0.5f, 0.3f, TEXT("linear")});     // Lid tightener
        Mappings.Add({TEXT("Cortisol"), TEXT("AU23"), 0.4f, 0.35f, TEXT("linear")});   // Lip tightener

        // --- Dopamine Phasic (Reward) ---
        Mappings.Add({TEXT("DopaminePhasic"), TEXT("AU12"), 0.95f, 0.1f, TEXT("quadratic")}); // Smile
        Mappings.Add({TEXT("DopaminePhasic"), TEXT("AU6"), 0.85f, 0.15f, TEXT("quadratic")}); // Cheek raise (genuine)
        Mappings.Add({TEXT("DopaminePhasic"), TEXT("AU25"), 0.4f, 0.2f, TEXT("linear")});     // Lips part
        Mappings.Add({TEXT("DopaminePhasic"), TEXT("AU5"), 0.3f, 0.3f, TEXT("linear")});      // Upper lid raise

        // --- Dopamine Tonic (Baseline mood) ---
        Mappings.Add({TEXT("DopamineTonic"), TEXT("AU12"), 0.4f, 0.3f, TEXT("linear")});  // Subtle smile
        Mappings.Add({TEXT("DopamineTonic"), TEXT("AU6"), 0.3f, 0.35f, TEXT("linear")});  // Subtle cheek raise

        // --- Norepinephrine (Alertness) ---
        Mappings.Add({TEXT("Norepinephrine"), TEXT("AU5"), 0.8f, 0.15f, TEXT("sigmoid")});  // Upper lid raise
        Mappings.Add({TEXT("Norepinephrine"), TEXT("AU7"), 0.6f, 0.2f, TEXT("sigmoid")});   // Lid tightener
        Mappings.Add({TEXT("Norepinephrine"), TEXT("AU2"), 0.4f, 0.3f, TEXT("linear")});    // Outer brow raise
        Mappings.Add({TEXT("Norepinephrine"), TEXT("AU25"), 0.3f, 0.25f, TEXT("linear")});  // Lips part

        // --- Serotonin (Contentment/Calm) ---
        Mappings.Add({TEXT("Serotonin"), TEXT("AU12"), 0.5f, 0.2f, TEXT("sigmoid")});  // Gentle smile
        Mappings.Add({TEXT("Serotonin"), TEXT("AU6"), 0.4f, 0.25f, TEXT("sigmoid")});  // Gentle cheek raise

        // --- Oxytocin (Social bonding) ---
        Mappings.Add({TEXT("Oxytocin"), TEXT("AU6"), 0.8f, 0.1f, TEXT("quadratic")});   // Cheek raise (warm)
        Mappings.Add({TEXT("Oxytocin"), TEXT("AU12"), 0.7f, 0.1f, TEXT("quadratic")});  // Warm smile
        Mappings.Add({TEXT("Oxytocin"), TEXT("AU25"), 0.5f, 0.15f, TEXT("linear")});    // Lips part
        Mappings.Add({TEXT("Oxytocin"), TEXT("AU53"), 0.3f, 0.2f, TEXT("linear")});     // Head up (openness)

        // --- Melatonin (Circadian/Drowsiness) ---
        Mappings.Add({TEXT("Melatonin"), TEXT("AU43"), 0.9f, 0.3f, TEXT("sigmoid")});  // Eyes closed
        Mappings.Add({TEXT("Melatonin"), TEXT("AU54"), 0.4f, 0.4f, TEXT("linear")});   // Head down

        // --- Endorphin (Pain relief/Euphoria) ---
        Mappings.Add({TEXT("Endorphin"), TEXT("AU12"), 0.6f, 0.2f, TEXT("sigmoid")});  // Blissful smile
        Mappings.Add({TEXT("Endorphin"), TEXT("AU6"), 0.5f, 0.25f, TEXT("sigmoid")});  // Cheek raise

        // --- Adrenaline (Fight/Flight) ---
        Mappings.Add({TEXT("Adrenaline"), TEXT("AU5"), 0.95f, 0.1f, TEXT("sigmoid")});   // Wide eyes
        Mappings.Add({TEXT("Adrenaline"), TEXT("AU20"), 0.7f, 0.15f, TEXT("sigmoid")});  // Lip stretch
        Mappings.Add({TEXT("Adrenaline"), TEXT("AU26"), 0.5f, 0.2f, TEXT("linear")});    // Jaw drop
        Mappings.Add({TEXT("Adrenaline"), TEXT("AU1"), 0.6f, 0.15f, TEXT("sigmoid")});   // Inner brow raise

        // --- GABA (Inhibitory/Relaxation) ---
        Mappings.Add({TEXT("GABA"), TEXT("AU12"), 0.3f, 0.3f, TEXT("linear")});  // Relaxed smile

        // --- Testosterone (Dominance) ---
        Mappings.Add({TEXT("Testosterone"), TEXT("AU4"), 0.4f, 0.3f, TEXT("linear")});   // Brow lowerer (determined)
        Mappings.Add({TEXT("Testosterone"), TEXT("AU17"), 0.3f, 0.35f, TEXT("linear")});  // Chin raise

        return Mappings;
    }

    // ========================================================================
    // SUPERHOT GIRL AESTHETIC BIAS TABLE
    // Biases AU activations toward confident, charismatic expressions
    // ========================================================================

    static TMap<FName, float> GetSuperHotGirlAUBiases()
    {
        TMap<FName, float> Biases;

        // Confidence: subtle brow raise + slight smile
        Biases.Add(TEXT("AU2"), 0.08f);   // Subtle outer brow raise
        Biases.Add(TEXT("AU12"), 0.12f);  // Slight confident smile
        Biases.Add(TEXT("AU6"), 0.06f);   // Subtle cheek raise
        Biases.Add(TEXT("AU53"), 0.05f);  // Slight head up (confidence)

        // Eye sparkle: wider eyes with lid control
        Biases.Add(TEXT("AU5"), 0.10f);   // Upper lid raise (sparkle)
        Biases.Add(TEXT("AU7"), -0.05f);  // Reduce squint (open eyes)

        // Graceful: lip control
        Biases.Add(TEXT("AU25"), 0.04f);  // Slight lips part
        Biases.Add(TEXT("AU18"), 0.03f);  // Very subtle pucker

        // Charisma glow: reduce negative expressions
        Biases.Add(TEXT("AU15"), -0.10f); // Suppress lip corner depress
        Biases.Add(TEXT("AU4"), -0.08f);  // Suppress brow lowerer
        Biases.Add(TEXT("AU9"), -0.06f);  // Suppress nose wrinkle

        return Biases;
    }

    // ========================================================================
    // APPLY SIGMOID MAPPING CURVE
    // ========================================================================

    static float ApplyMappingCurve(float Input, float Sensitivity, float Threshold, const FString& CurveType)
    {
        if (Input < Threshold) return 0.0f;

        float Normalized = (Input - Threshold) / (1.0f - Threshold);

        if (CurveType == TEXT("linear"))
        {
            return FMath::Clamp(Normalized * Sensitivity, 0.0f, 1.0f);
        }
        else if (CurveType == TEXT("quadratic"))
        {
            return FMath::Clamp(Normalized * Normalized * Sensitivity, 0.0f, 1.0f);
        }
        else // sigmoid
        {
            float Steepness = 6.0f * Sensitivity;
            float Sigmoid = 1.0f / (1.0f + FMath::Exp(-Steepness * (Normalized - 0.5f)));
            return FMath::Clamp(Sigmoid, 0.0f, 1.0f);
        }
    }
};
