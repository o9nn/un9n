// AuthoredExpressionSet.h
// The authored expression catalog, expressed as points in FMasteryEmbodimentPose space.
//
// WHY THIS EXISTS
//
// The Mastery binding produces a CONTINUOUS pose from measured competence. The artist authored a
// DISCRETE library of named expressions (JOY_01..06, WONDER_01..04, PUNK_01..05, ...) each with a
// documented semantic trigger. Nothing connected the two, which meant one important claim was
// unfalsifiable: that a competitive, winning Melody actually READS as competitive.
//
// She could be measurably excellent, drive a perfectly self-consistent pose, land inside the
// Live2D rig's authored envelope - and still wear the wrong face for the moment, because
// "self-consistent" and "correct register" are different properties. Every existing harness
// checks the first. This file makes the second checkable, by placing the named expressions in
// the same space the binding writes into and asking which one a given pose is nearest.
//
// PROVENANCE. Prototype values are transcribed from the user's authored analysis documents -
// semantic_affective_patterns.md section 1.2 (the consolidated library) and
// video_analysis_3db2fa59.md (the PUNK spectrum, from the punk-variant video). They are NOT
// tuned to make any test pass. That distinction is the whole value of the file: if a prototype
// is adjusted until a classification comes out right, the classifier stops measuring anything.
//
// TWO TRANSLATIONS were required, and both are judgment calls worth stating plainly:
//
//  1. CHANNEL COVERAGE. The catalog's morph vocabulary is wider than FMasteryEmbodimentPose.
//     mouth_open, cheek_raise, lip_pucker and nose_scrunch have no pose channel - cheek_raise is
//     derived inside the backends (the Duchenne term), the others are speech/affect shapes the
//     binding does not own. They are excluded from the distance rather than faked. So this
//     classifies on the nine channels the two vocabularies share, and expressions that differ
//     ONLY in an excluded channel are not distinguishable here. PLAY_01 (a lip pucker) is the
//     clearest casualty and is deliberately absent.
//
//  2. ASYMMETRY. The catalog encodes asymmetry as an explicit L/R value split (PUNK_01 is
//     mouth_smile_L 0.5 against mouth_smile_R 0.3). FMasteryEmbodimentPose carries a single
//     scalar instead. The conversion used is Asymmetry ~= (L - R) / 0.3, i.e. a 0.3 spread is
//     treated as fully asymmetric, since 0.3 is the largest spread anywhere in the catalog.
//     This is the one place a number here is mine rather than the artist's.
//
// UNSPECIFIED IS NOT ZERO. Each prototype declares WHICH channels the catalog actually pins
// down, and only those enter the distance. This matters more than it sounds. The catalog lists a
// channel when it is a defining feature of that expression, not to assert every other channel is
// at rest - JOY_01 does not specify asymmetry because asymmetry is not what makes it a broad
// smile, and PUNK_02's source says "head tilted with confident posture" while giving no angle.
// Reading those silences as hard zeroes made every prototype a much stronger claim than the
// artist made, and it produced exactly the error you would predict: an ordinary face carrying
// the binding's deliberate 0.15 non-synthetic asymmetry floor looked like evidence of a smirk,
// because the JOY prototypes had been made perfectly symmetric by omission.
//
// Distance is therefore normalized by the total weight of the channels a prototype specifies, so
// a tightly-specified expression is not penalized against a loosely-specified one.

#pragma once

#include "CoreMinimal.h"
#include "../MasteryEmbodimentPose.h"

/** Expression families, in the catalog's own grouping. */
enum class EAuthoredExpressionCategory : uint8
{
    Neutral,
    Joy,
    Wonder,
    Punk,      // the competitive/challenge register
    Focus,
    Vision,
};

/** Which pose channels a prototype actually constrains. Anything not listed is don't-care. */
enum EAuthoredChannel : uint32
{
    CH_None       = 0,
    CH_BrowRaise  = 1 << 0,
    CH_BrowFurrow = 1 << 1,
    CH_EyeWiden   = 1 << 2,
    CH_EyeNarrow  = 1 << 3,
    CH_Smile      = 1 << 4,
    CH_Frown      = 1 << 5,
    CH_ChinRaise  = 1 << 6,
    CH_Asymmetry  = 1 << 7,
    CH_HeadTilt   = 1 << 8,
};

struct FAuthoredExpression
{
    const TCHAR* Id = TEXT("");
    EAuthoredExpressionCategory Category = EAuthoredExpressionCategory::Neutral;

    /** Bitmask of channels the catalog pins down for this expression. */
    uint32 Spec = CH_None;

    /**
     * Whether a MASTERY signal should be able to produce this expression at all.
     *
     * The catalog describes the whole character, not only the competitive part of her. Some
     * entries have semantic triggers that competence simply does not carry - JOY_05 "Blissful
     * Serenity" is triggered by "transcendence, surrender, peace", which came from the singing
     * videos and has no counterpart in a duel. Marking it false says the gap is a scope boundary
     * rather than a defect, and lets the reachability sweep assert the exclusion explicitly
     * instead of quietly tolerating a hole.
     */
    bool bMasteryReachable = true;

    // Prototype coordinates. Only meaningful for channels present in Spec.
    float BrowRaise = 0.0f;
    float BrowFurrow = 0.0f;
    float EyeWiden = 0.0f;
    float EyeNarrow = 0.0f;
    float MouthCornerUp = 0.0f;
    float MouthCornerDown = 0.0f;
    float ChinRaise = 0.0f;
    float Asymmetry = 0.0f;
    float HeadTilt = 0.0f;
};

namespace AuthoredExpressionSet
{
    inline const TCHAR* CategoryName(EAuthoredExpressionCategory C)
    {
        switch (C)
        {
        case EAuthoredExpressionCategory::Joy:    return TEXT("JOY");
        case EAuthoredExpressionCategory::Wonder: return TEXT("WONDER");
        case EAuthoredExpressionCategory::Punk:   return TEXT("PUNK");
        case EAuthoredExpressionCategory::Focus:  return TEXT("FOCUS");
        case EAuthoredExpressionCategory::Vision: return TEXT("VISION");
        default:                                  return TEXT("NEUTRAL");
        }
    }

    /**
     * The library.
     *
     * eye_close in the catalog (JOY_05 "Blissful", eye_close 0.8-1.0) maps onto EyeNarrow, since
     * the pose struct expresses lid closure as narrowing rather than as a separate channel.
     */
    inline TArray<FAuthoredExpression> Library()
    {
        TArray<FAuthoredExpression> L;

        // ---- NEUTRAL --------------------------------------------------------------------------
        { FAuthoredExpression E; E.Id = TEXT("NEUTRAL_Reset");
          E.Category = EAuthoredExpressionCategory::Neutral;
          E.Spec = CH_Smile | CH_EyeNarrow | CH_EyeWiden | CH_BrowRaise | CH_ChinRaise;
          L.Add(E); }

        // ---- JOY (semantic_affective_patterns.md 1.2) ------------------------------------------
        { FAuthoredExpression E; E.Id = TEXT("JOY_01_BroadSmile");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_EyeNarrow;
          E.MouthCornerUp = 0.80f; E.EyeNarrow = 0.50f;
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("JOY_02_ExuberantLaugh");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_EyeNarrow | CH_HeadTilt;
          E.MouthCornerUp = 0.90f; E.EyeNarrow = 0.70f;
          E.HeadTilt = -0.50f;              // authored ParamAngleZ -4 .. -5 of an 8 degree envelope
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("JOY_03_GentleSmile");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_EyeNarrow;
          E.MouthCornerUp = 0.40f; E.EyeNarrow = 0.20f;
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("JOY_04_DelightedSurprise");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_EyeWiden | CH_BrowRaise;
          E.MouthCornerUp = 0.60f; E.EyeWiden = 0.30f; E.BrowRaise = 0.30f;
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("JOY_05_Blissful");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_EyeNarrow;
          E.MouthCornerUp = 0.40f; E.EyeNarrow = 0.90f;
          // Out of the mastery domain by design. Its triggers are "transcendence, surrender,
          // peace"; nothing a competence signal measures maps onto surrender. Reaching it would
          // require EyeNarrow ~0.90, while the binding tops out at 0.75 (0.45*Flow +
          // 0.3*Skill*Arousal) - a ceiling that is correct for absorption and wrong for bliss.
          // A speech or affect system owns this expression, the same way lip-sync owns
          // ParamMouthOpenY in the Live2D backend.
          E.bMasteryReachable = false;
          L.Add(E); }

        // JOY_06 is the catalog's own bridge toward the punk register: a confident grin that is
        // already asymmetric (smile_L 0.5 / smile_R 0.3, brow_raise_L 0.2).
        { FAuthoredExpression E; E.Id = TEXT("JOY_06_PlayfulGrin");
          E.Category = EAuthoredExpressionCategory::Joy;
          E.Spec = CH_Smile | CH_BrowRaise | CH_Asymmetry;
          E.MouthCornerUp = 0.40f; E.BrowRaise = 0.10f; E.Asymmetry = 0.67f;
          L.Add(E); }

        // ---- WONDER ----------------------------------------------------------------------------
        { FAuthoredExpression E; E.Id = TEXT("WONDER_01_Awe");
          E.Category = EAuthoredExpressionCategory::Wonder;
          E.Spec = CH_EyeWiden | CH_BrowRaise | CH_Smile;
          E.EyeWiden = 0.40f; E.BrowRaise = 0.30f;
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("WONDER_02_CuriousGaze");
          E.Category = EAuthoredExpressionCategory::Wonder;
          E.Spec = CH_EyeWiden | CH_BrowRaise | CH_HeadTilt;
          E.EyeWiden = 0.20f; E.BrowRaise = 0.20f;
          E.HeadTilt = 1.00f;               // authored ParamAngleZ +8, the envelope maximum
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("WONDER_03_Contemplative");
          E.Category = EAuthoredExpressionCategory::Wonder;
          E.Spec = CH_BrowFurrow | CH_EyeNarrow | CH_Smile;
          E.BrowFurrow = 0.10f; E.EyeNarrow = 0.10f;
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("WONDER_04_Discovery");
          E.Category = EAuthoredExpressionCategory::Wonder;
          E.Spec = CH_EyeWiden | CH_BrowRaise;
          E.EyeWiden = 0.50f; E.BrowRaise = 0.40f;
          L.Add(E); }

        // ---- PUNK (video_analysis_3db2fa59.md) -------------------------------------------------
        // The competitive register. Every member is asymmetric; that is what distinguishes the
        // smirk from the smile, and it is why the binding's Asymmetry channel has to rise with
        // engagement rather than with ease.
        { FAuthoredExpression E; E.Id = TEXT("PUNK_01_ConfidentSmirk");
          E.Category = EAuthoredExpressionCategory::Punk;
          E.Spec = CH_Smile | CH_BrowRaise | CH_EyeNarrow | CH_Asymmetry;
          E.MouthCornerUp = 0.40f; E.BrowRaise = 0.15f; E.EyeNarrow = 0.10f;
          E.Asymmetry = 0.67f;              // smile_L 0.5 / smile_R 0.3
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("PUNK_02_MischievousGrin");
          E.Category = EAuthoredExpressionCategory::Punk;
          E.Spec = CH_Smile | CH_EyeNarrow | CH_Asymmetry;
          E.MouthCornerUp = 0.50f; E.EyeNarrow = 0.30f;
          E.Asymmetry = 0.67f;              // smile_L 0.6 / smile_R 0.4
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("PUNK_03_DefiantConfidence");
          E.Category = EAuthoredExpressionCategory::Punk;
          E.Spec = CH_Smile | CH_EyeNarrow | CH_ChinRaise | CH_Asymmetry;
          E.MouthCornerUp = 0.45f; E.EyeNarrow = 0.20f; E.ChinRaise = 0.20f;
          E.Asymmetry = 0.33f;              // smile_L 0.5 / smile_R 0.4
          L.Add(E); }

        { FAuthoredExpression E; E.Id = TEXT("PUNK_05_KnowingSuperiority");
          E.Category = EAuthoredExpressionCategory::Punk;
          E.Spec = CH_Smile | CH_EyeNarrow | CH_Asymmetry;
          E.MouthCornerUp = 0.30f; E.EyeNarrow = 0.30f;
          E.Asymmetry = 0.67f;
          L.Add(E); }

        // ---- FOCUS ------------------------------------------------------------------------------
        { FAuthoredExpression E; E.Id = TEXT("FOCUS_01_AlertGaze");
          E.Category = EAuthoredExpressionCategory::Focus;
          E.Spec = CH_EyeWiden | CH_Smile | CH_EyeNarrow;
          E.EyeWiden = 0.10f;
          L.Add(E); }

        // ---- VISION -----------------------------------------------------------------------------
        { FAuthoredExpression E; E.Id = TEXT("VISION_02_ConfidentDeclaration");
          E.Category = EAuthoredExpressionCategory::Vision;
          E.Spec = CH_Smile | CH_EyeWiden | CH_ChinRaise;
          E.MouthCornerUp = 0.60f; E.EyeWiden = 0.10f; E.ChinRaise = 0.10f;
          L.Add(E); }

        return L;
    }

    struct FClassification
    {
        const TCHAR* Id = TEXT("");
        EAuthoredExpressionCategory Category = EAuthoredExpressionCategory::Neutral;
        float Distance = 0.0f;      // Euclidean in the shared nine channels
        float Confidence = 0.0f;    // separation from the runner-up, normalized [0,1]
    };

    /**
     * Nearest authored expression to a pose.
     *
     * Channels are weighted by how much discriminative work they do. Asymmetry carries the
     * heaviest weight because it is the single axis separating PUNK from JOY - in every other
     * respect a confident smirk and a gentle smile are close neighbours, which is precisely why
     * a binding that under-drives asymmetry will read as merely pleased when it should read as
     * dangerous.
     *
     * Confidence is the normalized gap to the next-nearest expression of a DIFFERENT category.
     * A low value means the pose sits on a boundary and the register is genuinely ambiguous -
     * which is information, not an error.
     */
    /**
     * Weight-normalized distance from a pose to one prototype, over the channels that prototype
     * actually specifies. Returns a large value for a prototype that specifies nothing, so an
     * empty entry can never win by default.
     */
    inline float PrototypeDistance(const FMasteryEmbodimentPose& Pose, const FAuthoredExpression& E)
    {
        float Sum = 0.0f;
        float TotalW = 0.0f;

        auto Accum = [&](uint32 Bit, float W, float PoseV, float ProtoV)
        {
            if ((E.Spec & Bit) == 0)
            {
                return;
            }
            const float D = PoseV - ProtoV;
            Sum += W * D * D;
            TotalW += W;
        };

        Accum(CH_BrowRaise,  1.0f, Pose.BrowRaise,       E.BrowRaise);
        Accum(CH_BrowFurrow, 1.0f, Pose.BrowFurrow,      E.BrowFurrow);
        Accum(CH_EyeWiden,   1.0f, Pose.EyeWiden,        E.EyeWiden);
        Accum(CH_EyeNarrow,  1.0f, Pose.EyeNarrow,       E.EyeNarrow);
        Accum(CH_Smile,      1.5f, Pose.MouthCornerUp,   E.MouthCornerUp);
        Accum(CH_Frown,      1.5f, Pose.MouthCornerDown, E.MouthCornerDown);
        Accum(CH_ChinRaise,  2.0f, Pose.ChinRaise,       E.ChinRaise);
        // Asymmetry carries the heaviest weight: it is the single axis separating PUNK from JOY.
        // In every other respect a confident smirk and a warm smile are close neighbours, which
        // is exactly why a binding that under-drives asymmetry reads as merely pleased when it
        // should read as dangerous.
        Accum(CH_Asymmetry,  2.5f, Pose.Asymmetry,       E.Asymmetry);
        Accum(CH_HeadTilt,   0.8f, Pose.HeadTilt,        E.HeadTilt);

        if (TotalW <= 0.0f)
        {
            return 1e9f;
        }
        return FMath::Sqrt(Sum / TotalW);
    }

    inline FClassification Classify(const FMasteryEmbodimentPose& Pose)
    {
        const TArray<FAuthoredExpression> Lib = Library();

        FClassification Best;
        float BestD = 1e9f;
        int32 BestIdx = -1;

        // Pass 1: nearest prototype overall.
        for (int32 i = 0; i < Lib.Num(); ++i)
        {
            const float D = PrototypeDistance(Pose, Lib[i]);
            if (D < BestD)
            {
                BestD = D;
                BestIdx = i;
            }
        }

        if (BestIdx < 0)
        {
            return Best;
        }

        Best.Id = Lib[BestIdx].Id;
        Best.Category = Lib[BestIdx].Category;
        Best.Distance = BestD;

        // Pass 2: nearest prototype of a DIFFERENT category. This cannot be folded into pass 1,
        // because "different category" is only defined once the winning category is known.
        float OtherD = 1e9f;
        for (int32 i = 0; i < Lib.Num(); ++i)
        {
            if (Lib[i].Category == Best.Category)
            {
                continue;
            }
            const float D = PrototypeDistance(Pose, Lib[i]);
            if (D < OtherD)
            {
                OtherD = D;
            }
        }

        Best.Confidence = (OtherD >= 1e8f)
            ? 1.0f
            : FMath::Clamp((OtherD - BestD) / 0.5f, 0.0f, 1.0f);
        return Best;
    }
}
