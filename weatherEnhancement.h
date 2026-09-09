#ifndef WEATHER_ENHANCEMENT_H
#define WEATHER_ENHANCEMENT_H

#include <string>
using namespace std;

class WeatherEnhancementManager
{
public:
    // Authentication
    bool login();
    bool loginAsUser();
    bool loginAsAdmin();
    bool registerUser();

    string getRole() const;
    string getUsername() const;

    // Admin-only account management
    void userManagement();

    // User dashboard features
    void showCurrentWeather();
    void showSevenDayForecast();
    void viewWeatherAlerts();
    void showWeatherInsights();
    void exportWeatherReport();

    // Existing weather intelligence / admin features
    void showWeatherIntelligence();
    void showApiPerformance();
    void testApiWithRetry();
    void automaticWeatherRefresh();

    // Admin dashboard
    void configureApi();
    void configureAlertThresholds();
    void monitorSystemLogs();

private:
    string currentUsername;
    string currentRole;
};

#endif
