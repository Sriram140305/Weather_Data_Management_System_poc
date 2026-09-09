#include "weatherStorageManager.h"
#include "userDataManager.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <vector>

using namespace std;

namespace
{
    string nowString()
    {
        time_t now=time(nullptr); tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime,&now);
#else
        localtime_r(&now,&localTime);
#endif
        char b[32]{}; strftime(b,sizeof(b),"%Y-%m-%d %H:%M:%S",&localTime); return b;
    }
}

void WeatherStorageManager::setCurrentUser(const string& username) { currentUser=username; }

void WeatherStorageManager::saveWeatherData(const string& city,float temp,float humidity,float pressure,
                                             const string& region,const string& country,double latitude,double longitude)
{
    if (currentUser.empty()) { cout << "\nNo active user. Weather data was not saved.\n"; return; }
    UserDataRecord r;
    r.type="WEATHER"; r.username=currentUser; r.city=city; r.region=region; r.country=country;
    r.latitude=latitude; r.longitude=longitude; r.timestamp=nowString(); r.detail="Current Weather";
    r.value1=to_string(temp); r.value2=to_string(humidity); r.value3=to_string(pressure);
    if(UserDataManager::append(r)) cout << "\nWeather data saved for user: " << currentUser << "\n";
    else cout << "\nUnable to save weather data.\n";
}

void WeatherStorageManager::saveCurrentWeather(const string& city,float temp,float humidity,float pressure,
                                                const string& region,const string& country,double latitude,double longitude)
{
    saveWeatherData(city,temp,humidity,pressure,region,country,latitude,longitude);
}

void WeatherStorageManager::viewStoredData() const
{
    UserDataManager::viewForUser(currentUser);
}

void WeatherStorageManager::viewAllUserData() const
{
    UserDataManager::viewAllForAdmin();
}

void WeatherStorageManager::backupData() const
{
    ifstream source("user_data.csv",ios::binary);
    ofstream backup("user_data_backup.csv",ios::binary|ios::trunc);
    if(!source.is_open()||!backup.is_open()){cout<<"\nUnable to create backup.\n";return;}
    backup<<source.rdbuf();
    cout<<"\nBackup created successfully: user_data_backup.csv\n";
}

void WeatherStorageManager::clearStorage()
{
    if(currentUser.empty()){cout<<"\nNo active user.\n";return;}
    vector<UserDataRecord> all=UserDataManager::loadAll();
    all.erase(remove_if(all.begin(),all.end(),[&](const UserDataRecord& r){return r.username==currentUser&&r.type=="WEATHER";}),all.end());
    if(UserDataManager::rewrite(all)) cout<<"\nWeather storage cleared for user: "<<currentUser<<"\n";
    else cout<<"\nUnable to clear storage.\n";
}

void WeatherStorageManager::clearAllStorage()
{
    ifstream source("user_data.csv");
    if (!source.is_open()) { cout << "\nNo stored user data found.\n"; return; }
    vector<UserDataRecord> all = UserDataManager::loadAll();
    all.erase(remove_if(all.begin(), all.end(), [](const UserDataRecord& r){ return r.type == "WEATHER"; }), all.end());
    if (UserDataManager::rewrite(all)) cout << "\nAll users' weather storage cleared successfully.\n";
    else cout << "\nUnable to clear weather storage.\n";
}
