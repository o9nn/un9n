// CharacterTemplate.h
// A character definition that keeps geometry, identity, appearance, expression and motion as
// SEPARATE layers with separate provenance.
//
// WHY SEPARATION IS THE WHOLE POINT
//
// The reference material for a character typically arrives from incompatible sources: a rigged
// 3D figure in bind pose, action-pose renders of that figure, and a pile of AI-generated concept
// portraits. It is tempting to treat all of it as "reference for the character" and fuse it.
// That produces a mesh that resembles none of the inputs, because the sources are authoritative
// for DIFFERENT THINGS and silently disagree about the rest.
//
// The rule this file enforces:
//
//     A REFERENCE ASSET IS AUTHORITATIVE FOR SPECIFIC LAYERS AND ADVISORY FOR THE REST.
//
// The distinction that matters is MEASUREMENT versus TARGET, not "usable" versus "unusable".
//
// AI-generated portraits cannot be PHOTOGRAMMETRIED. Structure-from-Motion recovers a scene from
// real cameras with true parallax; these images have no camera at all - no consistent focal
// length, no rigid scene, baked-in lighting, and hair and decals re-invented per image. SfM will
// not converge, or will bundle-adjust into a warped surface.
//
// They CAN drive geometry as a target rather than a measurement, and for a fictional character
// that is legitimate - there is no ground-truth face for a reconstruction to be wrong about.
// Three viable routes: sculpting reference for wrap-deforming the base head; a throwaway proxy
// from image-to-3D (TRELLIS2, Hunyuan3D, Tripo - these generate a 3D representation first and
// render from it, so their consistency is structural) used as a blockout or wrap target; or
// parametric fitters (FLAME/DECA family) for proportion ratios, though those are trained on real
// human scans and regress a stylised face hard toward an average one.
//
// What portraits cannot do is ORIGINATE the shippable, animation-ready surface. Auto-remeshed
// output has no edge loops around eyes and lips, no symmetry guarantee, no stable vertex order,
// and no UV layout shareable across a character family - nothing a blendshape rig can be authored
// against. Hence Advisory, not Authoritative: they may inform the shape, never define it.
//
// Layers, and what owns each:
//
//   GEOMETRY   - bind-pose mesh, topology, skeleton, proportions.  Owner: the 3D figure.
//   IDENTITY   - face shape, the thing that makes her recognisably her.  Owner: the 3D head,
//                optionally sculpted TOWARD portrait reference but never derived FROM it.
//   APPEARANCE - textures, hair colour, decals, accessories, materials.  Owner: portraits.
//   EXPRESSION - the blendshape/AU set the rig can hit.  Owner: the rig standard (ARKit/FACS),
//                with portraits serving as targets to match.
//   MOTION     - pose library, animation.  Owner: the action-pose renders and mocap.
//
// EXPRESSION is the layer that connects to the rest of this module: FMasteryEmbodimentPose emits
// FACS-adjacent action units, and this is the contract describing what the target rig can
// actually receive.

#pragma once

#include "CoreMinimal.h"

/** Which layer a reference asset is allowed to determine. */
enum class ECharacterLayer : uint8
{
    Geometry   = 0,
    Identity   = 1,
    Appearance = 2,
    Expression = 3,
    Motion     = 4,
    COUNT      = 5
};

/** How much weight a reference asset carries for a layer. */
enum class EReferenceAuthority : uint8
{
    /** Do not use this asset for this layer under any circumstances. */
    Forbidden = 0,
    /** Useful as a target to match by eye; never as a source to extract from. */
    Advisory  = 1,
    /** The source of truth for this layer. */
    Authoritative = 2
};

/** Where a reference asset came from - determines what it can be trusted for. */
enum class EReferenceSource : uint8
{
    /** Render of an actual 3D model. Geometrically self-consistent. */
    Render3D,
    /** Photograph of a real subject. Geometrically consistent, photogrammetry-viable in sets. */
    Photograph,
    /** Diffusion/AI generated. No recoverable camera geometry - never photogrammetry input,
     *  but usable as a shape TARGET (see the header comment). */
    Generated,
    /** Hand-drawn concept art. Same status as Generated for geometry purposes. */
    ConceptArt,
    /** Scan or point cloud. The strongest geometry source. */
    Scan
};

/**
 * One piece of reference material and what it may determine.
 */
struct FCharacterReference
{
    /** Human-readable label, e.g. "T-pose front", "portrait, three-quarter, laughing". */
    FString Label;

    /** Asset path or URI. */
    FString Path;

    EReferenceSource Source = EReferenceSource::Generated;

    /** Per-layer authority. Index with ECharacterLayer. */
    EReferenceAuthority Authority[static_cast<int32>(ECharacterLayer::COUNT)] = {
        EReferenceAuthority::Forbidden, EReferenceAuthority::Forbidden,
        EReferenceAuthority::Forbidden, EReferenceAuthority::Forbidden,
        EReferenceAuthority::Forbidden
    };

    /** Free-text note on what specifically this asset settles. */
    FString Notes;

    EReferenceAuthority AuthorityFor(ECharacterLayer Layer) const
    {
        return Authority[static_cast<int32>(Layer)];
    }

    /**
     * Default authority implied by the source type.
     *
     * This is the guard rail. A Generated asset is Advisory - never Authoritative - for Geometry
     * and Identity: it may inform the shape as a sculpting or wrap target, but must not be the
     * source of the shipped, riggable surface. Overriding that in data is possible but has to be
     * deliberate and visible.
     */
    static FCharacterReference FromSource(const FString& InLabel, const FString& InPath,
                                          EReferenceSource InSource)
    {
        FCharacterReference R;
        R.Label = InLabel;
        R.Path = InPath;
        R.Source = InSource;

        auto Set = [&R](ECharacterLayer L, EReferenceAuthority A)
        {
            R.Authority[static_cast<int32>(L)] = A;
        };

        switch (InSource)
        {
        case EReferenceSource::Scan:
            Set(ECharacterLayer::Geometry,   EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Identity,   EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Appearance, EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Expression, EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Motion,     EReferenceAuthority::Forbidden);
            break;

        case EReferenceSource::Render3D:
            // A render of the actual model is the ground truth for shape and proportion.
            Set(ECharacterLayer::Geometry,   EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Identity,   EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Appearance, EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Expression, EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Motion,     EReferenceAuthority::Advisory);
            break;

        case EReferenceSource::Photograph:
            Set(ECharacterLayer::Geometry,   EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Identity,   EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Appearance, EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Expression, EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Motion,     EReferenceAuthority::Advisory);
            break;

        case EReferenceSource::Generated:
        case EReferenceSource::ConceptArt:
            // THE IMPORTANT CASE. Advisory rather than Forbidden for geometry: these may be used
            // as a sculpting or wrap-deform TARGET, and image-to-3D can turn them into a proxy
            // blockout. They must never be the source of the shipped, riggable surface, and they
            // must never be fed to photogrammetry - there is no camera geometry in them to solve.
            Set(ECharacterLayer::Geometry,   EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Identity,   EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Appearance, EReferenceAuthority::Authoritative);
            Set(ECharacterLayer::Expression, EReferenceAuthority::Advisory);
            Set(ECharacterLayer::Motion,     EReferenceAuthority::Forbidden);
            break;
        }
        return R;
    }
};

/** Bind-pose convention of the source figure. Getting this wrong breaks every retarget. */
enum class EBindPose : uint8
{
    TPose,   // arms straight out horizontally
    APose,   // arms ~45 degrees down - the usual DAZ/MetaHuman convention
    Unknown
};

/**
 * GEOMETRY layer: the measurable, riggable facts.
 */
struct FCharacterGeometry
{
    FString BindMeshPath;
    EBindPose BindPose = EBindPose::Unknown;

    /** Skeleton the mesh is weighted to, e.g. "Genesis9", "UE5_Manny", "MetaHuman". */
    FString SkeletonId;

    /**
     * True when the body is NOT already MetaHuman topology (e.g. a DAZ/Genesis figure).
     *
     * Mesh to MetaHuman treats the HEAD as a fit - it solves an identity from Epic's scan
     * database, so arbitrary head topology is fine. The BODY is a conform, and expects
     * MetaHuman topology and joint hierarchy. For a non-MetaHuman body you must enable
     * "Match Vertices by UVs" or the conform silently produces a mangled result.
     */
    bool bRequiresUVMatchedBodyConform = false;

    /** Height in cm, for proportion matching across rigs. */
    float HeightCm = 0.0f;

    /** Rough head-heights, the classic proportion metric. Stylised figures run 7.5-8+. */
    float HeadHeights = 0.0f;

    /** True once the mesh has been confirmed to load and bind in the target engine. */
    bool bValidatedInEngine = false;
};

/**
 * APPEARANCE layer: everything the portraits legitimately settle.
 */
struct FCharacterAppearance
{
    /** Hair base colour as linear RGB. The portraits and the render often DISAGREE here - the
     *  template forces one answer rather than leaving it ambiguous per-asset. */
    float HairColorRGB[3] = {0.85f, 0.87f, 0.90f};

    FString SkinTexturePath;
    FString HairAssetPath;

    /** Face decals as separate layers rather than baked into the skin texture, so they can be
     *  toggled, animated, or swapped per outfit. */
    TArray<FString> FaceDecalPaths;

    /** Accessory meshes: headphones, choker, and so on. */
    TArray<FString> AccessoryMeshPaths;

    /** Outfit pieces. */
    TArray<FString> ClothingMeshPaths;
};

/**
 * EXPRESSION layer: what the target rig can actually receive.
 *
 * This is the seam with the rest of the module. FMasteryEmbodimentPose emits FACS-adjacent
 * action units; a backend translates those into whatever this rig speaks. The template records
 * WHICH standard the rig speaks so the correct backend is selected rather than assumed.
 */
enum class EFacialRigStandard : uint8
{
    /** Apple ARKit's 52 blendshapes - the de-facto interchange set. */
    ARKit52,
    /** MetaHuman's native facial control rig. */
    MetaHumanControlRig,
    /** Live2D Cubism 2D deformation parameters. Different semantics entirely - not 3D shapes. */
    Live2DCubism,
    /** A bespoke blendshape set; requires a hand-written mapping. */
    Custom,
    None
};

struct FCharacterExpression
{
    EFacialRigStandard Standard = EFacialRigStandard::None;

    /** Number of blendshapes/parameters the rig actually exposes. ARKit is 52; a partial rig
     *  exposing fewer is common and means some action units simply cannot be hit. */
    int32 AvailableShapeCount = 0;

    /** Shapes verified present on the rig. Anything an AU needs that is absent here degrades
     *  rather than erroring - see the backend's ApplyOptional pattern. */
    TArray<FString> VerifiedShapeNames;

    /** Portrait references used as expression TARGETS - the look to match by hand, since the
     *  geometry cannot be extracted from them. */
    TArray<FString> ExpressionTargetRefs;
};

/**
 * MOTION layer.
 */
struct FCharacterMotion
{
    /** Named poses extracted from action-pose reference (guard, jab, idle...). */
    TArray<FString> PoseLibraryPaths;

    /** Animation sequences. */
    TArray<FString> AnimationPaths;

    /** Skeleton these animations target - must match FCharacterGeometry::SkeletonId or be
     *  retargeted. Mismatch here is the single most common cause of broken imports. */
    FString AnimationSkeletonId;
};

/**
 * The whole character definition.
 */
struct FCharacterTemplate
{
    FString CharacterName;

    FCharacterGeometry Geometry;
    FCharacterAppearance Appearance;
    FCharacterExpression Expression;
    FCharacterMotion Motion;

    /** Every piece of reference material, with its provenance and per-layer authority. */
    TArray<FCharacterReference> References;

    /**
     * Validation: does this template have an authoritative source for each layer that needs one?
     *
     * Returns the number of problems found and appends a description of each. Geometry and
     * Identity MUST have an authoritative non-generated source; a template whose only face
     * reference is AI-generated is not buildable and should say so loudly rather than producing
     * a plausible-looking mesh of nobody.
     */
    int32 Validate(TArray<FString>& OutProblems) const
    {
        int32 Problems = 0;

        auto HasAuthoritative = [this](ECharacterLayer Layer) -> bool
        {
            for (const FCharacterReference& R : References)
            {
                if (R.AuthorityFor(Layer) == EReferenceAuthority::Authoritative) return true;
            }
            return false;
        };

        if (!HasAuthoritative(ECharacterLayer::Geometry))
        {
            OutProblems.Add(TEXT("No authoritative GEOMETRY source. AI portraits cannot supply "
                                 "shape - a 3D model, scan or photo set is required."));
            ++Problems;
        }
        if (!HasAuthoritative(ECharacterLayer::Identity))
        {
            OutProblems.Add(TEXT("No authoritative IDENTITY source. Face shape must come from a "
                                 "geometrically consistent reference."));
            ++Problems;
        }
        if (Geometry.BindPose == EBindPose::Unknown)
        {
            OutProblems.Add(TEXT("Bind pose is Unknown - retargeting will be wrong."));
            ++Problems;
        }
        if (Geometry.SkeletonId.IsEmpty())
        {
            OutProblems.Add(TEXT("No SkeletonId - cannot verify animation compatibility."));
            ++Problems;
        }
        if (!Motion.AnimationSkeletonId.IsEmpty() &&
            Motion.AnimationSkeletonId != Geometry.SkeletonId)
        {
            OutProblems.Add(TEXT("Animation skeleton differs from mesh skeleton - retarget required."));
            ++Problems;
        }
        if (Expression.Standard == EFacialRigStandard::None)
        {
            OutProblems.Add(TEXT("No facial rig standard declared - the embodiment binding has "
                                 "no backend to target."));
            ++Problems;
        }
        return Problems;
    }
};
