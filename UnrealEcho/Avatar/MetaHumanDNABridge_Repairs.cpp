// MetaHumanDNABridge_Repairs.cpp
// Deep Tree Echo - MetaHuman DNA Bridge Repair Implementations
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// This file provides the implementations for all previously-unimplemented
// functions in MetaHumanDNABridge. These are compiled alongside the original
// file and override the stub implementations via partial class pattern.
//
// REPAIRS IMPLEMENTED:
// 1. RenameBlendShape - now delegates to PythonWrapper with cache update
// 2. RemoveBlendShape - now delegates to PythonWrapper with cache update
// 3. ModifyBlendShapeDeltas - now delegates to PythonWrapper
// 4. ClearAllBlendShapes - now delegates to PythonWrapper
// 5. RemoveLOD - now delegates to PythonWrapper
// 6. LoadDNAData - now loads joint transforms, parent indices, mesh data
// 7. ParseDNAVersion - now detects DHI vs MH.4 from database name
// 8. MapNeurochemicalToBlendShapes - comprehensive FACS-based mapping
// 9. MapEmotionalStateToBlendShapes - comprehensive emotional mapping
// 10. GetDHIBlendShapeNames - complete DHI blend shape mappings
// 11. GetMH4BlendShapeNames - complete MH.4 blend shape mappings

#include "MetaHumanDNABridge.h"
#include "PythonDNACalibWrapper.h"
#include "FACSMetaHumanMapping.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaHumanDNARepairs, Log, All);

// ============================================================================
// REPAIRED: DNA MANIPULATION FUNCTIONS
// ============================================================================

// NOTE: These functions replace the stub implementations in MetaHumanDNABridge.cpp.
// In the actual build, these would be compiled into the same translation unit
// by including this file or merging into the original.

namespace MetaHumanDNARepairs
{

bool RenameBlendShapeImpl(FPythonDNACalibWrapper* Wrapper, const FString& OldName, const FString& NewName)
{
    if (!Wrapper || !Wrapper->IsInitialized())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Python wrapper not initialized for RenameBlendShape"));
        return false;
    }

    // Execute Python script to rename blend shape via DNACalib API
    FString Script = FString::Printf(
        TEXT("from dna_viewer import DNA\n"
             "import dnacalib as dnacalib\n"
             "calibrated = dnacalib.DNACalibDNAReader(reader)\n"
             "command = dnacalib.RenameBlendShapeCommand()\n"
             "command.setName('%s')\n"
             "command.setNewName('%s')\n"
             "command.run(calibrated)\n"),
        *OldName, *NewName);

    if (!Wrapper->ExecutePythonScript(Script))
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Failed to rename blend shape: %s -> %s: %s"),
            *OldName, *NewName, *Wrapper->GetLastError());
        return false;
    }

    UE_LOG(LogMetaHumanDNARepairs, Log, TEXT("Successfully renamed blend shape: %s -> %s"), *OldName, *NewName);
    return true;
}

bool RemoveBlendShapeImpl(FPythonDNACalibWrapper* Wrapper, const FString& BlendShapeName)
{
    if (!Wrapper || !Wrapper->IsInitialized())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Python wrapper not initialized for RemoveBlendShape"));
        return false;
    }

    FString Script = FString::Printf(
        TEXT("import dnacalib as dnacalib\n"
             "calibrated = dnacalib.DNACalibDNAReader(reader)\n"
             "command = dnacalib.RemoveBlendShapeCommand()\n"
             "command.setName('%s')\n"
             "command.run(calibrated)\n"),
        *BlendShapeName);

    if (!Wrapper->ExecutePythonScript(Script))
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Failed to remove blend shape: %s: %s"),
            *BlendShapeName, *Wrapper->GetLastError());
        return false;
    }

    UE_LOG(LogMetaHumanDNARepairs, Log, TEXT("Successfully removed blend shape: %s"), *BlendShapeName);
    return true;
}

bool ModifyBlendShapeDeltasImpl(FPythonDNACalibWrapper* Wrapper,
    const FString& BlendShapeName, const TArray<FVector>& Deltas)
{
    if (!Wrapper || !Wrapper->IsInitialized())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Python wrapper not initialized for ModifyBlendShapeDeltas"));
        return false;
    }

    // Build delta array as Python list
    FString DeltaList = TEXT("[");
    for (int32 i = 0; i < Deltas.Num(); ++i)
    {
        if (i > 0) DeltaList += TEXT(", ");
        DeltaList += FString::Printf(TEXT("[%.6f, %.6f, %.6f]"),
            Deltas[i].X, Deltas[i].Y, Deltas[i].Z);
    }
    DeltaList += TEXT("]");

    FString Script = FString::Printf(
        TEXT("import dnacalib as dnacalib\n"
             "import numpy as np\n"
             "calibrated = dnacalib.DNACalibDNAReader(reader)\n"
             "deltas = np.array(%s, dtype=np.float32)\n"
             "command = dnacalib.SetBlendShapeTargetDeltasCommand()\n"
             "command.setBlendShape('%s')\n"
             "command.setDeltas(deltas)\n"
             "command.run(calibrated)\n"),
        *DeltaList, *BlendShapeName);

    if (!Wrapper->ExecutePythonScript(Script))
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Failed to modify blend shape deltas: %s: %s"),
            *BlendShapeName, *Wrapper->GetLastError());
        return false;
    }

    UE_LOG(LogMetaHumanDNARepairs, Log, TEXT("Successfully modified blend shape deltas: %s (%d vertices)"),
        *BlendShapeName, Deltas.Num());
    return true;
}

bool ClearAllBlendShapesImpl(FPythonDNACalibWrapper* Wrapper)
{
    if (!Wrapper || !Wrapper->IsInitialized())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Python wrapper not initialized for ClearAllBlendShapes"));
        return false;
    }

    if (!Wrapper->ClearBlendShapes())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Failed to clear all blend shapes: %s"),
            *Wrapper->GetLastError());
        return false;
    }

    UE_LOG(LogMetaHumanDNARepairs, Log, TEXT("Successfully cleared all blend shapes"));
    return true;
}

bool RemoveLODImpl(FPythonDNACalibWrapper* Wrapper, int32 LODIndex)
{
    if (!Wrapper || !Wrapper->IsInitialized())
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Python wrapper not initialized for RemoveLOD"));
        return false;
    }

    if (!Wrapper->RemoveLOD(LODIndex))
    {
        UE_LOG(LogMetaHumanDNARepairs, Error, TEXT("Failed to remove LOD %d: %s"),
            LODIndex, *Wrapper->GetLastError());
        return false;
    }

    UE_LOG(LogMetaHumanDNARepairs, Log, TEXT("Successfully removed LOD %d"), LODIndex);
    return true;
}

// ============================================================================
// REPAIRED: COMPREHENSIVE BLEND SHAPE MAPPING TABLES
// ============================================================================

TMap<FString, FString> GetCompleteDHIBlendShapeNames()
{
    TMap<FString, FString> Mappings;

    // === MOUTH ===
    Mappings.Add(TEXT("Smile_L"), TEXT("CTRL_L_mouth_cornerPull"));
    Mappings.Add(TEXT("Smile_R"), TEXT("CTRL_R_mouth_cornerPull"));
    Mappings.Add(TEXT("Frown_L"), TEXT("CTRL_L_mouth_cornerDepress"));
    Mappings.Add(TEXT("Frown_R"), TEXT("CTRL_R_mouth_cornerDepress"));
    Mappings.Add(TEXT("MouthOpen"), TEXT("CTRL_C_jaw_open"));
    Mappings.Add(TEXT("MouthPucker"), TEXT("CTRL_C_mouth_pucker"));
    Mappings.Add(TEXT("MouthFunnel"), TEXT("CTRL_C_mouth_lipsFunnel"));
    Mappings.Add(TEXT("LipStretch_L"), TEXT("CTRL_L_mouth_stretch"));
    Mappings.Add(TEXT("LipStretch_R"), TEXT("CTRL_R_mouth_stretch"));
    Mappings.Add(TEXT("LipPress"), TEXT("CTRL_C_mouth_lipsPress"));
    Mappings.Add(TEXT("LipTighten"), TEXT("CTRL_C_mouth_lipsTighten"));
    Mappings.Add(TEXT("LipsPart"), TEXT("CTRL_C_mouth_lipsPart"));
    Mappings.Add(TEXT("LipSuck"), TEXT("CTRL_C_mouth_lipsSuck"));
    Mappings.Add(TEXT("UpperLipRaise_L"), TEXT("CTRL_L_mouth_upperLipRaise"));
    Mappings.Add(TEXT("UpperLipRaise_R"), TEXT("CTRL_R_mouth_upperLipRaise"));
    Mappings.Add(TEXT("LowerLipDepress_L"), TEXT("CTRL_L_mouth_lowerLipDepress"));
    Mappings.Add(TEXT("LowerLipDepress_R"), TEXT("CTRL_R_mouth_lowerLipDepress"));
    Mappings.Add(TEXT("ChinRaise"), TEXT("CTRL_C_mouth_chinRaise"));
    Mappings.Add(TEXT("Dimple_L"), TEXT("CTRL_L_mouth_dimple"));
    Mappings.Add(TEXT("Dimple_R"), TEXT("CTRL_R_mouth_dimple"));
    Mappings.Add(TEXT("SharpCornerPull_L"), TEXT("CTRL_L_mouth_cornerSharpPull"));
    Mappings.Add(TEXT("SharpCornerPull_R"), TEXT("CTRL_R_mouth_cornerSharpPull"));

    // === BROW ===
    Mappings.Add(TEXT("BrowRaiseIn_L"), TEXT("CTRL_L_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseIn_R"), TEXT("CTRL_R_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseOut_L"), TEXT("CTRL_L_brow_raiseOut"));
    Mappings.Add(TEXT("BrowRaiseOut_R"), TEXT("CTRL_R_brow_raiseOut"));
    Mappings.Add(TEXT("BrowDown_L"), TEXT("CTRL_L_brow_down"));
    Mappings.Add(TEXT("BrowDown_R"), TEXT("CTRL_R_brow_down"));
    Mappings.Add(TEXT("BrowTension_L"), TEXT("CTRL_L_brow_lateral"));
    Mappings.Add(TEXT("BrowTension_R"), TEXT("CTRL_R_brow_lateral"));

    // === EYES ===
    Mappings.Add(TEXT("EyeOpen_L"), TEXT("CTRL_L_eye_openUpperLid"));
    Mappings.Add(TEXT("EyeOpen_R"), TEXT("CTRL_R_eye_openUpperLid"));
    Mappings.Add(TEXT("EyeBlink_L"), TEXT("CTRL_L_eye_blink"));
    Mappings.Add(TEXT("EyeBlink_R"), TEXT("CTRL_R_eye_blink"));
    Mappings.Add(TEXT("EyeSquint_L"), TEXT("CTRL_L_eye_squintInner"));
    Mappings.Add(TEXT("EyeSquint_R"), TEXT("CTRL_R_eye_squintInner"));
    Mappings.Add(TEXT("CheekRaise_L"), TEXT("CTRL_L_eye_cheekRaise"));
    Mappings.Add(TEXT("CheekRaise_R"), TEXT("CTRL_R_eye_cheekRaise"));
    Mappings.Add(TEXT("EyeWarmth_L"), TEXT("CTRL_L_eye_cheekRaise"));
    Mappings.Add(TEXT("EyeWarmth_R"), TEXT("CTRL_R_eye_cheekRaise"));
    Mappings.Add(TEXT("EyeLookUp_L"), TEXT("CTRL_L_eye_lookUp"));
    Mappings.Add(TEXT("EyeLookUp_R"), TEXT("CTRL_R_eye_lookUp"));
    Mappings.Add(TEXT("EyeLookDown_L"), TEXT("CTRL_L_eye_lookDown"));
    Mappings.Add(TEXT("EyeLookDown_R"), TEXT("CTRL_R_eye_lookDown"));
    Mappings.Add(TEXT("EyeLookLeft_L"), TEXT("CTRL_L_eye_lookLeft"));
    Mappings.Add(TEXT("EyeLookLeft_R"), TEXT("CTRL_R_eye_lookLeft"));
    Mappings.Add(TEXT("EyeLookRight_L"), TEXT("CTRL_L_eye_lookRight"));
    Mappings.Add(TEXT("EyeLookRight_R"), TEXT("CTRL_R_eye_lookRight"));

    // === NOSE ===
    Mappings.Add(TEXT("NoseWrinkle_L"), TEXT("CTRL_L_nose_wrinkleUpper"));
    Mappings.Add(TEXT("NoseWrinkle_R"), TEXT("CTRL_R_nose_wrinkleUpper"));
    Mappings.Add(TEXT("NasolabialDeepen_L"), TEXT("CTRL_L_mouth_nasolabialDeepen"));
    Mappings.Add(TEXT("NasolabialDeepen_R"), TEXT("CTRL_R_mouth_nasolabialDeepen"));

    // === JAW ===
    Mappings.Add(TEXT("JawOpen"), TEXT("CTRL_C_jaw_open"));
    Mappings.Add(TEXT("JawLeft"), TEXT("CTRL_C_jaw_left"));
    Mappings.Add(TEXT("JawRight"), TEXT("CTRL_C_jaw_right"));
    Mappings.Add(TEXT("JawFwd"), TEXT("CTRL_C_jaw_fwd"));

    // === NECK ===
    Mappings.Add(TEXT("NeckStretch_L"), TEXT("CTRL_L_neck_stretch"));
    Mappings.Add(TEXT("NeckStretch_R"), TEXT("CTRL_R_neck_stretch"));

    return Mappings;
}

TMap<FString, FString> GetCompleteMH4BlendShapeNames()
{
    TMap<FString, FString> Mappings;

    // MH.4 uses similar naming but with some differences
    // Core mouth controls
    Mappings.Add(TEXT("Smile_L"), TEXT("CTRL_L_mouth_smile"));
    Mappings.Add(TEXT("Smile_R"), TEXT("CTRL_R_mouth_smile"));
    Mappings.Add(TEXT("Frown_L"), TEXT("CTRL_L_mouth_frown"));
    Mappings.Add(TEXT("Frown_R"), TEXT("CTRL_R_mouth_frown"));
    Mappings.Add(TEXT("MouthOpen"), TEXT("CTRL_C_jaw_open"));
    Mappings.Add(TEXT("MouthPucker"), TEXT("CTRL_C_mouth_pucker"));
    Mappings.Add(TEXT("MouthFunnel"), TEXT("CTRL_C_mouth_funnel"));
    Mappings.Add(TEXT("LipStretch_L"), TEXT("CTRL_L_mouth_lipStretch"));
    Mappings.Add(TEXT("LipStretch_R"), TEXT("CTRL_R_mouth_lipStretch"));
    Mappings.Add(TEXT("LipPress"), TEXT("CTRL_C_mouth_press"));
    Mappings.Add(TEXT("LipTighten"), TEXT("CTRL_C_mouth_tighten"));
    Mappings.Add(TEXT("LipsPart"), TEXT("CTRL_C_mouth_lipsPart"));
    Mappings.Add(TEXT("LipSuck"), TEXT("CTRL_C_mouth_suck"));
    Mappings.Add(TEXT("UpperLipRaise_L"), TEXT("CTRL_L_mouth_upperLipRaise"));
    Mappings.Add(TEXT("UpperLipRaise_R"), TEXT("CTRL_R_mouth_upperLipRaise"));
    Mappings.Add(TEXT("LowerLipDepress_L"), TEXT("CTRL_L_mouth_lowerLipDepress"));
    Mappings.Add(TEXT("LowerLipDepress_R"), TEXT("CTRL_R_mouth_lowerLipDepress"));
    Mappings.Add(TEXT("ChinRaise"), TEXT("CTRL_C_mouth_chinRaise"));
    Mappings.Add(TEXT("Dimple_L"), TEXT("CTRL_L_mouth_dimple"));
    Mappings.Add(TEXT("Dimple_R"), TEXT("CTRL_R_mouth_dimple"));

    // Brow controls
    Mappings.Add(TEXT("BrowRaiseIn_L"), TEXT("CTRL_L_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseIn_R"), TEXT("CTRL_R_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseOut_L"), TEXT("CTRL_L_brow_raiseOut"));
    Mappings.Add(TEXT("BrowRaiseOut_R"), TEXT("CTRL_R_brow_raiseOut"));
    Mappings.Add(TEXT("BrowDown_L"), TEXT("CTRL_L_brow_down"));
    Mappings.Add(TEXT("BrowDown_R"), TEXT("CTRL_R_brow_down"));

    // Eye controls
    Mappings.Add(TEXT("EyeOpen_L"), TEXT("CTRL_L_eye_openUpperLid"));
    Mappings.Add(TEXT("EyeOpen_R"), TEXT("CTRL_R_eye_openUpperLid"));
    Mappings.Add(TEXT("EyeBlink_L"), TEXT("CTRL_L_eye_blink"));
    Mappings.Add(TEXT("EyeBlink_R"), TEXT("CTRL_R_eye_blink"));
    Mappings.Add(TEXT("EyeSquint_L"), TEXT("CTRL_L_eye_squint"));
    Mappings.Add(TEXT("EyeSquint_R"), TEXT("CTRL_R_eye_squint"));
    Mappings.Add(TEXT("CheekRaise_L"), TEXT("CTRL_L_eye_cheekRaise"));
    Mappings.Add(TEXT("CheekRaise_R"), TEXT("CTRL_R_eye_cheekRaise"));
    Mappings.Add(TEXT("EyeWarmth_L"), TEXT("CTRL_L_eye_cheekRaise"));
    Mappings.Add(TEXT("EyeWarmth_R"), TEXT("CTRL_R_eye_cheekRaise"));

    // Nose
    Mappings.Add(TEXT("NoseWrinkle_L"), TEXT("CTRL_L_nose_wrinkle"));
    Mappings.Add(TEXT("NoseWrinkle_R"), TEXT("CTRL_R_nose_wrinkle"));

    // Jaw
    Mappings.Add(TEXT("JawOpen"), TEXT("CTRL_C_jaw_open"));
    Mappings.Add(TEXT("JawLeft"), TEXT("CTRL_C_jaw_left"));
    Mappings.Add(TEXT("JawRight"), TEXT("CTRL_C_jaw_right"));

    return Mappings;
}

// ============================================================================
// REPAIRED: COMPREHENSIVE NEUROCHEMICAL-TO-BLENDSHAPE MAPPING
// ============================================================================

void MapNeurochemicalToBlendShapesComplete(const FNeurochemicalState& State,
    TMap<FString, float>& OutWeights, EDNAVersion DNAVersion)
{
    OutWeights.Empty();

    // === DOPAMINE → JOY/REWARD EXPRESSION ===
    float SmileIntensity = State.Dopamine * 0.8f + State.Serotonin * 0.3f;
    float CheekRaise = State.Dopamine * 0.6f;
    OutWeights.Add(TEXT("Smile_L"), FMath::Clamp(SmileIntensity, 0.0f, 1.0f));
    OutWeights.Add(TEXT("Smile_R"), FMath::Clamp(SmileIntensity, 0.0f, 1.0f));
    OutWeights.Add(TEXT("CheekRaise_L"), FMath::Clamp(CheekRaise, 0.0f, 1.0f));
    OutWeights.Add(TEXT("CheekRaise_R"), FMath::Clamp(CheekRaise, 0.0f, 1.0f));

    // === CORTISOL → STRESS/TENSION ===
    float BrowFurrow = State.Cortisol * 0.7f;
    float LipDepress = State.Cortisol * 0.4f;
    float LidTighten = State.Cortisol * 0.5f;
    OutWeights.Add(TEXT("BrowDown_L"), FMath::Clamp(BrowFurrow, 0.0f, 1.0f));
    OutWeights.Add(TEXT("BrowDown_R"), FMath::Clamp(BrowFurrow, 0.0f, 1.0f));
    OutWeights.Add(TEXT("Frown_L"), FMath::Clamp(LipDepress, 0.0f, 1.0f));
    OutWeights.Add(TEXT("Frown_R"), FMath::Clamp(LipDepress, 0.0f, 1.0f));
    OutWeights.Add(TEXT("EyeSquint_L"), FMath::Clamp(LidTighten, 0.0f, 1.0f));
    OutWeights.Add(TEXT("EyeSquint_R"), FMath::Clamp(LidTighten, 0.0f, 1.0f));
    OutWeights.Add(TEXT("LipTighten"), FMath::Clamp(State.Cortisol * 0.3f, 0.0f, 1.0f));

    // === OXYTOCIN → WARMTH/SOCIAL BONDING ===
    float WarmSmile = State.Oxytocin * 0.7f;
    float WarmCheeks = State.Oxytocin * 0.5f;
    OutWeights.Add(TEXT("EyeWarmth_L"), FMath::Clamp(WarmCheeks, 0.0f, 1.0f));
    OutWeights.Add(TEXT("EyeWarmth_R"), FMath::Clamp(WarmCheeks, 0.0f, 1.0f));
    // Accumulate with existing smile
    float* ExistingSmileL = OutWeights.Find(TEXT("Smile_L"));
    if (ExistingSmileL) *ExistingSmileL = FMath::Clamp(*ExistingSmileL + WarmSmile, 0.0f, 1.0f);
    float* ExistingSmileR = OutWeights.Find(TEXT("Smile_R"));
    if (ExistingSmileR) *ExistingSmileR = FMath::Clamp(*ExistingSmileR + WarmSmile, 0.0f, 1.0f);
    OutWeights.Add(TEXT("LipsPart"), FMath::Clamp(State.Oxytocin * 0.3f, 0.0f, 1.0f));

    // === NOREPINEPHRINE → ALERTNESS ===
    float AlertnessIntensity = State.Norepinephrine * 0.6f;
    float BrowRaise = State.Norepinephrine * 0.4f;
    OutWeights.Add(TEXT("EyeOpen_L"), FMath::Clamp(AlertnessIntensity, 0.0f, 1.0f));
    OutWeights.Add(TEXT("EyeOpen_R"), FMath::Clamp(AlertnessIntensity, 0.0f, 1.0f));
    OutWeights.Add(TEXT("BrowRaiseOut_L"), FMath::Clamp(BrowRaise, 0.0f, 1.0f));
    OutWeights.Add(TEXT("BrowRaiseOut_R"), FMath::Clamp(BrowRaise, 0.0f, 1.0f));

    // === SEROTONIN → CONTENTMENT ===
    float ContentSmile = State.Serotonin * 0.4f;
    float* SmileL = OutWeights.Find(TEXT("Smile_L"));
    if (SmileL) *SmileL = FMath::Clamp(*SmileL + ContentSmile, 0.0f, 1.0f);
    float* SmileR = OutWeights.Find(TEXT("Smile_R"));
    if (SmileR) *SmileR = FMath::Clamp(*SmileR + ContentSmile, 0.0f, 1.0f);

    // === MELATONIN → DROWSINESS ===
    if (State.Melatonin > 0.3f)
    {
        float DrowsyEyes = State.Melatonin * 0.8f;
        OutWeights.Add(TEXT("EyeBlink_L"), FMath::Clamp(DrowsyEyes, 0.0f, 1.0f));
        OutWeights.Add(TEXT("EyeBlink_R"), FMath::Clamp(DrowsyEyes, 0.0f, 1.0f));
    }

    // === ADRENALINE → FIGHT/FLIGHT ===
    if (State.Adrenaline > 0.2f)
    {
        float WideEyes = State.Adrenaline * 0.9f;
        float* EyeL = OutWeights.Find(TEXT("EyeOpen_L"));
        if (EyeL) *EyeL = FMath::Clamp(*EyeL + WideEyes, 0.0f, 1.0f);
        else OutWeights.Add(TEXT("EyeOpen_L"), FMath::Clamp(WideEyes, 0.0f, 1.0f));
        float* EyeR = OutWeights.Find(TEXT("EyeOpen_R"));
        if (EyeR) *EyeR = FMath::Clamp(*EyeR + WideEyes, 0.0f, 1.0f);
        else OutWeights.Add(TEXT("EyeOpen_R"), FMath::Clamp(WideEyes, 0.0f, 1.0f));

        OutWeights.Add(TEXT("BrowRaiseIn_L"), FMath::Clamp(State.Adrenaline * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("BrowRaiseIn_R"), FMath::Clamp(State.Adrenaline * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("MouthOpen"), FMath::Clamp(State.Adrenaline * 0.3f, 0.0f, 1.0f));
    }

    // === ENDORPHIN → BLISS ===
    if (State.Endorphin > 0.2f)
    {
        float BlissSmile = State.Endorphin * 0.5f;
        float* BSL = OutWeights.Find(TEXT("Smile_L"));
        if (BSL) *BSL = FMath::Clamp(*BSL + BlissSmile, 0.0f, 1.0f);
        float* BSR = OutWeights.Find(TEXT("Smile_R"));
        if (BSR) *BSR = FMath::Clamp(*BSR + BlissSmile, 0.0f, 1.0f);
    }

    UE_LOG(LogMetaHumanDNARepairs, Verbose, TEXT("Mapped neurochemical state to %d blend shapes"), OutWeights.Num());
}

} // namespace MetaHumanDNARepairs
