// 定义天气时钟固件与 SDL 预览共用的静态布局。
#pragma once

#include "ui_work_page_layout.h"

namespace ui_clock_layout {

inline constexpr int kClockTimeCanvasX = 6;
inline constexpr int kClockTimeCanvasY = 54;
inline constexpr int kClockTimeCanvasWidth = 340;
inline constexpr int kClockTimeCanvasHeight = 126;
inline constexpr int kClockSecondCanvasX = 350;
inline constexpr int kClockSecondCanvasY = 140;
inline constexpr int kClockSecondCanvasWidth = 44;
inline constexpr int kClockSecondCanvasHeight = 36;
inline constexpr int kClockStatusGifCanvasX = 279;
inline constexpr int kClockStatusGifCanvasY = 196;

inline constexpr int kClockDateLabelX = 198;
inline constexpr int kClockDateLabelY = 15;
inline constexpr int kClockDateLabelWidth = 182;
inline constexpr int kClockDateLabelHeight = 26;

inline constexpr int kClockAlertPillX = 64;
inline constexpr int kClockAlertPillY = 11;
inline constexpr int kClockAlertPillWidth = 128;
inline constexpr int kClockAlertPillHeight = 26;
inline constexpr int kClockAlertPillRadius = 13;
inline constexpr int kClockAlertIconX = 4;
inline constexpr int kClockAlertIconY = 4;
inline constexpr int kClockAlertLabelX = 24;
inline constexpr int kClockAlertLabelY = 4;
inline constexpr int kClockAlertLabelWidth = 94;
inline constexpr int kClockAlertLabelHeight = 18;

inline constexpr int kClockChimeStatusIconX = 64;
inline constexpr int kClockChimeStatusIconY = 15;
inline constexpr int kClockWifiStatusIconX = 90;
inline constexpr int kClockWifiStatusIconY = 15;
inline constexpr int kClockAlarmStatusIconX = 116;
inline constexpr int kClockAlarmStatusIconY = 15;

inline constexpr int kClockDividerX =
    ui_work_page_layout::kTopSeparatorX;
inline constexpr int kClockTopDividerY =
    ui_work_page_layout::kTopSeparatorY;
inline constexpr int kClockBottomDividerY = 210;
inline constexpr int kClockDividerWidth =
    ui_work_page_layout::kTopSeparatorWidth;
inline constexpr int kClockDividerHeight =
    ui_work_page_layout::kTopSeparatorHeight;
inline constexpr int kClockSecondProgressCanvasY = 206;
inline constexpr int kClockLowerPanelSeparatorY = 216;
inline constexpr int kClockLowerPanelSeparatorWidth = 2;
inline constexpr int kClockLowerPanelSeparatorHeight = 60;
inline constexpr int kClockLowerPanelSeparatorAX = 230;
inline constexpr int kClockLowerPanelSeparatorBX = 230;

inline constexpr int kClockWeatherCityLabelX = 44;
inline constexpr int kClockWeatherCityLabelY = 215;
inline constexpr int kClockWeatherCityLabelWidth = 48;
inline constexpr int kClockWeatherCityLabelHeight = 20;
inline constexpr int kClockAirUvLabelX = 6;
inline constexpr int kClockAirUvLabelY = 256;
inline constexpr int kClockAirUvLabelWidth = 218;
inline constexpr int kClockAirUvLabelHeight = 20;
inline constexpr int kClockWarningLabelX = 6;
inline constexpr int kClockWarningLabelY = 279;
inline constexpr int kClockWarningLabelWidth = 388;
inline constexpr int kClockWarningLabelHeight = 20;
inline constexpr int kClockIndoorTitleX = 238;
inline constexpr int kClockIndoorTitleY = 215;
inline constexpr int kClockIndoorTitleWidth = 156;
inline constexpr int kClockIndoorTitleHeight = 20;
inline constexpr int kClockWeatherIconLabelX = 6;
inline constexpr int kClockWeatherIconLabelY = 215;
inline constexpr int kClockWeatherIconLabelWidth = 36;
inline constexpr int kClockWeatherIconLabelHeight = 38;
inline constexpr int kClockWeatherInfoLabelX = 92;
inline constexpr int kClockWeatherInfoLabelY = 215;
inline constexpr int kClockWeatherInfoLabelWidth = 132;
inline constexpr int kClockWeatherInfoLabelHeight = 20;
inline constexpr int kClockWeatherMetricLabelWidth = 84;
inline constexpr int kClockWeatherMetricLabelHeight = 20;
inline constexpr int kClockWeatherTempLabelX = 44;
inline constexpr int kClockWeatherHumiLabelX = 116;
inline constexpr int kClockWeatherTempLabelY = 236;
inline constexpr int kClockWeatherHumiLabelY = 236;

inline constexpr int kClockTempIconX = 238;
inline constexpr int kClockTempIconY = 237;
inline constexpr int kClockHumiIconX = 238;
inline constexpr int kClockHumiIconY = 258;
inline constexpr int kClockLocalMetricLabelX = 266;
inline constexpr int kClockLocalTempLabelY = 235;
inline constexpr int kClockLocalHumiLabelY = 256;
inline constexpr int kClockLocalMetricLabelWidth = 90;
inline constexpr int kClockLocalMetricLabelHeight = 21;
inline constexpr int kClockTrendCanvasX = 368;
inline constexpr int kClockTempTrendCanvasY = 237;
inline constexpr int kClockHumiTrendCanvasY = 258;
inline constexpr int kClockLowBatteryIconX = 216;
inline constexpr int kClockLowBatteryIconY = 224;

static_assert(kClockTimeCanvasX >= 0 && kClockTimeCanvasY >= 0 &&
                  kClockTimeCanvasWidth > 0 &&
                  kClockTimeCanvasHeight > 0,
              "clock time canvas must have a valid rectangle");
static_assert(kClockSecondCanvasX >= 0 && kClockSecondCanvasY >= 0 &&
                  kClockSecondCanvasWidth > 0 &&
                  kClockSecondCanvasHeight > 0,
              "clock second canvas must have a valid rectangle");
static_assert(kClockAlertIconX >= 0 && kClockAlertIconY >= 0 &&
                  kClockAlertLabelX >= 0 && kClockAlertLabelY >= 0,
              "clock alert children must have non-negative origins");
static_assert(kClockAlertLabelX + kClockAlertLabelWidth <=
                  kClockAlertPillWidth &&
                  kClockAlertLabelY + kClockAlertLabelHeight <=
                      kClockAlertPillHeight,
              "clock alert label must fit the alert pill");
static_assert(kClockBottomDividerY >
                  kClockTopDividerY + kClockDividerHeight,
              "clock body dividers must not overlap");
static_assert(kClockLowerPanelSeparatorY +
                      kClockLowerPanelSeparatorHeight <=
                  300,
              "clock lower-panel separators must fit the display");
static_assert(kClockWeatherTempLabelX <
                  kClockWeatherHumiLabelX,
              "clock weather metrics must keep temperature beside humidity");
static_assert(kClockLocalTempLabelY + kClockLocalMetricLabelHeight <=
                  kClockLocalHumiLabelY,
              "clock local metrics must not overlap");
static_assert(kClockAirUvLabelY + kClockAirUvLabelHeight <= kClockWarningLabelY &&
                  kClockLocalHumiLabelY + kClockLocalMetricLabelHeight <= kClockWarningLabelY,
              "clock details must leave a full-width warning row");
static_assert(kClockWarningLabelY + kClockWarningLabelHeight <= 300 &&
                  kClockWarningLabelX + kClockWarningLabelWidth <= 400,
              "clock warning row must fit the display");
static_assert(kClockAirUvLabelX + kClockAirUvLabelWidth < kClockLowerPanelSeparatorAX &&
                  kClockIndoorTitleX > kClockLowerPanelSeparatorAX,
              "outdoor and indoor panels must remain separated");

} // namespace ui_clock_layout
