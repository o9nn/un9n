"""Generate all Deep Tree Echo Content assets inside Unreal Editor.

Usage (editor Python console):
    py "Content/Python/dte_generate_all.py"
    py "Content/Python/dte_generate_all.py" --force

Outside Unreal this prints the planned asset list (dry-run).
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

_HERE = Path(__file__).resolve().parent
if str(_HERE) not in sys.path:
    sys.path.insert(0, str(_HERE))

from dte import abp, audio, materials, montages, niagara, postprocess  # noqa: E402
from dte.ue_util import has_unreal, log  # noqa: E402


def generate_all(force: bool = False) -> dict:
    results = {
        "materials": materials.generate_all(force),
        "abp": abp.generate_all(force),
        "montages": montages.generate_all(force),
        "niagara": niagara.generate_all(force),
        "postProcess": postprocess.generate_all(force),
        "audio": audio.generate_all(force),
    }
    total = sum(len(v) for v in results.values())
    mode = "Unreal" if has_unreal() else "dry-run"
    log(f"DTE generate_all complete ({mode}): {total} assets")
    return results


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate Deep Tree Echo Unreal content")
    parser.add_argument("--force", action="store_true", help="Recreate existing assets")
    args = parser.parse_args(argv)
    results = generate_all(force=args.force)
    if not has_unreal():
        for group, paths in results.items():
            print(f"[{group}] {len(paths)}")
            for path in paths:
                print(f"  {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
