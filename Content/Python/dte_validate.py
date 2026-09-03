"""Validate DTE asset contracts without Unreal Engine."""

from __future__ import annotations

import sys
from pathlib import Path

_HERE = Path(__file__).resolve().parent
if str(_HERE) not in sys.path:
    sys.path.insert(0, str(_HERE))

from dte.manifest import expected_asset_list, load_manifest  # noqa: E402


REQUIRED_SKIN = {
    "Smoothness",
    "Subsurface",
    "Translucency",
    "BlushIntensity",
    "BlushColor",
    "AuraIntensity",
    "AuraColor",
    "GlitchIntensity",
}
REQUIRED_HAIR = {"Anisotropy", "ShimmerIntensity", "ShimmerSpeed", "ChaosColorShift", "EchoGlow"}
REQUIRED_EYES = {"PupilDilation", "SparkleIntensity", "Moisture", "SparklePositionX", "SparklePositionY"}
REQUIRED_NIAGARA = {"NS_MemoryNode", "NS_EchoResonance", "NS_CognitiveLoad", "NS_EmotionalAura"}
REQUIRED_PP = {
    "PP_EmotionalAura",
    "PP_CognitiveLoadHeatMap",
    "PP_GlitchEffect",
    "PP_EchoResonanceDistortion",
}


def validate() -> list[str]:
    errors: list[str] = []
    data = load_manifest()
    montages = data["montages"]
    if len(montages) != 29:
        errors.append(f"expected 29 montages, got {len(montages)}")
    idle = [m for m in montages if m["category"] == "Idle"]
    emote = [m for m in montages if m["category"] == "Emote"]
    gesture = [m for m in montages if m["category"] == "Gesture"]
    if len(idle) != 8:
        errors.append(f"expected 8 idle montages, got {len(idle)}")
    if len(emote) != 11:
        errors.append(f"expected 11 emote montages, got {len(emote)}")
    if len(gesture) != 10:
        errors.append(f"expected 10 gesture montages, got {len(gesture)}")
    ids = [m["id"] for m in montages]
    if len(ids) != len(set(ids)):
        errors.append("duplicate montage ids")

    params = data["materialParameters"]
    missing_skin = REQUIRED_SKIN - set(params["Skin"])
    missing_hair = REQUIRED_HAIR - set(params["Hair"])
    missing_eyes = REQUIRED_EYES - set(params["Eyes"])
    if missing_skin:
        errors.append(f"missing skin params: {sorted(missing_skin)}")
    if missing_hair:
        errors.append(f"missing hair params: {sorted(missing_hair)}")
    if missing_eyes:
        errors.append(f"missing eye params: {sorted(missing_eyes)}")

    niagara_names = {item["name"] for item in data["niagara"]}
    if niagara_names != REQUIRED_NIAGARA:
        errors.append(f"niagara mismatch: {niagara_names}")
    pp_names = {item["name"] for item in data["postProcess"]}
    if pp_names != REQUIRED_PP:
        errors.append(f"post-process mismatch: {pp_names}")

    abp = data["animationBlueprint"]
    if abp["parentClass"] != "AvatarAnimInstance":
        errors.append("ABP parent must be AvatarAnimInstance")
    if abp["name"] != "ABP_DeepTreeEcho_Avatar":
        errors.append("unexpected ABP name")

    if len(data["audio"]["gestures"]) != 10:
        errors.append("expected 10 gesture SFX stubs")
    if len(data["audio"]["emotional"]) != 7:
        errors.append("expected 7 emotional SFX stubs")
    if len(data["audio"]["music"]) != 6:
        errors.append("expected 6 music stubs")

    assets = expected_asset_list(data)
    if len(assets) != len(set(assets)):
        errors.append("duplicate planned asset paths")

    manager_cpp = Path(__file__).resolve().parents[2] / "UnrealEcho" / "Avatar" / "AvatarMaterialManager.cpp"
    if manager_cpp.is_file():
        manager_text = manager_cpp.read_text(encoding="utf-8")
        for param in ("Smoothness", "Subsurface", "BlushIntensity", "PupilDilation", "Anisotropy", "GlitchIntensity"):
            if f'TEXT("{param}")' not in manager_text:
                errors.append(f"AvatarMaterialManager missing {param}")
    else:
        errors.append("AvatarMaterialManager.cpp not found")

    catalog_cpp = Path(__file__).resolve().parents[2] / "UnrealEcho" / "AssetManagement" / "DeepTreeEchoMontageCatalog.cpp"
    if catalog_cpp.is_file():
        text = catalog_cpp.read_text(encoding="utf-8")
        for montage in montages:
            token = f'TEXT("{montage["id"]}")'
            if token not in text:
                errors.append(f"C++ catalog missing {montage['id']}")
    else:
        errors.append("DeepTreeEchoMontageCatalog.cpp not found")
    return errors


def main() -> int:
    errors = validate()
    data = load_manifest()
    assets = expected_asset_list(data)
    print(f"manifest: {len(data['montages'])} montages, {len(assets)} planned assets")
    if errors:
        print("FAILED")
        for err in errors:
            print(f"  - {err}")
        return 1
    print("OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
