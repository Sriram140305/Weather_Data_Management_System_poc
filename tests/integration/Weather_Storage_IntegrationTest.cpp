#include "test_common.h"
#include "weatherStorageManager.h"
#include "userDataManager.h"

TEST(WeatherStorageIntegration, CurrentWeatherContainsExactSelectedLocation) {
    testutil::resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveCurrentWeather("Madurai",33,55,1009,"Tamil Nadu","India",9.9252,78.1198); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].city,"Madurai"); EXPECT_EQ(rows[0].region,"Tamil Nadu");
}
TEST(WeatherStorageIntegration, MultipleWeatherSnapshotsRemainOrdered) {
    testutil::resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveWeatherData("A",20,40,1000); s.saveWeatherData("B",21,41,1001); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),2u); EXPECT_EQ(rows[0].city,"A"); EXPECT_EQ(rows[1].city,"B");
}
TEST(WeatherStorageIntegration, AdminCanReadAllUsersWeatherRows) {
    testutil::resetDataFiles(); UserDataManager::append(testutil::makeRecord("WEATHER","alice","A")); UserDataManager::append(testutil::makeRecord("WEATHER","bob","B")); ScopedIO io(""); WeatherStorageManager s; s.viewAllUserData(); EXPECT_NE(io.output().find("alice"),std::string::npos); EXPECT_NE(io.output().find("bob"),std::string::npos);
}
