"""Generate 29 placeholder animation montages and DA_DTE_MontageLibrary."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest

MONTAGE_PACKAGE = "/Game/DeepTreeEcho/Animations/Montages"


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    paths = [asset_path(MONTAGE_PACKAGE, item["asset"]) for item in data["montages"]]
    lib = data["montageLibrary"]
    paths.append(asset_path(lib["package"], lib["name"]))
    return paths


def generate_montages(force: bool = False) -> List[str]:
    data = load_manifest()
    if not ue_util.has_unreal():
        return [asset_path(MONTAGE_PACKAGE, item["asset"]) for item in data["montages"]]
    created = []
    factory = ue_util.unreal.AnimMontageFactory()
    for item in data["montages"]:
        montage = ue_util.create_or_load(
            item["asset"], MONTAGE_PACKAGE, ue_util.unreal.AnimMontage, factory, force
        )
        if montage:
            try:
                montage.set_editor_property("blend_in_time", 0.15)
                montage.set_editor_property("blend_out_time", 0.25)
            except Exception:
                pass
            ue_util.save_asset(montage)
        created.append(asset_path(MONTAGE_PACKAGE, item["asset"]))
    return created


def generate_library(force: bool = False) -> List[str]:
    data = load_manifest()
    lib = data["montageLibrary"]
    path = asset_path(lib["package"], lib["name"])
    if not ue_util.has_unreal():
        return [path]
    factory = ue_util.unreal.DataAssetFactory()
    try:
        factory.set_editor_property("data_asset_class", ue_util.unreal.MontageLibraryDataAsset)
    except Exception:
        pass
    asset_class = None
    try:
        asset_class = ue_util.unreal.MontageLibraryDataAsset
    except Exception:
        asset_class = ue_util.unreal.DataAsset
    library = ue_util.create_or_load(lib["name"], lib["package"], asset_class, factory, force)
    if library:
        try:
            library.set_editor_property("library_id", "DTE")
        except Exception:
            pass
        ue_util.save_asset(library)
    ue_util.log("Montage library created; C++ FillDefaultMontageLibrary binds the 29 entries at runtime.")
    return [path]


def generate_all(force: bool = False) -> List[str]:
    results = generate_montages(force)
    results.extend(generate_library(force))
    return results
