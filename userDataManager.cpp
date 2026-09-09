#include "userDataManager.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

namespace
{
    const string FILE_NAME = "user_data.csv";

    string nowString()
    {
        time_t now = time(nullptr);
        tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        char buffer[32]{};
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
        return buffer;
    }

    bool sameCity(const UserDataRecord& a, const string& city)
    {
        string x = a.city;
        string y = city;
        transform(x.begin(), x.end(), x.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        transform(y.begin(), y.end(), y.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        return x == y;
    }
}

string UserDataManager::sanitize(const string& value)
{
    string result = value;
    for (char& c : result)
    {
        if (c == '\r' || c == '\n' || c == ',') c = ' ';
    }
    return result;
}

string UserDataManager::csvEscape(const string& value)
{
    string out = value;
    bool needsQuotes = false;
    for (char& c : out)
    {
        if (c == '"') { c = '\''; needsQuotes = true; }
        if (c == ',' || c == '\r' || c == '\n') { c = ' '; needsQuotes = true; }
    }
    return needsQuotes ? '"' + out + '"' : out;
}

vector<string> UserDataManager::splitCsv(const string& line)
{
    vector<string> fields;
    string current;
    bool quoted = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (c == '"')
        {
            quoted = !quoted;
        }
        else if (c == ',' && !quoted)
        {
            fields.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

void UserDataManager::ensureFile()
{
    ifstream in(FILE_NAME);
    if (in.good()) return;

    ofstream out(FILE_NAME);
    if (out.is_open())
    {
        out << "Type,Username,City,Region,Country,Latitude,Longitude,DateTime,Detail,Value1,Value2,Value3\n";
    }
}

vector<UserDataRecord> UserDataManager::loadAll()
{
    ensureFile();
    ifstream file(FILE_NAME);
    vector<UserDataRecord> records;
    string line;
    bool first = true;

    while (getline(file, line))
    {
        if (line.empty()) continue;
        if (first)
        {
            first = false;
            if (line.rfind("Type,Username,City", 0) == 0) continue;
        }

        vector<string> f = splitCsv(line);
        if (f.size() < 12) continue;

        UserDataRecord r;
        r.type = f[0]; r.username = f[1]; r.city = f[2]; r.region = f[3];
        r.country = f[4]; r.timestamp = f[7]; r.detail = f[8];
        r.value1 = f[9]; r.value2 = f[10]; r.value3 = f[11];
        try { r.latitude = stod(f[5]); } catch (...) {}
        try { r.longitude = stod(f[6]); } catch (...) {}
        records.push_back(r);
    }
    return records;
}

bool UserDataManager::append(const UserDataRecord& record)
{
    ensureFile();
    ofstream file(FILE_NAME, ios::app);
    if (!file.is_open()) return false;

    UserDataRecord r = record;
    if (r.timestamp.empty()) r.timestamp = nowString();

    file << csvEscape(r.type) << ',' << csvEscape(r.username) << ',' << csvEscape(r.city) << ','
         << csvEscape(r.region) << ',' << csvEscape(r.country) << ','
         << fixed << setprecision(6) << r.latitude << ',' << r.longitude << ','
         << csvEscape(r.timestamp) << ',' << csvEscape(r.detail) << ','
         << csvEscape(r.value1) << ',' << csvEscape(r.value2) << ',' << csvEscape(r.value3) << '\n';
    return true;
}

bool UserDataManager::rewrite(const vector<UserDataRecord>& records)
{
    ofstream file(FILE_NAME, ios::trunc);
    if (!file.is_open()) return false;

    file << "Type,Username,City,Region,Country,Latitude,Longitude,DateTime,Detail,Value1,Value2,Value3\n";
    for (const UserDataRecord& r : records)
    {
        file << csvEscape(r.type) << ',' << csvEscape(r.username) << ',' << csvEscape(r.city) << ','
             << csvEscape(r.region) << ',' << csvEscape(r.country) << ','
             << fixed << setprecision(6) << r.latitude << ',' << r.longitude << ','
             << csvEscape(r.timestamp) << ',' << csvEscape(r.detail) << ','
             << csvEscape(r.value1) << ',' << csvEscape(r.value2) << ',' << csvEscape(r.value3) << '\n';
    }
    return true;
}

vector<UserDataRecord> UserDataManager::favorites(const string& username)
{
    vector<UserDataRecord> result;
    for (const auto& r : loadAll())
        if (r.type == "FAVORITE" && r.username == username) result.push_back(r);
    return result;
}

vector<UserDataRecord> UserDataManager::recentSearches(const string& username)
{
    vector<UserDataRecord> result;
    for (const auto& r : loadAll())
        if (r.type == "SEARCH" && r.username == username) result.push_back(r);

    if (result.size() > MAX_RECENT_SEARCHES)
        result.erase(result.begin(), result.end() - MAX_RECENT_SEARCHES);
    return result;
}

int UserDataManager::searchCount(const string& username, const string& city)
{
    int count = 0;
    for (const auto& r : loadAll())
        if (r.type == "SEARCH" && r.username == username && sameCity(r, city)) ++count;
    return count;
}

bool UserDataManager::addFavorite(const string& username, const UserDataRecord& record)
{
    vector<UserDataRecord> all = loadAll();
    vector<UserDataRecord> fav;
    for (const auto& r : all)
        if (r.type == "FAVORITE" && r.username == username) fav.push_back(r);

    if (fav.size() >= MAX_FAVORITES) return false;
    for (const auto& r : fav)
        if (sameCity(r, record.city)) return false;

    UserDataRecord copy = record;
    copy.type = "FAVORITE";
    copy.username = username;
    if (copy.timestamp.empty()) copy.timestamp = nowString();
    return append(copy);
}

bool UserDataManager::removeFavorite(const string& username, const string& city)
{
    vector<UserDataRecord> all = loadAll();
    size_t oldSize = all.size();
    all.erase(remove_if(all.begin(), all.end(), [&](const UserDataRecord& r){
        return r.type == "FAVORITE" && r.username == username && sameCity(r, city);
    }), all.end());
    if (all.size() == oldSize) return false;
    return rewrite(all);
}

bool UserDataManager::addSearch(const string& username, const UserDataRecord& record)
{
    vector<UserDataRecord> all = loadAll();
    UserDataRecord copy = record;
    copy.type = "SEARCH";
    copy.username = username;
    if (copy.timestamp.empty()) copy.timestamp = nowString();

    all.push_back(copy);

    vector<size_t> indexes;
    for (size_t i = 0; i < all.size(); ++i)
        if (all[i].type == "SEARCH" && all[i].username == username) indexes.push_back(i);

    while (indexes.size() > MAX_RECENT_SEARCHES)
    {
        size_t removeIndex = indexes.front();
        all.erase(all.begin() + static_cast<long>(removeIndex));
        indexes.clear();
        for (size_t i = 0; i < all.size(); ++i)
            if (all[i].type == "SEARCH" && all[i].username == username) indexes.push_back(i);
    }

    return rewrite(all);
}

void UserDataManager::viewAllForAdmin()
{
    vector<UserDataRecord> records = loadAll();

    cout << "\n============================================================== ALL USER STORED DATA ===============================================================\n";
    if (records.empty())
    {
        cout << "No user data available.\n";
        return;
    }

    // Display stored records as a readable table. For WEATHER records,
    // Value1/Value2/Value3 are shown with their actual metric names.
    cout << left
         << setw(10) << "Type"
         << setw(14) << "Username"
         << setw(18) << "City"
         << setw(16) << "Region"
         << setw(14) << "Country"
         << setw(11) << "Latitude"
         << setw(12) << "Longitude"
         << setw(20) << "Date/Time"
         << setw(18) << "Detail"
         << setw(16) << "Temperature"
         << setw(13) << "Humidity"
         << setw(14) << "Pressure"
         << '\n';

    cout << string(176, '-') << '\n';

    cout << fixed << setprecision(2);
    for (const auto& r : records)
    {
        cout << left
             << setw(10) << r.type
             << setw(14) << r.username
             << setw(18) << r.city
             << setw(16) << r.region
             << setw(14) << r.country
             << setw(11) << r.latitude
             << setw(12) << r.longitude
             << setw(20) << r.timestamp
             << setw(18) << r.detail;

        if (r.type == "WEATHER")
        {
            cout << setw(16) << (r.value1.empty() ? "-" : r.value1)
                 << setw(13) << (r.value2.empty() ? "-" : r.value2)
                 << setw(14) << (r.value3.empty() ? "-" : r.value3);
        }
        else
        {
            cout << setw(16) << "-"
                 << setw(13) << "-"
                 << setw(14) << "-";
        }
        cout << '\n';
    }

    cout << string(176, '-') << '\n';
    cout << "Temperature: °C   Humidity: %   Pressure: hPa\n";
}

void UserDataManager::viewForUser(const string& username)
{
    vector<UserDataRecord> records = loadAll();
    cout << "\n========== MY WEATHER DATA ==========" << '\n';
    bool found = false;
    for (const auto& r : records)
    {
        if (r.username != username) continue;
        if (r.type != "WEATHER") continue;
        found = true;
        cout << "City       : " << r.city << '\n';
        cout << "Date/Time  : " << r.timestamp << '\n';
        cout << r.detail << '\n';
        cout << "Value 1    : " << r.value1 << '\n';
        cout << "Value 2    : " << r.value2 << '\n';
        cout << "Value 3    : " << r.value3 << '\n';
        cout << "-------------------------------------\n";
    }
    if (!found) cout << "No historical weather data found for " << username << ".\n";
}
