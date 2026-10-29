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
    assert(std::strcmp(text, "天氣警告：沒有生效警告") == 0);
    alert.active = true;
    alert.count = 2;
    std::strcpy(alert.titles[0], "雷暴警告");
    std::strcpy(alert.titles[1], "酷熱天氣警告");
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text));
    assert(std::strcmp(text, "天氣警告：雷暴警告 / 酷熱天氣警告") == 0);
    alert.count = 1;
    alert.titles[0][0] = '\0';
    ui_hong_kong_panel::format_warning(alert, true, text, sizeof(text));
    assert(std::strcmp(text, "天氣警告：生效中") == 0);
    char small[8] = {};
    ui_hong_kong_panel::format_warning(alert, true, small, sizeof(small));
    assert(small[sizeof(small) - 1] == '\0');
    ui_hong_kong_panel::format_warning(alert, true, nullptr, 0);
}
