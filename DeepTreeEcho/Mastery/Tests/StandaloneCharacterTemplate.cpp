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
struct FRecorder : public IMelodyRigSink, public IMetaHumanRigSink, public IBodySkeletonSink
{
    TArray<FString> Written;
    TArray<FString> Bones;
    bool HasBone(const FString&) const override { return true; }
    void SetBoneRotationOffset(const FString& B, float, float, float) override { Bones.Add(B); }
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

    // Each face standard gets its matching body, so "buildable" still means buildable now that a
    // missing body rig is a validation problem.
    if (Standard == EFacialRigStandard::Live2DCubism)
    {
        T.Motion.BodyStandard = EBodyRigStandard::Live2DCubism;
    }
    else if (Standard == EFacialRigStandard::MetaHumanControlRig)
    {
        T.Geometry.SkeletonId = TEXT("MetaHuman");
        T.Motion.BodyStandard = EBodyRigStandard::UE5Skeleton;
    }
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

    // ------------------------------------------------------------------ [5] the body is routed
    std::printf("\n[5] the declared BODY rig selects the body backend\n");
    {
        // A real frame from the driver, mid-reaction, so every body output is non-zero.
        FMasteryEmbodimentPose Pose = ExpressivePose();
        Pose.MicroMovementRate = 1.0f;
        Pose.ShoulderTension = 1.0f;
        MasteryBackendBody::FDriver Driver(42u);
        Driver.React(1.0f, Pose);
        for (int32 i = 0; i < 30; ++i) Driver.Tick(Pose, 1.0f / 60.0f);
        const FBodyMotionFrame Frame = Driver.Current();

        // MetaHuman face + UE5 body: both routes, each to its own sink.
        {
            FRecorder Rec;
            const FCharacterTemplate Mh = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
            const FEmbodimentDispatchResult R =
                CharacterRigDispatch::ApplyEmbodiment(Mh, Pose, &Frame, &Rec, &Rec, &Rec);
            std::printf("      MetaHuman: face=%s body=%s  curves=%d bones=%d\n",
                        CharacterRigDispatch::ResultName(R.Face), CharacterRigDispatch::ResultName(R.Body),
                        Rec.Written.Num(), Rec.Bones.Num());
            Check(R.Face == ERigDispatchResult::Applied && R.Body == ERigDispatchResult::Applied,
                  "a MetaHuman template routes the face AND the body");
            Check(Rec.Bones.Num() == 12, "  all 12 declared joints receive the body frame");
        }

        // Live2D: the body travels inside the face call, so it changes the face's own writes.
        {
            FRecorder FaceOnly, Full;
            const FCharacterTemplate L2 = BuildableTemplate(EFacialRigStandard::Live2DCubism);
            CharacterRigDispatch::ApplyPose(L2, Pose, &FaceOnly, nullptr);
            const FEmbodimentDispatchResult R =
                CharacterRigDispatch::ApplyEmbodiment(L2, Pose, &Frame, &Full, nullptr, nullptr);
            std::printf("      Live2D:    face=%s body=%s  params face-only=%d with-body=%d\n",
                        CharacterRigDispatch::ResultName(R.Face), CharacterRigDispatch::ResultName(R.Body),
                        FaceOnly.Written.Num(), Full.Written.Num());
            Check(R.Body == ERigDispatchResult::Applied && Full.Wrote("ParamBodyAngleY") &&
                  !FaceOnly.Wrote("ParamBodyAngleY"),
                  "a Live2D template carries the body inside the face call - no skeleton sink needed");
        }

        // Refusals: the body analogue of routing an ARKit face to the MetaHuman backend.
        {
            FRecorder Rec;
            FCharacterTemplate Genesis = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
            Genesis.Geometry.SkeletonId = TEXT("Genesis9");
            const FEmbodimentDispatchResult R =
                CharacterRigDispatch::ApplyEmbodiment(Genesis, Pose, &Frame, &Rec, &Rec, &Rec);
            Check(R.Body == ERigDispatchResult::NoBackendForStandard && Rec.Bones.Num() == 0,
                  "a UE5 body on a Genesis9 skeleton is REFUSED - no bone writes that would silently miss");
            TArray<FString> P;
            Check(Genesis.Validate(P) > 0, "  and Validate() reports it before runtime does");

            FCharacterTemplate Mixed = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
            Mixed.Motion.BodyStandard = EBodyRigStandard::Live2DCubism;
            TArray<FString> P2;
            Check(CharacterRigDispatch::ApplyEmbodiment(Mixed, Pose, &Frame, &Rec, &Rec, &Rec).Body
                      == ERigDispatchResult::NoBackendForStandard && Mixed.Validate(P2) > 0,
                  "a Live2D body without a Live2D face is refused and reported");

            FCharacterTemplate NoBody = BuildableTemplate(EFacialRigStandard::Live2DCubism);
            NoBody.Motion.BodyStandard = EBodyRigStandard::None;
            TArray<FString> P3;
            Check(CharacterRigDispatch::ApplyEmbodiment(NoBody, Pose, &Frame, &Rec, nullptr, nullptr).Body
                      == ERigDispatchResult::NoStandardDeclared && NoBody.Validate(P3) > 0,
                  "an undeclared body rig is reported - mastery would show only in the face");
        }

        // Missing inputs are named, not collapsed into one failure.
        {
            FRecorder Rec;
            const FCharacterTemplate Mh = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
            Check(CharacterRigDispatch::ApplyEmbodiment(Mh, Pose, nullptr, &Rec, &Rec, &Rec).Body
                      == ERigDispatchResult::MotionNotSupplied,
                  "no body frame -> MotionNotSupplied");
            Check(CharacterRigDispatch::ApplyEmbodiment(Mh, Pose, &Frame, &Rec, &Rec, nullptr).Body
                      == ERigDispatchResult::SinkNotSupplied,
                  "no skeleton sink -> SinkNotSupplied");
        }

        // VerifiedShapeNames now sees what the body adds to a Live2D rig.
        {
            FCharacterTemplate T = BuildableTemplate(EFacialRigStandard::Live2DCubism);
            T.Expression.VerifiedShapeNames.Add(TEXT("ParamEyeLOpen"));
            auto Contains = [](const TArray<FString>& A, const TCHAR* N) {
                for (int32 i = 0; i < A.Num(); ++i) if (A[i] == FString(N)) return true;
                return false;
            };
            Check(!Contains(CharacterRigDispatch::FindUnverifiedParameters(T, Pose), TEXT("ParamBodyAngleY")) &&
                  Contains(CharacterRigDispatch::FindUnverifiedParameters(T, Pose, &Frame), TEXT("ParamBodyAngleY")),
                  "the pre-flight check reports body-only parameters when the body is routed");
        }
    }

    // ------------------------------------------------------------------ [5b] VerifiedBoneNames
    std::printf("\n[5b] VerifiedBoneNames: the body's pre-flight check\n");
    {
        FCharacterTemplate T = BuildableTemplate(EFacialRigStandard::MetaHumanControlRig);
        Check(CharacterRigDispatch::FindUnverifiedBones(T).Num() == 0,
              "an empty VerifiedBoneNames makes no claim, so reports nothing");

        // A skeleton with the spine and neck but no shoulders and no eye joints - e.g. a body-only
        // export with the face rig stripped.
        const TCHAR* Present[] = { TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
                                   TEXT("spine_04"), TEXT("spine_05"), TEXT("neck_01"), TEXT("neck_02") };
        for (const TCHAR* B : Present) T.Motion.VerifiedBoneNames.Add(B);

        const TArray<FString> Missing = CharacterRigDispatch::FindUnverifiedBones(T);
        std::printf("      partial skeleton is missing %d driven joints:", Missing.Num());
        for (int32 i = 0; i < Missing.Num(); ++i) std::printf(" %s", Missing[i].Str.c_str());
        std::printf("\n");
        Check(Missing.Num() == 4 && Missing.Contains(FString(TEXT("clavicle_l"))) &&
              Missing.Contains(FString(TEXT("FACIAL_R_Eye"))),
              "reports exactly the 4 driven joints the rig lacks - both clavicles, both eyes");
        Check(!Missing.Contains(FString(TEXT("spine_03"))), "  and none of the joints it has");

        // A refused template reports nothing: those bones will never be written anyway, and the
        // refusal itself is what Validate() and ApplyEmbodiment already surface.
        FCharacterTemplate Genesis = T;
        Genesis.Geometry.SkeletonId = TEXT("Genesis9");
        Check(CharacterRigDispatch::FindUnverifiedBones(Genesis).Num() == 0,
              "a template whose body is refused reports no bones - it will never drive them");

        FCharacterTemplate L2 = BuildableTemplate(EFacialRigStandard::Live2DCubism);
        L2.Motion.VerifiedBoneNames.Add(TEXT("pelvis"));
        Check(CharacterRigDispatch::FindUnverifiedBones(L2).Num() == 0,
              "a Live2D body drives no joints, so a bone list cannot be contradicted");
    }

    // ------------------------------------------------------------------ [6] authority coherence
    std::printf("\n[6] source authority matches each layer's documented owner\n");
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
