"""Register audio path stubs (no recorded media)."""

from __future__ import annotations

from typing import Any, Dict, List

from . import ue_util
from .manifest import asset_path, load_manifest


def planned_assets(data: Dict[str, Any] | None = None) -> List[str]:
    data = data or load_manifest()
    audio = data["audio"]
    paths = [asset_path(item["package"], item["name"]) for item in audio["voice"]]
    paths.extend(asset_path("/Game/DeepTreeEcho/Audio/Gestures", item["name"]) for item in audio["gestures"])
    paths.extend(asset_path("/Game/DeepTreeEcho/Audio/Emotional", item["name"]) for item in audio["emotional"])
    paths.extend(asset_path("/Game/DeepTreeEcho/Audio/Music", item["name"]) for item in audio["music"])
    paths.extend(asset_path(item["package"], item["name"]) for item in audio["ambient"])
    return paths


def generate_all(force: bool = False) -> List[str]:
    """Audio assets are path stubs. C++ AudioManagerComponent holds SoftObjectPaths.

    Unreal cannot create a playable USoundWave without samples. We only log the
    expected object paths so designers can drop WAV/OGG files later.
    """
    planned = planned_assets()
    if ue_util.has_unreal():
        ue_util.log(f"Audio stubs ({len(planned)} paths) — no media generated.")
        for path in planned:
            ue_util.log(f"  audio stub: {path}")
    return planned
