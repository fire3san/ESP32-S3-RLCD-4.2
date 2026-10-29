#pragma once
#include "weather_types.h"
#include "weather_update.h"

// A resolved location is accepted only for routing; observations are always HKO's.
WeatherUpdateResult perform_hko_update(WeatherUpdateScope scope,
                                       uint32_t generation);
bool hko_validate_configuration();
