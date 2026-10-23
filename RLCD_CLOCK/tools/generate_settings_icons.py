#!/usr/bin/env python3
# 将固定来源的五枚设置图标离线转换为共享的20像素单色资源。
from __future__ import annotations

import argparse
import io
from pathlib import Path
import shutil
import subprocess
import tempfile
import xml.etree.ElementTree as ET

from PIL import Image


PROJECT = Path(__file__).resolve().parents[1]
SOURCE = PROJECT / "main/assets/naive_icons"
OUTPUT = PROJECT / "main/assets/settings_naive_icons.h"
REVISION = "a8619441efc40609e72c00e860f4f98afc654a2b"
NAMES = ("wifi", "volume", "monitor", "info", "settings")
SIZE = 20
NAMESPACE = "http://www.w3.org/2000/svg"
ET.register_namespace("", NAMESPACE)


def monochrome_svg(name: str, content: bytes) -> bytes:
    root = ET.fromstring(content)
    allowed = {f"{{{NAMESPACE}}}{tag}" for tag in ("svg", "path", "line", "circle", "rect")}
    for element in root.iter():
        if element.tag not in allowed or any("href" in key for key in element.attrib):
            raise ValueError(f"unsupported SVG content: {name}")
        if "stroke" in element.attrib and element.get("stroke") != "none":
            element.set("stroke", "#000000")
        if "fill" in element.attrib and element.get("fill") != "none":
            element.set("fill", "#ffffff")
        is_circle = element.tag == f"{{{NAMESPACE}}}circle"
        if is_circle and (name == "wifi" or (name == "info" and element.get("r") == "2.2")):
            element.set("fill", "#000000")
    return ET.tostring(root, encoding="utf-8", xml_declaration=True)


def icon_bits(name: str, source: Path) -> bytes:
    renderer = shutil.which("rsvg-convert")
    if renderer is None:
        raise RuntimeError("rsvg-convert is required only to regenerate or check icons")
    svg = monochrome_svg(name, (source / f"{name}.svg").read_bytes())
    raster = subprocess.run(
        [renderer, "-w", str(SIZE * 8), "-h", str(SIZE * 8)],
        input=svg, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True, timeout=15,
    ).stdout
    with Image.open(io.BytesIO(raster)) as image:
        rgba = image.convert("RGBA")
    background = Image.new("RGBA", rgba.size, "white")
    background.alpha_composite(rgba)
    gray = background.convert("L").resize((SIZE, SIZE), Image.Resampling.LANCZOS)
    packed = bytearray()
    for y in range(SIZE):
        for byte_x in range((SIZE + 7) // 8):
            value = 0
            for bit in range(8):
                x = byte_x * 8 + bit
                if x < SIZE and gray.getpixel((x, y)) < 128:
                    value |= 0x80 >> bit
            packed.append(value)
    return bytes(packed)


def generate_header(source: Path) -> str:
    icons = [(name, icon_bits(name, source)) for name in NAMES]
    lines = [
        "// 设置页共享20像素单色图标，由tools/generate_settings_icons.py生成。",
        "// Copyright (c) 2026 Naive Icons. MIT; see naive_icons/LICENSE.",
        f"// Source revision: {REVISION}; monochrome fills adapted locally.",
        "#pragma once",
        "#include <stdint.h>",
        "namespace settings_naive_icons {",
        f"inline constexpr int kSize = {SIZE};",
        "inline constexpr int kBytesPerRow = (kSize + 7) / 8;",
    ]
    for name, data in icons:
        lines.append(f"inline constexpr uint8_t k_{name}[] = {{")
        for offset in range(0, len(data), 12):
            lines.append("    " + ", ".join(f"0x{value:02x}" for value in data[offset:offset + 12]) + ",")
        lines.extend(["};", f"static_assert(sizeof(k_{name}) == kBytesPerRow * kSize);"])
    lines.extend(["} // namespace settings_naive_icons", ""])
    return "\n".join(lines)


def write_header_atomic(output: Path, content: str) -> bool:
    if output.is_file() and output.read_text(encoding="utf-8") == content:
        return False
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=output.parent,
                                         prefix=f"{output.name}.", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(content)
        temporary.replace(output)
        return True
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate offline settings icon masks")
    parser.add_argument("--source-dir", type=Path, default=SOURCE)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    content = generate_header(args.source_dir)
    if args.check:
        if not args.output.is_file() or args.output.read_text(encoding="utf-8") != content:
            parser.exit(1, "Settings icon header is missing or stale\n")
        print("Settings icon header matches five SVG sources (300 bytes)")
        return 0
    changed = write_header_atomic(args.output, content)
    print("Generated settings icons" if changed else "Settings icons unchanged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
