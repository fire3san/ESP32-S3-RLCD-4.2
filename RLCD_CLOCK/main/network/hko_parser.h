#pragma once

#include "weather_types.h"

bool hko_city_matches(const char *city);
bool parse_hko_current(const char *json, WeatherData *weather);
bool parse_hko_forecast(const char *json, WeatherForecastData *forecast);
bool parse_hko_warnings(const char *json, WeatherAlertData *alert);
bool parse_epd_aqhi_rss(const char *xml, WeatherAirData *air);
