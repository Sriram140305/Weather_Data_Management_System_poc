#include "test_common.h"
#include "weather.h"
#include "weatherStorageManager.h"
using namespace testutil;
TEST(WeatherFunctional, CurrentWeatherRequiresAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); EXPECT_FALSE(isWeatherDataAvailable()); EXPECT_NE(fetchWeather("Chennai").find("not configured"),std::string::npos); }
TEST(WeatherFunctional, WeatherSnapshotCanBeStoredAfterAcquisitionBoundary) { resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveCurrentWeather("Chennai",30,70,1008,"TN","India",13.08,80.27); EXPECT_EQ(UserDataManager::loadAll().size(),1u); }
TEST(WeatherFunctional, StoredWeatherIsUserScoped) { resetDataFiles(); WeatherStorageManager a; a.setCurrentUser("alice"); a.saveCurrentWeather("A",30,70,1008); WeatherStorageManager b; b.setCurrentUser("bob"); b.saveCurrentWeather("B",20,60,1005); EXPECT_EQ(UserDataManager::loadAll().size(),2u); }
TEST(WeatherFunctional, WeatherFailureDoesNotCreateWeatherRecord) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); fetchWeather("Madurai"); EXPECT_TRUE(UserDataManager::loadAll().empty()); }
