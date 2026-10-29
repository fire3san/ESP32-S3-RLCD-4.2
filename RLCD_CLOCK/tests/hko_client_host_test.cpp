#include "hko_client.h"
#include "open_meteo_client.h"
#include "weather_provider.h"
#include "weather_snapshot_store.h"
#include "network_http_client.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {
std::vector<std::string> requests;
std::string manual="香港";
bool continuation=true, fail_air=false, fail_warnings=false;
int interrupt_at=0;
bool change_generation=false;
int commits=0;
WeatherSnapshotStore store={};
const char *current=R"({"updateTime":"2026-10-29T19:02:00+08:00","temperature":{"data":[{"place":"香港天文台","value":26}]},"humidity":{"data":[{"place":"香港天文台","value":68}]},"icon":[74],"uvindex":""})";
void reset() { requests.clear();continuation=true;fail_air=false;fail_warnings=false;interrupt_at=0;change_generation=false;commits=0; }
}
bool network_sync_continuation_allowed() { return continuation; }
int64_t esp_timer_get_time() { return 1; }
bool manual_weather_city_snapshot(char *out,size_t n) { std::snprintf(out,n,"%s",manual.c_str());return true; }
bool ip_geolocation_lookup_cached(char *,size_t,char *,size_t) {
    assert(false && "Hong Kong default must not need IP geolocation");return false;
}
esp_err_t http_get_text(const char *url,char *out,size_t size,const char *) {
    requests.emplace_back(url);const char *body=nullptr;
    if(strstr(url,"rhrread"))body=current;
    else if(strstr(url,"warnsum")) { if(fail_warnings)return ESP_FAIL;body="{}"; }
    else if(strstr(url,"aqhi_ind_rss")) {
        if(fail_air)return ESP_FAIL;
        body="<rss><channel><item><title>東區</title><description>一般監測站: 5 中</description></item></channel></rss>";
    } else if(strstr(url,"dataType=fnd"))body=R"({"weatherForecast":[{"forecastDate":"20261030","forecastMaxtemp":{"value":30},"forecastMintemp":{"value":25},"ForecastIcon":50}]})";
    else if(strstr(url,"geocoding-api"))body=R"({"results":[{"latitude":51.5,"longitude":-0.12,"name":"London"}]})";
    else if(strstr(url,"api.open-meteo.com"))body=R"({"utc_offset_seconds":28800,"current":{"time":"2026-10-29T19:00","temperature_2m":20,"relative_humidity_2m":60,"weather_code":0,"is_day":1,"wind_speed_10m":3,"wind_direction_10m":45},"daily":{"time":["2026-10-29"],"temperature_2m_max":[24],"temperature_2m_min":[15],"weather_code":[0],"sunrise":["2026-10-29T06:00"],"sunset":["2026-10-29T18:00"]}})";
    assert(body);assert(strlen(body)<size);strcpy(out,body);
    if(static_cast<int>(requests.size())==interrupt_at) {
        if(change_generation)weather_provider_store(WeatherProvider::kQweather);else continuation=false;
    }
    return ESP_OK;
}
void commit_weather_update_snapshot(const WeatherData &w,const WeatherAlertData &a,const WeatherForecastData &f,const WeatherAirData &air,bool ao,bool fo,bool io) {
    assert(continuation&&w.configuration_generation==weather_provider_generation());++commits;
    weather_snapshot_store_commit(&store,w,a,f,air,ao,fo,io,123);
}
int main() {
    weather_provider_store(WeatherProvider::kOpenMeteo);
    reset();assert(perform_open_meteo_update(WeatherUpdateScope::kCurrentAndAlerts)==WeatherUpdateResult::kSuccess);
    assert(requests.size()==3&&commits==1&&store.weather.hko&&store.air.hk_aqhi&&store.alert.updated_at);
    reset();fail_air=true;fail_warnings=true;
    assert(perform_open_meteo_update(WeatherUpdateScope::kCurrentAndAlerts)==WeatherUpdateResult::kSuccess);
    assert(store.air.ready&&store.air.hk_aqhi&&store.alert.updated_at);
    reset();assert(perform_open_meteo_update(WeatherUpdateScope::kFull)==WeatherUpdateResult::kSuccess);
    assert(requests.size()==4&&store.forecast.ready);
    for(int boundary=1;boundary<=4;++boundary) {
        for(bool generation_change:{false,true}) {
            weather_provider_store(WeatherProvider::kOpenMeteo);reset();interrupt_at=boundary;change_generation=generation_change;
            assert(perform_open_meteo_update(WeatherUpdateScope::kFull)==WeatherUpdateResult::kFailed);
            assert(commits==0&&static_cast<int>(requests.size())==boundary);
        }
    }
    weather_provider_store(WeatherProvider::kOpenMeteo);reset();manual="";
    assert(perform_open_meteo_update(WeatherUpdateScope::kCurrentAndAlerts)==WeatherUpdateResult::kSuccess&&store.weather.hko);
    reset();manual="London";
    assert(perform_open_meteo_update(WeatherUpdateScope::kCurrentAndAlerts)==WeatherUpdateResult::kSuccess);
    assert(!store.weather.hko&&store.weather.open_meteo&&!store.air.hk_aqhi);
    assert(requests.size()==2&&requests[0].find("geocoding-api")!=std::string::npos&&requests[1].find("api.open-meteo.com")!=std::string::npos);
}
