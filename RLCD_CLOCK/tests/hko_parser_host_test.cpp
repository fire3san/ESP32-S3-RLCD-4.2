#include "hko_parser.h"
#include "weather_snapshot_store.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

static std::string read(const char *path) {
    std::ifstream in(path);assert(in);return {std::istreambuf_iterator<char>(in),{}};
}
int main(int argc,char **argv) {
    assert(hko_city_matches("香港")&&hko_city_matches("Hong Kong"));
    assert(!hko_city_matches("London")&&!hko_city_matches(nullptr)&&!hko_city_matches("香港附近"));
    const char *current=R"({"updateTime":"2026-10-29T19:02:00+08:00","temperature":{"data":[{"place":"京士柏","value":21},{"place":"香港天文台","value":26}]},"humidity":{"data":[{"place":"香港天文台","value":68}]},"icon":[74],"uvindex":{"data":[{"value":3.5,"desc":"中等"}]}})";
    WeatherData w;assert(parse_hko_current(current,&w));
    assert(w.hko&&!w.open_meteo&&!strcmp(w.temp,"26")&&!strcmp(w.humidity,"68"));
    assert(!strcmp(w.uv,"3.5")&&!strcmp(w.uv_risk,"中等"));
    const auto saved=w;assert(!parse_hko_current("{}",&w));assert(!strcmp(w.temp,saved.temp));
    std::string night=current;const auto uv=night.find("\"uvindex\"");night.replace(uv,night.size()-uv,"\"uvindex\":\"\"}");
    assert(parse_hko_current(night.c_str(),&w)&&!w.uv[0]);
    WeatherAlertData alert;assert(parse_hko_warnings(R"({"WHOT":{"name":"酷熱天氣警告","actionCode":"ISSUE"},"WRAIN":{"name":"暴雨警告","actionCode":"CANCEL"}})",&alert));
    assert(alert.active&&alert.count==1&&alert.updated_at>0);
    assert(!parse_hko_warnings("[]",&alert)&&alert.active);
    assert(parse_hko_warnings("{}",&alert)&&!alert.active&&alert.count==0&&alert.updated_at>0);
    WeatherForecastData forecast;
    assert(parse_hko_forecast(R"({"weatherForecast":[{"forecastDate":"20261030","forecastMaxtemp":{"value":30},"forecastMintemp":{"value":25},"ForecastIcon":50}]})",&forecast));
    assert(forecast.count==1&&!strcmp(forecast.days[0].date,"2026-10-30"));
    assert(!parse_hko_forecast("{}",&forecast));
    WeatherAirData air;
    const char *rss=R"(<rss><channel><item><title>路邊</title><description><![CDATA[路邊監測站: 10+ 嚴重]]></description></item><item><title>南區</title><description><![CDATA[南區 - 一般監測站: 3 低]]></description></item><item><title>東區</title><description><![CDATA[東區 - 一般監測站: 7 高]]></description></item></channel></rss>)";
    assert(parse_epd_aqhi_rss(rss,&air));assert(air.ready&&air.hk_aqhi&&!air.us_aqi);
    assert(!strcmp(air.aqi,"7")&&!strcmp(air.category,"高")&&!strcmp(air.station,"東區"));
    const char *severe="<rss><channel><item><title>東區</title><description>一般監測站: 10+ 嚴重</description></item></channel></rss>";
    assert(parse_epd_aqhi_rss(severe,&air)&&!strcmp(air.aqi,"10+")&&!strcmp(air.category,"嚴重"));
    assert(!parse_epd_aqhi_rss("<html>unavailable</html>",&air));
    assert(!parse_epd_aqhi_rss("<rss><channel><item><title>南區</title><description>一般監測站: - 中</description></item></channel></rss>",&air));
    WeatherSnapshotStore store={};WeatherForecastData f={};WeatherAlertData a={};
    weather_snapshot_store_commit(&store,w,a,f,air,true,false,true,1);
    WeatherData read_weather;WeatherAlertData read_alert;WeatherAirData read_air;
    weather_snapshot_store_read(store,&read_weather,&read_alert,nullptr,&read_air);
    assert(read_weather.hko&&read_air.hk_aqhi);
    weather_snapshot_store_commit(&store,w,a,f,{},false,false,false,2);
    assert(store.air.hk_aqhi&&!strcmp(store.air.aqi,"10+"));
    w.hko=false;w.open_meteo=true;
    weather_snapshot_store_commit(&store,w,a,f,{},false,false,false,3);
    assert(!store.air.ready);
    if(argc==5) {
        assert(parse_hko_current(read(argv[1]).c_str(),&w));
        assert(parse_hko_forecast(read(argv[2]).c_str(),&forecast));
        assert(parse_hko_warnings(read(argv[3]).c_str(),&alert));
        assert(parse_epd_aqhi_rss(read(argv[4]).c_str(),&air));
    }
}
