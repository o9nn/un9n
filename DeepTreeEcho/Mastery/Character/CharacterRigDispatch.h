// CharacterRigDispatch.h
// Routes a pose to the backend the character template actually declares.
//
// WHY THIS FILE EXISTS
//
// CharacterTemplate.h described the EXPRESSION layer as "the seam with the rest of the module"
// and said the rig standard is recorded "so the correct backend is selected rather than assumed".
// Nothing selected anything. EFacialRigStandard was written and never read, and
// FCharacterExpression::VerifiedShapeNames had no consumer at all - the template declared what
// the rig could do and the backends independently guessed at runtime, with no link between the
// two notions. The contract was documentation, not a mechanism.
//
// The gap is easy to miss because both halves look finished on their own. It shows up the moment
// you ask Christopher Alexander's question, in the form Anthony Judge gave it when he factored
// Alexander's patterns into a content-free Template plus one elaboration per domain: for every
// slot the template declares, WHERE IS THE REALIZATION? Geometry, Appearance, Expression and
// Motion each had a struct. Identity had none. Expression had a struct but no code that acted on
// it. A declared slot with no realization is not a small omission - it is the part of the design
// that was never actually built, wearing the same typeface as the parts that were.
//
// REFUSING TO GUESS IS THE POINT
//
// The dangerous outcome here is not an error, it is a near miss. Routing an ARKit52 rig to the
// MetaHuman backend writes curve names like "browRaiseInner" to a rig that only knows
// "browInnerUp": every write silently no-ops and the character sits there with a perfectly still
// face while the logs stay clean. StandaloneMetaHumanBackend already asserts the two vocabularies
// are distinct for exactly this reason. So an undeclared or unimplemented standard is refused
// explicitly rather than approximated.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTemplate.h"
#include "../MasteryEmbodimentPose.h"
#include "../Backends/MelodyLive2DBackend.h"
#include "../Backends/MasteryBackendMetaHuman.h"

/** Outcome of a dispatch attempt. Every non-Applied value means nothing was written. */
enum class ERigDispatchResult : uint8
{
    /** The pose reached a backend. */
    Applied = 0,
    /** Template declares EFacialRigStandard::None - there is nothing to target. */
    NoStandardDeclared,
    /** Standard is real but this module implements no backend for it (ARKit52, Custom). */
    NoBackendForStandard,
    /** Standard is supported but the caller supplied no sink of the matching type. */
    SinkNotSupplied,
};

namespace CharacterRigDispatch
{
    inline const TCHAR* ResultName(ERigDispatchResult R)
    {
        switch (R)
        {
        case ERigDispatchResult::Applied:              return TEXT("Applied");
        case ERigDispatchResult::NoStandardDeclared:   return TEXT("NoStandardDeclared");
        case ERigDispatchResult::NoBackendForStandard: return TEXT("NoBackendForStandard");
        default:                                       return TEXT("SinkNotSupplied");
        }
    }

    /** True if this module can drive the given standard at all. */
    inline bool IsStandardSupported(EFacialRigStandard Standard)
    {
        return Standard == EFacialRigStandard::Live2DCubism
            || Standard == EFacialRigStandard::MetaHumanControlRig;
    }

    /**
     * Apply a pose through whichever backend the template declares.
     *
     * Both sinks are passed and the unused one may be null; the template decides, not the caller.
     * That ordering matters - a caller that picks the backend itself has already made the
     * assumption this file exists to prevent.
     */
    inline ERigDispatchResult ApplyPose(const FCharacterTemplate& Template,
                                        const FMasteryEmbodimentPose& Pose,
                                        IMelodyRigSink* Live2DSink,
                                        IMetaHumanRigSink* MetaHumanSink)
    {
        switch (Template.Expression.Standard)
        {
        case EFacialRigStandard::Live2DCubism:
            if (Live2DSink == nullptr)
            {
                return ERigDispatchResult::SinkNotSupplied;
            }
            MelodyLive2DBackend::ApplyPose(*Live2DSink, Pose);
            return ERigDispatchResult::Applied;

        case EFacialRigStandard::MetaHumanControlRig:
            if (MetaHumanSink == nullptr)
            {
                return ERigDispatchResult::SinkNotSupplied;
            }
            MasteryBackendMetaHuman::ApplyPose(*MetaHumanSink, Pose);
            return ERigDispatchResult::Applied;

        case EFacialRigStandard::ARKit52:
        case EFacialRigStandard::Custom:
            // Deliberately NOT routed to the MetaHuman backend. See the header note on near
            // misses: a wrong-vocabulary write is silent, and silence reads as "working".
            return ERigDispatchResult::NoBackendForStandard;

        default:
            return ERigDispatchResult::NoStandardDeclared;
        }
    }

    /**
     * Pre-flight check: which parameters would the backend try to write that the template does
     * NOT list in VerifiedShapeNames?
     *
     * This is what finally gives VerifiedShapeNames a job. The backends already degrade
     * gracefully at runtime - MelodyLive2DBackend::ApplyOptional and the MetaHuman Apply() both
     * skip anything the sink does not expose - but graceful degradation is invisible. A channel
     * that silently never reaches the rig looks identical to a channel the character simply is
     * not using, which is precisely how the Live2D brow-angle gap survived until it was measured.
     *
     * An empty VerifiedShapeNames means the template makes no claim, so nothing can be missing
     * from it; that returns empty rather than reporting every parameter as absent.
     */
    inline TArray<FString> FindUnverifiedParameters(const FCharacterTemplate& Template,
                                                    const FMasteryEmbodimentPose& Pose)
    {
        TArray<FString> Missing;

        if (Template.Expression.VerifiedShapeNames.Num() == 0)
        {
            return Missing;   // no claim made, nothing to contradict
        }

        // Recording sink that accepts everything, so we learn what the backend WOULD drive on a
        // fully-featured rig rather than what it settles for on this one.
        struct FProbe : public IMelodyRigSink, public IMetaHumanRigSink
        {
            TArray<FString> Written;
            bool HasParameter(const FString&) const override { return true; }
            void SetParameter(const FString& Id, float) override { Written.Add(Id); }
            bool HasCurve(const FString&) const override { return true; }
            void SetCurve(const FString& Name, float) override { Written.Add(Name); }
        };

        FProbe Probe;
        if (ApplyPose(Template, Pose, &Probe, &Probe) != ERigDispatchResult::Applied)
        {
            return Missing;
        }

        for (int32 i = 0; i < Probe.Written.Num(); ++i)
        {
            bool bFound = false;
            for (int32 j = 0; j < Template.Expression.VerifiedShapeNames.Num(); ++j)
            {
                if (Template.Expression.VerifiedShapeNames[j] == Probe.Written[i])
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                bool bAlreadyReported = false;
                for (int32 k = 0; k < Missing.Num(); ++k)
                {
                    if (Missing[k] == Probe.Written[i]) { bAlreadyReported = true; break; }
                }
                if (!bAlreadyReported)
                {
                    Missing.Add(Probe.Written[i]);
                }
            }
        }
        return Missing;
    }
}
