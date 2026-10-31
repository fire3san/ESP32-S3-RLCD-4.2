#pragma once

#include "weather_types.h"

#include <cstdio>
#include <cstring>
#include <ctime>

namespace ui_hong_kong_panel {

inline void format_warning(const WeatherAlertData &alert,
                           bool available,
                           char *out,
                           size_t size,
                           time_t updated_at = 0,
                           time_t now = 0)
{
    if (!out || size == 0) return;
    if (!available) {
        std::snprintf(out, size, "天氣警告：等待數據");
        return;
    }
    std::snprintf(out, size, "天氣警告：%s", alert.active ? "" : "現時沒有生效警告");
    const size_t reserve_suffix = (updated_at > 0) ? 24 : 0;
    bool appended = false;
    for (int i = 0; alert.active && i < alert.count && i < kMaxWeatherAlerts; ++i) {
        if (!alert.titles[i][0]) continue;
        const size_t used = std::strlen(out);
        if (used + reserve_suffix >= size) break;
        const size_t available_for_title = size - used - reserve_suffix;
        const size_t required = std::strlen(alert.titles[i]) + (appended ? 3 : 0);
        if (required >= available_for_title) break;
        std::snprintf(out + used, size - used, "%s%s", appended ? " / " : "", alert.titles[i]);
        appended = true;
    }
    if (alert.active && !appended) {
        std::snprintf(out, size, "天氣警告：生效中");
    }

    if (updated_at > 0) {
        char suffix[32] = {};
        if (now <= 0) {
            now = time(nullptr);
        }
        const time_t diff = (now >= updated_at) ? (now - updated_at) : 0;
        if (diff < 60) {
            std::snprintf(suffix, sizeof(suffix), " (剛剛)");
        } else if (diff < 3600) {
            std::snprintf(suffix, sizeof(suffix), " (%d分鐘前)", static_cast<int>(diff / 60));
        } else if (diff < 86400) {
            std::snprintf(suffix, sizeof(suffix), " (%d小時前)", static_cast<int>(diff / 3600));
        } else {
            std::snprintf(suffix, sizeof(suffix), " (%d天前)", static_cast<int>(diff / 86400));
        }
        const size_t used = std::strlen(out);
        if (std::strlen(suffix) < size - used) {
            std::snprintf(out + used, size - used, "%s", suffix);
        }
    }
}

} // namespace ui_hong_kong_panel
