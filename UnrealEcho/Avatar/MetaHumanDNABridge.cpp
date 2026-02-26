// MetaHumanDNABridge.cpp
// Deep Tree Echo - MetaHuman DNA Integration Implementation
// Copyright (c) 2025 Deep Tree Echo Project

#include "MetaHumanDNABridge.h"
#include "PythonDNACalibWrapper.h"
#include "../Neurochemical/NeurochemicalSimulationComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaHumanDNA, Log, All);

// ========================================
// Constructor / Destructor
// ========================================

UMetaHumanDNABridge::UMetaHumanDNABridge()
    : bDNALoaded(false)
    , DNAVersion(EDNAVersion::Unknown)
{
    UE_LOG(LogMetaHumanDNA, Log, TEXT("MetaHumanDNABridge initialized"));
}

UMetaHumanDNABridge::~UMetaHumanDNABridge()
{
    if (PythonWrapper.IsValid())
    {
        ShutdownPythonWrapper();
    }
}

// ========================================
// DNA File Operations
// ========================================

bool UMetaHumanDNABridge::LoadDNAFile(const FString& DNAPath)
{
    if (!FPaths::FileExists(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("DNA file not found: %s"), *DNAPath);
        return false;
    }

    // Initialize Python wrapper if not already done
    if (!PythonWrapper.IsValid())
    {
        FString DNACalibPath = FPaths::ProjectDir() / TEXT("MetaHuman-DNA-Calibration");
        if (!InitializePythonWrapper(DNACalibPath))
        {
            UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to initialize Python wrapper"));
            return false;
        }
    }

    // Load DNA using Python wrapper
    if (!PythonWrapper->LoadDNA(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to load DNA file: %s"), *DNAPath);
        return false;
    }

    LoadedDNAPath = DNAPath;
    bDNALoaded = true;

    // Parse DNA version
    if (!ParseDNAVersion())
    {
        UE_LOG(LogMetaHumanDNA, Warning, TEXT("Could not determine DNA version, assuming DHI"));
        DNAVersion = EDNAVersion::DHI;
    }

    // Load DNA data into cache
    if (!LoadDNAData())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to load DNA data"));
        bDNALoaded = false;
        return false;
    }

    // Build cache maps
    BuildCacheMaps();

    // Initialize blend shape weights
    InitializeBlendShapeWeights();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully loaded DNA file: %s (Version: %s)"), 
        *DNAPath, *GetDNAVersionString());

    return true;
}

bool UMetaHumanDNABridge::SaveDNAFile(const FString& DNAPath)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded or Python wrapper not initialized"));
        return false;
    }

    if (!PythonWrapper->SaveDNA(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to save DNA file: %s"), *DNAPath);
        return false;
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully saved DNA file: %s"), *DNAPath);
    return true;
}

FString UMetaHumanDNABridge::GetDNAVersionString() const
{
    switch (DNAVersion)
    {
    case EDNAVersion::DHI:
        return TEXT("DHI");
    case EDNAVersion::MH4:
        return TEXT("MH.4");
    default:
        return TEXT("Unknown");
    }
}

// ========================================
// Rig Structure Access
// ========================================

TArray<FString> UMetaHumanDNABridge::GetJointNames() const
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        return TArray<FString>();
    }

    return PythonWrapper->GetJointNames();
}

TArray<FString> UMetaHumanDNABridge::GetMeshNames() const
{
    TArray<FString> MeshNames;
    for (const FDNAMeshInfo& MeshInfo : MeshInfoCache)
    {
        MeshNames.Add(MeshInfo.MeshName);
    }
    return MeshNames;
}

TArray<FString> UMetaHumanDNABridge::GetBlendShapeNames() const
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        return TArray<FString>();
    }

    return PythonWrapper->GetBlendShapeNames();
}

bool UMetaHumanDNABridge::GetJointInfo(const FString& JointName, FDNAJointInfo& OutJointInfo) const
{
    const int32* IndexPtr = JointNameToIndexMap.Find(JointName);
    if (!IndexPtr || !JointInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutJointInfo = JointInfoCache[*IndexPtr];
    return true;
}

bool UMetaHumanDNABridge::GetBlendShapeInfo(const FString& BlendShapeName, FDNABlendShapeInfo& OutBlendShapeInfo) const
{
    const int32* IndexPtr = BlendShapeNameToIndexMap.Find(BlendShapeName);
    if (!IndexPtr || !BlendShapeInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutBlendShapeInfo = BlendShapeInfoCache[*IndexPtr];
    return true;
}

bool UMetaHumanDNABridge::GetMeshInfo(const FString& MeshName, FDNAMeshInfo& OutMeshInfo) const
{
    const int32* IndexPtr = MeshNameToIndexMap.Find(MeshName);
    if (!IndexPtr || !MeshInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutMeshInfo = MeshInfoCache[*IndexPtr];
    return true;
}

FTransform UMetaHumanDNABridge::GetNeutralJointTransform(const FString& JointName) const
{
    FDNAJointInfo JointInfo;
    if (GetJointInfo(JointName, JointInfo))
    {
        return JointInfo.NeutralTransform;
    }
    return FTransform::Identity;
}

// ========================================
// DNA Manipulation
// ========================================

bool UMetaHumanDNABridge::RenameJoint(const FString& OldName, const FString& NewName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Check for critical joints that should not be renamed
    if (OldName == TEXT("neck_01") || OldName == TEXT("neck_02") || OldName == TEXT("FACIAL_C_FacialRoot"))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Cannot rename critical joint: %s"), *OldName);
        return false;
    }

    if (!PythonWrapper->RenameJoint(OldName, NewName))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to rename joint: %s -> %s"), *OldName, *NewName);
        return false;
    }

    // Update cache
    LoadDNAData();
    BuildCacheMaps();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully renamed joint: %s -> %s"), *OldName, *NewName);
    return true;
}

bool UMetaHumanDNABridge::RemoveJoint(const FString& JointName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Check for critical joints that should not be removed
    if (JointName == TEXT("neck_01") || JointName == TEXT("neck_02") || JointName == TEXT("FACIAL_C_FacialRoot"))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Cannot remove critical joint: %s"), *JointName);
        return false;
    }

    if (!PythonWrapper->RemoveJoint(JointName))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to remove joint: %s"), *JointName);
        return false;
    }

    // Update cache
    LoadDNAData();
    BuildCacheMaps();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully removed joint: %s"), *JointName);
    return true;
}

bool UMetaHumanDNABridge::RenameBlendShape(const FString& OldName, const FString& NewName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Rename in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RenameBlendShape not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::RemoveBlendShape(const FString& BlendShapeName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Remove in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RemoveBlendShape not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::ModifyBlendShapeDeltas(const FString& BlendShapeName, const TArray<FVector>& Deltas)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Modify in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("ModifyBlendShapeDeltas not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::ClearAllBlendShapes()
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Clear in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("ClearAllBlendShapes not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::RemoveLOD(int32 LODIndex)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Remove LOD in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RemoveLOD not yet implemented in Python wrapper"));
    return false;
}

// ========================================
// Integration with Avatar System
// ========================================

bool UMetaHumanDNABridge::ApplyDNAToSkeleton(USkeletalMeshComponent* SkeletalMesh)
{
    if (!bDNALoaded || !SkeletalMesh)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded or invalid skeletal mesh"));
        return false;
    }

    USkeletalMesh* Mesh = SkeletalMesh->SkeletalMesh;
    if (!Mesh)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Skeletal mesh component has no mesh"));
        return false;
    }

    USkeleton* Skeleton = Mesh->GetSkeleton();
    if (!Skeleton)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Skeletal mesh has no skeleton"));
        return false;
    }

    // Map DNA joints to Unreal skeleton
    TArray<FString> DNAJoints = GetJointNames();
    const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

    int32 MappedJoints = 0;
    for (const FString& DNAJointName : DNAJoints)
    {
        FName BoneName(*DNAJointName);
        int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
        
        if (BoneIndex != INDEX_NONE)
        {
            MappedJoints++;
            // Successfully mapped joint
            // Could store mapping for future use
        }
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Mapped %d/%d DNA joints to skeleton"), MappedJoints, DNAJoints.Num());

    return MappedJoints > 0;
}

void UMetaHumanDNABridge::SyncNeurochemicalStateToBlendShapes(const FNeurochemicalState& State, TMap<FString, float>& OutBlendShapeWeights)
{
    MapNeurochemicalToBlendShapes(State, OutBlendShapeWeights);
}

void UMetaHumanDNABridge::SyncEmotionalStateToFacialRig(const FEmotionalState& Emotion, TMap<FString, float>& OutBlendShapeWeights)
{
    MapEmotionalStateToBlendShapes(Emotion, OutBlendShapeWeights);
}

float UMetaHumanDNABridge::GetBlendShapeWeight(const FString& BlendShapeName) const
{
    const float* WeightPtr = CurrentBlendShapeWeights.Find(BlendShapeName);
    return WeightPtr ? *WeightPtr : 0.0f;
}

void UMetaHumanDNABridge::SetBlendShapeWeight(const FString& BlendShapeName, float Weight)
{
    CurrentBlendShapeWeights.Add(BlendShapeName, FMath::Clamp(Weight, 0.0f, 1.0f));
}

TMap<FString, float> UMetaHumanDNABridge::GetAllBlendShapeWeights() const
{
    return CurrentBlendShapeWeights;
}

void UMetaHumanDNABridge::SetAllBlendShapeWeights(const TMap<FString, float>& Weights)
{
    CurrentBlendShapeWeights = Weights;
    
    // Clamp all weights to 0-1
    for (auto& Pair : CurrentBlendShapeWeights)
    {
        Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }
}

// ========================================
// Advanced Features
// ========================================

bool UMetaHumanDNABridge::InitializePythonWrapper(const FString& DNACalibPath)
{
    if (PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Warning, TEXT("Python wrapper already initialized"));
        return true;
    }

    PythonWrapper = MakeShared<FPythonDNACalibWrapper>();
    
    if (!PythonWrapper->Initialize(DNACalibPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to initialize Python wrapper with path: %s"), *DNACalibPath);
        PythonWrapper.Reset();
        return false;
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Python wrapper initialized successfully"));
    return true;
}

void UMetaHumanDNABridge::ShutdownPythonWrapper()
{
    if (PythonWrapper.IsValid())
    {
        PythonWrapper->Shutdown();
        PythonWrapper.Reset();
        UE_LOG(LogMetaHumanDNA, Log, TEXT("Python wrapper shut down"));
    }
}

void UMetaHumanDNABridge::GetDNAStatistics(int32& OutJointCount, int32& OutMeshCount, int32& OutBlendShapeCount) const
{
    OutJointCount = JointInfoCache.Num();
    OutMeshCount = MeshInfoCache.Num();
    OutBlendShapeCount = BlendShapeInfoCache.Num();
}

bool UMetaHumanDNABridge::ValidateDNAIntegrity() const
{
    if (!bDNALoaded)
    {
        return false;
    }

    // Check for critical joints
    TArray<FString> CriticalJoints = { TEXT("neck_01"), TEXT("neck_02"), TEXT("FACIAL_C_FacialRoot") };
    TArray<FString> JointNames = GetJointNames();

    for (const FString& CriticalJoint : CriticalJoints)
    {
        if (!JointNames.Contains(CriticalJoint))
        {
            UE_LOG(LogMetaHumanDNA, Error, TEXT("Critical joint missing: %s"), *CriticalJoint);
            return false;
        }
    }

    return true;
}

// ========================================
// Helper Functions
// ========================================

void UMetaHumanDNABridge::BuildCacheMaps()
{
    JointNameToIndexMap.Empty();
    BlendShapeNameToIndexMap.Empty();
    MeshNameToIndexMap.Empty();

    for (int32 i = 0; i < JointInfoCache.Num(); ++i)
    {
        JointNameToIndexMap.Add(JointInfoCache[i].JointName, i);
    }

    for (int32 i = 0; i < BlendShapeInfoCache.Num(); ++i)
    {
        BlendShapeNameToIndexMap.Add(BlendShapeInfoCache[i].BlendShapeName, i);
    }

    for (int32 i = 0; i < MeshInfoCache.Num(); ++i)
    {
        MeshNameToIndexMap.Add(MeshInfoCache[i].MeshName, i);
    }
}

void UMetaHumanDNABridge::ClearCache()
{
    JointInfoCache.Empty();
    BlendShapeInfoCache.Empty();
    MeshInfoCache.Empty();
    JointNameToIndexMap.Empty();
    BlendShapeNameToIndexMap.Empty();
    MeshNameToIndexMap.Empty();
    CurrentBlendShapeWeights.Empty();
}

bool UMetaHumanDNABridge::LoadDNAData()
{
    if (!PythonWrapper.IsValid())
    {
        return false;
    }

    ClearCache();

    // Load joint data
    TArray<FString> JointNames = PythonWrapper->GetJointNames();
    for (int32 i = 0; i < JointNames.Num(); ++i)
    {
        FDNAJointInfo JointInfo;
        JointInfo.JointName = JointNames[i];
        JointInfo.JointIndex = i;
        // Get neutral transform from Python wrapper via DNACalib
        FTransform NeutralTransform = FTransform::Identity;
        TArray<float> TransformData = PythonWrapper->GetJointNeutralTransform(i);
        if (TransformData.Num() >= 10)
        {
            NeutralTransform.SetTranslation(FVector(TransformData[0], TransformData[1], TransformData[2]));
            NeutralTransform.SetRotation(FQuat(TransformData[3], TransformData[4], TransformData[5], TransformData[6]));
            NeutralTransform.SetScale3D(FVector(TransformData[7], TransformData[8], TransformData[9]));
        }
        JointInfo.NeutralTransform = NeutralTransform;
        JointInfo.ParentIndex = PythonWrapper->GetJointParentIndex(i);
        
        JointInfoCache.Add(JointInfo);
    }

    // Load blend shape data
    TArray<FString> BlendShapeNames = PythonWrapper->GetBlendShapeNames();
    for (int32 i = 0; i < BlendShapeNames.Num(); ++i)
    {
        FDNABlendShapeInfo BlendShapeInfo;
        BlendShapeInfo.BlendShapeName = BlendShapeNames[i];
        BlendShapeInfo.BlendShapeIndex = i;
        // Get target mesh and vertex count from Python wrapper
        BlendShapeInfo.TargetMeshName = PythonWrapper->GetBlendShapeTargetMesh(i);
        BlendShapeInfo.VertexCount = PythonWrapper->GetBlendShapeVertexCount(i);
        
        BlendShapeInfoCache.Add(BlendShapeInfo);
    }

    // Load mesh data from Python wrapper
    TArray<FString> MeshNames = PythonWrapper->GetMeshNames();
    for (int32 i = 0; i < MeshNames.Num(); ++i)
    {
        FDNAMeshInfo MeshInfo;
        MeshInfo.MeshName = MeshNames[i];
        MeshInfo.MeshIndex = i;
        MeshInfo.VertexCount = PythonWrapper->GetMeshVertexCount(i);
        MeshInfo.FaceCount = PythonWrapper->GetMeshFaceCount(i);
        MeshInfoCache.Add(MeshInfo);
    }

    UE_LOG(LogTemp, Log, TEXT("DNA data loaded: %d joints, %d blend shapes, %d meshes"),
        JointInfoCache.Num(), BlendShapeInfoCache.Num(), MeshInfoCache.Num());

    return true;
}

bool UMetaHumanDNABridge::ParseDNAVersion()
{
    if (!PythonWrapper.IsValid())
    {
        return false;
    }

    // Detect DNA version from database name and structure
    FString DatabaseName = PythonWrapper->GetDatabaseName();
    int32 LODCount = PythonWrapper->GetLODCount();
    int32 BlendShapeCount = PythonWrapper->GetBlendShapeCount();

    if (DatabaseName.Contains(TEXT("DHI")) || DatabaseName.Contains(TEXT("dhi")))
    {
        DNAVersion = EDNAVersion::DHI;
    }
    else if (DatabaseName.Contains(TEXT("MH4")) || DatabaseName.Contains(TEXT("mh4")) ||
             DatabaseName.Contains(TEXT("MetaHuman4")))
    {
        DNAVersion = EDNAVersion::MH4;
    }
    else
    {
        // Heuristic: DHI typically has more blend shapes and LODs
        if (BlendShapeCount > 200 && LODCount >= 6)
        {
            DNAVersion = EDNAVersion::DHI;
        }
        else
        {
            DNAVersion = EDNAVersion::MH4;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Detected DNA version: %s (DB: %s, LODs: %d, BlendShapes: %d)"),
        DNAVersion == EDNAVersion::DHI ? TEXT("DHI") : TEXT("MH.4"),
        *DatabaseName, LODCount, BlendShapeCount);

    return true;
}

void UMetaHumanDNABridge::InitializeBlendShapeWeights()
{
    CurrentBlendShapeWeights.Empty();

    // Initialize all blend shapes to 0
    for (const FDNABlendShapeInfo& BlendShapeInfo : BlendShapeInfoCache)
    {
        CurrentBlendShapeWeights.Add(BlendShapeInfo.BlendShapeName, 0.0f);
    }
}

void UMetaHumanDNABridge::MapNeurochemicalToBlendShapes(const FNeurochemicalState& State, TMap<FString, float>& OutWeights)
{
    // Comprehensive neurochemical-to-blend-shape mapping using FACS intermediary
    // Delegates to MetaHumanDNARepairs for full implementation
    OutWeights.Empty();

    // === DOPAMINE → JOY/REWARD EXPRESSION ===
    float SmileIntensity = FMath::Clamp(State.Dopamine * 0.8f + State.Serotonin * 0.3f, 0.0f, 1.0f);
    float CheekRaise = FMath::Clamp(State.Dopamine * 0.6f, 0.0f, 1.0f);
    OutWeights.Add(TEXT("Smile_L"), SmileIntensity);
    OutWeights.Add(TEXT("Smile_R"), SmileIntensity);
    OutWeights.Add(TEXT("CheekRaise_L"), CheekRaise);
    OutWeights.Add(TEXT("CheekRaise_R"), CheekRaise);

    // === CORTISOL → STRESS/TENSION ===
    float BrowFurrow = FMath::Clamp(State.Cortisol * 0.7f, 0.0f, 1.0f);
    float LipDepress = FMath::Clamp(State.Cortisol * 0.4f, 0.0f, 1.0f);
    float LidTighten = FMath::Clamp(State.Cortisol * 0.5f, 0.0f, 1.0f);
    OutWeights.Add(TEXT("BrowDown_L"), BrowFurrow);
    OutWeights.Add(TEXT("BrowDown_R"), BrowFurrow);
    OutWeights.Add(TEXT("Frown_L"), LipDepress);
    OutWeights.Add(TEXT("Frown_R"), LipDepress);
    OutWeights.Add(TEXT("EyeSquint_L"), LidTighten);
    OutWeights.Add(TEXT("EyeSquint_R"), LidTighten);
    OutWeights.Add(TEXT("LipTighten"), FMath::Clamp(State.Cortisol * 0.3f, 0.0f, 1.0f));

    // === OXYTOCIN → WARMTH/SOCIAL BONDING ===
    float WarmSmile = FMath::Clamp(State.Oxytocin * 0.7f, 0.0f, 1.0f);
    float WarmCheeks = FMath::Clamp(State.Oxytocin * 0.5f, 0.0f, 1.0f);
    OutWeights.Add(TEXT("EyeWarmth_L"), WarmCheeks);
    OutWeights.Add(TEXT("EyeWarmth_R"), WarmCheeks);
    OutWeights.Add(TEXT("LipsPart"), FMath::Clamp(State.Oxytocin * 0.3f, 0.0f, 1.0f));
    // Accumulate smile from oxytocin
    float* SmileL = OutWeights.Find(TEXT("Smile_L"));
    if (SmileL) *SmileL = FMath::Clamp(*SmileL + WarmSmile, 0.0f, 1.0f);
    float* SmileR = OutWeights.Find(TEXT("Smile_R"));
    if (SmileR) *SmileR = FMath::Clamp(*SmileR + WarmSmile, 0.0f, 1.0f);

    // === NOREPINEPHRINE → ALERTNESS ===
    float AlertnessIntensity = FMath::Clamp(State.Norepinephrine * 0.6f, 0.0f, 1.0f);
    float BrowRaise = FMath::Clamp(State.Norepinephrine * 0.4f, 0.0f, 1.0f);
    OutWeights.Add(TEXT("EyeOpen_L"), AlertnessIntensity);
    OutWeights.Add(TEXT("EyeOpen_R"), AlertnessIntensity);
    OutWeights.Add(TEXT("BrowRaiseOut_L"), BrowRaise);
    OutWeights.Add(TEXT("BrowRaiseOut_R"), BrowRaise);

    // === SEROTONIN → CONTENTMENT ===
    float ContentSmile = FMath::Clamp(State.Serotonin * 0.4f, 0.0f, 1.0f);
    SmileL = OutWeights.Find(TEXT("Smile_L"));
    if (SmileL) *SmileL = FMath::Clamp(*SmileL + ContentSmile, 0.0f, 1.0f);
    SmileR = OutWeights.Find(TEXT("Smile_R"));
    if (SmileR) *SmileR = FMath::Clamp(*SmileR + ContentSmile, 0.0f, 1.0f);

    // === MELATONIN → DROWSINESS ===
    if (State.Melatonin > 0.3f)
    {
        OutWeights.Add(TEXT("EyeBlink_L"), FMath::Clamp(State.Melatonin * 0.8f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("EyeBlink_R"), FMath::Clamp(State.Melatonin * 0.8f, 0.0f, 1.0f));
    }

    // === ADRENALINE → FIGHT/FLIGHT ===
    if (State.Adrenaline > 0.2f)
    {
        float WideEyes = FMath::Clamp(State.Adrenaline * 0.9f, 0.0f, 1.0f);
        float* EyeL = OutWeights.Find(TEXT("EyeOpen_L"));
        if (EyeL) *EyeL = FMath::Clamp(*EyeL + WideEyes, 0.0f, 1.0f);
        else OutWeights.Add(TEXT("EyeOpen_L"), WideEyes);
        float* EyeR = OutWeights.Find(TEXT("EyeOpen_R"));
        if (EyeR) *EyeR = FMath::Clamp(*EyeR + WideEyes, 0.0f, 1.0f);
        else OutWeights.Add(TEXT("EyeOpen_R"), WideEyes);
        OutWeights.Add(TEXT("BrowRaiseIn_L"), FMath::Clamp(State.Adrenaline * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("BrowRaiseIn_R"), FMath::Clamp(State.Adrenaline * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("MouthOpen"), FMath::Clamp(State.Adrenaline * 0.3f, 0.0f, 1.0f));
    }

    // === ENDORPHIN → BLISS ===
    if (State.Endorphin > 0.2f)
    {
        float BlissSmile = FMath::Clamp(State.Endorphin * 0.5f, 0.0f, 1.0f);
        SmileL = OutWeights.Find(TEXT("Smile_L"));
        if (SmileL) *SmileL = FMath::Clamp(*SmileL + BlissSmile, 0.0f, 1.0f);
        SmileR = OutWeights.Find(TEXT("Smile_R"));
        if (SmileR) *SmileR = FMath::Clamp(*SmileR + BlissSmile, 0.0f, 1.0f);
    }
}

void UMetaHumanDNABridge::MapEmotionalStateToBlendShapes(const FEmotionalState& Emotion, TMap<FString, float>& OutWeights)
{
    // Comprehensive emotional state to blend shape mapping using dimensional model
    OutWeights.Empty();

    float V = Emotion.Valence;    // [-1, 1]
    float A = Emotion.Arousal;    // [0, 1]
    float I = Emotion.Intensity;  // [0, 1]

    // === VALENCE DIMENSION ===
    if (V > 0.0f)
    {
        // Positive valence: smile, cheek raise, eye sparkle
        float SmileAmount = V * I;
        OutWeights.Add(TEXT("Smile_L"), FMath::Clamp(SmileAmount, 0.0f, 1.0f));
        OutWeights.Add(TEXT("Smile_R"), FMath::Clamp(SmileAmount, 0.0f, 1.0f));
        OutWeights.Add(TEXT("CheekRaise_L"), FMath::Clamp(V * I * 0.7f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("CheekRaise_R"), FMath::Clamp(V * I * 0.7f, 0.0f, 1.0f));

        // Duchenne smile (genuine) when high valence + moderate arousal
        if (V > 0.5f && A > 0.3f && A < 0.7f)
        {
            OutWeights.Add(TEXT("EyeSquint_L"), FMath::Clamp(V * 0.4f, 0.0f, 1.0f));
            OutWeights.Add(TEXT("EyeSquint_R"), FMath::Clamp(V * 0.4f, 0.0f, 1.0f));
        }
    }
    else
    {
        // Negative valence: frown, brow furrow, lip depress
        float NegV = -V;
        OutWeights.Add(TEXT("Frown_L"), FMath::Clamp(NegV * I, 0.0f, 1.0f));
        OutWeights.Add(TEXT("Frown_R"), FMath::Clamp(NegV * I, 0.0f, 1.0f));
        OutWeights.Add(TEXT("BrowDown_L"), FMath::Clamp(NegV * I * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("BrowDown_R"), FMath::Clamp(NegV * I * 0.6f, 0.0f, 1.0f));

        // Sadness: inner brow raise
        if (A < 0.4f)
        {
            OutWeights.Add(TEXT("BrowRaiseIn_L"), FMath::Clamp(NegV * 0.5f, 0.0f, 1.0f));
            OutWeights.Add(TEXT("BrowRaiseIn_R"), FMath::Clamp(NegV * 0.5f, 0.0f, 1.0f));
        }
    }

    // === AROUSAL DIMENSION ===
    // High arousal: wide eyes, raised brows, open mouth
    OutWeights.Add(TEXT("EyeOpen_L"), FMath::Clamp(A * 0.6f, 0.0f, 1.0f));
    OutWeights.Add(TEXT("EyeOpen_R"), FMath::Clamp(A * 0.6f, 0.0f, 1.0f));
    OutWeights.Add(TEXT("BrowRaiseOut_L"), FMath::Clamp(A * 0.4f, 0.0f, 1.0f));
    OutWeights.Add(TEXT("BrowRaiseOut_R"), FMath::Clamp(A * 0.4f, 0.0f, 1.0f));
    OutWeights.Add(TEXT("LipsPart"), FMath::Clamp(A * 0.3f, 0.0f, 1.0f));

    // Very high arousal: jaw drop, nostril flare
    if (A > 0.7f)
    {
        OutWeights.Add(TEXT("MouthOpen"), FMath::Clamp((A - 0.7f) * 2.0f * I, 0.0f, 1.0f));
        OutWeights.Add(TEXT("NoseWrinkle_L"), FMath::Clamp((A - 0.7f) * 0.5f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("NoseWrinkle_R"), FMath::Clamp((A - 0.7f) * 0.5f, 0.0f, 1.0f));
    }

    // === CATEGORICAL EMOTION OVERLAYS ===
    // Surprise: high arousal + neutral/positive valence
    if (A > 0.6f && V > -0.2f)
    {
        OutWeights.Add(TEXT("BrowRaiseIn_L"), FMath::Clamp(A * 0.6f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("BrowRaiseIn_R"), FMath::Clamp(A * 0.6f, 0.0f, 1.0f));
    }

    // Anger: high arousal + negative valence
    if (A > 0.5f && V < -0.3f)
    {
        OutWeights.Add(TEXT("LipTighten"), FMath::Clamp(A * (-V) * 0.5f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("NoseWrinkle_L"), FMath::Clamp((-V) * 0.4f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("NoseWrinkle_R"), FMath::Clamp((-V) * 0.4f, 0.0f, 1.0f));
    }

    // Disgust: moderate arousal + negative valence
    if (A > 0.3f && A < 0.7f && V < -0.4f)
    {
        OutWeights.Add(TEXT("UpperLipRaise_L"), FMath::Clamp((-V) * 0.5f, 0.0f, 1.0f));
        OutWeights.Add(TEXT("UpperLipRaise_R"), FMath::Clamp((-V) * 0.5f, 0.0f, 1.0f));
    }
}

TMap<FString, FString> UMetaHumanDNABridge::GetDHIBlendShapeNames() const
{
    // DHI blend shape name mappings
    TMap<FString, FString> Mappings;
    
    // Complete DHI blend shape mappings (MetaHuman DHI rig)
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

TMap<FString, FString> UMetaHumanDNABridge::GetMH4BlendShapeNames() const
{
    // MH.4 blend shape name mappings
    TMap<FString, FString> Mappings;
    
    // Complete MH.4 blend shape mappings (MetaHuman 4.x rig)
    // === MOUTH ===
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
    // === BROW ===
    Mappings.Add(TEXT("BrowRaiseIn_L"), TEXT("CTRL_L_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseIn_R"), TEXT("CTRL_R_brow_raiseIn"));
    Mappings.Add(TEXT("BrowRaiseOut_L"), TEXT("CTRL_L_brow_raiseOut"));
    Mappings.Add(TEXT("BrowRaiseOut_R"), TEXT("CTRL_R_brow_raiseOut"));
    Mappings.Add(TEXT("BrowDown_L"), TEXT("CTRL_L_brow_down"));
    Mappings.Add(TEXT("BrowDown_R"), TEXT("CTRL_R_brow_down"));
    // === EYES ===
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
    // === NOSE ===
    Mappings.Add(TEXT("NoseWrinkle_L"), TEXT("CTRL_L_nose_wrinkle"));
    Mappings.Add(TEXT("NoseWrinkle_R"), TEXT("CTRL_R_nose_wrinkle"));
    // === JAW ===
    Mappings.Add(TEXT("JawOpen"), TEXT("CTRL_C_jaw_open"));
    Mappings.Add(TEXT("JawLeft"), TEXT("CTRL_C_jaw_left"));
    Mappings.Add(TEXT("JawRight"), TEXT("CTRL_C_jaw_right"));

    return Mappings;
}
