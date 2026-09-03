#include "Misc/AutomationTest.h"
#include "DeepTreeEchoContentPaths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTEMontageCatalogCountTest,
	"UnrealEngineCog.Avatar.Content.MontageCatalogCount",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDTEMontageCatalogCountTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Phase 2 catalog is 29 montages"), DTEContent::GetRequiredMontageCount(), 29);

	UMontageLibraryDataAsset* Library = NewObject<UMontageLibraryDataAsset>();
	DTEContent::FillDefaultMontageLibrary(*Library);
	TestEqual(TEXT("Filled library has 29 entries"), Library->Montages.Num(), 29);

	int32 Idle = 0, Emote = 0, Gesture = 0;
	TSet<FName> Ids;
	for (const FMontageEntry& Entry : Library->Montages)
	{
		Ids.Add(Entry.MontageId);
		TestTrue(TEXT("Each entry has a variant"), Entry.Variants.Num() > 0);
		if (Entry.Category == EMontageCategory::Idle) { ++Idle; }
		else if (Entry.Category == EMontageCategory::Emote) { ++Emote; }
		else if (Entry.Category == EMontageCategory::Gesture) { ++Gesture; }
	}
	TestEqual(TEXT("Unique montage ids"), Ids.Num(), 29);
	TestEqual(TEXT("8 idle montages"), Idle, 8);
	TestEqual(TEXT("11 emote montages"), Emote, 11);
	TestEqual(TEXT("10 gesture montages"), Gesture, 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTEMaterialParameterContractTest,
	"UnrealEngineCog.Avatar.Content.MaterialParameterNames",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDTEMaterialParameterContractTest::RunTest(const FString& Parameters)
{
	auto ExpectContains = [this](const TArray<FName>& Names, const TCHAR* Param)
	{
		TestTrue(FString::Printf(TEXT("missing param %s"), Param), Names.Contains(FName(Param)));
	};

	const TArray<FName> Skin = DTEContent::SkinParameterNames();
	ExpectContains(Skin, TEXT("Smoothness"));
	ExpectContains(Skin, TEXT("Subsurface"));
	ExpectContains(Skin, TEXT("Translucency"));
	ExpectContains(Skin, TEXT("BlushIntensity"));
	ExpectContains(Skin, TEXT("BlushColor"));
	ExpectContains(Skin, TEXT("AuraIntensity"));
	ExpectContains(Skin, TEXT("AuraColor"));
	ExpectContains(Skin, TEXT("GlitchIntensity"));

	const TArray<FName> Hair = DTEContent::HairParameterNames();
	ExpectContains(Hair, TEXT("Anisotropy"));
	ExpectContains(Hair, TEXT("ShimmerIntensity"));
	ExpectContains(Hair, TEXT("EchoGlow"));

	const TArray<FName> Eyes = DTEContent::EyeParameterNames();
	ExpectContains(Eyes, TEXT("PupilDilation"));
	ExpectContains(Eyes, TEXT("SparkleIntensity"));
	ExpectContains(Eyes, TEXT("Moisture"));
	ExpectContains(Eyes, TEXT("SparklePositionX"));
	ExpectContains(Eyes, TEXT("SparklePositionY"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTEAssetPathContractTest,
	"UnrealEngineCog.Avatar.Content.AssetPaths",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDTEAssetPathContractTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("ABP path"), DTEContent::AnimBlueprintPath().Contains(TEXT("ABP_DeepTreeEcho_Avatar")));
	TestTrue(TEXT("Skin master path"), DTEContent::SkinMasterPath().Contains(TEXT("M_DTE_Skin")));
	TestTrue(TEXT("Eye master path"), DTEContent::EyeMasterPath().Contains(TEXT("M_DTE_Eye")));
	TestTrue(TEXT("Hair master path"), DTEContent::HairMasterPath().Contains(TEXT("M_DTE_Hair")));
	TestTrue(TEXT("Montage library path"), DTEContent::MontageLibraryPath().Contains(TEXT("DA_DTE_MontageLibrary")));

	const TArray<FName> Niagara = DTEContent::NiagaraSystemNames();
	TestEqual(TEXT("4 Niagara systems"), Niagara.Num(), 4);
	TestTrue(TEXT("NS_MemoryNode"), Niagara.Contains(FName(TEXT("NS_MemoryNode"))));
	TestTrue(TEXT("NS_EchoResonance"), Niagara.Contains(FName(TEXT("NS_EchoResonance"))));
	TestTrue(TEXT("NS_CognitiveLoad"), Niagara.Contains(FName(TEXT("NS_CognitiveLoad"))));
	TestTrue(TEXT("NS_EmotionalAura"), Niagara.Contains(FName(TEXT("NS_EmotionalAura"))));

	const TArray<FName> PP = DTEContent::PostProcessNames();
	TestEqual(TEXT("4 post-process materials"), PP.Num(), 4);
	return true;
}
