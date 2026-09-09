#include "weatherDataProcessor.h"
#include "userDataManager.h"

#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <algorithm>

using namespace std;

void WeatherDataProcessor::loadStoredWeatherData()
{
    UserDataManager::ensureFile();
}

static void reportFor(const vector<UserDataRecord>& records, const string& title)
{
    double tempSum=0, humSum=0, pressSum=0, maxT=-101, minT=101;
    int count=0;
    for(const auto& r:records)
    {
        if(r.type!="WEATHER" || r.detail!="Current Weather") continue;
        try {
            double t=stod(r.value1), h=stod(r.value2), p=stod(r.value3);
            tempSum+=t; humSum+=h; pressSum+=p; maxT=max(maxT,t); minT=min(minT,t); ++count;
        } catch(...) {}
    }
    cout << "\n========== "<<title<<" ==========\n";
    cout << "Total Weather Records : "<<count<<'\n';
    if(count==0){cout<<"No weather data available.\n";return;}
    cout<<fixed<<setprecision(2);
    cout<<"Average Temperature   : "<<tempSum/count<<" C\n";
    cout<<"Maximum Temperature   : "<<maxT<<" C\n";
    cout<<"Minimum Temperature   : "<<minT<<" C\n";
    cout<<"Average Humidity      : "<<humSum/count<<" %\n";
    cout<<"Average Pressure      : "<<pressSum/count<<" mb\n";
}

void WeatherDataProcessor::generateWeatherReport()
{
    reportFor(UserDataManager::loadAll(), "ADMIN WEATHER REPORT - ALL USERS");
}

void WeatherDataProcessor::generateUserHistoricalReport(const string& username)
{
    vector<UserDataRecord> mine;
    for(const auto& r:UserDataManager::loadAll())
        if(r.username==username && r.type=="WEATHER" && r.detail=="Current Weather") mine.push_back(r);

    cout << "\n========== MY HISTORICAL WEATHER DATA ==========\n";
    if(mine.empty()) { cout << "No historical weather data found for " << username << ".\n"; return; }

    for(const auto& r:mine)
    {
        cout << "Date/Time : " << r.timestamp << '\n';
        cout << "City      : " << r.city << ", " << r.region << ", " << r.country << '\n';
        cout << "Temperature: " << r.value1 << " C\n";
        cout << "Humidity   : " << r.value2 << " %\n";
        cout << "Pressure   : " << r.value3 << " mb\n";
        cout << "-----------------------------------------------\n";
    }

    reportFor(mine, "MY HISTORICAL WEATHER SUMMARY");
}
