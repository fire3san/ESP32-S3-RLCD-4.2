#pragma once

#include "weather_types.h"

#include <cstdio>
#include <cstring>

namespace ui_hong_kong_panel {

inline void format_warning(const WeatherAlertData &alert, bool available, char *out, size_t size)
{
    if (!out || size == 0) return;
    if (!available) {
        std::snprintf(out, size, "天氣警告：等待數據");
        return;
    }
    std::snprintf(out, size, "天氣警告：%s", alert.active ? "" : "沒有生效警告");
    bool appended = false;
    for (int i = 0; alert.active && i < alert.count && i < kMaxWeatherAlerts; ++i) {
        if (!alert.titles[i][0]) continue;
        const size_t used = std::strlen(out);
        const size_t required = std::strlen(alert.titles[i]) + (appended ? 3 : 0);
        if (required >= size - used) break;
        std::snprintf(out + used, size - used, "%s%s", appended ? " / " : "", alert.titles[i]);
        appended = true;
    }
    if (alert.active && !appended) {
        std::snprintf(out, size, "天氣警告：生效中");
    }
}

} // namespace ui_hong_kong_panel
