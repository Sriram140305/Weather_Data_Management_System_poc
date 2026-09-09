#ifndef WEATHER_DATA_PROCESSOR_H
#define WEATHER_DATA_PROCESSOR_H

#include <string>

class WeatherDataProcessor
{
public:
    void loadStoredWeatherData();
    void generateWeatherReport();
    void generateUserHistoricalReport(const std::string& username);
};

#endif
