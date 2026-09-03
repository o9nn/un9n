"""Generate four cognitive Niagara systems."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    return [asset_path(item["package"], item["name"]) for item in data["niagara"]]


def generate_all(force: bool = False) -> List[str]:
    data = load_manifest()
    planned = planned_assets(data)
    if not ue_util.has_unreal():
        return planned
    factory = None
    asset_class = None
    try:
        factory = ue_util.unreal.NiagaraSystemFactoryNew()
        asset_class = ue_util.unreal.NiagaraSystem
    except Exception:
        ue_util.log("Niagara plugin not available; skipping NS_* generation.")
        return planned
    created = []
    for item in data["niagara"]:
        system = ue_util.create_or_load(item["name"], item["package"], asset_class, factory, force)
        if system:
            ue_util.log(f"{item['name']} user params: {', '.join(item['userParams'])}")
            ue_util.save_asset(system)
        created.append(asset_path(item["package"], item["name"]))
    return created
