#include "test_common.h"
#include "APIManager.h"
#include "weather.h"

TEST(APIWeatherIntegration, WeatherUsesAPIManagerURLBuilder) {
    testutil::resetDataFiles(); setenv("WEATHER_API_KEY","KEY",1); APIManager api; EXPECT_NE(api.buildURL("Chennai").find("forecast"),std::string::npos);
    unsetenv("WEATHER_API_KEY");
}
TEST(APIWeatherIntegration, MissingKeyPreventsNetworkRequest) { testutil::resetDataFiles(); unsetenv("WEATHER_API_KEY"); auto result=fetchWeather("Chennai"); EXPECT_NE(result.find("not configured"),std::string::npos); }
TEST(APIWeatherIntegration, FailedFetchLeavesAvailabilityFalse) { testutil::resetDataFiles(); unsetenv("WEATHER_API_KEY"); fetchWeather("Chennai"); EXPECT_FALSE(isWeatherDataAvailable()); }
