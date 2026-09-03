"""Unreal Editor helpers. Safe to import when `unreal` is missing (dry-run)."""

from __future__ import annotations

from typing import Any, Optional

try:
    import unreal  # type: ignore
except ImportError:  # pragma: no cover - editor-only
    unreal = None  # type: ignore


def has_unreal() -> bool:
    return unreal is not None


def log(message: str) -> None:
    if unreal is not None:
        unreal.log(message)
    else:
        print(message)


def asset_exists(object_path: str) -> bool:
    if unreal is None:
        return False
    return unreal.EditorAssetLibrary.does_asset_exist(object_path)


def load_asset(object_path: str) -> Optional[Any]:
    if unreal is None or not asset_exists(object_path):
        return None
    return unreal.EditorAssetLibrary.load_asset(object_path)


def save_asset(asset: Any) -> None:
    if unreal is None or asset is None:
        return
    unreal.EditorAssetLibrary.save_loaded_asset(asset)


def create_or_load(name: str, package: str, asset_class: Any, factory: Any, force: bool) -> Any:
    object_path = f"{package}/{name}.{name}"
    if not force and asset_exists(object_path):
        log(f"Skip existing {object_path}")
        return load_asset(object_path)
    if force and asset_exists(object_path):
        unreal.EditorAssetLibrary.delete_asset(object_path)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = tools.create_asset(name, package, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {object_path}")
    log(f"Created {object_path}")
    return asset


def add_scalar_param(material: Any, name: str, default: float, x: int, y: int) -> Any:
    expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, x, y
    )
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", default)
    return expr


def add_vector_param(material: Any, name: str, default: tuple, x: int, y: int) -> Any:
    expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, x, y
    )
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", unreal.LinearColor(*default))
    return expr


def add_texture_param(material: Any, name: str, x: int, y: int) -> Any:
    expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSampleParameter2D, x, y
    )
    expr.set_editor_property("parameter_name", name)
    return expr
