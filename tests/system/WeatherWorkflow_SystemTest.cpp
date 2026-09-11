#include "test_common.h"
#include "weather.h"
#include "weatherStorageManager.h"
using namespace testutil;
TEST(WeatherWorkflowSystem, APIFailureStopsBeforeStorage) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); remove("api_config.conf"); auto result=fetchWeather("Chennai"); EXPECT_NE(result.find("not configured"),std::string::npos); EXPECT_TRUE(UserDataManager::loadAll().empty()); }
TEST(WeatherWorkflowSystem, StoredSnapshotCanBeDisplayedForUser) { resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveCurrentWeather("Madurai",31,65,1007,"TN","India",9.9,78.1); ScopedIO io(""); s.viewStoredData(); EXPECT_NE(io.output().find("Madurai"),std::string::npos); }
TEST(WeatherWorkflowSystem, ClearWorkflowRemovesWeatherHistory) { resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveCurrentWeather("A",1,2,3); ASSERT_EQ(UserDataManager::loadAll().size(),1u); s.clearStorage(); EXPECT_TRUE(UserDataManager::loadAll().empty()); }
