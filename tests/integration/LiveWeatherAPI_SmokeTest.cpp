#include "test_common.h"
#include "APIManager.h"
#include "weather.h"

TEST(LiveWeatherAPISmoke, SearchEndpointReturnsDataWhenConfigured) {
    const char* key = std::getenv("WEATHER_API_KEY");
    if (key == nullptr || std::string(key).empty()) GTEST_SKIP() << "WEATHER_API_KEY is not configured";
    APIManager api; const std::string response=api.searchLocations("Chennai");
    ASSERT_FALSE(response.empty()); EXPECT_NE(response.front(),'\0');
}
TEST(LiveWeatherAPISmoke, CurrentWeatherEndpointCanReturnARealSnapshot) {
    const char* key = std::getenv("WEATHER_API_KEY");
    if (key == nullptr || std::string(key).empty()) GTEST_SKIP() << "WEATHER_API_KEY is not configured";
    const std::string response=fetchWeather("Chennai");
    EXPECT_TRUE(isWeatherDataAvailable()) << response;
    if (isWeatherDataAvailable()) { EXPECT_FALSE(getLastLocation().empty()); EXPECT_GE(getLastHumidity(),0.0f); EXPECT_GE(getLastPressure(),0.0f); }
}
