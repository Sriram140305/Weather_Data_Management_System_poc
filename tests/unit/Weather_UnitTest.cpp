#include "test_common.h"
#include "weather.h"

TEST(WeatherUnit, InitialStateIsConsistent) {
    EXPECT_FLOAT_EQ(getLastTemperature(),0.0f); EXPECT_FLOAT_EQ(getLastHumidity(),0.0f); EXPECT_FLOAT_EQ(getLastWindSpeed(),0.0f); EXPECT_FLOAT_EQ(getLastPressure(),0.0f);
}
TEST(WeatherUnit, InitialConditionAndLocationAreEmpty) { EXPECT_TRUE(getLastCondition().empty()); EXPECT_TRUE(getLastLocation().empty()); }
TEST(WeatherUnit, FetchWithoutAPIKeyFailsCleanly) {
    unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); std::string result=fetchWeather("Chennai"); EXPECT_NE(result.find("not configured"),std::string::npos); EXPECT_FALSE(isWeatherDataAvailable());
}
TEST(WeatherUnit, MissingAPIKeyDoesNotReportValidWeather) { unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); fetchWeather("Madurai"); EXPECT_FALSE(isWeatherDataAvailable()); }
TEST(WeatherUnit, GettersRemainCallableAfterFailure) { unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); fetchWeather("X"); EXPECT_NO_THROW({ (void)getLastTemperature(); (void)getLastHumidity(); (void)getLastPressure(); }); }
