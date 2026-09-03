"""Generate DTE material functions, masters, and instances."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    paths = []
    for group in ("materialFunctions", "masterMaterials", "materialInstances"):
        for item in data[group]:
            paths.append(asset_path(item["package"], item["name"]))
    return paths


def _create_function(name: str, package: str, force: bool) -> Any:
    factory = ue_util.unreal.MaterialFunctionFactoryNew()
    return ue_util.create_or_load(name, package, ue_util.unreal.MaterialFunction, factory, force)


def _create_material(name: str, package: str, force: bool) -> Any:
    factory = ue_util.unreal.MaterialFactoryNew()
    return ue_util.create_or_load(name, package, ue_util.unreal.Material, factory, force)


def generate_material_functions(force: bool = False) -> List[str]:
    data = load_manifest()
    if not ue_util.has_unreal():
        return [asset_path(item["package"], item["name"]) for item in data["materialFunctions"]]
    created = []
    for item in load_manifest()["materialFunctions"]:
        func = _create_function(item["name"], item["package"], force)
        if func:
            created.append(asset_path(item["package"], item["name"]))
            ue_util.save_asset(func)
    return created


def _wire_skin(material: Any, params: Dict[str, List[str]]) -> None:
    u = ue_util.unreal
    material.set_editor_property("shading_model", u.MaterialShadingModel.MSM_SUBSURFACE)
    y = 0
    smoothness = ue_util.add_scalar_param(material, "Smoothness", 0.6, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "Subsurface", 0.8, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "Translucency", 0.3, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "BlushIntensity", 0.0, -400, y)
    y += 80
    ue_util.add_vector_param(material, "BlushColor", (1.0, 0.5, 0.5, 1.0), -400, y)
    y += 80
    ue_util.add_scalar_param(material, "AuraIntensity", 0.0, -400, y)
    y += 80
    ue_util.add_vector_param(material, "AuraColor", (0.5, 0.8, 1.0, 1.0), -400, y)
    y += 80
    ue_util.add_scalar_param(material, "GlitchIntensity", 0.0, -400, y)
    y += 80
    base = ue_util.add_vector_param(material, "BaseColor", (1.0, 0.9, 0.85, 1.0), -400, y)
    y += 80
    ue_util.add_vector_param(material, "SubsurfaceColor", (1.0, 0.8, 0.7, 1.0), -400, y)
    y += 80
    ue_util.add_scalar_param(material, "GlobalGlitchIntensity", 0.0, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "CognitiveLoad", 0.0, -400, y)
    u.MaterialEditingLibrary.connect_material_property(base, "", u.MaterialProperty.MP_BASE_COLOR)
    # Smoothness -> inverted-ish roughness via default connection
    u.MaterialEditingLibrary.connect_material_property(smoothness, "", u.MaterialProperty.MP_ROUGHNESS)
    u.MaterialEditingLibrary.recompile_material(material)


def _wire_eye(material: Any) -> None:
    u = ue_util.unreal
    material.set_editor_property("shading_model", u.MaterialShadingModel.MSM_DEFAULT_LIT)
    y = 0
    ue_util.add_scalar_param(material, "PupilDilation", 0.5, -400, y)
    y += 80
    sparkle = ue_util.add_scalar_param(material, "SparkleIntensity", 0.8, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "Moisture", 0.6, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "SparklePositionX", 0.3, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "SparklePositionY", 0.3, -400, y)
    y += 80
    base = ue_util.add_vector_param(material, "IrisColor", (0.2, 0.4, 0.7, 1.0), -400, y)
    y += 80
    ue_util.add_scalar_param(material, "GlobalGlitchIntensity", 0.0, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "CognitiveLoad", 0.0, -400, y)
    u.MaterialEditingLibrary.connect_material_property(base, "", u.MaterialProperty.MP_BASE_COLOR)
    u.MaterialEditingLibrary.connect_material_property(sparkle, "", u.MaterialProperty.MP_EMISSIVE_COLOR)
    u.MaterialEditingLibrary.recompile_material(material)


def _wire_hair(material: Any) -> None:
    u = ue_util.unreal
    try:
        material.set_editor_property("shading_model", u.MaterialShadingModel.MSM_HAIR)
    except Exception:
        material.set_editor_property("shading_model", u.MaterialShadingModel.MSM_DEFAULT_LIT)
    y = 0
    ue_util.add_scalar_param(material, "Anisotropy", 0.7, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "ShimmerIntensity", 0.5, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "ShimmerSpeed", 2.0, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "ChaosColorShift", 0.0, -400, y)
    y += 80
    glow = ue_util.add_scalar_param(material, "EchoGlow", 0.0, -400, y)
    y += 80
    base = ue_util.add_vector_param(material, "HairColor", (0.15, 0.08, 0.05, 1.0), -400, y)
    y += 80
    ue_util.add_scalar_param(material, "GlobalGlitchIntensity", 0.0, -400, y)
    y += 80
    ue_util.add_scalar_param(material, "CognitiveLoad", 0.0, -400, y)
    u.MaterialEditingLibrary.connect_material_property(base, "", u.MaterialProperty.MP_BASE_COLOR)
    u.MaterialEditingLibrary.connect_material_property(glow, "", u.MaterialProperty.MP_EMISSIVE_COLOR)
    u.MaterialEditingLibrary.recompile_material(material)


def generate_masters(force: bool = False) -> List[str]:
    data = load_manifest()
    if not ue_util.has_unreal():
        return [asset_path(i["package"], i["name"]) for i in data["masterMaterials"]]
    created = []
    for item in data["masterMaterials"]:
        mat = _create_material(item["name"], item["package"], force)
        if mat is None:
            continue
        if item["name"] == "M_DTE_Skin":
            _wire_skin(mat, data["materialParameters"])
        elif item["name"] == "M_DTE_Eye":
            _wire_eye(mat)
        elif item["name"] == "M_DTE_Hair":
            _wire_hair(mat)
        ue_util.save_asset(mat)
        created.append(asset_path(item["package"], item["name"]))
    return created


def generate_instances(force: bool = False) -> List[str]:
    data = load_manifest()
    if not ue_util.has_unreal():
        return [asset_path(i["package"], i["name"]) for i in data["materialInstances"]]
    created = []
    factory = ue_util.unreal.MaterialInstanceConstantFactoryNew()
    for item in data["materialInstances"]:
        inst = ue_util.create_or_load(
            item["name"], item["package"], ue_util.unreal.MaterialInstanceConstant, factory, force
        )
        parent_path = f"/Game/DeepTreeEcho/Materials/Master/{item['parent']}.{item['parent']}"
        parent = ue_util.load_asset(parent_path)
        if inst and parent:
            inst.set_editor_property("parent", parent)
            ue_util.save_asset(inst)
        created.append(asset_path(item["package"], item["name"]))
    return created


def generate_all(force: bool = False) -> List[str]:
    results = []
    results.extend(generate_material_functions(force))
    results.extend(generate_masters(force))
    results.extend(generate_instances(force))
    return results
