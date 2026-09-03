#include "DeepTreeEchoContentPaths.h"
#include "Animation/AnimMontage.h"

namespace DTEContent
{
	struct FCatalogRow
	{
		const TCHAR* Id;
		const TCHAR* Asset;
		EMontageCategory Category;
		float Duration;
		float Extraversion;
		float Openness;
		float Valence;
		float Arousal;
	};

	static const FCatalogRow GCatalog[] = {
		{ TEXT("Idle_ThinkingPose"), TEXT("AM_Idle_ThinkingPose"), EMontageCategory::Idle, 3.0f, 0.3f, 0.8f, 0.1f, 0.3f },
		{ TEXT("Idle_HairTuck"), TEXT("AM_Idle_HairTuck"), EMontageCategory::Idle, 2.0f, 0.6f, 0.5f, 0.4f, 0.4f },
		{ TEXT("Idle_ArmsCrossed"), TEXT("AM_Idle_ArmsCrossed"), EMontageCategory::Idle, 4.0f, 0.7f, 0.3f, 0.2f, 0.3f },
		{ TEXT("Idle_LookAround"), TEXT("AM_Idle_LookAround"), EMontageCategory::Idle, 5.0f, 0.5f, 0.7f, 0.2f, 0.4f },
		{ TEXT("Idle_ShiftWeight"), TEXT("AM_Idle_ShiftWeight"), EMontageCategory::Idle, 2.0f, 0.5f, 0.5f, 0.0f, 0.2f },
		{ TEXT("Idle_StretchArms"), TEXT("AM_Idle_StretchArms"), EMontageCategory::Idle, 4.0f, 0.4f, 0.5f, 0.3f, 0.3f },
		{ TEXT("Idle_Fidget"), TEXT("AM_Idle_Fidget"), EMontageCategory::Idle, 3.0f, 0.3f, 0.4f, -0.2f, 0.6f },
		{ TEXT("Idle_PlayWithHair"), TEXT("AM_Idle_PlayWithHair"), EMontageCategory::Idle, 4.0f, 0.6f, 0.5f, 0.4f, 0.4f },
		{ TEXT("Emote_Laugh"), TEXT("AM_Emote_Laugh"), EMontageCategory::Emote, 2.0f, 0.8f, 0.6f, 0.9f, 0.7f },
		{ TEXT("Emote_Giggle"), TEXT("AM_Emote_Giggle"), EMontageCategory::Emote, 1.5f, 0.7f, 0.6f, 0.7f, 0.5f },
		{ TEXT("Emote_Sigh"), TEXT("AM_Emote_Sigh"), EMontageCategory::Emote, 2.0f, 0.3f, 0.4f, -0.5f, 0.2f },
		{ TEXT("Emote_Gasp"), TEXT("AM_Emote_Gasp"), EMontageCategory::Emote, 1.0f, 0.5f, 0.7f, 0.1f, 0.9f },
		{ TEXT("Emote_Pout"), TEXT("AM_Emote_Pout"), EMontageCategory::Emote, 2.0f, 0.4f, 0.4f, -0.4f, 0.3f },
		{ TEXT("Emote_Wink"), TEXT("AM_Emote_Wink"), EMontageCategory::Emote, 0.5f, 0.8f, 0.6f, 0.6f, 0.5f },
		{ TEXT("Emote_Blush"), TEXT("AM_Emote_Blush"), EMontageCategory::Emote, 2.0f, 0.4f, 0.5f, 0.3f, 0.5f },
		{ TEXT("Emote_Facepalm"), TEXT("AM_Emote_Facepalm"), EMontageCategory::Emote, 2.0f, 0.4f, 0.4f, -0.4f, 0.5f },
		{ TEXT("Emote_Shrug"), TEXT("AM_Emote_Shrug"), EMontageCategory::Emote, 1.5f, 0.5f, 0.5f, 0.0f, 0.3f },
		{ TEXT("Emote_Nod"), TEXT("AM_Emote_Nod"), EMontageCategory::Emote, 1.0f, 0.5f, 0.5f, 0.3f, 0.3f },
		{ TEXT("Emote_Shake"), TEXT("AM_Emote_Shake"), EMontageCategory::Emote, 1.0f, 0.5f, 0.5f, -0.2f, 0.3f },
		{ TEXT("Gesture_Wave"), TEXT("AM_Gesture_Wave"), EMontageCategory::Gesture, 2.0f, 0.8f, 0.5f, 0.5f, 0.4f },
		{ TEXT("Gesture_Point"), TEXT("AM_Gesture_Point"), EMontageCategory::Gesture, 1.5f, 0.6f, 0.5f, 0.1f, 0.4f },
		{ TEXT("Gesture_Beckon"), TEXT("AM_Gesture_Beckon"), EMontageCategory::Gesture, 2.0f, 0.7f, 0.5f, 0.4f, 0.4f },
		{ TEXT("Gesture_ThumbsUp"), TEXT("AM_Gesture_ThumbsUp"), EMontageCategory::Gesture, 1.5f, 0.7f, 0.5f, 0.6f, 0.4f },
		{ TEXT("Gesture_ThumbsDown"), TEXT("AM_Gesture_ThumbsDown"), EMontageCategory::Gesture, 1.5f, 0.5f, 0.4f, -0.4f, 0.4f },
		{ TEXT("Gesture_Explain"), TEXT("AM_Gesture_Explain"), EMontageCategory::Gesture, 4.0f, 0.6f, 0.7f, 0.2f, 0.4f },
		{ TEXT("Gesture_CountFingers"), TEXT("AM_Gesture_CountFingers"), EMontageCategory::Gesture, 3.0f, 0.5f, 0.6f, 0.1f, 0.3f },
		{ TEXT("Gesture_Shush"), TEXT("AM_Gesture_Shush"), EMontageCategory::Gesture, 2.0f, 0.4f, 0.4f, 0.0f, 0.3f },
		{ TEXT("Gesture_ComeHere"), TEXT("AM_Gesture_ComeHere"), EMontageCategory::Gesture, 2.0f, 0.7f, 0.5f, 0.4f, 0.5f },
		{ TEXT("Gesture_Stop"), TEXT("AM_Gesture_Stop"), EMontageCategory::Gesture, 1.5f, 0.5f, 0.4f, -0.1f, 0.5f },
	};

	int32 GetRequiredMontageCount()
	{
		return UE_ARRAY_COUNT(GCatalog);
	}

	TArray<FName> GetRequiredMontageIds()
	{
		TArray<FName> Ids;
		Ids.Reserve(GetRequiredMontageCount());
		for (const FCatalogRow& Row : GCatalog)
		{
			Ids.Add(Row.Id);
		}
		return Ids;
	}

	void FillDefaultMontageLibrary(UMontageLibraryDataAsset& Library)
	{
		Library.LibraryId = TEXT("DTE");
		Library.LibraryName = NSLOCTEXT("DTE", "MontageLibrary", "Deep Tree Echo Montage Library");
		Library.Montages.Reset();
		Library.Montages.Reserve(GetRequiredMontageCount());

		for (const FCatalogRow& Row : GCatalog)
		{
			FMontageEntry Entry;
			Entry.MontageId = Row.Id;
			Entry.DisplayName = FText::FromString(Row.Id);
			Entry.Category = Row.Category;
			Entry.Cooldown = Row.Duration * 0.5f;
			Entry.bCanInterrupt = true;
			Entry.Priority = (Row.Category == EMontageCategory::Emote) ? 20 : ((Row.Category == EMontageCategory::Gesture) ? 10 : 0);

			FMontageVariant Variant;
			Variant.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(MontagePath(Row.Asset)));
			Variant.PlayRate = 1.0f;
			Variant.ExtraversionWeight = Row.Extraversion;
			Variant.OpennessWeight = Row.Openness;
			Variant.ValenceWeight = Row.Valence;
			Variant.ArousalWeight = Row.Arousal;
			Entry.Variants.Add(Variant);

			Library.Montages.Add(Entry);
		}
	}
}
