#include <iostream>
#include <limits>
#include <string>

#include "location.h"
#include "weather.h"
#include "weatherDataProcessor.h"
#include "weatherEnhancement.h"
#include "weatherStorageManager.h"

using namespace std;

static bool readChoice(int& choice)
{
    if (cin >> choice) return true;
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "\nInvalid choice. Please enter a number.\n";
    return false;
}

static void printUserDashboard(const string& username)
{
    cout << "\n=========================================================\n";
    cout << "              WEATHER DATA MANAGEMENT SYSTEM\n";
    cout << "                    USER DASHBOARD\n";
    cout << "=========================================================\n\n";
    cout << "Welcome, " << username << "!\n\n";
    cout << "---------------------------------------------------------\n\n";
    cout << " 1. View Current Weather\n";
    cout << " 2. View 7-Day Forecast\n";
    cout << " 3. Add Favorite City\n";
    cout << " 4. View Favorite Cities\n";
    cout << " 5. Remove Favorite City\n";
    cout << " 6. Recent Searches\n";
    cout << " 7. Most Searched City\n";
    cout << " 8. View Historical Data\n";
    cout << " 9. Weather Comfort Score\n";
    cout << "10. Travel Recommendation\n";
    cout << "11. Compare Cities\n";
    cout << "12. View Weather Alerts\n";
    cout << "13. Weather Insights\n";
    cout << "14. Export Weather Report\n";
    cout << "15. Automatic Weather Refresh\n\n";
    cout << " 0. Logout\n\n";
    cout << "---------------------------------------------------------\n";
}

static void printAdminDashboard(const string& username)
{
    cout << "\n=========================================================\n";
    cout << "              WEATHER DATA MANAGEMENT SYSTEM\n";
    cout << "                    ADMIN DASHBOARD\n";
    cout << "=========================================================\n\n";
    cout << "Welcome, Admin " << username << "!\n\n";
    cout << "---------------------------------------------------------\n\n";
    cout << "1. API Configuration\n";
    cout << "2. Manage Stored Data\n";
    cout << "3. Configure Alert Thresholds\n";
    cout << "4. Generate Reports\n";
    cout << "5. Monitor System Logs\n";
    cout << "6. API Performance Monitoring\n";
    cout << "7. Automatic Weather Refresh\n\n";
    cout << "0. Logout\n\n";
    cout << "---------------------------------------------------------\n";
}

static void adminSaveWeatherData(WeatherStorageManager& storage, const string& adminUsername)
{
    LocationManager lm(adminUsername);
    LocationResult selected;
    if (!lm.selectLocation(selected, "Search Location: ")) return;

    string result = fetchWeather(to_string(selected.latitude) + "," + to_string(selected.longitude));
    if (!isWeatherDataAvailable())
    {
        cout << result << '\n';
        return;
    }

    storage.saveCurrentWeather(selected.name,
                               getLastTemperature(),
                               getLastHumidity(),
                               getLastPressure(),
                               selected.region,
                               selected.country,
                               selected.latitude,
                               selected.longitude);
}

static void manageStoredData(WeatherStorageManager& storage, const string& adminUsername)
{
    while (true)
    {
        cout << "\n========== MANAGE STORED DATA ==========\n";
        cout << "1. Save Current Weather Data\n";
        cout << "2. View All User Data\n";
        cout << "3. Backup Data\n";
        cout << "4. Clear All Weather Storage\n";
        cout << "0. Back to Admin Dashboard\n";
        cout << "\nEnter your choice: ";

        int choice;
        if (!readChoice(choice)) continue;
        if (choice == 0) return;

        switch (choice)
        {
            case 1: adminSaveWeatherData(storage, adminUsername); break;
            case 2: storage.viewAllUserData(); break;
            case 3: storage.backupData(); break;
            case 4: storage.clearAllStorage(); break;
            default: cout << "\nInvalid choice.\n"; break;
        }
    }
}

static void runUserDashboard(LocationManager& lm,
                             WeatherStorageManager& storage,
                             WeatherDataProcessor& processor,
                             WeatherEnhancementManager& enhancement)
{
    lm.setCurrentUser(enhancement.getUsername());
    storage.setCurrentUser(enhancement.getUsername());

    while (true)
    {
        printUserDashboard(enhancement.getUsername());
        cout << "\nEnter your choice: ";
        int choice;
        if (!readChoice(choice)) continue;

        switch (choice)
        {
            case 1: enhancement.showCurrentWeather(); break;
            case 2: enhancement.showSevenDayForecast(); break;
            case 3: lm.addFavoriteCity(); break;
            case 4: lm.viewFavoriteCities(); break;
            case 5: lm.removeFavoriteCity(); break;
            case 6: lm.showRecentSearches(); break;
            case 7: lm.showMostSearchedCity(); break;
            case 8: processor.generateUserHistoricalReport(enhancement.getUsername()); break;
            case 9: lm.weatherComfortScore(); break;
            case 10: lm.travelRecommendation(); break;
            case 11: lm.compareCities(); break;
            case 12: enhancement.viewWeatherAlerts(); break;
            case 13: enhancement.showWeatherInsights(); break;
            case 14: enhancement.exportWeatherReport(); break;
            case 15: enhancement.automaticWeatherRefresh(); break;
            case 0:
                cout << "\nLogging out... Returning to Main Menu.\n";
                return;
            default: cout << "\nInvalid choice.\n"; break;
        }
    }
}

static void runAdminDashboard(WeatherDataProcessor& processor,
                              WeatherStorageManager& storage,
                              WeatherEnhancementManager& enhancement)
{
    storage.setCurrentUser(enhancement.getUsername());

    while (true)
    {
        printAdminDashboard(enhancement.getUsername());
        cout << "\nEnter your choice: ";
        int choice;
        if (!readChoice(choice)) continue;

        switch (choice)
        {
            case 1: enhancement.configureApi(); break;
            case 2: manageStoredData(storage, enhancement.getUsername()); break;
            case 3: enhancement.configureAlertThresholds(); break;
            case 4: processor.generateWeatherReport(); break;
            case 5: enhancement.monitorSystemLogs(); break;
            case 6: enhancement.showApiPerformance(); break;
            case 7: enhancement.automaticWeatherRefresh(); break;
            case 0:
                cout << "\nLogging out... Returning to Main Menu.\n";
                return;
            default: cout << "\nInvalid choice.\n"; break;
        }
    }
}

int main()
{
    LocationManager lm;
    WeatherDataProcessor processor;
    WeatherStorageManager storage;
    WeatherEnhancementManager enhancement;

    while (true)
    {
        if (!enhancement.login())
        {
            cout << "\nExiting Weather Data Management System.\n";
            return 0;
        }

        if (enhancement.getRole() == "ADMIN")
            runAdminDashboard(processor, storage, enhancement);
        else
            runUserDashboard(lm, storage, processor, enhancement);

        // Logout returns here instead of terminating the entire process.
    }
}
