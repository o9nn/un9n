// MasteryEmbodimentPose.h
// The backend-neutral output currency of the mastery binding.
//
// Nothing rig-specific crosses this boundary. No Live2D parameter IDs, no MetaHuman curve names,
// no bone names - only descriptions of what the body is DOING, in terms any rig can realize.
// That is what makes backend-agnosticism structural rather than merely intended: a backend
// cannot leak its vocabulary upward, because there is nowhere in this struct to put it.
//
// Channels are largely FACS-adjacent (action units) plus whole-body scalars. All [0,1] except
// where a signed range is noted.

#pragma once

#include "CoreMinimal.h"

struct FMasteryEmbodimentPose
{
    // ---- Face: brow -------------------------------------------------------------------------

    /** AU1/AU2 - inner+outer brow raise. Surprise, alertness, uncertainty. */
    float BrowRaise = 0.0f;

    /** AU4 - brow lowerer/furrow. Concentration under load, frustration. */
    float BrowFurrow = 0.0f;

    // ---- Face: eyes -------------------------------------------------------------------------

    /** AU5 - upper lid raise. Wide-eyed alarm or startle. */
    float EyeWiden = 0.0f;

    /** AU7 - lid tightener. Focused scrutiny, target lock. */
    float EyeNarrow = 0.0f;

    /** Gaze fixation stability. 1 = locked and still; 0 = darting, searching. */
    float GazeSteadiness = 0.5f;

    /** Blinks per minute. Drops markedly under genuine absorption. */
    float BlinkRate = 15.0f;

    /** Saccade frequency, normalized. High = visually hunting; low = already knows where to look. */
    float SaccadeRate = 0.5f;

    // ---- Face: mouth ------------------------------------------------------------------------

    /** AU12 - lip corner puller. Ease, enjoyment, quiet satisfaction. */
    float MouthCornerUp = 0.0f;

    /** AU15 - lip corner depressor. Displeasure. */
    float MouthCornerDown = 0.0f;

    /** AU23/AU24 - lip tightener/pressor. Effortful restraint. */
    float MouthTension = 0.0f;

    /** Jaw clench. Strain, tilt. */
    float JawTension = 0.0f;

    /**
     * AU17 - chin raiser, as DEFIANCE rather than as strain.
     *
     * Separate from JawTension on purpose. Both can recruit AU17, but they mean opposite things:
     * a clenched jaw is someone struggling, a raised chin is someone asserting. The authored
     * catalog is unambiguous that the expressive sense is the second one - chin raise appears in
     * PUNK_03 "Defiant Confidence", VISION_02/04, and microexpression mEXP_09, all glossed
     * "defiance, confidence". Driving it from strain (as an earlier version did) made a losing
     * character lift her chin as though she were winning.
     */
    float ChinRaise = 0.0f;

    // ---- Body -------------------------------------------------------------------------------

    /** Spinal uprightness. 1 = tall and settled; 0 = slumped or hunched. */
    float PostureUprightness = 0.5f;

    /** Forward lean, signed: -1 = withdrawn, 0 = neutral, +1 = pressing in. */
    float PostureLean = 0.0f;

    /** Shoulder elevation/tension. High = braced; low = loose. */
    float ShoulderTension = 0.0f;

    /**
     * Head tilt, SIGNED [-1,1]. Curiosity, playfulness, appraisal.
     *
     * The sign is not decorative and not arbitrary - it is read off the authored expression set,
     * where the two directions carry different meaning: WONDER_02 "Curious Gaze" tilts to +8
     * degrees, while JOY_02 "Laughing" and PHOTO_ExuberantLaugh tilt to -4 and -5. So
     *   positive = inquisitive, still reading the situation
     *   negative = mirthful, the situation already resolved in her favour
     * An unsigned magnitude can only ever produce one of those, and a character who tilts her
     * head exactly one way reads as broken.
     */
    float HeadTilt = 0.0f;

    // ---- Motion quality ---------------------------------------------------------------------
    //
    // These are what actually read as "skilled" to a viewer. Expertise is legible less in the
    // face than in economy of motion: the expert does not visibly hurry, and does not fidget.

    /** Involuntary micro-movement. High = restless; low = composed stillness. */
    float MicroMovementRate = 0.5f;

    /** Deliberate idle fidgeting (adjusting, shifting, tapping). */
    float IdleFidget = 0.5f;

    /** Crispness of reaction onset. High = decisive, no wind-up; low = hesitant. */
    float ReactionSharpness = 0.5f;

    /** Economy of motion. 1 = no wasted movement (expert); 0 = flailing, overcorrecting. */
    float MotionEconomy = 0.5f;

    /** Breaths per minute. */
    float BreathRate = 14.0f;

    // ---- Expressive meta --------------------------------------------------------------------

    /** Facial asymmetry. Genuine affect is asymmetric; perfect symmetry reads as artificial. */
    float Asymmetry = 0.0f;

    /**
     * How far this pose departs from neutral rest, [0,1]. Consumers may use it as a master
     * blend weight. Held DOWN when signal trust is low - the mechanism by which "unknown is not
     * excellence" reaches the rig.
     */
    float ExpressionIntensity = 0.0f;

    /** Neutral rest pose - what an unmeasured, untrusted, or freshly-initialized signal yields. */
    static FMasteryEmbodimentPose Neutral()
    {
        return FMasteryEmbodimentPose();
    }
};
