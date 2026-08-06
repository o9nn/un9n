// StandaloneCharacterTemplate.cpp
// Does the character template's declared contract actually DO anything?
//
// The audit that produced this harness came from Anthony Judge's 1984 factoring of Christopher
// Alexander's pattern language, which splits every pattern into a content-free Template plus one
// elaboration per domain. Applied to CharacterTemplate.h it asks a blunt question: for every slot
// the template declares, where is the realization? Three answers were missing.
//
//   [1] ECharacterLayer declared five layers and Validate() enforced authority over IDENTITY, but
//       there was no FCharacterIdentity - a layer that could be policed but not populated.
//   [2] EFacialRigStandard was written and never read. The header called EXPRESSION "the seam
//       with the rest of the module" and said the standard is recorded "so the correct backend is
//       selected rather than assumed"; no code selected anything.
//   [3] FCharacterExpression::VerifiedShapeNames had no consumer, so the template's account of
//       what the rig exposes and the backends' runtime guards were two unrelated notions of the
//       same fact.
//
// All three are the same failure, and it is one a reader cannot see: a declared slot with no
// realization looks exactly like a finished one. Only asking for the realization finds it.
//
// Build & run:
//   g++ -std=c++17 -O2 -I StandaloneShim -o chartmpl StandaloneCharacterTemplate.cpp

#include <cstdio>
#include <string>

#include "../Character/CharacterRigDispatch.h"
#include "../Personas/MelodyPersona.h"

namespace
{

int TestsFailed = 0;

void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

/** Records everything written, accepting all names. */
struct FRecorder : public IMelodyRigSink, public IMetaHumanRigSink
{
    TArray<FString> Written;
    bool HasParameter(const FString&) const override { return true; }
    void SetParameter(const FString& Id, float) override { Written.Add(Id); }
    bool HasCurve(const FString&) const override { return true; }
    void SetCurve(const FString& Name, float) override { Written.Add(Name); }

    bool Wrote(const char* Name) const
    {
        for (int32 i = 0; i < Written.Num(); ++i)
        {
            if (Written[i] == FString(Name)) return true;
        }
        return false;
    }
};

FMasteryEmbodimentPose ExpressivePose()
{
    FMasteryEmbodimentPose P;
    P.ExpressionIntensity = 1.0f;
    P.BrowRaise = 0.5f;
    P.EyeNarrow = 0.4f;
    P.MouthCornerUp = 0.6f;
    P.Asymmetry = 0.4f;
    P.HeadTilt = 0.3f;
    return P;
}

/** A template that is complete enough to pass validation, for the given rig standard. */
FCharacterTemplate BuildableTemplate(EFacialRigStandard Standard)
{
    FCharacterTemplate T;
    T.CharacterName = TEXT("Melody");

    T.References.Add(FCharacterReference::FromSource(
        TEXT("bind-pose render, front"), TEXT("/ref/bind_front.png"),
        EReferenceSource::Render3D));
    T.References.Add(FCharacterReference::FromSource(
        TEXT("portrait, three-quarter"), TEXT("/ref/portrait_01.png"),
        EReferenceSource::Generated));

    T.Geometry.BindMeshPath = TEXT("/mesh/melody_bind");
    T.Geometry.BindPose = EBindPose::APose;
    T.Geometry.SkeletonId = TEXT("Genesis9");

    T.Identity.SourceHeadMeshPath = TEXT("/mesh/melody_head");
    T.Identity.SculptTargetRefLabels.Add(TEXT("portrait, three-quarter"));

    T.Expression.Standard = Standard;
    return T;
}

} // namespace

int main()
{
    std::printf("=== Character template: is the declared contract realized? ===\n\n");

    // ------------------------------------------------------------------ [1] identity layer
    std::printf("[1] the IDENTITY layer can be populated, not merely policed\n");
    {
        FCharacterTemplate T = BuildableTemplate(EFacialRigStandard::Live2DCubism);
        TArray<FString> Problems;
        const int32 N = T.Validate(Problems);
        for (int32 i = 0; i < Problems.Num(); ++i)
        {
            std::printf("      - %s\n", Problems[i].Str.c_str());
        }
        Check(N == 0, "a complete template validates clean");

        // The layer's reason for existing: an identity originated from a generated source is
        // reported rather than shipped silently.
        FCharacterTemplate Bad = T;
        Bad.Identity.bOriginatedFromGeneratedSource = true;
        TArray<FString> BadProblems;
        Check(Bad.Validate(BadProblems) > 0,
              "an identity originated from a generated source is reported");

        // And an identity slot with authority but no actual mesh recorded is caught - the exact
        // hole that existed while the layer had no struct at all.
        FCharacterTemplate Empty = T;
        Empty.Identity.SourceHeadMeshPath = FString();
        TArray<FString> EmptyProblems;
        Check(Empty.Validate(EmptyProblems) > 0,
              "an IDENTITY layer with no recorded head mesh is reported");
    }

    // ------------------------------------------------------------------ [2] the seam is real
    std::printf("\n[2] the declared rig standard actually selects the backend\n");
    {
        FRecorder L2D, MH;
        const FCharacterTemplate Live2D = BuildableTemplate(EFacialRigStandard::Live2DCubism);
        const auto R1 = CharacterRigDispatch::ApplyPose(Live2D, ExpressivePose(), &L2D, &MH);

        std::printf("      Live2DCubism -> %s   live2d writes=%d metahuman writes=%d\n",
                    CharacterRigDispatch::ResultName(R1), L2D.Written.Num(), MH.Written.Num());
        Check(R1 == ERigDispatchResult::Applied, "a Live2D template dispatches");
        Check(L2D.Wrote("ParamEyeLOpen"), "  and drives Live2D parameters");
        Check(MH.Written.Num() == 0, "  and leaves the MetaHuman sink untouched");

        FRecorder L2D2, MH2;
        const FCharacterTemplate Mh = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
        const auto R2 = CharacterRigDispatch::ApplyPose(Mh, ExpressivePose(), &L2D2, &MH2);

        std::printf("      MetaHuman    -> %s   live2d writes=%d metahuman writes=%d\n",
                    CharacterRigDispatch::ResultName(R2), L2D2.Written.Num(), MH2.Written.Num());
        Check(R2 == ERigDispatchResult::Applied, "a MetaHuman template dispatches");
        Check(MH2.Wrote("mouthCornerPullL"), "  and drives MetaHuman curves");
        Check(L2D2.Written.Num() == 0, "  and leaves the Live2D sink untouched");
    }

    // ------------------------------------------------------------------ [3] refuse to guess
    std::printf("\n[3] an unimplemented standard is REFUSED, not approximated\n");
    {
        // The failure this prevents is silent. MetaHuman curve names written to an ARKit rig
        // no-op on every single write, so the character holds a dead-still face and nothing in
        // the logs objects. StandaloneMetaHumanBackend already pins the two vocabularies apart.
        FRecorder L2D, MH;
        FCharacterTemplate ARKit = BuildableTemplate(EFacialRigStandard::ARKit52);
        const auto R = CharacterRigDispatch::ApplyPose(ARKit, ExpressivePose(), &L2D, &MH);

        std::printf("      ARKit52      -> %s   total writes=%d\n",
                    CharacterRigDispatch::ResultName(R), L2D.Written.Num() + MH.Written.Num());
        Check(R == ERigDispatchResult::NoBackendForStandard,
              "ARKit52 is refused - no backend exists for it here");
        Check(L2D.Written.Num() == 0 && MH.Written.Num() == 0,
              "  and absolutely nothing is written to either rig");

        TArray<FString> Problems;
        Check(ARKit.Validate(Problems) > 0,
              "  and validation says so up front, not at runtime");

        // None is likewise refused rather than defaulted to a backend.
        FRecorder L3, M3;
        FCharacterTemplate NoStd = BuildableTemplate(EFacialRigStandard::None);
        Check(CharacterRigDispatch::ApplyPose(NoStd, ExpressivePose(), &L3, &M3)
                  == ERigDispatchResult::NoStandardDeclared,
              "an undeclared standard is refused");
        Check(L3.Written.Num() == 0 && M3.Written.Num() == 0,
              "  and writes nothing");

        // A supported standard with no sink of the matching type must not fall through to the
        // other backend just because one happens to be available.
        FRecorder WrongRig;
        const FCharacterTemplate Live2D = BuildableTemplate(EFacialRigStandard::Live2DCubism);
        Check(CharacterRigDispatch::ApplyPose(Live2D, ExpressivePose(), nullptr, &WrongRig)
                  == ERigDispatchResult::SinkNotSupplied,
              "a missing sink is refused rather than falling back to the other backend");
        Check(WrongRig.Written.Num() == 0, "  and the wrong rig is not written to");
    }

    // ------------------------------------------------------------------ [4] VerifiedShapeNames
    std::printf("\n[4] VerifiedShapeNames finally has a consumer\n");
    {
        FCharacterTemplate T = BuildableTemplate(EFacialRigStandard::Live2DCubism);

        // No claim made -> nothing can be missing.
        Check(CharacterRigDispatch::FindUnverifiedParameters(T, ExpressivePose()).Num() == 0,
              "an empty VerifiedShapeNames makes no claim, so reports nothing");

        // A rig that only exposes the eyes: everything else the binding drives is unreachable,
        // and the backends' graceful degradation would hide that completely.
        T.Expression.VerifiedShapeNames.Add(TEXT("ParamEyeLOpen"));
        T.Expression.VerifiedShapeNames.Add(TEXT("ParamEyeROpen"));

        const TArray<FString> Missing =
            CharacterRigDispatch::FindUnverifiedParameters(T, ExpressivePose());

        std::printf("      partial rig is missing %d driven parameters:\n", Missing.Num());
        for (int32 i = 0; i < Missing.Num() && i < 6; ++i)
        {
            std::printf("        %s\n", Missing[i].Str.c_str());
        }
        Check(Missing.Num() > 0,
              "a partial rig reports the parameters it cannot receive");

        bool bReportsMouth = false;
        for (int32 i = 0; i < Missing.Num(); ++i)
        {
            if (Missing[i] == FString(TEXT("ParamMouthForm"))) { bReportsMouth = true; }
        }
        Check(bReportsMouth, "  including the mouth, which this rig genuinely cannot drive");

        bool bReportsEyes = false;
        for (int32 i = 0; i < Missing.Num(); ++i)
        {
            if (Missing[i] == FString(TEXT("ParamEyeLOpen"))) { bReportsEyes = true; }
        }
        Check(!bReportsEyes, "  and NOT the eyes, which it does expose");
    }

    // ------------------------------------------------------------------ [5] authority coherence
    std::printf("\n[5] source authority matches each layer's documented owner\n");
    {
        const FCharacterReference Photo = FCharacterReference::FromSource(
            TEXT("photo"), TEXT("/p.png"), EReferenceSource::Photograph);
        const FCharacterReference Gen = FCharacterReference::FromSource(
            TEXT("gen"), TEXT("/g.png"), EReferenceSource::Generated);
        const FCharacterReference Render = FCharacterReference::FromSource(
            TEXT("render"), TEXT("/r.png"), EReferenceSource::Render3D);

        // EXPRESSION is owned by the rig standard. No reference asset - photo or otherwise - can
        // be authoritative about what curves a rig exposes; it can only be a target to match.
        Check(Photo.AuthorityFor(ECharacterLayer::Expression) != EReferenceAuthority::Authoritative,
              "no source claims authority over EXPRESSION - the rig standard owns it");

        // Generated stays Advisory for geometry: usable as a sculpt/wrap target, never as the
        // origin of a riggable surface.
        Check(Gen.AuthorityFor(ECharacterLayer::Geometry) == EReferenceAuthority::Advisory,
              "generated art is Advisory for GEOMETRY - a target, not a measurement");
        Check(Gen.AuthorityFor(ECharacterLayer::Appearance) == EReferenceAuthority::Authoritative,
              "  but authoritative for APPEARANCE, which it genuinely settles");
        Check(Render.AuthorityFor(ECharacterLayer::Geometry) == EReferenceAuthority::Authoritative,
              "a render of the actual model is authoritative for GEOMETRY");

        // A default-constructed reference must grant nothing.
        FCharacterReference Bare;
        bool bAnyGranted = false;
        for (int32 i = 0; i < static_cast<int32>(ECharacterLayer::COUNT); ++i)
        {
            if (Bare.Authority[i] != EReferenceAuthority::Forbidden) bAnyGranted = true;
        }
        Check(!bAnyGranted, "a hand-built reference grants nothing until authority is stated");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "CharacterTemplate=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
