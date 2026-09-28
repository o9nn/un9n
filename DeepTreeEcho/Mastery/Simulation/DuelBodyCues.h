// DuelBodyCues.h
// Feeds what happens in a duel to the body layer, so a fighter's body answers the fight.
//
// MasteryBackendBody's two most important channels - MotionEconomy (does she overcorrect?) and
// ReactionSharpness (does she wind up?) - only exist in how the body answers a cue. Until this
// file nothing called FDriver::React(), so outside the tests those channels were computed,
// realized by the driver, and never exercised: a character who is never asked to move cannot
// show that she moves well.
//
// The duel is the obvious producer because it already knows, frame by frame, the events a body
// visibly answers. The mapping is a sign and a size:
//
//     positive = presses in        negative = pulls back
//
// Only the single strongest event on a frame is sent. Two cues on one frame would make the second
// overwrite the first inside React(), so choosing explicitly is the same outcome made legible.
//
// WHAT THIS DOES NOT DO. The cue carries no information about skill. A master and a novice who
// both get hit receive the identical React(-1.0); every visible difference in how they take it
// comes from their poses, i.e. from measured play through the binding. Scaling cues by skill here
// would put a second, unmeasured path from "who she is" to "how she moves" - exactly what the
// signal contract exists to prevent.

#pragma once

#include "CoreMinimal.h"
#include "DuelGame.h"
#include "../Backends/MasteryBackendBody.h"

namespace DuelBodyCues
{
    /**
     * The React() magnitude for this frame's events, or 0 if nothing on this frame warrants one.
     * Ordered by how hard the event lands on the body: being hit outranks everything, since a
     * body that is struck answers the strike before anything it was doing.
     */
    inline float CueFor(const Duel::FFrameEvents& E)
    {
        if (E.bWasPunished) return -1.0f;   // caught helpless - the biggest recoil
        if (E.bTookHit)     return -0.8f;
        if (E.bBlockedHit)  return -0.4f;   // absorbed: braced, still pushed back
        if (E.bDodgedHit)   return -0.5f;   // evasion carries the body away
        if (E.bLandedHit)   return  0.6f;   // follow-through
        if (E.bAttackStarted) return 0.4f;  // committing forward
        if (E.bWhiffed)     return  0.3f;   // overextension into empty space
        return 0.0f;
    }

    /** One fighter's body, driven by her pose and by the fight. */
    struct FDuelBody
    {
        MasteryBackendBody::FDriver Driver;
        int32 CuesSent = 0;

        explicit FDuelBody(uint32 Seed) : Driver(Seed) {}

        /** Call once per duel frame, after Duel::StepFrame, with this fighter's events. */
        const FBodyMotionFrame& Step(const Duel::FFrameEvents& Events,
                                     const FMasteryEmbodimentPose& Pose)
        {
            const float Cue = CueFor(Events);
            if (Cue != 0.0f)
            {
                Driver.React(Cue, Pose);
                ++CuesSent;
            }
            return Driver.Tick(Pose, 1.0f / (float)Duel::FPS);
        }
    };
}
