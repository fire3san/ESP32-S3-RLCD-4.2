#include "ui_clock_layout.h"
#include "ui_hong_kong_panel_text.h"

#include <cassert>
#include <cstring>

int main()
{
    using namespace ui_clock_layout;
    static_assert(kClockBottomDividerY == 210, "preserve large clock area");
    static_assert(kClockSecondCanvasX + kClockSecondCanvasWidth <= 400);
    static_assert(kClockWarningLabelY > kClockLowerPanelSeparatorY + kClockLowerPanelSeparatorHeight);
    WeatherAlertData alert = {};
    char text[256] = {};
    ui_hong_kong_panel::format_warning(alert, false, text, sizeof(text));
    assert(std::strcmp(text, "天氣警告：等待數據") == 0);
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text));
    assert(std::strcmp(text, "天氣警告：現時沒有生效警告") == 0);

    const time_t sync_base = 1700000000;
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 10);
    assert(std::strcmp(text, "天氣警告：現時沒有生效警告 (剛剛)") == 0);

    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 300);
    assert(std::strcmp(text, "天氣警告：現時沒有生效警告 (5分鐘前)") == 0);

    alert.active = true;
    alert.count = 2;
    std::strcpy(alert.titles[0], "雷暴警告");
    std::strcpy(alert.titles[1], "酷熱天氣警告");
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text));
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告") == 0);

    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 120);
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告 (2分鐘前)") == 0);

    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 7200);
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告 (2小時前)") == 0);

    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 90000);
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告 (1天前)") == 0);

    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base - 10);
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告 (剛剛)") == 0);

    alert.count = 1;
    alert.titles[0][0] = '\0';
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text), sync_base, sync_base + 60);
    assert(std::strcmp(text, "天氣警告：生效中 (1分鐘前)") == 0);

    char small[8] = {};
    ui_hong_kong_panel::format_warning(alert, true, small, sizeof(small), sync_base, sync_base + 300);
    assert(small[sizeof(small) - 1] == '\0');
    ui_hong_kong_panel::format_warning(alert, true, nullptr, 0);
}
