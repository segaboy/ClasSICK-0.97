# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Audit boot-scene import allowlist consistency across verification scripts and CMake.

Supported literal layouts (single-line '#' comments ignored):
- scripts/Verify-DrawingAppObjects.ps1: @('apps/boot-scene/scene.c','boot','<csv>')
- scripts/Verify-NativeFramebuffer.ps1: @('apps/boot-scene/scene.c','<csv>')
- scripts/Verify-UEFIImage.ps1: $sources=@('<path>',...) and $allowedImports=@{<int>='<csv>';...}
- CMakeLists.txt: add_test(NAME scene.freestanding ... "-DALLOWED=<csv>" ... $<TARGET_OBJECTS:classick_boot_scene_freestanding> ...)
"""
from __future__ import annotations
import argparse
import pathlib
import re
import sys

SCENE_PATH = "apps/boot-scene/scene.c"
DRAWING_REL = "scripts/Verify-DrawingAppObjects.ps1"
NATIVE_REL = "scripts/Verify-NativeFramebuffer.ps1"
UEFI_REL = "scripts/Verify-UEFIImage.ps1"
CMAKE_REL = "CMakeLists.txt"


def _read_stripped(root: pathlib.Path, rel: str, diags: list[str]) -> str | None:
    path = root / rel
    try:
        if not path.is_file():
            diags.append(f"{rel}: missing file")
            return None
        raw = path.read_bytes()
        text = raw.decode("utf-8-sig")
    except Exception as exc:
        diags.append(f"{rel}: unreadable UTF-8 file ({exc})")
        return None
    lines = [
        ln for ln in text.replace("\r\n", "\n").replace("\r", "\n").split("\n")
        if not ln.lstrip().startswith("#")
    ]
    return "\n".join(lines)


def _parse_tokens(raw: str, label: str, diags: list[str]) -> set[str] | None:
    parts = [p.strip() for p in raw.split(",")]
    if not parts or any(not p for p in parts):
        diags.append(f"{label}: empty import token")
        return None
    seen: set[str] = set()
    dups: list[str] = []
    for p in parts:
        if p in seen and p not in dups:
            dups.append(p)
        seen.add(p)
    if dups:
        diags.append(f"{label}: duplicate import token(s): {', '.join(sorted(dups))}")
        return None
    return seen


def _extract_drawing(text: str, diags: list[str]) -> set[str] | None:
    pat = re.compile(r"@\(\s*'" + re.escape(SCENE_PATH) + r"'\s*,\s*'([^']*)'\s*,\s*'([^']*)'\s*\)")
    matches = pat.findall(text)
    if len(matches) != 1 or matches[0][0] != "boot":
        diags.append(f"{DRAWING_REL}: expected exactly 1 ('{SCENE_PATH}','boot','...') record, found {len(matches)}")
        return None
    tokens = _parse_tokens(matches[0][1], DRAWING_REL, diags)
    if tokens is None:
        return None
    if "cs_drawing_app_probe" in tokens:
        diags.append(f"{DRAWING_REL}: canonical allowlist retains forbidden cs_drawing_app_probe")
    if "cs_native_draw" not in tokens:
        diags.append(f"{DRAWING_REL}: canonical allowlist missing required cs_native_draw")
    return tokens


def _extract_native(text: str, diags: list[str]) -> set[str] | None:
    pat = re.compile(r"@\(\s*'" + re.escape(SCENE_PATH) + r"'\s*,\s*'([^']*)'\s*\)")
    matches = pat.findall(text)
    if len(matches) != 1:
        diags.append(f"{NATIVE_REL}: expected exactly 1 ('{SCENE_PATH}','...') record, found {len(matches)}")
        return None
    return _parse_tokens(matches[0], NATIVE_REL, diags)


def _extract_uefi(text: str, diags: list[str]) -> set[str] | None:
    src_blocks = re.findall(r"\$sources\s*=\s*@\((.*?)\)", text, flags=re.S)
    if len(src_blocks) != 1:
        diags.append(f"{UEFI_REL}: expected unique $sources=@(...) block, found {len(src_blocks)}")
        return None
    src_body = src_blocks[0].strip()
    items = re.findall(r"'([^']*)'", src_body)
    rem = re.sub(r"'[^']*'", "", src_body)
    if not items or re.sub(r"[,\s]", "", rem):
        diags.append(f"{UEFI_REL}: malformed $sources literal list")
        return None
    indices = [i for i, s in enumerate(items) if s == SCENE_PATH]
    if len(indices) != 1:
        diags.append(f"{UEFI_REL}: expected '{SCENE_PATH}' once in $sources, found {len(indices)}")
        return None
    idx = indices[0]
    allow_blocks = re.findall(r"\$allowedImports\s*=\s*@\{(.*?)\}", text, flags=re.S)
    if len(allow_blocks) != 1:
        diags.append(f"{UEFI_REL}: expected unique $allowedImports=@{{...}} block, found {len(allow_blocks)}")
        return None
    allow_body = allow_blocks[0].strip()
    entries = re.findall(r"(\d+)\s*=\s*'([^']*)'", allow_body)
    rem_allow = re.sub(r"\d+\s*=\s*'[^']*'", "", allow_body)
    if not entries or re.sub(r"[;\s]", "", rem_allow):
        diags.append(f"{UEFI_REL}: malformed $allowedImports literal map")
        return None
    selected = [val for key, val in entries if int(key) == idx]
    if len(selected) != 1:
        diags.append(f"{UEFI_REL}: expected unique $allowedImports[{idx}] entry, found {len(selected)}")
        return None
    return _parse_tokens(selected[0], f"{UEFI_REL}[{idx}]", diags)


def _extract_cmake(text: str, diags: list[str]) -> set[str] | None:
    blocks = re.findall(r"add_test\s*\((.*?)\)", text, flags=re.S)
    scene_blocks = [b for b in blocks if re.search(r"\bNAME\s+scene\.freestanding(?=\s|$)", b)]
    if len(scene_blocks) != 1:
        diags.append(f"{CMAKE_REL}: expected unique add_test(NAME scene.freestanding ...), found {len(scene_blocks)}")
        return None
    block = scene_blocks[0]
    if "$<TARGET_OBJECTS:classick_boot_scene_freestanding>" not in block:
        diags.append(f"{CMAKE_REL}: scene.freestanding does not target classick_boot_scene_freestanding")
        return None
    allowed = re.findall(r'"-DALLOWED=([^"]*)"', block)
    if len(allowed) != 1:
        diags.append(f"{CMAKE_REL}: expected unique -DALLOWED in scene.freestanding, found {len(allowed)}")
        return None
    return _parse_tokens(allowed[0], f"{CMAKE_REL}:scene.freestanding", diags)


def audit(root: pathlib.Path) -> list[str]:
    root = pathlib.Path(root)
    diags: list[str] = []
    d_txt = _read_stripped(root, DRAWING_REL, diags)
    n_txt = _read_stripped(root, NATIVE_REL, diags)
    u_txt = _read_stripped(root, UEFI_REL, diags)
    c_txt = _read_stripped(root, CMAKE_REL, diags)
    canon = _extract_drawing(d_txt, diags) if d_txt is not None else None
    native = _extract_native(n_txt, diags) if n_txt is not None else None
    uefi = _extract_uefi(u_txt, diags) if u_txt is not None else None
    cmake = _extract_cmake(c_txt, diags) if c_txt is not None else None
    if canon is not None:
        for label, other in ((NATIVE_REL, native), (UEFI_REL, uefi), (f"{CMAKE_REL}:scene.freestanding", cmake)):
            if other is not None and other != canon:
                missing = sorted(canon - other)
                extra = sorted(other - canon)
                parts = []
                if missing:
                    parts.append(f"missing {missing}")
                if extra:
                    parts.append(f"extra {extra}")
                diags.append(f"{label}: mismatch vs {DRAWING_REL} ({'; '.join(parts)})")
    return diags


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Verify boot-scene import contract consistency.")
    default_root = pathlib.Path(__file__).resolve().parent.parent
    parser.add_argument("--root", type=pathlib.Path, default=default_root, help="Repository root path")
    args = parser.parse_args(argv)
    diags = audit(args.root)
    if diags:
        for d in diags:
            print(f"ERROR: {d}")
        return 1
    print("Scene import contracts PASS.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
