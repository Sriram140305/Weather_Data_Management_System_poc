#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "weatherEnhancement.h"
#include "APIManager.h"
#include "weather.h"
#include "location.h"
#include "userDataManager.h"
#include "weatherStorageManager.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using json = nlohmann::json;

namespace
{
    mutex logMutex;
    mutex storageMutex;

    struct WeatherSnapshot
    {
        string location;
        string region;
        string country;
        double latitude = 0.0;
        double longitude = 0.0;

        double temperature = 0.0;
        double feelsLike = 0.0;
        double humidity = 0.0;
        double windKph = 0.0;
        double pressure = 0.0;
        double precipitation = 0.0;
        double uv = 0.0;
        string condition;

        string sunrise;
        string sunset;

        json forecast;
        json alerts;
    };

    string nowString()
    {
        time_t now = time(nullptr);
        tm localTime{};
#if defined(_WIN32)
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
        return buffer;
    }

    void logLine(const string& fileName, const string& message)
    {
        lock_guard<mutex> lock(logMutex);
        ofstream file(fileName, ios::app);
        if (file.is_open())
        {
            file << nowString() << " | " << message << '\n';
        }
    }

    size_t writeCallback(void* contents, size_t size, size_t nmemb, string* output)
    {
        size_t total = size * nmemb;
        if (output != nullptr)
        {
            output->append(static_cast<char*>(contents), total);
        }
        return total;
    }

    string urlEncode(const string& value)
    {
        const char* hex = "0123456789ABCDEF";
        string result;
        for (unsigned char c : value)
        {
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~')
            {
                result += static_cast<char>(c);
            }
            else
            {
                result += '%';
                result += hex[(c >> 4) & 0x0F];
                result += hex[c & 0x0F];
            }
        }
        return result;
    }

    string buildForecastUrl(const string& query)
    {
        APIManager api;
        return "https://api.weatherapi.com/v1/forecast.json?key=" +
               api.getApiKey() +
               "&q=" + urlEncode(query) +
               "&days=7&aqi=no&alerts=yes";
    }

    bool requestJsonWithRetry(const string& url, string& response,
                              long& statusCode, double& elapsedMs,
                              int maxAttempts = 3)
    {
        response.clear();
        statusCode = 0;
        elapsedMs = 0.0;

        CURL* curl = curl_easy_init();
        if (curl == nullptr)
        {
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        bool success = false;

        for (int attempt = 1; attempt <= maxAttempts; ++attempt)
        {
            response.clear();
            auto start = chrono::steady_clock::now();
            CURLcode result = curl_easy_perform(curl);
            auto end = chrono::steady_clock::now();

            elapsedMs = chrono::duration<double, milli>(end - start).count();
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

            logLine("api_performance.log",
                    "attempt=" + to_string(attempt) +
                    " status=" + to_string(statusCode) +
                    " response_time_ms=" + to_string(elapsedMs));

            if (result == CURLE_OK && statusCode == 200)
            {
                success = true;
                break;
            }

            logLine("error.log",
                    "API retry attempt=" + to_string(attempt) +
                    " curl=" + string(curl_easy_strerror(result)) +
                    " status=" + to_string(statusCode));

            if (attempt < maxAttempts)
            {
                // Requirement document specifies a 5-second retry interval.
                this_thread::sleep_for(chrono::seconds(5));
            }
        }

        curl_easy_cleanup(curl);
        return success;
    }

    // Compatibility helpers for nlohmann-json 2.1.1.
    bool hasJsonKey(const json& object, const string& key)
    {
        return object.is_object() && object.find(key) != object.end();
    }

    template <typename T>
    T jsonValue(const json& object, const string& key, const T& fallback)
    {
        if (!object.is_object())
        {
            return fallback;
        }

        json::const_iterator it = object.find(key);

        if (it == object.end() || it->is_null())
        {
            return fallback;
        }

        try
        {
            return it->get<T>();
        }
        catch (...)
        {
            return fallback;
        }
    }

    bool parseSnapshot(const json& data, WeatherSnapshot& w)
    {
        try
        {
            w.location = jsonValue<string>(data["location"], "name", "");
            w.region = jsonValue<string>(data["location"], "region", "");
            w.country = jsonValue<string>(data["location"], "country", "");
            w.latitude = jsonValue<double>(data["location"], "lat", 0.0);
            w.longitude = jsonValue<double>(data["location"], "lon", 0.0);

            const auto& current = data["current"];
            w.temperature = jsonValue<double>(current, "temp_c", 0.0);
            w.feelsLike = jsonValue<double>(current, "feelslike_c", 0.0);
            w.humidity = jsonValue<double>(current, "humidity", 0.0);
            w.windKph = jsonValue<double>(current, "wind_kph", 0.0);
            w.pressure = jsonValue<double>(current, "pressure_mb", 0.0);
            w.precipitation = jsonValue<double>(current, "precip_mm", 0.0);
            w.uv = jsonValue<double>(current, "uv", 0.0);
            w.condition = jsonValue<string>(current["condition"], "text", "Unknown");

            w.forecast = hasJsonKey(data, "forecast") ? data["forecast"] : json();
            w.alerts = hasJsonKey(data, "alerts") ? data["alerts"] : json();

            if (hasJsonKey(w.forecast, "forecastday") &&
                w.forecast["forecastday"].is_array() &&
                !w.forecast["forecastday"].empty())
            {
                const auto& firstDay = w.forecast["forecastday"][0];
                if (hasJsonKey(firstDay, "astro"))
                {
                    w.sunrise = jsonValue<string>(firstDay["astro"], "sunrise", "Unknown");
                    w.sunset = jsonValue<string>(firstDay["astro"], "sunset", "Unknown");
                }
            }

            return !w.location.empty();
        }
        catch (const json::exception&)
        {
            return false;
        }
    }

    string severity(double value, double threshold, bool highIsBad)
    {
        double distance = highIsBad ? value - threshold : threshold - value;
        if (distance >= 10.0)
            return "HIGH";
        if (distance >= 5.0)
            return "MEDIUM";
        return "LOW";
    }

    void printAlert(const string& name, const string& detail, const string& level)
    {
        cout << "\n[" << level << "] " << name << "\n";
        cout << "    " << detail << '\n';
    }

    struct AlertThresholds
    {
        double highTemp = 40.0;
        double lowTemp = 10.0;
        double rain = 40.0;
        double wind = 40.0;
        double humidity = 80.0;
    };

    AlertThresholds loadAlertThresholds()
    {
        AlertThresholds t;
        ifstream file("alert_thresholds.conf");
        if (!file.is_open()) return t;
        string key; double value;
        while (file >> key >> value)
        {
            if (key == "high_temperature") t.highTemp = value;
            else if (key == "low_temperature") t.lowTemp = value;
            else if (key == "rain") t.rain = value;
            else if (key == "wind") t.wind = value;
            else if (key == "humidity") t.humidity = value;
        }
        return t;
    }

    void generateSmartAlerts(const WeatherSnapshot& w, bool printOutput)
    {
        bool any = false;
        const AlertThresholds t = loadAlertThresholds();

        if (w.temperature > t.highTemp)
        {
            any = true;
            printAlert("Heat Alert",
                       "Temperature = " + to_string(w.temperature) + " C (threshold > " + to_string(t.highTemp) + " C)",
                       severity(w.temperature, t.highTemp, true));
        }

        if (w.temperature < t.lowTemp)
        {
            any = true;
            printAlert("Temperature Alert",
                       "Temperature = " + to_string(w.temperature) + " C (threshold < " + to_string(t.lowTemp) + " C)",
                       severity(w.temperature, t.lowTemp, false));
        }

        if (w.precipitation > t.rain)
        {
            any = true;
            printAlert("Heavy Rain Alert",
                       "Rainfall = " + to_string(w.precipitation) + " mm (threshold > " + to_string(t.rain) + " mm)",
                       severity(w.precipitation, t.rain, true));
        }

        if (w.windKph > t.wind)
        {
            any = true;
            printAlert("Strong Wind Alert",
                       "Wind speed = " + to_string(w.windKph) + " km/h (threshold > " + to_string(t.wind) + " km/h)",
                       severity(w.windKph, t.wind, true));
        }

        if (w.humidity > t.humidity)
        {
            any = true;
            printAlert("High Humidity Alert",
                       "Humidity = " + to_string(w.humidity) + "% (threshold > " + to_string(t.humidity) + "%)",
                       severity(w.humidity, t.humidity, true));
        }

        if (!any && printOutput)
        {
            cout << "\nNo Smart Weather Alerts Triggered.\n";
        }

        if (any)
        {
            logLine("alert.log", w.location + " | Smart alert(s) generated");
        }
    }

    void showForecast(const WeatherSnapshot& w)
    {
        cout << "\n========== HOURLY FORECAST ==========" << '\n';

        if (!hasJsonKey(w.forecast, "forecastday"))
        {
            cout << "Forecast data unavailable.\n";
            return;
        }

        int shown = 0;
        for (const auto& day : w.forecast["forecastday"])
        {
            if (!hasJsonKey(day, "hour"))
                continue;

            for (const auto& hour : day["hour"])
            {
                string time = jsonValue<string>(hour, "time", "");
                double temp = jsonValue<double>(hour, "temp_c", 0.0);
                double rainChance = jsonValue<double>(hour, "chance_of_rain", 0.0);
                double wind = jsonValue<double>(hour, "wind_kph", 0.0);
                string condition = jsonValue<string>(hour["condition"], "text", "Unknown");

                cout << time
                     << " | " << fixed << setprecision(1) << temp << " C"
                     << " | Rain: " << rainChance << "%"
                     << " | Wind: " << wind << " km/h"
                     << " | " << condition << '\n';

                ++shown;
                if (shown >= 12)
                    return;
            }
        }
    }

    void printSevenDayForecast(const WeatherSnapshot& w)
    {
        cout << "\n========== 7-DAY FORECAST ==========" << '\n';

        if (!hasJsonKey(w.forecast, "forecastday"))
        {
            cout << "Forecast data unavailable.\n";
            return;
        }

        for (const auto& day : w.forecast["forecastday"])
        {
            string date = jsonValue<string>(day, "date", "");
            const auto& d = day["day"];

            cout << date
                 << " | Min: " << fixed << setprecision(1) << jsonValue<double>(d, "mintemp_c", 0.0) << " C"
                 << " | Max: " << jsonValue<double>(d, "maxtemp_c", 0.0) << " C"
                 << " | Avg: " << jsonValue<double>(d, "avgtemp_c", 0.0) << " C"
                 << " | Rain: " << jsonValue<double>(d, "daily_chance_of_rain", 0.0) << "%"
                 << " | UV: " << jsonValue<double>(d, "uv", 0.0)
                 << " | " << jsonValue<string>(d["condition"], "text", "Unknown")
                 << '\n';
        }
    }

    void showApiAlerts(const WeatherSnapshot& w)
    {
        cout << "\n========== WEATHER ALERTS ==========" << '\n';

        if (!hasJsonKey(w.alerts, "alert") || !w.alerts["alert"].is_array() ||
            w.alerts["alert"].empty())
        {
            cout << "No alerts supplied by the Weather API.\n";
            return;
        }

        for (const auto& alert : w.alerts["alert"])
        {
            cout << "\nHeadline : " << jsonValue<string>(alert, "headline", "Unknown") << '\n';
            cout << "Event    : " << jsonValue<string>(alert, "event", "Unknown") << '\n';
            cout << "Severity : " << jsonValue<string>(alert, "severity", "Unknown") << '\n';
            cout << "Effective: " << jsonValue<string>(alert, "effective", "Unknown") << '\n';
            cout << "Expires  : " << jsonValue<string>(alert, "expires", "Unknown") << '\n';
            cout << "Description: " << jsonValue<string>(alert, "desc", "No description") << '\n';
        }
    }

    void showRiskAssessment(const WeatherSnapshot& w)
    {
        const AlertThresholds t = loadAlertThresholds();
        int riskScore = 0;

        if (w.temperature > t.highTemp || w.temperature < t.lowTemp) riskScore += 3;
        else if (w.temperature > t.highTemp - 5.0 || w.temperature < t.lowTemp + 5.0) riskScore += 1;

        if (w.precipitation > t.rain) riskScore += 3;
        else if (w.precipitation > 20.0) riskScore += 1;

        if (w.windKph > t.wind) riskScore += 3;
        else if (w.windKph > 30.0) riskScore += 1;

        if (w.humidity > t.humidity) riskScore += 2;

        string level = "LOW";
        if (riskScore >= 6) level = "HIGH";
        else if (riskScore >= 3) level = "MEDIUM";

        cout << "\n========== WEATHER RISK ASSESSMENT ==========" << '\n';
        cout << "Risk Score : " << riskScore << '\n';
        cout << "Risk Level : " << level << '\n';
    }

    void showInsights(const WeatherSnapshot& w)
    {
        cout << "\n========== WEATHER INSIGHTS ==========" << '\n';
        cout << "Feels Like       : " << w.feelsLike << " C\n";
        cout << "UV Index         : " << w.uv << '\n';
        cout << "Precipitation    : " << w.precipitation << " mm\n";
        cout << "Sunrise          : " << w.sunrise << '\n';
        cout << "Sunset           : " << w.sunset << '\n';

        if (w.feelsLike > w.temperature + 3.0)
            cout << "Insight           : It feels noticeably warmer than the actual temperature.\n";
        else if (w.feelsLike < w.temperature - 3.0)
            cout << "Insight           : It feels noticeably cooler than the actual temperature.\n";
        else
            cout << "Insight           : Feels-like temperature is close to the actual temperature.\n";

        if (w.uv >= 8.0)
            cout << "UV Advice         : Very high UV conditions; use strong sun protection.\n";
        else if (w.uv >= 6.0)
            cout << "UV Advice         : High UV conditions; sun protection is recommended.\n";
        else
            cout << "UV Advice         : UV level is not in the high-alert range.\n";

        if (w.precipitation > 0.0)
            cout << "Rain Insight      : Measurable precipitation is currently reported.\n";
        else
            cout << "Rain Insight      : No current precipitation is reported.\n";
    }

    string csvEscape(const string& value)
    {
        string out = value;
        size_t pos = 0;
        while ((pos = out.find('"', pos)) != string::npos)
        {
            out.insert(pos, 1, '"');
            pos += 2;
        }
        return '"' + out + '"';
    }

    void exportReport(const WeatherSnapshot& w, const string& username)
    {
        const bool exists = static_cast<bool>(ifstream("weather_report.csv"));
        ofstream file("weather_report.csv", ios::app);
        if (!file.is_open())
        {
            cout << "\nUnable to open weather_report.csv\n";
            return;
        }

        if (!exists)
            file << "Section,Date/Time,Username,Location,Detail,Value 1,Value 2,Value 3\n";

        const string now = nowString();
        auto row = [&](const string& section, const string& detail,
                       const string& v1 = "", const string& v2 = "", const string& v3 = "")
        {
            file << csvEscape(section) << ',' << csvEscape(now) << ','
                 << csvEscape(username) << ',' << csvEscape(w.location) << ','
                 << csvEscape(detail) << ',' << csvEscape(v1) << ','
                 << csvEscape(v2) << ',' << csvEscape(v3) << '\n';
        };

        row("CURRENT WEATHER", "Region", w.region);
        row("CURRENT WEATHER", "Country", w.country);
        row("CURRENT WEATHER", "Temperature", to_string(w.temperature), "C");
        row("CURRENT WEATHER", "Feels Like", to_string(w.feelsLike), "C");
        row("CURRENT WEATHER", "Humidity", to_string(w.humidity), "%");
        row("CURRENT WEATHER", "Wind Speed", to_string(w.windKph), "km/h");
        row("CURRENT WEATHER", "Pressure", to_string(w.pressure), "mb");
        row("CURRENT WEATHER", "Precipitation", to_string(w.precipitation), "mm");
        row("CURRENT WEATHER", "UV Index", to_string(w.uv));
        row("CURRENT WEATHER", "Condition", w.condition);
        row("CURRENT WEATHER", "Sunrise", w.sunrise);
        row("CURRENT WEATHER", "Sunset", w.sunset);

        if (hasJsonKey(w.forecast, "forecastday"))
        {
            for (const auto& day : w.forecast["forecastday"])
            {
                const auto& d = day["day"];
                row("7-DAY FORECAST", "Date",
                    jsonValue<string>(day, "date", ""),
                    "Min=" + to_string(jsonValue<double>(d, "mintemp_c", 0.0)) + " C",
                    "Max=" + to_string(jsonValue<double>(d, "maxtemp_c", 0.0)) + " C");
                row("7-DAY FORECAST", "Forecast",
                    "Avg=" + to_string(jsonValue<double>(d, "avgtemp_c", 0.0)) + " C",
                    "Rain=" + to_string(jsonValue<double>(d, "daily_chance_of_rain", 0.0)) + "%",
                    "Condition=" + jsonValue<string>(d["condition"], "text", "Unknown"));
            }
        }

        if (hasJsonKey(w.forecast, "forecastday"))
        {
            int shown = 0;
            for (const auto& day : w.forecast["forecastday"])
            {
                if (!hasJsonKey(day, "hour")) continue;
                for (const auto& hour : day["hour"])
                {
                    row("HOURLY FORECAST", jsonValue<string>(hour, "time", ""),
                        "Temp=" + to_string(jsonValue<double>(hour, "temp_c", 0.0)) + " C",
                        "Rain=" + to_string(jsonValue<double>(hour, "chance_of_rain", 0.0)) + "%",
                        "Wind=" + to_string(jsonValue<double>(hour, "wind_kph", 0.0)) + " km/h");
                    if (++shown >= 12) break;
                }
                if (shown >= 12) break;
            }
        }

        if (hasJsonKey(w.alerts, "alert") && w.alerts["alert"].is_array() && !w.alerts["alert"].empty())
        {
            for (const auto& alert : w.alerts["alert"])
            {
                row("WEATHER API ALERT", jsonValue<string>(alert, "headline", "Unknown"),
                    "Event=" + jsonValue<string>(alert, "event", "Unknown"),
                    "Severity=" + jsonValue<string>(alert, "severity", "Unknown"),
                    jsonValue<string>(alert, "desc", "No description"));
            }
        }
        else
        {
            row("WEATHER API ALERT", "No alerts supplied by the Weather API.");
        }

        const AlertThresholds t = loadAlertThresholds();
        bool any = false;
        if (w.temperature > t.highTemp) { row("SMART ALERT", "Heat Alert", to_string(w.temperature) + " C", "Threshold > " + to_string(t.highTemp) + " C", severity(w.temperature,t.highTemp,true)); any=true; }
        if (w.temperature < t.lowTemp) { row("SMART ALERT", "Temperature Alert", to_string(w.temperature) + " C", "Threshold < " + to_string(t.lowTemp) + " C", severity(w.temperature,t.lowTemp,false)); any=true; }
        if (w.precipitation > t.rain) { row("SMART ALERT", "Heavy Rain Alert", to_string(w.precipitation) + " mm", "Threshold > " + to_string(t.rain) + " mm", severity(w.precipitation,t.rain,true)); any=true; }
        if (w.windKph > t.wind) { row("SMART ALERT", "Strong Wind Alert", to_string(w.windKph) + " km/h", "Threshold > " + to_string(t.wind) + " km/h", severity(w.windKph,t.wind,true)); any=true; }
        if (w.humidity > t.humidity) { row("SMART ALERT", "High Humidity Alert", to_string(w.humidity) + "%", "Threshold > " + to_string(t.humidity) + "%", severity(w.humidity,t.humidity,true)); any=true; }
        if (!any) row("SMART ALERT", "No Smart Weather Alerts Triggered.");

        int riskScore = 0;
        if (w.temperature > t.highTemp || w.temperature < t.lowTemp) riskScore += 3;
        else if (w.temperature > t.highTemp - 5.0 || w.temperature < t.lowTemp + 5.0) riskScore += 1;
        if (w.precipitation > t.rain) riskScore += 3; else if (w.precipitation > 20.0) riskScore += 1;
        if (w.windKph > t.wind) riskScore += 3; else if (w.windKph > 30.0) riskScore += 1;
        if (w.humidity > t.humidity) riskScore += 2;
        string riskLevel = riskScore >= 6 ? "HIGH" : riskScore >= 3 ? "MEDIUM" : "LOW";
        row("RISK ASSESSMENT", "Risk Score", to_string(riskScore), "Risk Level=" + riskLevel);

        row("WEATHER INSIGHTS", "Feels Like", to_string(w.feelsLike), "C");
        row("WEATHER INSIGHTS", "UV Index", to_string(w.uv));
        row("WEATHER INSIGHTS", "Precipitation", to_string(w.precipitation), "mm");
        row("WEATHER INSIGHTS", "Sunrise", w.sunrise);
        row("WEATHER INSIGHTS", "Sunset", w.sunset);
        if (w.feelsLike > w.temperature + 3.0) row("WEATHER INSIGHTS", "Insight", "Feels noticeably warmer than actual temperature.");
        else if (w.feelsLike < w.temperature - 3.0) row("WEATHER INSIGHTS", "Insight", "Feels noticeably cooler than actual temperature.");
        else row("WEATHER INSIGHTS", "Insight", "Feels-like temperature is close to actual temperature.");
        if (w.uv >= 8.0) row("WEATHER INSIGHTS", "UV Advice", "Very high UV conditions; use strong sun protection.");
        else if (w.uv >= 6.0) row("WEATHER INSIGHTS", "UV Advice", "High UV conditions; sun protection is recommended.");
        else row("WEATHER INSIGHTS", "UV Advice", "UV level is not in the high-alert range.");
        row("WEATHER INSIGHTS", "Rain Insight", w.precipitation > 0.0 ? "Measurable precipitation is currently reported." : "No current precipitation is reported.");

        file.close();
        cout << "\nWeather intelligence report appended to weather_report.csv\n";
        logLine("system.log", w.location + " | Weather intelligence report exported");
    }

    void showHistoricalTrend(const string& username)
    {
        vector<UserDataRecord> records;
        for (const auto& r : UserDataManager::loadAll())
            if (r.username == username && r.type == "WEATHER" && r.detail == "Current Weather") records.push_back(r);

        if (records.size() < 2)
        {
            cout << "\nAt least two historical records are required for trend analysis.\n";
            return;
        }

        auto trend = [](double a, double b) {
            if (b > a + 0.01) return string("RISING");
            if (b < a - 0.01) return string("FALLING");
            return string("STABLE");
        };

        double ft, fh, fp, lt, lh, lp;
        try { ft=stod(records.front().value1); fh=stod(records.front().value2); fp=stod(records.front().value3);
              lt=stod(records.back().value1); lh=stod(records.back().value2); lp=stod(records.back().value3); }
        catch (...) { cout << "\nHistorical data contains invalid values.\n"; return; }

        cout << "\n========== HISTORICAL TREND ANALYSIS ==========\n";
        cout << "Temperature : " << trend(ft,lt) << " (" << ft << " -> " << lt << " C)\n";
        cout << "Humidity    : " << trend(fh,lh) << " (" << fh << " -> " << lh << " %)\n";
        cout << "Pressure    : " << trend(fp,lp) << " (" << fp << " -> " << lp << " mb)\n";
    }

    bool fetchForecast(const string& query, WeatherSnapshot& snapshot)
    {
        string response;
        long status = 0;
        double elapsed = 0.0;

        APIManager api;
        if (api.getApiKey().empty())
        {
            cout << "\nAPI key is not configured. Set WEATHER_API_KEY before running.\n";
            logLine("error.log", query + " | Forecast skipped | API key not configured");
            return false;
        }

        string url = buildForecastUrl(query);
        cout << "\nRequesting forecast data...\n";

        if (!requestJsonWithRetry(url, response, status, elapsed, 3))
        {
            cout << "\nForecast request failed after 3 attempts.\n";
            logLine("error.log", query + " | Forecast API failed after retries");
            return false;
        }

        try
        {
            json data = json::parse(response);
            if (!parseSnapshot(data, snapshot))
            {
                cout << "\nInvalid forecast JSON data.\n";
                logLine("error.log", query + " | Forecast JSON parse/structure error");
                return false;
            }
        }
        catch (const json::exception& e)
        {
            cout << "\nJSON Error: " << e.what() << '\n';
            logLine("error.log", query + " | JSON Error | " + string(e.what()));
            return false;
        }

        logLine("api.log", query + " | Forecast success | response_ms=" + to_string(elapsed));
        return true;
    }

    void appendRefreshRecord(const WeatherSnapshot& w, const string& username)
    {
        lock_guard<mutex> lock(storageMutex);
        UserDataRecord r;
        r.type = "WEATHER";
        r.username = username;
        r.city = w.location;
        r.region = w.region;
        r.country = w.country;
        r.latitude = w.latitude;
        r.longitude = w.longitude;
        r.timestamp = nowString();
        r.detail = "Current Weather";
        r.value1 = to_string(w.temperature);
        r.value2 = to_string(w.humidity);
        r.value3 = to_string(w.pressure);
        UserDataManager::append(r);
    }
}

// ============================================================================
// ROLE-BASED AUTHENTICATION + SHA-256 PASSWORD HASHING
// ============================================================================
// Passwords are NEVER stored as plaintext.  SHA-256 is a one-way hash rather
// than encryption: during login the entered password is hashed and compared
// with the stored digest.
namespace
{
    struct Account
    {
        string username;
        string passwordHash;
        string role;
    };

    const string USER_DB = "users.csv";
    const string ADMIN_USERNAME = "admin";
    const string ADMIN_PASSWORD_HASH =
        "e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7";

    // Compact, dependency-free SHA-256 implementation.
    uint32_t rotr(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    string sha256(const string& input)
    {
        static const uint32_t K[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
            0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
            0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
            0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
            0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
            0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
            0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
            0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
            0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

        uint32_t H[8] = {
            0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
            0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
        };

        vector<unsigned char> data(input.begin(), input.end());
        uint64_t bitLength = static_cast<uint64_t>(data.size()) * 8ULL;
        data.push_back(0x80);
        while ((data.size() % 64) != 56)
            data.push_back(0x00);

        for (int i = 7; i >= 0; --i)
            data.push_back(static_cast<unsigned char>((bitLength >> (i * 8)) & 0xff));

        for (size_t chunk = 0; chunk < data.size(); chunk += 64)
        {
            uint32_t w[64]{};
            for (int i = 0; i < 16; ++i)
            {
                size_t j = chunk + static_cast<size_t>(i) * 4;
                w[i] = (static_cast<uint32_t>(data[j]) << 24) |
                       (static_cast<uint32_t>(data[j + 1]) << 16) |
                       (static_cast<uint32_t>(data[j + 2]) << 8) |
                       static_cast<uint32_t>(data[j + 3]);
            }

            for (int i = 16; i < 64; ++i)
            {
                uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a=H[0], b=H[1], c=H[2], d=H[3];
            uint32_t e=H[4], f=H[5], g=H[6], h=H[7];

            for (int i = 0; i < 64; ++i)
            {
                uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h + S1 + ch + K[i] + w[i];
                uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;

                h=g; g=f; f=e; e=d+temp1; d=c; c=b; b=a; a=temp1+temp2;
            }

            H[0]+=a; H[1]+=b; H[2]+=c; H[3]+=d;
            H[4]+=e; H[5]+=f; H[6]+=g; H[7]+=h;
        }

        ostringstream out;
        out << hex << setfill('0');
        for (uint32_t value : H)
            out << setw(8) << value;
        return out.str();
    }

    vector<Account> loadAccounts()
    {
        vector<Account> accounts;
        ifstream file(USER_DB);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            Account account;
            getline(ss, account.username, ',');
            getline(ss, account.passwordHash, ',');
            getline(ss, account.role);

            if (account.username.empty() || account.passwordHash.empty())
                continue;

            // Backward compatibility with the old two-column users.csv.
            if (account.role.empty())
                account.role = "USER";

            accounts.push_back(account);
        }
        return accounts;
    }

    bool saveAccounts(const vector<Account>& accounts)
    {
        ofstream file(USER_DB, ios::trunc);
        if (!file.is_open())
            return false;

        for (const Account& account : accounts)
            file << account.username << ',' << account.passwordHash << ',' << account.role << '\n';
        return true;
    }

    void ensureDefaultAdmin()
    {
        vector<Account> accounts = loadAccounts();
        for (const Account& account : accounts)
        {
            if (account.username == ADMIN_USERNAME && account.role == "ADMIN")
                return;
        }

        accounts.push_back({ADMIN_USERNAME, ADMIN_PASSWORD_HASH, "ADMIN"});
        saveAccounts(accounts);
    }

    bool validUsername(const string& username)
    {
        if (username.empty() || username.size() > 32)
            return false;
        for (unsigned char c : username)
        {
            if (!(isalnum(c) || c == '_' || c == '-' || c == '.'))
                return false;
        }
        return true;
    }

    bool validPassword(const string& password)
    {
        return password.size() >= 6 && password.size() <= 128;
    }
}

static bool authenticateAccount(const string& username,
                                 const string& password,
                                 const string& requiredRole,
                                 string& matchedUsername,
                                 string& matchedRole)
{
    vector<Account> accounts = loadAccounts();
    string enteredHash = sha256(password);

    for (Account& account : accounts)
    {
        if (account.username != username)
            continue;

        // Legacy two-column users.csv support.
        bool legacyPlaintext = account.passwordHash.size() != 64;
        bool passwordMatches = legacyPlaintext
            ? (account.passwordHash == password)
            : (account.passwordHash == enteredHash);

        if (!passwordMatches)
            return false;

        string actualRole = (account.role == "ADMIN") ? "ADMIN" : "USER";
        if (actualRole != requiredRole)
            return false;

        if (legacyPlaintext)
        {
            account.passwordHash = enteredHash;
            saveAccounts(accounts);
        }

        matchedUsername = account.username;
        matchedRole = actualRole;
        return true;
    }

    return false;
}

static bool registerAccount(const string& role)
{
    string username, password, confirm;

    cout << "\n========== " << role << " REGISTRATION ==========" << '\n';
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;
    cout << "Confirm Password: ";
    cin >> confirm;

    if (!validUsername(username))
    {
        cout << "\nInvalid username. Use 1-32 letters, numbers, _, - or .\n";
        return false;
    }

    if (!validPassword(password))
    {
        cout << "\nPassword must contain at least 6 characters.\n";
        return false;
    }

    if (password != confirm)
    {
        cout << "\nPasswords do not match.\n";
        return false;
    }

    vector<Account> accounts = loadAccounts();
    for (const Account& account : accounts)
    {
        if (account.username == username)
        {
            cout << "\nUsername already exists. Registration failed.\n";
            return false;
        }
    }

    accounts.push_back({username, sha256(password), role});
    if (!saveAccounts(accounts))
    {
        cout << "\nUnable to save users.csv.\n";
        logLine("error.log", "Registration failed | Cannot write users.csv");
        return false;
    }

    cout << "\nRegistration Successful!\n";
    cout << "Password stored as SHA-256 hash.\n";
    cout << "Role: " << role << "\n";
    logLine("system.log", username + " | Registration successful | role=" + role);
    return true;
}

bool WeatherEnhancementManager::loginAsUser()
{
    string username, password;
    cout << "\n========== USER LOGIN ==========\n";
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;

    if (authenticateAccount(username, password, "USER", currentUsername, currentRole))
    {
        cout << "\nLogin Successful!\n";
        cout << "Welcome, " << currentUsername << "!\n";
        cout << "Role: USER\n";
        logLine("system.log", currentUsername + " | Login successful | role=USER");
        return true;
    }

    cout << "\nInvalid USER username or password.\n";
    logLine("error.log", username + " | User login failed");
    return false;
}

bool WeatherEnhancementManager::loginAsAdmin()
{
    ensureDefaultAdmin();

    string username, password;
    cout << "\n========== ADMIN LOGIN ==========\n";
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;

    if (authenticateAccount(username, password, "ADMIN", currentUsername, currentRole))
    {
        cout << "\nLogin Successful!\n";
        cout << "Welcome, Admin " << currentUsername << "!\n";
        cout << "Role: ADMIN\n";
        logLine("system.log", currentUsername + " | Admin login successful");
        return true;
    }

    cout << "\nInvalid ADMIN username or password.\n";
    logLine("error.log", username + " | Admin login failed");
    return false;
}

bool WeatherEnhancementManager::registerUser()
{
    return registerAccount("USER");
}


bool WeatherEnhancementManager::login()
{
    ensureDefaultAdmin();

    while (true)
    {
        cout << "\n=========================================================\n";
        cout << "              WEATHER DATA MANAGEMENT SYSTEM\n";
        cout << "=========================================================\n\n";
        cout << "                1. User Login\n";
        cout << "                2. User Registration\n";
        cout << "                3. Admin Login\n";
        cout << "                0. Exit\n\n";
        cout << "---------------------------------------------------------\n";
        cout << "Enter your choice: ";

        int choice = -1;
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice.\n";
            continue;
        }

        switch (choice)
        {
            case 1:
                if (loginAsUser()) return true;
                break;
            case 2:
                registerUser();
                break;
            case 3:
                if (loginAsAdmin()) return true;
                break;
            case 0:
                return false;
            default:
                cout << "\nInvalid choice. Please select 0-3.\n";
                break;
        }
    }
}

string WeatherEnhancementManager::getRole() const
{
    return currentRole;
}

string WeatherEnhancementManager::getUsername() const
{
    return currentUsername;
}

void WeatherEnhancementManager::userManagement()
{
    if (currentRole != "ADMIN")
    {
        cout << "\nAccess denied. Admin privileges required.\n";
        return;
    }

    while (true)
    {
        cout << "\n========== USER MANAGEMENT ==========\n";
        cout << "1. View Users\n";
        cout << "2. Reset User Password\n";
        cout << "3. Delete User\n";
        cout << "0. Back To Admin Menu\n";
        cout << "\nEnter Choice: ";

        int choice;
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nInvalid Choice.\n";
            continue;
        }

        vector<Account> accounts = loadAccounts();

        if (choice == 0)
            return;

        if (choice == 1)
        {
            cout << "\n----------------------------------------\n";
            cout << left << setw(5) << "No." << setw(25) << "Username" << "Role\n";
            cout << "----------------------------------------\n";
            for (size_t i = 0; i < accounts.size(); ++i)
                cout << left << setw(5) << i + 1 << setw(25) << accounts[i].username
                     << accounts[i].role << '\n';
            continue;
        }

        if (choice == 2)
        {
            string username, newPassword, confirm;
            cout << "\nEnter Username: ";
            cin >> username;
            if (username == ADMIN_USERNAME)
            {
                cout << "Use the admin account only for administration.\n";
                continue;
            }
            cout << "New Password: ";
            cin >> newPassword;
            cout << "Confirm Password: ";
            cin >> confirm;

            if (!validPassword(newPassword) || newPassword != confirm)
            {
                cout << "\nInvalid password or passwords do not match.\n";
                continue;
            }

            bool found = false;
            for (Account& account : accounts)
            {
                if (account.username == username)
                {
                    account.passwordHash = sha256(newPassword);
                    found = true;
                    break;
                }
            }

            if (found && saveAccounts(accounts))
                cout << "\nPassword reset successfully. SHA-256 hash updated.\n";
            else
                cout << "\nUser not found or database update failed.\n";
            continue;
        }

        if (choice == 3)
        {
            string username;
            cout << "\nEnter Username To Delete: ";
            cin >> username;

            if (username == ADMIN_USERNAME || username == currentUsername)
            {
                cout << "\nThe active/default admin account cannot be deleted.\n";
                continue;
            }

            size_t oldSize = accounts.size();
            accounts.erase(remove_if(accounts.begin(), accounts.end(),
                [&](const Account& account) { return account.username == username; }),
                accounts.end());

            if (accounts.size() == oldSize)
            {
                cout << "\nUser not found.\n";
                continue;
            }

            if (saveAccounts(accounts))
                cout << "\nUser deleted successfully.\n";
            else
                cout << "\nUnable to update users.csv.\n";
            continue;
        }

        cout << "\nInvalid Choice.\n";
    }
}

static bool readSnapshotForMenu(const string& username, WeatherSnapshot& snapshot)
{
    LocationManager lm(username);
    LocationResult selected;
    if (!lm.selectLocation(selected, "Search Location: "))
        return false;

    ostringstream query;
    query << selected.latitude << "," << selected.longitude;
    return fetchForecast(query.str(), snapshot);
}

static void saveUserSnapshot(const string& username, const WeatherSnapshot& w)
{
    if (username.empty()) return;
    UserDataRecord r;
    r.type = "WEATHER"; r.username = username; r.city = w.location;
    r.region = w.region; r.country = w.country; r.latitude = w.latitude; r.longitude = w.longitude;
    r.detail = "Current Weather"; r.value1 = to_string(w.temperature);
    r.value2 = to_string(w.humidity); r.value3 = to_string(w.pressure);
    UserDataManager::append(r);
}

void WeatherEnhancementManager::showCurrentWeather()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;
    saveUserSnapshot(currentUsername, snapshot);

    cout << "\n========== CURRENT WEATHER ==========" << '\n';
    cout << "Location       : " << snapshot.location << '\n';
    cout << "Region         : " << snapshot.region << '\n';
    cout << "Country        : " << snapshot.country << '\n';
    cout << "Temperature    : " << snapshot.temperature << " C\n";
    cout << "Feels Like     : " << snapshot.feelsLike << " C\n";
    cout << "Humidity       : " << snapshot.humidity << " %\n";
    cout << "Wind Speed     : " << snapshot.windKph << " km/h\n";
    cout << "Pressure       : " << snapshot.pressure << " mb\n";
    cout << "Precipitation  : " << snapshot.precipitation << " mm\n";
    cout << "UV Index       : " << snapshot.uv << '\n';
    cout << "Condition      : " << snapshot.condition << '\n';
    cout << "Sunrise        : " << snapshot.sunrise << '\n';
    cout << "Sunset         : " << snapshot.sunset << '\n';
}

void WeatherEnhancementManager::showSevenDayForecast()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;

    cout << "\n========== 7-DAY FORECAST ==========" << '\n';
    printSevenDayForecast(snapshot);
}

void WeatherEnhancementManager::viewWeatherAlerts()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;

    showApiAlerts(snapshot);
    generateSmartAlerts(snapshot, true);
}

void WeatherEnhancementManager::showWeatherInsights()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;

    showInsights(snapshot);
    showRiskAssessment(snapshot);
}

void WeatherEnhancementManager::exportWeatherReport()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;

    cout << "\n========== WEATHER REPORT EXPORT ==========" << '\n';
    exportReport(snapshot, currentUsername);
}

void WeatherEnhancementManager::showWeatherIntelligence()
{
    WeatherSnapshot snapshot;
    if (!readSnapshotForMenu(currentUsername, snapshot)) return;

    cout << "\n=========================================================\n";
    cout << "              WEATHER INTELLIGENCE REPORT\n";
    cout << "=========================================================\n";
    cout << "Location       : " << snapshot.location << '\n';
    cout << "Region         : " << snapshot.region << '\n';
    cout << "Country        : " << snapshot.country << '\n';
    cout << "Temperature    : " << snapshot.temperature << " C\n";
    cout << "Feels Like     : " << snapshot.feelsLike << " C\n";
    cout << "Humidity       : " << snapshot.humidity << " %\n";
    cout << "Wind Speed     : " << snapshot.windKph << " km/h\n";
    cout << "Pressure       : " << snapshot.pressure << " mb\n";
    cout << "Precipitation  : " << snapshot.precipitation << " mm\n";
    cout << "UV Index       : " << snapshot.uv << '\n';
    cout << "Condition      : " << snapshot.condition << '\n';
    cout << "Sunrise        : " << snapshot.sunrise << '\n';
    cout << "Sunset         : " << snapshot.sunset << '\n';

    showForecast(snapshot);
    printSevenDayForecast(snapshot);
    showApiAlerts(snapshot);
    generateSmartAlerts(snapshot, true);
    showRiskAssessment(snapshot);
    showInsights(snapshot);
    showHistoricalTrend(currentUsername);

    cout << "\nExport this intelligence report? (1=Yes, 0=No): ";
    int exportChoice = 0;
    if (cin >> exportChoice && exportChoice == 1)
        exportReport(snapshot, currentUsername);
}

void WeatherEnhancementManager::showApiPerformance()
{
    ifstream file("api_performance.log");
    if (!file.is_open())
    {
        cout << "\nNo API performance records available yet.\n";
        return;
    }

    string line;
    vector<double> times;
    int successes = 0;
    int attempts = 0;

    while (getline(file, line))
    {
        ++attempts;
        size_t pos = line.find("response_time_ms=");
        if (pos != string::npos)
        {
            try
            {
                times.push_back(stod(line.substr(pos + 17)));
            }
            catch (...) {}
        }
        if (line.find("status=200") != string::npos)
            ++successes;
    }

    cout << "\n========== API PERFORMANCE MONITOR ==========" << '\n';
    cout << "Requests/Attempts : " << attempts << '\n';
    cout << "Successful (200)  : " << successes << '\n';

    if (!times.empty())
    {
        double sum = 0.0;
        for (double t : times) sum += t;
        cout << fixed << setprecision(2);
        cout << "Average Response  : " << sum / times.size() << " ms\n";
        cout << "Last Response     : " << times.back() << " ms\n";
    }

    if (attempts > 0)
    {
        cout << "Availability       : "
             << fixed << setprecision(2)
             << (100.0 * successes / attempts)
             << "%\n";
    }
}

void WeatherEnhancementManager::testApiWithRetry()
{
    LocationManager lm(currentUsername);
    LocationResult selected;
    if (!lm.selectLocation(selected, "Search Location: ")) return;
    ostringstream query;
    query << selected.latitude << "," << selected.longitude;

    WeatherSnapshot snapshot;
    if (fetchForecast(query.str(), snapshot))
    {
        cout << "\nAPI request succeeded. Retry mechanism is operational.\n";
    }
    else
    {
        cout << "\nAPI request failed after the configured retry attempts.\n";
    }
}


void WeatherEnhancementManager::configureApi()
{
    if (currentRole != "ADMIN") { cout << "\nAccess denied.\n"; return; }

    cout << "\n========== API CONFIGURATION ==========\n";
    APIManager api;
    cout << "Current API key status: " << (api.getApiKey().empty() ? "NOT CONFIGURED" : "CONFIGURED") << '\n';
    cout << "Enter new WeatherAPI key (0 to cancel): ";
    string key;
    cin >> key;
    if (key == "0") return;

#ifdef _WIN32
    _putenv_s("WEATHER_API_KEY", key.c_str());
#else
    setenv("WEATHER_API_KEY", key.c_str(), 1);
#endif
    ofstream file("api_config.conf", ios::trunc);
    if (file.is_open()) file << "WEATHER_API_KEY=" << key << '\n';
    cout << "API key configured for this program session.\n";
    cout << "Configuration saved to api_config.conf.\n";
    logLine("system.log", currentUsername + " | API configuration updated");
}

void WeatherEnhancementManager::configureAlertThresholds()
{
    if (currentRole != "ADMIN") { cout << "\nAccess denied.\n"; return; }
    AlertThresholds t = loadAlertThresholds();
    cout << "\n========== ALERT THRESHOLDS ==========\n";
    cout << "High Temperature (C) [" << t.highTemp << "]: "; cin >> t.highTemp;
    cout << "Low Temperature (C)  [" << t.lowTemp << "]: "; cin >> t.lowTemp;
    cout << "Rain (mm)             [" << t.rain << "]: "; cin >> t.rain;
    cout << "Wind (km/h)            [" << t.wind << "]: "; cin >> t.wind;
    cout << "Humidity (%)           [" << t.humidity << "]: "; cin >> t.humidity;
    if (t.lowTemp >= t.highTemp || t.rain < 0 || t.wind < 0 || t.humidity < 0 || t.humidity > 100)
    {
        cout << "Invalid threshold values. No changes saved.\n";
        return;
    }
    ofstream file("alert_thresholds.conf", ios::trunc);
    file << "high_temperature " << t.highTemp << '\n'
         << "low_temperature " << t.lowTemp << '\n'
         << "rain " << t.rain << '\n'
         << "wind " << t.wind << '\n'
         << "humidity " << t.humidity << '\n';
    cout << "Alert thresholds saved successfully.\n";
}

void WeatherEnhancementManager::monitorSystemLogs()
{
    if (currentRole != "ADMIN") { cout << "\nAccess denied.\n"; return; }
    const char* files[] = {"system.log", "error.log", "api.log", "alert.log", "weather.log", "api_performance.log"};
    cout << "\n========== SYSTEM LOG MONITOR ==========\n";
    for (const char* name : files)
    {
        ifstream file(name);
        cout << "\n--- " << name << " ---\n";
        if (!file.is_open()) { cout << "No records.\n"; continue; }
        vector<string> lines; string line;
        while (getline(file, line)) lines.push_back(line);
        const size_t start = lines.size() > 10 ? lines.size() - 10 : 0;
        for (size_t i = start; i < lines.size(); ++i) cout << lines[i] << '\n';
    }
}

void WeatherEnhancementManager::automaticWeatherRefresh()
{
    int intervalSeconds;
    int updateCount;
    LocationManager lm(currentUsername);
    LocationResult selected;
    if (!lm.selectLocation(selected, "Search Location: ")) return;
    ostringstream locationQuery;
    locationQuery << selected.latitude << "," << selected.longitude;
    string city = locationQuery.str();

    cout << "Refresh Interval (seconds, minimum 5): ";
    cin >> intervalSeconds;

    cout << "Number Of Updates: ";
    cin >> updateCount;

    if (intervalSeconds < 5 || updateCount <= 0 || updateCount > 100)
    {
        cout << "\nInvalid refresh settings.\n";
        return;
    }

    cout << "\nAutomatic monitoring started.\n";
    cout << "The monitoring worker runs in a separate thread.\n";

    thread worker([city, intervalSeconds, updateCount, username = currentUsername]()
    {
        for (int i = 1; i <= updateCount; ++i)
        {
            WeatherSnapshot snapshot;
            cout << "\n[Refresh " << i << "/" << updateCount << "]\n";

            if (fetchForecast(city, snapshot))
            {
                cout << "Location      : " << snapshot.location << '\n';
                cout << "Temperature   : " << snapshot.temperature << " C\n";
                cout << "Humidity      : " << snapshot.humidity << " %\n";
                cout << "Wind Speed    : " << snapshot.windKph << " km/h\n";
                cout << "Condition     : " << snapshot.condition << '\n';

                appendRefreshRecord(snapshot, username);
                generateSmartAlerts(snapshot, true);
                logLine("weather.log", snapshot.location + " | Automatic weather update");
            }

            if (i < updateCount)
            {
                this_thread::sleep_for(chrono::seconds(intervalSeconds));
            }
        }

        cout << "\nAutomatic monitoring completed.\n";
        logLine("system.log", city + " | Automatic monitoring completed");
    });

    worker.join();
}

