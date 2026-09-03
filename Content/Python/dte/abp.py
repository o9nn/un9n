"""Generate ABP_DeepTreeEcho_Avatar parented to UAvatarAnimInstance."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    abp = data["animationBlueprint"]
    return [asset_path(abp["package"], abp["name"])]


def _find_avatar_anim_instance():
    u = ue_util.unreal
    try:
        return u.AvatarAnimInstance.static_class()
    except Exception:
        return u.AnimInstance.static_class()


def generate_all(force: bool = False) -> List[str]:
    data = load_manifest()
    abp = data["animationBlueprint"]
    planned = planned_assets(data)
    if not ue_util.has_unreal():
        return planned
    factory = ue_util.unreal.AnimBlueprintFactory()
    parent = _find_avatar_anim_instance()
    factory.set_editor_property("parent_class", parent)
    try:
        factory.set_editor_property("target_skeleton", None)
    except Exception:
        pass
    asset = ue_util.create_or_load(
        abp["name"], abp["package"], ue_util.unreal.AnimBlueprint, factory, force
    )
    if asset:
        ue_util.log(
            "ABP_DeepTreeEcho_Avatar created. Locomotion states "
            f"{abp['locomotionStates']} and gesture slot '{abp['gestureSlot']}' "
            "are driven by UAvatarAnimInstance; wire placeholder sequences in Persona."
        )
        ue_util.save_asset(asset)
    return planned
