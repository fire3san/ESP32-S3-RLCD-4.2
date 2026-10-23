# Settings Icon Sources

The five original SVG files in this directory are from
[guokaigdg/naive-icons](https://github.com/guokaigdg/naive-icons), pinned to
commit `a8619441efc40609e72c00e860f4f98afc654a2b`.
Copyright (c) 2026 Naive Icons. The original MIT license is retained in `LICENSE`.

| SVG | Use |
| --- | --- |
| `wifi.svg` | Network category |
| `volume.svg` | Sound category |
| `monitor.svg` | Display category |
| `info.svg` | System / device information category |
| `settings.svg` | Settings heading |

Original SVGs are unchanged. The generator adapts colored strokes to black,
colored bodies to white, and Wi-Fi / information dots to black before rasterizing
at 8x resolution and reducing to 20x20 pixels with a fixed threshold of 128.
There is no dithering, animation, runtime SVG decoder, or React dependency.
The five generated 1-bit masks occupy 300 bytes, excluding image descriptors.

From `RLCD_CLOCK/`, regenerate with:

```sh
python3 tools/generate_settings_icons.py
python3 tools/generate_settings_icons.py --check
```

Only regeneration requires Python Pillow and `rsvg-convert` (librsvg).
Normal ESP-IDF and SDL builds use the checked-in `../settings_naive_icons.h`
and do not access the network or require an SVG rendering tool.
