#include "test_common.h"
#include "weatherStorageManager.h"
#include "userDataManager.h"

using namespace testutil;

TEST(WeatherStorageUnit, SaveWithoutActiveUserDoesNotPersist) {
    resetDataFiles(); WeatherStorageManager s; ScopedIO io(""); s.saveWeatherData("Chennai",30,70,1008); EXPECT_TRUE(UserDataManager::loadAll().empty()); EXPECT_NE(io.output().find("No active user"),std::string::npos);
}
TEST(WeatherStorageUnit, SaveWeatherPersistsLocationAndMetrics) {
    resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveWeatherData("Madurai",31.2f,65.0f,1007.4f,"TN","India",9.9252,78.1198);
    auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].city,"Madurai"); EXPECT_EQ(rows[0].region,"TN"); EXPECT_EQ(rows[0].country,"India"); EXPECT_EQ(rows[0].value1,"31.200001");
}
TEST(WeatherStorageUnit, SaveCurrentWeatherDelegatesToSaveWeatherData) {
    resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveCurrentWeather("Chennai",30,70,1008); EXPECT_EQ(UserDataManager::loadAll().size(),1u);
}
TEST(WeatherStorageUnit, ClearStorageRemovesOnlyCurrentUsersWeather) {
    resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","A")); UserDataManager::append(makeRecord("FAVORITE","alice","F")); UserDataManager::append(makeRecord("WEATHER","bob","B"));
    WeatherStorageManager s; s.setCurrentUser("alice"); s.clearStorage(); auto rows=UserDataManager::loadAll();
    ASSERT_EQ(rows.size(),2u); EXPECT_EQ(rows[0].type,"FAVORITE"); EXPECT_EQ(rows[1].username,"bob");
}
TEST(WeatherStorageUnit, ClearAllStoragePreservesNonWeatherRecords) {
    resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","A")); UserDataManager::append(makeRecord("FAVORITE","bob","F"));
    WeatherStorageManager s; s.clearAllStorage(); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].type,"FAVORITE");
}
TEST(WeatherStorageUnit, BackupCopiesStoredCSV) {
    resetDataFiles(); UserDataManager::append(makeRecord()); WeatherStorageManager s; ScopedIO io(""); s.backupData();
    ASSERT_TRUE(std::filesystem::exists("user_data_backup.csv")); EXPECT_EQ(readText("user_data_backup.csv"),readText("user_data.csv"));
}
