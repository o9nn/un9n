#pragma once

#include "CoreMinimal.h"
#include "AnimationMontageSystem.h"

/**
 * Deep Tree Echo Content path and parameter contracts.
 * Must stay aligned with Content/DeepTreeEcho/dte_asset_manifest.json
 * and Content/Python generators.
 */
namespace DTEContent
{
	inline FString Root() { return TEXT("/Game/DeepTreeEcho"); }

	inline FString ObjectPath(const TCHAR* Package, const TCHAR* Name)
	{
		return FString::Printf(TEXT("%s/%s.%s"), Package, Name, Name);
	}

	inline FString AnimBlueprintPath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Animations/Blueprints"), TEXT("ABP_DeepTreeEcho_Avatar"));
	}

	inline FString MontageLibraryPath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Animations"), TEXT("DA_DTE_MontageLibrary"));
	}

	inline FString MontagePath(const TCHAR* AssetName)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Animations/Montages"), AssetName);
	}

	inline FString SkinMasterPath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Master"), TEXT("M_DTE_Skin"));
	}

	inline FString EyeMasterPath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Master"), TEXT("M_DTE_Eye"));
	}

	inline FString HairMasterPath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Master"), TEXT("M_DTE_Hair"));
	}

	inline FString SkinInstancePath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Instances"), TEXT("MI_DTE_Skin_Default"));
	}

	inline FString EyeInstancePath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Instances"), TEXT("MI_DTE_Eye_Default"));
	}

	inline FString HairInstancePath()
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Materials/Instances"), TEXT("MI_DTE_Hair_Default"));
	}

	inline FString NiagaraPath(const TCHAR* Name)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Particles/Niagara"), Name);
	}

	inline FString PostProcessPath(const TCHAR* Name)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/PostProcessing"), Name);
	}

	inline FString GestureSfxPath(const TCHAR* Name)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Audio/Gestures"), Name);
	}

	inline FString EmotionSfxPath(const TCHAR* Name)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Audio/Emotional"), Name);
	}

	inline FString MusicPath(const TCHAR* Name)
	{
		return ObjectPath(TEXT("/Game/DeepTreeEcho/Audio/Music"), Name);
	}

	inline TArray<FName> SkinParameterNames()
	{
		return {
			TEXT("Smoothness"),
			TEXT("Subsurface"),
			TEXT("Translucency"),
			TEXT("BlushIntensity"),
			TEXT("BlushColor"),
			TEXT("AuraIntensity"),
			TEXT("AuraColor"),
			TEXT("GlitchIntensity"),
			TEXT("BaseColor"),
			TEXT("SubsurfaceColor")
		};
	}

	inline TArray<FName> HairParameterNames()
	{
		return {
			TEXT("Anisotropy"),
			TEXT("ShimmerIntensity"),
			TEXT("ShimmerSpeed"),
			TEXT("ChaosColorShift"),
			TEXT("EchoGlow")
		};
	}

	inline TArray<FName> EyeParameterNames()
	{
		return {
			TEXT("PupilDilation"),
			TEXT("SparkleIntensity"),
			TEXT("Moisture"),
			TEXT("SparklePositionX"),
			TEXT("SparklePositionY")
		};
	}

	inline TArray<FName> GlobalParameterNames()
	{
		return {
			TEXT("GlobalGlitchIntensity"),
			TEXT("CognitiveLoad")
		};
	}

	inline TArray<FName> NiagaraSystemNames()
	{
		return {
			TEXT("NS_MemoryNode"),
			TEXT("NS_EchoResonance"),
			TEXT("NS_CognitiveLoad"),
			TEXT("NS_EmotionalAura")
		};
	}

	inline TArray<FName> PostProcessNames()
	{
		return {
			TEXT("PP_EmotionalAura"),
			TEXT("PP_CognitiveLoadHeatMap"),
			TEXT("PP_GlitchEffect"),
			TEXT("PP_EchoResonanceDistortion")
		};
	}

	UNREALECHO_API void FillDefaultMontageLibrary(UMontageLibraryDataAsset& Library);
	UNREALECHO_API TArray<FName> GetRequiredMontageIds();
	UNREALECHO_API int32 GetRequiredMontageCount();
}
