# Comprehensive GoogleTest Suite — Weather Data Management System

This directory contains a full multi-level automated test suite for the project. It is intentionally split into many focused files rather than putting all tests into one or two files.

## Test levels

- **Unit:** individual classes/functions, validation, persistence rules, CSV handling, authentication boundaries.
- **Integration:** interactions between API/configuration, location, user data, weather storage, authentication and reports.
- **Functional:** user-visible features such as login, registration, favorites, search history, weather, forecast, alerts, insights, travel, comparison, export, automatic refresh and admin functions.
- **System:** end-to-end workflows and executable/menu checks.

## Coverage strategy

Every major feature includes positive, negative, boundary, duplicate, empty-input, isolation, persistence or failure-path tests where the current public API makes deterministic testing possible.

Live WeatherAPI calls are **not required** for the deterministic suite. Tests that depend on an API key deliberately verify controlled failure behavior when the key is absent. This keeps CI runs repeatable and avoids API quota consumption.

## Build

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

GoogleTest is fetched with CMake `FetchContent` using GoogleTest 1.16.0. The machine running the build therefore needs network access the first time unless GoogleTest is already available/cached.

## File count

The suite contains multiple dedicated files under each level. Each file contains multiple `TEST`/`TEST_F` cases so the deliverable is suitable for a project-level testing submission rather than a minimal smoke-test sample.

## Optional live API smoke tests

`integration/LiveWeatherAPI_SmokeTest.cpp` contains two tests that use the real WeatherAPI only when `WEATHER_API_KEY` is present in the environment. Without a key they are skipped rather than failed. These tests are intended for manual/CI environments where network access and an API key are intentionally available.
