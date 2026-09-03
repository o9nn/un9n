# Deep Tree Echo - Content Organization

This directory contains Unreal Engine content assets for the Deep Tree Echo Avatar system.

Binary `.uasset` files are **generated in the Unreal Editor**, not committed. Run:

```
py Content/Python/dte_generate_all.py
```

from the editor Python console (`py Content/Python/dte_generate_all.py`) or:

```
File → Execute Python Script → Content/Python/dte_generate_all.py
```

Dry-run without Unreal:

```
python Content/Python/dte_validate.py
```

## Directory Structure

### Materials/

- **Master/** - Master materials (`M_DTE_Skin`, `M_DTE_Eye`, `M_DTE_Hair`)
- **Functions/** - Reusable material functions (`MF_*`)
- **Instances/** - Material instances (`MI_*`)
- **Textures/** - Placeholder / production textures (`T_*`)

### Animations/

- **Blueprints/** - `ABP_DeepTreeEcho_Avatar` (parent: `UAvatarAnimInstance`)
- **Montages/** - Animation montages (`AM_*`)
- **Sequences/** - Animation sequences (`AS_*`)

### Audio/

- **Voice/** - Voice / TTS integration stubs
- **Gestures/** - Gesture SFX path stubs
- **Emotional/** - Emotional state SFX stubs
- **Music/** - Emotional theme stubs

### Particles/

- **Niagara/** - Niagara systems (`NS_*`)

### PostProcessing/

- Post-process materials (`PP_*`)

### Blueprints/

- Blueprint actors (`BP_*`)

## Asset Naming Conventions

`[Prefix]_[System]_[AssetName]_[Variant]`

| Prefix | Type |
|--------|------|
| M_ | Material |
| MI_ | Material Instance |
| MF_ | Material Function |
| T_ | Texture |
| ABP_ | Animation Blueprint |
| AM_ | Animation Montage |
| AS_ | Animation Sequence |
| NS_ | Niagara System |
| PP_ | Post Process Material |
| BP_ | Blueprint |
| SFX_ | Sound Effect |
| MUS_ | Music |
| DA_ | Data Asset |

Examples:

- `M_DTE_Skin` - Skin master material
- `MI_DTE_Skin_Default` - Default skin instance
- `AM_Idle_ThinkingPose` - Idle thinking montage
- `NS_MemoryNode` - Memory node particle system

## Implementation Order

### Phase 1: Critical Assets

1. Animation Blueprint: `ABP_DeepTreeEcho_Avatar`
2. Material Functions: `MF_SubsurfaceScattering`, `MF_EmotionalBlush`, `MF_GlitchEffect`, `MF_EyeRefraction`
3. Master Materials: `M_DTE_Skin`, `M_DTE_Eye`

### Phase 2: Animation Library

1. Idle gestures (8 montages)
2. Emotional emotes (11 montages)
3. Communicative gestures (10 montages)

### Phase 3: Advanced Assets

1. Hair: `M_DTE_Hair` + `MF_AnisotropicSpecular`
2. Particle systems (4 Niagara systems)
3. Post-processing materials (4)
4. Audio path stubs (voice, SFX, music)

## C++ Integration

Parameter names on generated materials must match `UAvatarMaterialManager`. Asset paths live in `UnrealEcho/AssetManagement/DeepTreeEchoContentPaths.h`. The montage catalog is `UMontageLibraryDataAsset` at `/Game/DeepTreeEcho/Animations/DA_DTE_MontageLibrary`.

## Notes

- Use Git LFS for large binary files if they are later committed
- Test C++ integration as assets are generated
- Refer to `/Documentation/Implementation/` for visual specifications
