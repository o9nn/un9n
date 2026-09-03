"""Load the DTE asset manifest (source of truth for generators and validators)."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List

_MANIFEST_CANDIDATES = [
    Path(__file__).resolve().parents[2] / "DeepTreeEcho" / "dte_asset_manifest.json",
    Path(__file__).resolve().parents[3] / "Content" / "DeepTreeEcho" / "dte_asset_manifest.json",
]


def manifest_path() -> Path:
    for candidate in _MANIFEST_CANDIDATES:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError("dte_asset_manifest.json not found under Content/DeepTreeEcho")


def load_manifest() -> Dict[str, Any]:
    with manifest_path().open("r", encoding="utf-8") as handle:
        return json.load(handle)


def asset_path(package: str, name: str) -> str:
    return f"{package}/{name}.{name}"


def expected_asset_list(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    paths: List[str] = []
    abp = data["animationBlueprint"]
    paths.append(asset_path(abp["package"], abp["name"]))
    for item in data["materialFunctions"]:
        paths.append(asset_path(item["package"], item["name"]))
    for item in data["masterMaterials"]:
        paths.append(asset_path(item["package"], item["name"]))
    for item in data["materialInstances"]:
        paths.append(asset_path(item["package"], item["name"]))
    montage_pkg = "/Game/DeepTreeEcho/Animations/Montages"
    for item in data["montages"]:
        paths.append(asset_path(montage_pkg, item["asset"]))
    lib = data["montageLibrary"]
    paths.append(asset_path(lib["package"], lib["name"]))
    for item in data["niagara"]:
        paths.append(asset_path(item["package"], item["name"]))
    for item in data["postProcess"]:
        paths.append(asset_path(item["package"], item["name"]))
    audio = data["audio"]
    for item in audio["voice"]:
        paths.append(asset_path(item["package"], item["name"]))
    for item in audio["gestures"]:
        paths.append(asset_path("/Game/DeepTreeEcho/Audio/Gestures", item["name"]))
    for item in audio["emotional"]:
        paths.append(asset_path("/Game/DeepTreeEcho/Audio/Emotional", item["name"]))
    for item in audio["music"]:
        paths.append(asset_path("/Game/DeepTreeEcho/Audio/Music", item["name"]))
    for item in audio["ambient"]:
        paths.append(asset_path(item["package"], item["name"]))
    return paths
