#include "hko_parser.h"
#include "network_json_root.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {
const cJSON *item(const cJSON *o, const char *key) { return cJSON_GetObjectItemCaseSensitive(o,key); }
const char *text(const cJSON *o, const char *key) {
    const cJSON *v=item(o,key);
    return cJSON_IsString(v) && v->valuestring ? v->valuestring : "";
}
template<size_t N> void copy(char (&out)[N], const char *s) {
    size_t n=std::strlen(s);
    if(n>=N) { n=N-1; while(n && (static_cast<unsigned char>(s[n])&0xc0)==0x80)--n; }
    std::memcpy(out,s,n);out[n]='\0';
}
bool numeric(const cJSON *v,double low,double high) {
    return cJSON_IsNumber(v)&&std::isfinite(v->valuedouble)&&v->valuedouble>=low&&v->valuedouble<=high;
}
bool observation(const cJSON *root,const char *key,double low,double high,char *out,size_t size) {
    const cJSON *rows=item(item(root,key),"data");
    for(int i=0;i<cJSON_GetArraySize(rows);++i) {
        const cJSON *row=cJSON_GetArrayItem(rows,i),*v=item(row,"value");
        if(std::strcmp(text(row,"place"),"香港天文台")==0 && numeric(v,low,high)) {
            std::snprintf(out,size,"%.0f",v->valuedouble);return true;
        }
    }
    return false;
}
void condition(int icon,char *out,size_t size,char *code,size_t code_size) {
    const char *label="",*mapped="";
    switch(icon) {
    case 50: label="天晴";mapped="100";break;
    case 51: case 52: label="間有陽光";mapped="101";break;
    case 53: case 54: label="短暫陽光有驟雨";mapped="300";break;
    case 60: label="多雲";mapped="104";break;
    case 61: label="密雲";mapped="104";break;
    case 62: case 63: case 64: label="有雨";mapped="305";break;
    case 65: label="雷暴";mapped="302";break;
    case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: label="天色良好";mapped="150";break;
    case 80: label="大風";mapped="100";break;
    case 81: label="乾燥";mapped="100";break;
    case 82: label="潮濕";mapped="104";break;
    case 83: case 84: case 85: label="有霧";mapped="501";break;
    case 90: label="酷熱";mapped="100";break;
    case 91: label="寒冷";mapped="100";break;
    case 92: label="天氣清涼";mapped="100";break;
    case 93: label="天氣炎熱";mapped="100";break;
    }
    std::snprintf(out,size,"%s",label);std::snprintf(code,code_size,"%s",mapped);
}
}

bool hko_city_matches(const char *city) {
    return city && (!std::strcmp(city,"香港") || !std::strcmp(city,"香港特別行政區") ||
        !std::strcmp(city,"香港特别行政区") || !std::strcmp(city,"Hong Kong") ||
        !std::strcmp(city,"Hong Kong SAR") || !std::strcmp(city,"hong kong"));
}

bool parse_hko_current(const char *json,WeatherData *weather) {
    if(!weather)return false;
    NetworkJsonRoot root(json);
    if(!root || !*text(root.get(),"updateTime"))return false;
    WeatherData next={};next.hko=true;
    if(!observation(root.get(),"temperature",-50,60,next.temp,sizeof(next.temp)) ||
       !observation(root.get(),"humidity",0,100,next.humidity,sizeof(next.humidity)))return false;
    const cJSON *icon=cJSON_GetArrayItem(item(root.get(),"icon"),0);
    if(!numeric(icon,50,93))return false;
    condition(icon->valueint,next.text,sizeof(next.text),next.icon,sizeof(next.icon));
    if(!*next.text)return false;
    copy(next.city,"香港");copy(next.lat,"22.3022");copy(next.lon,"114.1742");
    const cJSON *uv=cJSON_GetArrayItem(item(item(root.get(),"uvindex"),"data"),0);
    const cJSON *value=item(uv,"value");
    if(numeric(value,0,30)) {
        std::snprintf(next.uv,sizeof(next.uv),"%.1f",value->valuedouble);
        copy(next.uv_risk,text(uv,"desc"));
    }
    *weather=next;return true;
}

bool parse_hko_forecast(const char *json,WeatherForecastData *forecast) {
    if(!forecast)return false;
    NetworkJsonRoot root(json);
    const cJSON *days=root?item(root.get(),"weatherForecast"):nullptr;
    if(!cJSON_IsArray(days)||cJSON_GetArraySize(days)==0)return false;
    WeatherForecastData next={};
    for(int i=0;i<cJSON_GetArraySize(days)&&i<kWeatherForecastDays;++i) {
        const cJSON *row=cJSON_GetArrayItem(days,i);
        const char *date=text(row,"forecastDate");
        const cJSON *hi=item(item(row,"forecastMaxtemp"),"value"),*lo=item(item(row,"forecastMintemp"),"value"),*icon=item(row,"ForecastIcon");
        if(std::strlen(date)!=8 || std::strspn(date,"0123456789")!=8 || !numeric(hi,-50,60)||!numeric(lo,-50,60)||hi->valuedouble<lo->valuedouble||!numeric(icon,50,93))return false;
        auto &day=next.days[next.count++];day.valid=true;
        std::snprintf(day.date,sizeof(day.date),"%.4s-%.2s-%.2s",date,date+4,date+6);
        condition(icon->valueint,day.text,sizeof(day.text),day.icon,sizeof(day.icon));
        std::snprintf(day.temp_max,sizeof(day.temp_max),"%.0f",hi->valuedouble);
        std::snprintf(day.temp_min,sizeof(day.temp_min),"%.0f",lo->valuedouble);
    }
    next.ready=true;next.updated_at=time(nullptr);*forecast=next;return true;
}

bool parse_hko_warnings(const char *json,WeatherAlertData *alert) {
    if(!alert)return false;
    NetworkJsonRoot root(json);
    if(!root||!cJSON_IsObject(root.get()))return false;
    WeatherAlertData next={};
    for(const cJSON *row=root.get()->child;row;row=row->next) {
        if(!cJSON_IsObject(row)||!*text(row,"name")||!*text(row,"actionCode"))return false;
        if(!std::strcmp(text(row,"actionCode"),"CANCEL"))continue;
        if(next.count<kMaxWeatherAlerts) {
            copy(next.titles[next.count],text(row,"name"));
            next.ranks[next.count++]=1;
        }
    }
    next.active=next.count>0;next.updated_at=time(nullptr);*alert=next;return true;
}

bool parse_epd_aqhi_rss(const char *xml,WeatherAirData *air) {
    if(!xml||!air||!std::strstr(xml,"<rss")||!std::strstr(xml,"</channel>")||!std::strstr(xml,"</rss>"))return false;
    const std::string feed(xml);size_t pos=0;int maximum=0;std::string station;
    while((pos=feed.find("<item>",pos))!=std::string::npos) {
        const size_t end=feed.find("</item>",pos);
        if(end==std::string::npos)return false;
        const std::string row=feed.substr(pos,end-pos);pos=end+7;
        const size_t general=row.find("一般監測站:");
        if(general==std::string::npos || row.find("路邊監測站")!=std::string::npos)continue;
        const char *start=row.c_str()+general+std::strlen("一般監測站:");char *tail=nullptr;
        long value=std::strtol(start,&tail,10);
        if(tail==start||value<1||value>10)continue;
        if(*tail=='+') { if(value!=10)continue;value=11;++tail; }
        if(*tail!=' '&&*tail!='\t')continue;
        const size_t a=row.find("<title>"),b=row.find("</title>");
        if(a==std::string::npos||b==std::string::npos||b<=a+7)continue;
        if(value>maximum) { maximum=static_cast<int>(value);station=row.substr(a+7,b-a-7); }
    }
    if(!maximum)return false;
    WeatherAirData next={};next.hk_aqhi=true;next.ready=true;
    if(maximum==11)copy(next.aqi,"10+");else std::snprintf(next.aqi,sizeof(next.aqi),"%d",maximum);
    copy(next.category,maximum<=3?"低":maximum<=6?"中":maximum==7?"高":maximum<=10?"甚高":"嚴重");
    copy(next.station,station.c_str());next.updated_at=time(nullptr);*air=next;return true;
}
