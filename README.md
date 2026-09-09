# Weather Data Management System

Linux C++17 console application with role-based User/Admin dashboards and SHA-256 password hashing.

## Main Menu

```text
=========================================================
              WEATHER DATA MANAGEMENT SYSTEM
=========================================================

                1. User Login
                2. User Registration
                3. Admin Login
                0. Exit
```

There is **no Admin Registration**. The default admin is:

- Username: `admin`
- Password: `Admin@123`

Passwords are stored as SHA-256 hashes in `users.csv`.

## User Dashboard

```text
1. View Current Weather
2. View 7-Day Forecast
3. Add Favorite City
4. View Favorite Cities
5. Remove Favorite City
6. Recent Searches
7. Most Searched City
8. View Historical Data
9. Weather Comfort Score
10. Travel Recommendation
11. Compare Cities
12. View Weather Alerts
13. Weather Insights
14. Export Weather Report
0. Logout
```

## Admin Dashboard

The Admin Dashboard contains **only** these seven functions:

```text
1. API Configuration
2. Manage Stored Data
3. Configure Alert Thresholds
4. Generate Reports
5. Monitor System Logs
6. API Performance Monitoring
7. Automatic Weather Refresh
0. Logout
```

`Manage Stored Data` contains:

```text
1. Save Current Weather Data
2. View All User Data
3. Backup Data
4. Clear All Weather Storage
0. Back to Admin Dashboard
```

Admin saving weather data uses the selected location and **live WeatherAPI data**. There is no hard-coded weather value.

## Per-user data storage

The project intentionally avoids separate `favorites.csv`, `location_history.csv`, and `weather_storage.csv` files. User-owned operational data is stored in one structured file:

`user_data.csv`

Columns:

```text
Type,Username,City,Region,Country,Latitude,Longitude,DateTime,Detail,Value1,Value2,Value3
```

Every record contains a `Username`, so data can be separated by user.

- `FAVORITE` records belong to the logged-in user.
- `SEARCH` records belong to the logged-in user.
- `WEATHER` records belong to the user/admin who saved or automatically refreshed the weather data.

### Limits

- Maximum **5 favorite cities per user**.
- Maximum **10 recent searches stored per user**.
- Duplicate favorite cities are rejected for the same user.
- User data is never mixed when displaying the User dashboard.
- Admin can view all users' stored data with the username shown for every record.

## Logout behavior

Choosing `0. Logout` from either dashboard returns to the **main authentication menu**. It does not terminate the complete program.

## Weather location search

Whenever a feature requires a location, the common WeatherAPI location search is used first. The user selects a verified result and its coordinates are used for the weather request.

This applies to current weather, forecasts, favorites, comfort score, travel recommendation, city comparison, alerts, insights, export, API retry testing and automatic refresh.

## Weather reports and export

`weather_report.csv` is **append-only**. A new export never deletes previous reports.

Each row contains:

```text
Section,Date/Time,Username,Location,Detail,Value 1,Value 2,Value 3
```

The report contains the details produced by the Weather Intelligence functions: current weather, hourly forecast, 7-day forecast, Weather API alerts, smart alerts, risk assessment, weather insights and historical trend information.

## Linux dependencies

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev
```

## Build with CMake

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j$(nproc)
./weather_system
```

## Direct g++ build

```bash
g++ -std=c++17 -Wall -Wextra -pedantic *.cpp -lcurl -pthread -o weather_system
./weather_system
```

## API key

Set the key before starting:

```bash
export WEATHER_API_KEY="YOUR_WEATHERAPI_KEY"
```

or use **Admin Dashboard -> API Configuration**. The configured key is persisted in `api_config.conf`.

## Windows compatibility

The Admin API configuration uses `_putenv_s()` on Windows and `setenv()` on Linux/macOS, so the source can be compiled on either platform. The primary target remains Linux.
