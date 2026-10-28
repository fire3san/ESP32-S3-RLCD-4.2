// 实现固件与 SDL 预览共用的 DSEG 字形查找和像素绘制。
#include "ui_dseg_render.h"

#include "dseg_digits.h"

#include <string.h>

namespace {
constexpr uint32_t kBitsPerByte = 8;
constexpr uint8_t kPackedBitMask = 0x80;

bool packed_bit_is_set(const uint8_t *bits, uint32_t bit_index)
{
    if (!bits) {
        return false;
    }
    return (bits[bit_index / kBitsPerByte] &
            static_cast<uint8_t>(kPackedBitMask >> (bit_index % kBitsPerByte))) != 0;
}
} // namespace

const DsegGlyph *find_dseg_glyph(const DsegFont &font, char ch)
{
    if (!font.chars || !font.glyphs) {
        return nullptr;
    }
    const char *pos = strchr(font.chars, ch);
    if (!pos) {
        return nullptr;
    }
    return &font.glyphs[pos - font.chars];
}

int draw_dseg_text(lv_obj_t *canvas,
                   const DsegFont &font,
                   const char *text,
                   int cursor_x,
                   int baseline_y)
{
    int x_cursor = cursor_x;
    if (!canvas || !text || !font.bitmap) {
        return x_cursor;
    }
    lv_img_dsc_t *image = lv_canvas_get_img(canvas);
    if (!image) {
        return x_cursor;
    }
    for (const char *p = text; *p; ++p) {
        const DsegGlyph *glyph = find_dseg_glyph(font, *p);
        if (!glyph) {
            continue;
        }
        uint32_t bit = 0;
        const uint8_t *glyph_bitmap = font.bitmap + glyph->bitmap_offset;
        for (int y = 0; y < glyph->height; ++y) {
            for (int x = 0; x < glyph->width; ++x, ++bit) {
                if (packed_bit_is_set(glyph_bitmap, bit)) {
                    lv_img_buf_set_px_color(image,
                                            x_cursor + glyph->x_offset + x,
                                            baseline_y + glyph->y_offset + y,
                                            lv_color_black());
                }
            }
        }
        x_cursor += glyph->x_advance;
    }
    return x_cursor;
}

int draw_dseg_scaled_text(lv_obj_t *canvas,
                          const DsegFont &font,
                          const char *text,
                          int cursor_x,
                          int baseline_y,
                          int scale_num,
                          int scale_den)
{
    if (scale_num <= 0 || scale_den <= 0 || scale_num == scale_den) {
        return draw_dseg_text(canvas, font, text, cursor_x, baseline_y);
    }
    int x_cursor = cursor_x;
    if (!canvas || !text || !font.bitmap) {
        return x_cursor;
    }
    lv_img_dsc_t *image = lv_canvas_get_img(canvas);
    if (!image) {
        return x_cursor;
    }
    const int img_w = image->header.w;
    const int img_h = image->header.h;

    for (const char *p = text; *p; ++p) {
        const DsegGlyph *glyph = find_dseg_glyph(font, *p);
        if (!glyph) {
            continue;
        }
        int dst_x_offset = (glyph->x_offset * scale_num) / scale_den;
        int dst_y_offset = (glyph->y_offset * scale_num) / scale_den;
        int dst_w = (glyph->width * scale_num) / scale_den;
        int dst_h = (glyph->height * scale_num) / scale_den;
        int dst_advance = (glyph->x_advance * scale_num) / scale_den;

        const uint8_t *glyph_bitmap = font.bitmap + glyph->bitmap_offset;

        for (int dy = 0; dy < dst_h; ++dy) {
            int src_y = (dy * scale_den) / scale_num;
            if (src_y >= glyph->height) {
                src_y = glyph->height - 1;
            }
            int py = baseline_y + dst_y_offset + dy;
            if (py < 0 || py >= img_h) {
                continue;
            }
            for (int dx = 0; dx < dst_w; ++dx) {
                int src_x = (dx * scale_den) / scale_num;
                if (src_x >= glyph->width) {
                    src_x = glyph->width - 1;
                }
                uint32_t bit = static_cast<uint32_t>(src_y * glyph->width + src_x);
                if (packed_bit_is_set(glyph_bitmap, bit)) {
                    int px = x_cursor + dst_x_offset + dx;
                    if (px >= 0 && px < img_w) {
                        lv_img_buf_set_px_color(image, px, py, lv_color_black());
                    }
                }
            }
        }
        x_cursor += dst_advance;
    }
    return x_cursor;
}
