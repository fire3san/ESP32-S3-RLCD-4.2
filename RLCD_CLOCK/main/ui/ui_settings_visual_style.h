// 设置页测试皮肤：仅改变视觉属性，菜单结构与输入状态不变。
#pragma once
#include "lvgl.h"
#include "settings_naive_icons.h"
#include <string.h>

namespace settings_icon_style {
inline constexpr int kIconGap = 6;
inline constexpr int kPrimaryIconX = 26;
inline constexpr int kPrimaryTextPadding = 36;
inline constexpr int kSelectedPrimaryTextPadding = 40;

constexpr lv_img_dsc_t image(const uint8_t *bits)
{
    lv_img_dsc_t result = {};
    result.header.cf = LV_IMG_CF_ALPHA_1BIT;
    result.header.w = settings_naive_icons::kSize;
    result.header.h = settings_naive_icons::kSize;
    result.data_size = settings_naive_icons::kBytesPerRow * settings_naive_icons::kSize;
    result.data = bits;
    return result;
}

inline constexpr lv_img_dsc_t kPrimaryIcons[] = {
    image(settings_naive_icons::k_wifi),
    image(settings_naive_icons::k_volume),
    image(settings_naive_icons::k_monitor),
    image(settings_naive_icons::k_info),
};
inline constexpr lv_img_dsc_t kTitleIcon = image(settings_naive_icons::k_settings);

inline void draw(lv_event_t *event, bool title)
{
    auto *label = lv_event_get_target(event);
    auto *icon = static_cast<const lv_img_dsc_t *>(lv_event_get_user_data(event));
    if (!label || !icon) {
        return;
    }
    const lv_font_t *font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    const char *text = lv_label_get_text(label);
    if (!font || !text) {
        return;
    }
    lv_area_t bounds;
    lv_obj_get_coords(label, &bounds);
    const int text_width = lv_txt_get_width(text, static_cast<uint32_t>(strlen(text)),
                                          font, 0, LV_TEXT_FLAG_NONE);
    const int width = lv_area_get_width(&bounds);
    const int icon_size = settings_naive_icons::kSize;
    const int x = bounds.x1 + (title ? (width - icon_size - kIconGap - text_width) / 2
                                   : kPrimaryIconX);
    const int y = bounds.y1 + lv_obj_get_style_pad_top(label, LV_PART_MAIN) +
                  (font->line_height - icon_size) / 2;
    lv_area_t area = {static_cast<lv_coord_t>(x), static_cast<lv_coord_t>(y),
                      static_cast<lv_coord_t>(x + icon_size - 1),
                      static_cast<lv_coord_t>(y + icon_size - 1)};
    lv_draw_img_dsc_t style;
    lv_draw_img_dsc_init(&style);
    style.recolor = lv_color_black();
    style.recolor_opa = LV_OPA_COVER;
    lv_draw_img(lv_event_get_draw_ctx(event), &style, &area, icon);
}
} // namespace settings_icon_style

inline void settings_attach_primary_icon(lv_obj_t *label, int index)
{
    if (!label || index < 0 || index >= 4) {
        return;
    }
    lv_obj_add_event_cb(label, [](lv_event_t *event) {
        settings_icon_style::draw(event, false);
    }, LV_EVENT_DRAW_MAIN,
    const_cast<lv_img_dsc_t *>(&settings_icon_style::kPrimaryIcons[index]));
}

inline void settings_attach_title_icon(lv_obj_t *label)
{
    if (!label) {
        return;
    }
    lv_obj_set_style_pad_left(label,
                            settings_naive_icons::kSize + settings_icon_style::kIconGap,
                            LV_PART_MAIN);
    lv_obj_set_style_pad_right(label, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(label, [](lv_event_t *event) {
        settings_icon_style::draw(event, true);
    }, LV_EVENT_DRAW_MAIN, const_cast<lv_img_dsc_t *>(&settings_icon_style::kTitleIcon));
}

inline void settings_attach_category_marker(lv_obj_t *label) {
    if(!label) return;
    lv_obj_add_event_cb(label,[](lv_event_t *e) {
        lv_obj_t *obj=lv_event_get_target(e);
        const int width=lv_obj_get_style_line_width(obj,0);
        if(width==0) return;
        lv_area_t area; lv_obj_get_coords(obj,&area);
        lv_area_t marker={(lv_coord_t)(area.x1+4),(lv_coord_t)(area.y1+6),
                          (lv_coord_t)(area.x1+3+width),(lv_coord_t)(area.y2-6)};
        lv_draw_rect_dsc_t style; lv_draw_rect_dsc_init(&style);
        style.bg_color=lv_color_black();
        lv_draw_rect(lv_event_get_draw_ctx(e),&style,&marker);
    },LV_EVENT_DRAW_MAIN,nullptr);
}

inline void settings_visual_style(lv_obj_t *label,bool selected,bool primary) {
    if(!label) return;
    const bool reverse=selected && !primary;
    lv_obj_set_style_bg_color(label,reverse?lv_color_black():lv_color_white(),0);
    lv_obj_set_style_bg_opa(label,LV_OPA_COVER,0);
    lv_obj_set_style_text_color(label,reverse?lv_color_white():lv_color_black(),0);
    lv_obj_set_style_border_color(label,lv_color_black(),0);
    lv_obj_set_style_border_side(label,LV_BORDER_SIDE_FULL,0);
    lv_obj_set_style_border_width(label,1,0);
    lv_obj_set_style_radius(label,4,0);
    lv_obj_set_style_line_width(label,primary && selected?3:0,0);
    // Center-aligned text moves two pixels; the option frame stays fixed.
    lv_obj_set_style_pad_left(label, primary
        ? (selected ? settings_icon_style::kSelectedPrimaryTextPadding
                    : settings_icon_style::kPrimaryTextPadding)
        : 10, 0);
    lv_obj_set_style_pad_right(label,10,0);
    lv_obj_set_style_pad_top(label,5,0);
    lv_obj_set_style_pad_bottom(label,5,0);
}
