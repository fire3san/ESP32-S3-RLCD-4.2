#!/usr/bin/env python3
# 验证设置图标离线生成的一致性、原子写入和错误输入保护。
import importlib.util
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

PROJECT = Path(__file__).resolve().parents[1]
GENERATOR_PATH = PROJECT / "tools/generate_settings_icons.py"
SPEC = importlib.util.spec_from_file_location("settings_icon_generator", GENERATOR_PATH)
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class GeneratorTests(unittest.TestCase):
    def test_sources_match_checked_in_header(self):
        self.assertEqual(GENERATOR.generate_header(GENERATOR.SOURCE),
                         GENERATOR.OUTPUT.read_text(encoding="utf-8"))

    def test_payload_and_padding(self):
        total = 0
        for name in GENERATOR.NAMES:
            bits = GENERATOR.icon_bits(name, GENERATOR.SOURCE)
            self.assertEqual(len(bits), 60)
            self.assertTrue(all(bits[row * 3 + 2] & 0x0f == 0 for row in range(20)))
            total += len(bits)
        self.assertEqual(total, 300)

    def test_unchanged_header_preserves_mtime(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "icons.h"
            self.assertTrue(GENERATOR.write_header_atomic(path, "first\n"))
            modified = path.stat().st_mtime_ns
            self.assertFalse(GENERATOR.write_header_atomic(path, "first\n"))
            self.assertEqual(path.stat().st_mtime_ns, modified)
            self.assertTrue(GENERATOR.write_header_atomic(path, "second\n"))
            self.assertEqual(path.read_text(encoding="utf-8"), "second\n")
            self.assertEqual(sorted(item.name for item in path.parent.iterdir()), ["icons.h"])

    def test_missing_source_does_not_replace_output(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "icons.h"
            output.write_text("keep\n", encoding="utf-8")
            result = subprocess.run([sys.executable, str(GENERATOR_PATH), "--source-dir", directory,
                                     "--output", str(output)], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_text(encoding="utf-8"), "keep\n")

    def test_malformed_source_does_not_replace_output(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "svg"
            shutil.copytree(GENERATOR.SOURCE, source)
            (source / "info.svg").write_text("<svg", encoding="utf-8")
            output = Path(directory) / "icons.h"
            output.write_text("keep\n", encoding="utf-8")
            result = subprocess.run([sys.executable, str(GENERATOR_PATH), "--source-dir", str(source),
                                     "--output", str(output)], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_text(encoding="utf-8"), "keep\n")

    def test_check_mode_does_not_rewrite(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "icons.h"
            output.write_text("stale\n", encoding="utf-8")
            result = subprocess.run([sys.executable, str(GENERATOR_PATH), "--output", str(output),
                                     "--check"], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_text(encoding="utf-8"), "stale\n")

    def test_external_svg_content_is_rejected(self):
        svg = b'<svg xmlns="http://www.w3.org/2000/svg"><image href="external.svg"/></svg>'
        with self.assertRaises(ValueError):
            GENERATOR.monochrome_svg("wifi", svg)


if __name__ == "__main__":
    unittest.main()
