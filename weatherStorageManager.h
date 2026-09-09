#ifndef WEATHER_STORAGE_MANAGER_H
#define WEATHER_STORAGE_MANAGER_H

#include <string>

class WeatherStorageManager
{
public:
    void setCurrentUser(const std::string& username);
    void saveWeatherData(const std::string& city, float temp, float humidity, float pressure,
                         const std::string& region = "", const std::string& country = "",
                         double latitude = 0.0, double longitude = 0.0);
    void saveCurrentWeather(const std::string& city, float temp, float humidity, float pressure,
                            const std::string& region = "", const std::string& country = "",
                            double latitude = 0.0, double longitude = 0.0);
    void viewStoredData() const;
    void viewAllUserData() const;
    void backupData() const;
    void clearStorage();
    void clearAllStorage();
private:
    std::string currentUser;
};

#endif
