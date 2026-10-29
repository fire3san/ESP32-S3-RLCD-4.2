#include "hko_client.h"
#include "hko_parser.h"
#include "weather_provider.h"
#include "weather_state_internal.h"
#include "network_http_client.h"
#include "network_sync_runtime.h"
#include "scoped_heap_buffer.h"
#include "esp_attr.h"

namespace {
constexpr const char *kCurrent="https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=rhrread&lang=tc";
constexpr const char *kForecast="https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=fnd&lang=tc";
constexpr const char *kWarnings="https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=warnsum&lang=tc";
constexpr const char *kAir="https://www.aqhi.gov.hk/epd/ddata/html/out/aqhi_ind_rss_ChT.xml";
struct Workspace { WeatherData weather;WeatherForecastData forecast;WeatherAlertData alert;WeatherAirData air; };
EXT_RAM_BSS_ATTR Workspace workspace;
bool allowed(uint32_t generation) {
    return network_sync_continuation_allowed() && generation==weather_provider_generation();
}
bool fetch(const char *url,char *out,size_t size,uint32_t generation) {
    return allowed(generation)&&http_get_text(url,out,size)==ESP_OK&&allowed(generation);
}
}

bool hko_validate_configuration() {
    const auto generation=weather_provider_generation();
    ScopedHeapBuffer<char> response(24576,HeapBufferInit::kUninitialized,HeapBufferStorage::kPsramPreferred);
    WeatherData current;
    return response&&fetch(kCurrent,response.get(),response.size(),generation)&&parse_hko_current(response.get(),&current);
}

WeatherUpdateResult perform_hko_update(WeatherUpdateScope scope,uint32_t generation) {
    if(!allowed(generation))return WeatherUpdateResult::kFailed;
    workspace={};
    ScopedHeapBuffer<char> response(24576,HeapBufferInit::kUninitialized,HeapBufferStorage::kPsramPreferred);
    if(!response||!fetch(kCurrent,response.get(),response.size(),generation)||
       !parse_hko_current(response.get(),&workspace.weather))return WeatherUpdateResult::kFailed;
    workspace.weather.configuration_generation=generation;
    // Warnings and AQHI are part of the Hong Kong current conditions panel.
    const bool alerts_ok=fetch(kWarnings,response.get(),response.size(),generation)&&parse_hko_warnings(response.get(),&workspace.alert);
    const bool air_ok=fetch(kAir,response.get(),response.size(),generation)&&parse_epd_aqhi_rss(response.get(),&workspace.air);
    bool forecast_ok=false;
    if(scope==WeatherUpdateScope::kFull)forecast_ok=fetch(kForecast,response.get(),response.size(),generation)&&parse_hko_forecast(response.get(),&workspace.forecast);
    if(!allowed(generation))return WeatherUpdateResult::kFailed;
    commit_weather_update_snapshot(workspace.weather,workspace.alert,workspace.forecast,workspace.air,alerts_ok,forecast_ok,air_ok);
    return WeatherUpdateResult::kSuccess;
}
