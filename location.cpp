#include "location.h"
#include "APIManager.h"
#include "userDataManager.h"
#include "weather.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>

using namespace std;
using json = nlohmann::json;

namespace
{
    template <typename T>
    T jsonValue(const json& object, const string& key, const T& fallback)
    {
        if (!object.is_object()) return fallback;
        auto it = object.find(key);
        if (it == object.end() || it->is_null()) return fallback;
        try { return it->get<T>(); } catch (...) { return fallback; }
    }

    string lower(string value)
    {
        transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        return value;
    }

    string coordQuery(const LocationResult& r)
    {
        ostringstream q;
        q << fixed << setprecision(6) << r.latitude << ',' << r.longitude;
        return q.str();
    }

    void printLocation(const LocationResult& r)
    {
        cout << "Selected Location: " << r.name;
        if (!r.region.empty()) cout << ", " << r.region;
        if (!r.country.empty()) cout << ", " << r.country;
        cout << "\nLatitude : " << r.latitude << "\nLongitude: " << r.longitude << "\n";
    }
}

LocationManager::LocationManager(const string& username) : currentUsername(username) {}

void LocationManager::setCurrentUser(const string& username)
{
    currentUsername = username;
}

void LocationManager::loadFavoriteCities() { UserDataManager::ensureFile(); }
void LocationManager::loadSearchHistory() { UserDataManager::ensureFile(); }

bool LocationManager::selectLocation(LocationResult& selected, const string& prompt, bool recordSearch)
{
    string query;
    cout << "\n" << prompt;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, query);

    const size_t first = query.find_first_not_of(" \t");
    if (first == string::npos) { cout << "\nSearch text cannot be empty.\n"; return false; }
    const size_t last = query.find_last_not_of(" \t");
    query = query.substr(first, last - first + 1);

    APIManager api;
    string response = api.searchLocations(query);
    if (response.empty()) { cout << "\nUnable to retrieve location search results.\n"; return false; }

    vector<LocationResult> locations;
    try
    {
        json data = json::parse(response);
        if (!data.is_array()) throw runtime_error("not array");
        for (const auto& item : data)
        {
            LocationResult r;
            r.name = jsonValue<string>(item, "name", "");
            r.region = jsonValue<string>(item, "region", "");
            r.country = jsonValue<string>(item, "country", "");
            r.latitude = jsonValue<double>(item, "lat", 0.0);
            r.longitude = jsonValue<double>(item, "lon", 0.0);
            if (!r.name.empty()) locations.push_back(r);
        }
    }
    catch (...) { cout << "\nInvalid location search response.\n"; return false; }

    sort(locations.begin(), locations.end(), [&](const LocationResult& a, const LocationResult& b){
        const string q = lower(query);
        auto score = [&](const LocationResult& r) {
            int s = 0;
            string n = lower(r.name), region = lower(r.region), country = lower(r.country);
            if (n == q) s -= 100;
            else if (n.rfind(q, 0) == 0) s -= 50;
            if (country == "india") s -= 20;
            if (region == "tamil nadu") s -= 30;
            return s;
        };
        return score(a) < score(b);
    });

    if (locations.empty())
    {
        cout << "\nNo match found.\n";
        return false;
    }

    if (locations.size() > 10) locations.resize(10);
    cout << "\n========== LOCATION SEARCH RESULTS ==========\n";
    for (size_t i = 0; i < locations.size(); ++i)
    {
        cout << i + 1 << ". " << locations[i].name;
        if (!locations[i].region.empty()) cout << ", " << locations[i].region;
        if (!locations[i].country.empty()) cout << ", " << locations[i].country;
        cout << "\n   Latitude : " << locations[i].latitude
             << "\n   Longitude: " << locations[i].longitude << "\n";
    }
    cout << "0. Cancel\n\nSelect Location: ";

    int selection;
    if (!(cin >> selection))
    {
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\nInvalid selection.\n"; return false;
    }
    if (selection == 0) return false;
    if (selection < 1 || static_cast<size_t>(selection) > locations.size())
    { cout << "\nInvalid location selection.\n"; return false; }

    selected = locations[static_cast<size_t>(selection - 1)];
    cout << '\n'; printLocation(selected);

    if (recordSearch && !currentUsername.empty())
    {
        UserDataRecord r;
        r.city = selected.name; r.region = selected.region; r.country = selected.country;
        r.latitude = selected.latitude; r.longitude = selected.longitude;
        if (!UserDataManager::addSearch(currentUsername, r))
            cout << "Warning: unable to update recent searches.\n";
    }
    return true;
}

void LocationManager::searchLocation()
{
    LocationResult selected;
    selectLocation(selected, "Search Location: ");
}

void LocationManager::addFavoriteCity()
{
    LocationResult selected;
    if (!selectLocation(selected, "Search Location: ", false)) return;

    UserDataRecord r;
    r.city = selected.name; r.region = selected.region; r.country = selected.country;
    r.latitude = selected.latitude; r.longitude = selected.longitude;
    if (UserDataManager::addFavorite(currentUsername, r))
        cout << "\nFavorite City Added Successfully: " << selected.name << '\n';
    else
    {
        vector<UserDataRecord> fav = UserDataManager::favorites(currentUsername);
        if (fav.size() >= UserDataManager::MAX_FAVORITES)
            cout << "\nFavorite city limit reached. Maximum 5 cities per user.\n";
        else
            cout << "\nCity is already in your favorites.\n";
    }
}

void LocationManager::viewFavoriteCities()
{
    vector<UserDataRecord> fav = UserDataManager::favorites(currentUsername);
    cout << "\n========== MY FAVORITE CITIES ==========" << '\n';
    if (fav.empty()) { cout << "No favorite cities found.\n"; return; }

    for (size_t i = 0; i < fav.size(); ++i)
    {
        cout << i + 1 << ". " << fav[i].city;
        if (!fav[i].region.empty()) cout << ", " << fav[i].region;
        if (!fav[i].country.empty()) cout << ", " << fav[i].country;
        cout << '\n';
    }
    cout << "Total: " << fav.size() << "/" << UserDataManager::MAX_FAVORITES << '\n';

    cout << "\nEnter favorite city number to check weather (0 = Back): ";
    int choice;
    if (!(cin >> choice))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid choice.\n";
        return;
    }
    if (choice == 0) return;
    if (choice < 1 || static_cast<size_t>(choice) > fav.size())
    {
        cout << "Invalid favorite city number.\n";
        return;
    }

    const UserDataRecord& selected = fav[static_cast<size_t>(choice - 1)];
    LocationResult location;
    location.name = selected.city;
    location.region = selected.region;
    location.country = selected.country;
    location.latitude = selected.latitude;
    location.longitude = selected.longitude;

    cout << "\nChecking weather for: " << location.name << "\n";
    string result = fetchWeather(coordQuery(location));
    if (!isWeatherDataAvailable())
    {
        cout << result << '\n';
        return;
    }

    cout << "\n========== CURRENT WEATHER ==========" << '\n';
    cout << "Location       : " << location.name << '\n';
    cout << "Region         : " << location.region << '\n';
    cout << "Country        : " << location.country << '\n';
    cout << "Temperature    : " << getLastTemperature() << " C\n";
    cout << "Humidity       : " << getLastHumidity() << " %\n";
    cout << "Wind Speed     : " << getLastWindSpeed() << " km/h\n";
    cout << "Pressure       : " << getLastPressure() << " mb\n";
    cout << "Condition      : " << getLastCondition() << '\n';
}

void LocationManager::removeFavoriteCity()
{
    vector<UserDataRecord> fav = UserDataManager::favorites(currentUsername);
    if (fav.empty()) { cout << "\nNo favorite cities found.\n"; return; }
    cout << "\n========== REMOVE FAVORITE CITY ==========" << '\n';
    for (size_t i = 0; i < fav.size(); ++i) cout << i + 1 << ". " << fav[i].city << '\n';
    cout << "0. Cancel\nSelect City: ";
    int choice;
    if (!(cin >> choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid choice.\n"; return; }
    if (choice == 0) return;
    if (choice < 1 || static_cast<size_t>(choice) > fav.size()) { cout << "Invalid choice.\n"; return; }
    if (UserDataManager::removeFavorite(currentUsername, fav[static_cast<size_t>(choice - 1)].city))
        cout << "Favorite City Removed Successfully.\n";
    else cout << "Unable to remove favorite city.\n";
}

void LocationManager::showRecentSearches()
{
    vector<UserDataRecord> searches = UserDataManager::recentSearches(currentUsername);
    cout << "\n========== MY RECENT SEARCHES ==========" << '\n';
    if (searches.empty()) { cout << "No recent searches found.\n"; return; }
    for (auto it = searches.rbegin(); it != searches.rend(); ++it)
        cout << it->city << ", " << it->region << ", " << it->country << " | " << it->timestamp << '\n';
    cout << "Stored recent searches: " << searches.size() << "/" << UserDataManager::MAX_RECENT_SEARCHES << '\n';
}

void LocationManager::showMostSearchedCity()
{
    vector<UserDataRecord> searches = UserDataManager::recentSearches(currentUsername);
    map<string, pair<string,int>> counts;
    for (const auto& r : searches)
    {
        string key = lower(r.city);
        counts[key].first = r.city;
        counts[key].second++;
    }
    cout << "\n========== MY MOST SEARCHED CITY ==========" << '\n';
    if (counts.empty()) { cout << "No search history available.\n"; return; }
    auto best = max_element(counts.begin(), counts.end(), [](const auto& a, const auto& b){ return a.second.second < b.second.second; });
    cout << "City         : " << best->second.first << '\n';
    cout << "Search Count : " << best->second.second << '\n';
}

void LocationManager::findHottestCity()
{
    vector<UserDataRecord> records = UserDataManager::loadAll();
    string hottestCity; double hottest = -101; string owner;
    for (const auto& r : records)
    {
        if (r.type != "WEATHER" || r.detail != "Current Weather") continue;
        double temp;
        try { temp = stod(r.value1); } catch (...) { continue; }
        if (temp > hottest) { hottest = temp; hottestCity = r.city; owner = r.username; }
    }
    cout << "\n========== HOTTEST CITY ==========" << '\n';
    if (hottestCity.empty()) { cout << "No saved weather data available.\n"; return; }
    cout << "City        : " << hottestCity << '\n';
    cout << "Temperature : " << hottest << " C\n";
    cout << "User        : " << owner << '\n';
}

void LocationManager::weatherComfortScore()
{
    LocationResult selected; if (!selectLocation(selected, "Search Location: ")) return;
    string result = fetchWeather(coordQuery(selected));
    if (!isWeatherDataAvailable()) { cout << result << '\n'; return; }
    float t=getLastTemperature(), h=getLastHumidity(); string c=lower(getLastCondition());
    int ts=(t>=20&&t<=28)?40:(t>=15&&t<=32)?30:(t>=10&&t<=35)?20:10;
    int hs=(h>=40&&h<=60)?30:(h>=30&&h<=70)?20:(h>=20&&h<=80)?10:5;
    int cs=(c.find("rain")!=string::npos||c.find("storm")!=string::npos||c.find("snow")!=string::npos)?10:(c.find("cloud")!=string::npos?20:30);
    int score=min(100,ts+hs+cs);
    cout << "\n========== WEATHER COMFORT SCORE ==========\n";
    cout << "City        : " << selected.name << "\nTemperature : " << t << " C\nHumidity    : " << h << "%\nCondition   : " << getLastCondition() << "\nScore       : " << score << "/100\n";
    cout << (score>=90?"Excellent Weather\n":score>=75?"Good Weather\n":score>=50?"Moderate Weather\n":"Poor Weather\n");
}

void LocationManager::travelRecommendation()
{
    LocationResult selected; if (!selectLocation(selected, "Search Location: ")) return;
    string result=fetchWeather(coordQuery(selected)); if(!isWeatherDataAvailable()){cout<<result<<'\n';return;}
    float t=getLastTemperature(),h=getLastHumidity(),w=getLastWindSpeed(); string c=lower(getLastCondition());
    bool bad=c.find("rain")!=string::npos||c.find("storm")!=string::npos||c.find("snow")!=string::npos;
    cout << "\n========== TRAVEL RECOMMENDATION ==========\nCity        : "<<selected.name<<"\nTemperature : "<<t<<" C\nHumidity    : "<<h<<"%\nWind Speed  : "<<w<<" km/h\nCondition   : "<<getLastCondition()<<'\n';
    cout << (bad||t>38||t<12||w>40?"Recommendation: Travel conditions are not ideal. Use caution.\n":"Recommendation: Current weather looks suitable for travel.\n");
}

void LocationManager::compareCities()
{
    LocationResult a,b;
    if(!selectLocation(a,"Search First Location: ")) return;
    if(!selectLocation(b,"Search Second Location: ")) return;
    string r=fetchWeather(coordQuery(a)); if(!isWeatherDataAvailable()){cout<<r<<'\n';return;}
    float t1=getLastTemperature(),h1=getLastHumidity(),w1=getLastWindSpeed(); string c1=lower(getLastCondition());
    r=fetchWeather(coordQuery(b)); if(!isWeatherDataAvailable()){cout<<r<<'\n';return;}
    float t2=getLastTemperature(),h2=getLastHumidity(),w2=getLastWindSpeed(); string c2=lower(getLastCondition());
    auto score=[](float t,float h,float w,const string& c){int s=0;if(t>=20&&t<=30)s+=2;else if(t>=15&&t<=35)s++;if(h>=40&&h<=60)s+=2;else if(h>=30&&h<=70)s++;if(w<=20)s+=2;else if(w<=35)s++;if(c.find("rain")==string::npos&&c.find("storm")==string::npos&&c.find("snow")==string::npos)s+=2;return s;};
    int s1=score(t1,h1,w1,c1),s2=score(t2,h2,w2,c2);
    cout<<"\n========== CITY COMPARISON ==========\n"<<a.name<<" Weather Score : "<<s1<<"/8\n"<<b.name<<" Weather Score : "<<s2<<"/8\n";
    if(s1>s2)cout<<"Better Weather: "<<a.name<<'\n';else if(s2>s1)cout<<"Better Weather: "<<b.name<<'\n';else cout<<"Both cities have similar overall weather conditions.\n";
}
