// FACSToMetaHumanMapping.cpp
// Complete FACS Action Unit to MetaHuman CTRL_ Morph Target Mapping
// Implementation of the full bidirectional mapping database

#include "FACSToMetaHumanMapping.h"

UFACSToMetaHumanMapping::UFACSToMetaHumanMapping()
{
}

void UFACSToMetaHumanMapping::InitializeMappings()
{
    AllMappings.Empty();
    InitializeUpperFaceMappings();
    InitializeLowerFaceMappings();
    InitializeEyeMappings();
    InitializeHeadMappings();
    InitializeMiscMappings();
    InitializeEmotionPresets();
}

void UFACSToMetaHumanMapping::AddMapping(const FName& AU, const FString& Desc,
                                          const FName& Ctrl, float Weight,
                                          bool bLeft, bool bRight)
{
    FFACSToCtrlMapping M;
    M.FACSActionUnit = AU;
    M.Description = Desc;
    M.CtrlMorphTarget = Ctrl;
    M.Weight = Weight;
    M.bLeftSide = bLeft;
    M.bRightSide = bRight;
    AllMappings.Add(M);
}

// ============================================================================
// UPPER FACE MAPPINGS (AU1-AU7)
// ============================================================================

void UFACSToMetaHumanMapping::InitializeUpperFaceMappings()
{
    // AU1 - Inner Brow Raise (Frontalis, pars medialis)
    AddMapping(FName("AU1"), TEXT("Inner Brow Raise"),
               FName("CTRL_L_brow_raiseIn"), 1.0f, true, false);
    AddMapping(FName("AU1"), TEXT("Inner Brow Raise"),
               FName("CTRL_R_brow_raiseIn"), 1.0f, false, true);

    // AU2 - Outer Brow Raise (Frontalis, pars lateralis)
    AddMapping(FName("AU2"), TEXT("Outer Brow Raise"),
               FName("CTRL_L_brow_raiseOut"), 1.0f, true, false);
    AddMapping(FName("AU2"), TEXT("Outer Brow Raise"),
               FName("CTRL_R_brow_raiseOut"), 1.0f, false, true);

    // AU4 - Brow Lowerer (Corrugator supercilii, Depressor supercilii)
    AddMapping(FName("AU4"), TEXT("Brow Lowerer"),
               FName("CTRL_L_brow_down"), 1.0f, true, false);
    AddMapping(FName("AU4"), TEXT("Brow Lowerer"),
               FName("CTRL_R_brow_down"), 1.0f, false, true);
    AddMapping(FName("AU4"), TEXT("Brow Lowerer"),
               FName("CTRL_L_brow_lateral"), 0.3f, true, false);
    AddMapping(FName("AU4"), TEXT("Brow Lowerer"),
               FName("CTRL_R_brow_lateral"), 0.3f, false, true);

    // AU5 - Upper Lid Raise (Levator palpebrae superioris)
    AddMapping(FName("AU5"), TEXT("Upper Lid Raise"),
               FName("CTRL_L_eye_lidUpperOpen"), 1.0f, true, false);
    AddMapping(FName("AU5"), TEXT("Upper Lid Raise"),
               FName("CTRL_R_eye_lidUpperOpen"), 1.0f, false, true);

    // AU6 - Cheek Raise (Orbicularis oculi, pars orbitalis)
    AddMapping(FName("AU6"), TEXT("Cheek Raise"),
               FName("CTRL_L_eye_cheekRaise"), 1.0f, true, false);
    AddMapping(FName("AU6"), TEXT("Cheek Raise"),
               FName("CTRL_R_eye_cheekRaise"), 1.0f, false, true);
    AddMapping(FName("AU6"), TEXT("Cheek Raise"),
               FName("CTRL_L_eye_squint"), 0.5f, true, false);
    AddMapping(FName("AU6"), TEXT("Cheek Raise"),
               FName("CTRL_R_eye_squint"), 0.5f, false, true);

    // AU7 - Lid Tightener (Orbicularis oculi, pars palpebralis)
    AddMapping(FName("AU7"), TEXT("Lid Tightener"),
               FName("CTRL_L_eye_squint"), 0.8f, true, false);
    AddMapping(FName("AU7"), TEXT("Lid Tightener"),
               FName("CTRL_R_eye_squint"), 0.8f, false, true);
    AddMapping(FName("AU7"), TEXT("Lid Tightener"),
               FName("CTRL_L_eye_lidLowerUp"), 0.5f, true, false);
    AddMapping(FName("AU7"), TEXT("Lid Tightener"),
               FName("CTRL_R_eye_lidLowerUp"), 0.5f, false, true);
}

// ============================================================================
// LOWER FACE MAPPINGS (AU9-AU28)
// ============================================================================

void UFACSToMetaHumanMapping::InitializeLowerFaceMappings()
{
    // AU9 - Nose Wrinkler (Levator labii superioris alaeque nasi)
    AddMapping(FName("AU9"), TEXT("Nose Wrinkler"),
               FName("CTRL_L_nose_wrinkleUpper"), 1.0f, true, false);
    AddMapping(FName("AU9"), TEXT("Nose Wrinkler"),
               FName("CTRL_R_nose_wrinkleUpper"), 1.0f, false, true);

    // AU10 - Upper Lip Raiser (Levator labii superioris)
    AddMapping(FName("AU10"), TEXT("Upper Lip Raiser"),
               FName("CTRL_L_mouth_upperLipRaise"), 1.0f, true, false);
    AddMapping(FName("AU10"), TEXT("Upper Lip Raiser"),
               FName("CTRL_R_mouth_upperLipRaise"), 1.0f, false, true);

    // AU11 - Nasolabial Deepener (Zygomaticus minor)
    AddMapping(FName("AU11"), TEXT("Nasolabial Deepener"),
               FName("CTRL_L_nasolabialDeepener"), 1.0f, true, false);
    AddMapping(FName("AU11"), TEXT("Nasolabial Deepener"),
               FName("CTRL_R_nasolabialDeepener"), 1.0f, false, true);

    // AU12 - Lip Corner Puller (Zygomaticus major) - THE SMILE
    AddMapping(FName("AU12"), TEXT("Lip Corner Puller"),
               FName("CTRL_L_mouth_cornerPull"), 1.0f, true, false);
    AddMapping(FName("AU12"), TEXT("Lip Corner Puller"),
               FName("CTRL_R_mouth_cornerPull"), 1.0f, false, true);

    // AU13 - Sharp Lip Puller (Levator anguli oris)
    AddMapping(FName("AU13"), TEXT("Sharp Lip Puller"),
               FName("CTRL_L_mouth_sharpCornerPull"), 1.0f, true, false);
    AddMapping(FName("AU13"), TEXT("Sharp Lip Puller"),
               FName("CTRL_R_mouth_sharpCornerPull"), 1.0f, false, true);

    // AU14 - Dimpler (Buccinator)
    AddMapping(FName("AU14"), TEXT("Dimpler"),
               FName("CTRL_L_mouth_dimple"), 1.0f, true, false);
    AddMapping(FName("AU14"), TEXT("Dimpler"),
               FName("CTRL_R_mouth_dimple"), 1.0f, false, true);

    // AU15 - Lip Corner Depressor (Depressor anguli oris)
    AddMapping(FName("AU15"), TEXT("Lip Corner Depressor"),
               FName("CTRL_L_mouth_cornerDepress"), 1.0f, true, false);
    AddMapping(FName("AU15"), TEXT("Lip Corner Depressor"),
               FName("CTRL_R_mouth_cornerDepress"), 1.0f, false, true);

    // AU16 - Lower Lip Depressor (Depressor labii inferioris)
    AddMapping(FName("AU16"), TEXT("Lower Lip Depressor"),
               FName("CTRL_L_mouth_lowerLipDepress"), 1.0f, true, false);
    AddMapping(FName("AU16"), TEXT("Lower Lip Depressor"),
               FName("CTRL_R_mouth_lowerLipDepress"), 1.0f, false, true);

    // AU17 - Chin Raiser (Mentalis)
    AddMapping(FName("AU17"), TEXT("Chin Raiser"),
               FName("CTRL_C_mouth_chinRaise"), 1.0f);

    // AU18 - Lip Puckerer (Incisivii labii superioris and inferioris)
    AddMapping(FName("AU18"), TEXT("Lip Puckerer"),
               FName("CTRL_C_mouth_pucker"), 1.0f);
    AddMapping(FName("AU18"), TEXT("Lip Puckerer"),
               FName("CTRL_L_mouth_lipsPurse"), 0.5f, true, false);
    AddMapping(FName("AU18"), TEXT("Lip Puckerer"),
               FName("CTRL_R_mouth_lipsPurse"), 0.5f, false, true);

    // AU20 - Lip Stretcher (Risorius with platysma)
    AddMapping(FName("AU20"), TEXT("Lip Stretcher"),
               FName("CTRL_L_mouth_stretch"), 1.0f, true, false);
    AddMapping(FName("AU20"), TEXT("Lip Stretcher"),
               FName("CTRL_R_mouth_stretch"), 1.0f, false, true);

    // AU22 - Lip Funneler (Orbicularis oris)
    AddMapping(FName("AU22"), TEXT("Lip Funneler"),
               FName("CTRL_C_mouth_lipsFunnel"), 1.0f);

    // AU23 - Lip Tightener (Orbicularis oris)
    AddMapping(FName("AU23"), TEXT("Lip Tightener"),
               FName("CTRL_L_mouth_tighten"), 1.0f, true, false);
    AddMapping(FName("AU23"), TEXT("Lip Tightener"),
               FName("CTRL_R_mouth_tighten"), 1.0f, false, true);

    // AU24 - Lip Pressor (Orbicularis oris)
    AddMapping(FName("AU24"), TEXT("Lip Pressor"),
               FName("CTRL_C_mouth_lipsPress"), 1.0f);

    // AU25 - Lips Part (Depressor labii, or mentalis)
    AddMapping(FName("AU25"), TEXT("Lips Part"),
               FName("CTRL_C_mouth_lipsPart"), 1.0f);

    // AU26 - Jaw Drop (Masseter, internal pterygoid, digastric)
    AddMapping(FName("AU26"), TEXT("Jaw Drop"),
               FName("CTRL_C_jaw_open"), 0.5f);

    // AU27 - Mouth Stretch (Pterygoids, digastric)
    AddMapping(FName("AU27"), TEXT("Mouth Stretch"),
               FName("CTRL_C_jaw_open"), 1.0f);

    // AU28 - Lip Suck (Mentalis)
    AddMapping(FName("AU28"), TEXT("Lip Suck"),
               FName("CTRL_C_mouth_lipsSuck"), 1.0f);
}

// ============================================================================
// EYE AND LID MAPPINGS (AU41-AU46)
// ============================================================================

void UFACSToMetaHumanMapping::InitializeEyeMappings()
{
    // AU41 - Lid Droop
    AddMapping(FName("AU41"), TEXT("Lid Droop"),
               FName("CTRL_L_eye_lidUpperClose"), 0.5f, true, false);
    AddMapping(FName("AU41"), TEXT("Lid Droop"),
               FName("CTRL_R_eye_lidUpperClose"), 0.5f, false, true);

    // AU42 - Slit
    AddMapping(FName("AU42"), TEXT("Slit"),
               FName("CTRL_L_eye_lidUpperClose"), 0.7f, true, false);
    AddMapping(FName("AU42"), TEXT("Slit"),
               FName("CTRL_R_eye_lidUpperClose"), 0.7f, false, true);
    AddMapping(FName("AU42"), TEXT("Slit"),
               FName("CTRL_L_eye_lidLowerUp"), 0.3f, true, false);
    AddMapping(FName("AU42"), TEXT("Slit"),
               FName("CTRL_R_eye_lidLowerUp"), 0.3f, false, true);

    // AU43 - Eyes Closed
    AddMapping(FName("AU43"), TEXT("Eyes Closed"),
               FName("CTRL_L_eye_lidUpperClose"), 1.0f, true, false);
    AddMapping(FName("AU43"), TEXT("Eyes Closed"),
               FName("CTRL_R_eye_lidUpperClose"), 1.0f, false, true);

    // AU44 - Squint
    AddMapping(FName("AU44"), TEXT("Squint"),
               FName("CTRL_L_eye_squint"), 1.0f, true, false);
    AddMapping(FName("AU44"), TEXT("Squint"),
               FName("CTRL_R_eye_squint"), 1.0f, false, true);

    // AU45 - Blink
    AddMapping(FName("AU45"), TEXT("Blink"),
               FName("CTRL_L_eye_blink"), 1.0f, true, false);
    AddMapping(FName("AU45"), TEXT("Blink"),
               FName("CTRL_R_eye_blink"), 1.0f, false, true);

    // AU46 - Wink
    AddMapping(FName("AU46"), TEXT("Wink"),
               FName("CTRL_R_eye_blink"), 1.0f, false, true);

    // Eye look directions (AU61-AU64)
    AddMapping(FName("AU61"), TEXT("Eyes Turn Left"),
               FName("CTRL_L_eye_lookLeft"), 1.0f, true, false);
    AddMapping(FName("AU61"), TEXT("Eyes Turn Left"),
               FName("CTRL_R_eye_lookLeft"), 1.0f, false, true);

    AddMapping(FName("AU62"), TEXT("Eyes Turn Right"),
               FName("CTRL_L_eye_lookRight"), 1.0f, true, false);
    AddMapping(FName("AU62"), TEXT("Eyes Turn Right"),
               FName("CTRL_R_eye_lookRight"), 1.0f, false, true);

    AddMapping(FName("AU63"), TEXT("Eyes Up"),
               FName("CTRL_L_eye_lookUp"), 1.0f, true, false);
    AddMapping(FName("AU63"), TEXT("Eyes Up"),
               FName("CTRL_R_eye_lookUp"), 1.0f, false, true);

    AddMapping(FName("AU64"), TEXT("Eyes Down"),
               FName("CTRL_L_eye_lookDown"), 1.0f, true, false);
    AddMapping(FName("AU64"), TEXT("Eyes Down"),
               FName("CTRL_R_eye_lookDown"), 1.0f, false, true);
}

// ============================================================================
// HEAD AND NECK MAPPINGS (AU51-AU58)
// ============================================================================

void UFACSToMetaHumanMapping::InitializeHeadMappings()
{
    // AU51 - Head Turn Left
    AddMapping(FName("AU51"), TEXT("Head Turn Left"),
               FName("CTRL_C_head_turnLeft"), 1.0f);

    // AU52 - Head Turn Right
    AddMapping(FName("AU52"), TEXT("Head Turn Right"),
               FName("CTRL_C_head_turnRight"), 1.0f);

    // AU53 - Head Up
    AddMapping(FName("AU53"), TEXT("Head Up"),
               FName("CTRL_C_head_tiltUp"), 1.0f);

    // AU54 - Head Down
    AddMapping(FName("AU54"), TEXT("Head Down"),
               FName("CTRL_C_head_tiltDown"), 1.0f);

    // AU55 - Head Tilt Left
    AddMapping(FName("AU55"), TEXT("Head Tilt Left"),
               FName("CTRL_C_head_tiltLeft"), 1.0f);

    // AU56 - Head Tilt Right
    AddMapping(FName("AU56"), TEXT("Head Tilt Right"),
               FName("CTRL_C_head_tiltRight"), 1.0f);

    // AU57 - Head Forward
    AddMapping(FName("AU57"), TEXT("Head Forward"),
               FName("CTRL_C_head_forward"), 1.0f);

    // AU58 - Head Back
    AddMapping(FName("AU58"), TEXT("Head Back"),
               FName("CTRL_C_head_back"), 1.0f);
}

// ============================================================================
// MISCELLANEOUS MAPPINGS
// ============================================================================

void UFACSToMetaHumanMapping::InitializeMiscMappings()
{
    // Jaw lateral movement
    AddMapping(FName("AU30"), TEXT("Jaw Sideways Left"),
               FName("CTRL_C_jaw_left"), 1.0f);
    AddMapping(FName("AU30R"), TEXT("Jaw Sideways Right"),
               FName("CTRL_C_jaw_right"), 1.0f);

    // Jaw thrust
    AddMapping(FName("AU29"), TEXT("Jaw Thrust"),
               FName("CTRL_C_jaw_forward"), 1.0f);

    // Tongue
    AddMapping(FName("AU19"), TEXT("Tongue Show"),
               FName("CTRL_C_tongue_out"), 1.0f);

    // Nostril
    AddMapping(FName("AU38"), TEXT("Nostril Dilator"),
               FName("CTRL_L_nose_nostrilDilate"), 1.0f, true, false);
    AddMapping(FName("AU38"), TEXT("Nostril Dilator"),
               FName("CTRL_R_nose_nostrilDilate"), 1.0f, false, true);

    AddMapping(FName("AU39"), TEXT("Nostril Compressor"),
               FName("CTRL_L_nose_nostrilCompress"), 1.0f, true, false);
    AddMapping(FName("AU39"), TEXT("Nostril Compressor"),
               FName("CTRL_R_nose_nostrilCompress"), 1.0f, false, true);

    // Cheek
    AddMapping(FName("AU34"), TEXT("Cheek Puff"),
               FName("CTRL_L_mouth_cheekPuff"), 1.0f, true, false);
    AddMapping(FName("AU34"), TEXT("Cheek Puff"),
               FName("CTRL_R_mouth_cheekPuff"), 1.0f, false, true);

    AddMapping(FName("AU35"), TEXT("Cheek Suck"),
               FName("CTRL_L_mouth_cheekSuck"), 1.0f, true, false);
    AddMapping(FName("AU35"), TEXT("Cheek Suck"),
               FName("CTRL_R_mouth_cheekSuck"), 1.0f, false, true);
}

// ============================================================================
// EMOTION PRESETS
// ============================================================================

void UFACSToMetaHumanMapping::InitializeEmotionPresets()
{
    EmotionPresets.Empty();

    // Happiness (Duchenne smile)
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Happiness");
        P.ActionUnitIntensities.Add(FName("AU6"), 0.8f);
        P.ActionUnitIntensities.Add(FName("AU12"), 1.0f);
        P.ActionUnitIntensities.Add(FName("AU25"), 0.3f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Sadness
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Sadness");
        P.ActionUnitIntensities.Add(FName("AU1"), 0.7f);
        P.ActionUnitIntensities.Add(FName("AU4"), 0.5f);
        P.ActionUnitIntensities.Add(FName("AU15"), 0.8f);
        P.ActionUnitIntensities.Add(FName("AU17"), 0.3f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Surprise
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Surprise");
        P.ActionUnitIntensities.Add(FName("AU1"), 0.9f);
        P.ActionUnitIntensities.Add(FName("AU2"), 0.9f);
        P.ActionUnitIntensities.Add(FName("AU5"), 0.7f);
        P.ActionUnitIntensities.Add(FName("AU26"), 0.6f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Fear
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Fear");
        P.ActionUnitIntensities.Add(FName("AU1"), 0.8f);
        P.ActionUnitIntensities.Add(FName("AU2"), 0.6f);
        P.ActionUnitIntensities.Add(FName("AU4"), 0.7f);
        P.ActionUnitIntensities.Add(FName("AU5"), 0.9f);
        P.ActionUnitIntensities.Add(FName("AU7"), 0.6f);
        P.ActionUnitIntensities.Add(FName("AU20"), 0.7f);
        P.ActionUnitIntensities.Add(FName("AU26"), 0.5f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Anger
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Anger");
        P.ActionUnitIntensities.Add(FName("AU4"), 0.9f);
        P.ActionUnitIntensities.Add(FName("AU5"), 0.6f);
        P.ActionUnitIntensities.Add(FName("AU7"), 0.8f);
        P.ActionUnitIntensities.Add(FName("AU23"), 0.7f);
        P.ActionUnitIntensities.Add(FName("AU24"), 0.5f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Disgust
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Disgust");
        P.ActionUnitIntensities.Add(FName("AU9"), 0.8f);
        P.ActionUnitIntensities.Add(FName("AU10"), 0.5f);
        P.ActionUnitIntensities.Add(FName("AU15"), 0.6f);
        P.ActionUnitIntensities.Add(FName("AU17"), 0.5f);
        P.ActionUnitIntensities.Add(FName("AU25"), 0.3f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Contempt (asymmetric)
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Contempt");
        P.ActionUnitIntensities.Add(FName("AU12R"), 0.5f);
        P.ActionUnitIntensities.Add(FName("AU14R"), 0.4f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Confidence (SuperHotGirl signature)
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Confidence");
        P.ActionUnitIntensities.Add(FName("AU12"), 0.4f);
        P.ActionUnitIntensities.Add(FName("AU6"), 0.3f);
        P.ActionUnitIntensities.Add(FName("AU2"), 0.2f);
        P.ActionUnitIntensities.Add(FName("AU5"), 0.15f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Mischief (HyperChaotic signature)
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Mischief");
        P.ActionUnitIntensities.Add(FName("AU12"), 0.6f);
        P.ActionUnitIntensities.Add(FName("AU2"), 0.4f);
        P.ActionUnitIntensities.Add(FName("AU7"), 0.3f);
        P.ActionUnitIntensities.Add(FName("AU14"), 0.3f);
        EmotionPresets.Add(P.EmotionName, P);
    }

    // Ethereal (AI Angel signature)
    {
        FEmotionFACSPreset P;
        P.EmotionName = FName("Ethereal");
        P.ActionUnitIntensities.Add(FName("AU12"), 0.3f);
        P.ActionUnitIntensities.Add(FName("AU6"), 0.2f);
        P.ActionUnitIntensities.Add(FName("AU5"), 0.25f);
        P.ActionUnitIntensities.Add(FName("AU1"), 0.15f);
        P.ActionUnitIntensities.Add(FName("AU25"), 0.2f);
        EmotionPresets.Add(P.EmotionName, P);
    }
}

// ============================================================================
// CONVERSION API
// ============================================================================

TArray<FFACSToCtrlMapping> UFACSToMetaHumanMapping::GetCtrlTargetsForAU(FName ActionUnit) const
{
    TArray<FFACSToCtrlMapping> Result;
    for (const FFACSToCtrlMapping& M : AllMappings)
    {
        if (M.FACSActionUnit == ActionUnit)
        {
            Result.Add(M);
        }
    }
    return Result;
}

TArray<FFACSToCtrlMapping> UFACSToMetaHumanMapping::GetAUsForCtrlTarget(FName CtrlTarget) const
{
    TArray<FFACSToCtrlMapping> Result;
    for (const FFACSToCtrlMapping& M : AllMappings)
    {
        if (M.CtrlMorphTarget == CtrlTarget)
        {
            Result.Add(M);
        }
    }
    return Result;
}

TMap<FName, float> UFACSToMetaHumanMapping::ConvertFACSToCtrl(
    const TMap<FName, float>& FACSActivations) const
{
    TMap<FName, float> CtrlValues;

    for (const auto& Pair : FACSActivations)
    {
        TArray<FFACSToCtrlMapping> Targets = GetCtrlTargetsForAU(Pair.Key);
        for (const FFACSToCtrlMapping& Target : Targets)
        {
            float Contribution = Pair.Value * Target.Weight;
            float* Existing = CtrlValues.Find(Target.CtrlMorphTarget);
            if (Existing)
            {
                *Existing = FMath::Min(1.0f, *Existing + Contribution);
            }
            else
            {
                CtrlValues.Add(Target.CtrlMorphTarget, FMath::Min(1.0f, Contribution));
            }
        }
    }

    return CtrlValues;
}

TMap<FName, float> UFACSToMetaHumanMapping::ConvertCtrlToFACS(
    const TMap<FName, float>& CtrlValues) const
{
    TMap<FName, float> FACSActivations;

    for (const auto& Pair : CtrlValues)
    {
        TArray<FFACSToCtrlMapping> AUs = GetAUsForCtrlTarget(Pair.Key);
        for (const FFACSToCtrlMapping& AU : AUs)
        {
            float Contribution = (AU.Weight > 0.001f) ? Pair.Value / AU.Weight : 0.0f;
            float* Existing = FACSActivations.Find(AU.FACSActionUnit);
            if (Existing)
            {
                *Existing = FMath::Max(*Existing, Contribution);
            }
            else
            {
                FACSActivations.Add(AU.FACSActionUnit, FMath::Min(1.0f, Contribution));
            }
        }
    }

    return FACSActivations;
}

FEmotionFACSPreset UFACSToMetaHumanMapping::GetEmotionPreset(FName EmotionName) const
{
    const FEmotionFACSPreset* Preset = EmotionPresets.Find(EmotionName);
    return Preset ? *Preset : FEmotionFACSPreset();
}

TArray<FName> UFACSToMetaHumanMapping::GetAvailableEmotions() const
{
    TArray<FName> Emotions;
    EmotionPresets.GetKeys(Emotions);
    return Emotions;
}
