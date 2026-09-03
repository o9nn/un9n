"""Generate four DTE post-process materials."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    return [asset_path(item["package"], item["name"]) for item in data["postProcess"]]


def generate_all(force: bool = False) -> List[str]:
    data = load_manifest()
    planned = planned_assets(data)
    if not ue_util.has_unreal():
        return planned
    u = ue_util.unreal
    factory = u.MaterialFactoryNew()
    created = []
    for item in data["postProcess"]:
        mat = ue_util.create_or_load(item["name"], item["package"], u.Material, factory, force)
        if mat:
            try:
                mat.set_editor_property("material_domain", u.MaterialDomain.MD_POST_PROCESS)
            except Exception:
                pass
            y = 0
            for param in item["params"]:
                if "Color" in param:
                    ue_util.add_vector_param(mat, param, (0.5, 0.8, 1.0, 1.0), -400, y)
                else:
                    ue_util.add_scalar_param(mat, param, 0.5, -400, y)
                y += 80
            u.MaterialEditingLibrary.recompile_material(mat)
            ue_util.save_asset(mat)
        created.append(asset_path(item["package"], item["name"]))
    return created
