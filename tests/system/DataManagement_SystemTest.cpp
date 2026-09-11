#include "test_common.h"
#include "userDataManager.h"
#include "weatherStorageManager.h"
using namespace testutil;
TEST(DataManagementSystem, BackupAndClearAreRecoverableOperations) { resetDataFiles(); UserDataManager::append(makeRecord()); WeatherStorageManager s; ScopedIO io(""); s.backupData(); ASSERT_TRUE(std::filesystem::exists("user_data_backup.csv")); s.clearAllStorage(); EXPECT_TRUE(UserDataManager::loadAll().empty()); EXPECT_FALSE(readText("user_data_backup.csv").empty()); }
TEST(DataManagementSystem, NonWeatherRecordsSurviveGlobalWeatherClear) { resetDataFiles(); UserDataManager::append(makeRecord("FAVORITE","alice","Fav")); UserDataManager::append(makeRecord("WEATHER","alice","Weather")); WeatherStorageManager s; s.clearAllStorage(); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].type,"FAVORITE"); }
TEST(DataManagementSystem, AdminTableContainsAllUsers) { resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","A")); UserDataManager::append(makeRecord("WEATHER","bob","B")); ScopedIO io(""); UserDataManager::viewAllForAdmin(); EXPECT_NE(io.output().find("alice"),std::string::npos); EXPECT_NE(io.output().find("bob"),std::string::npos); }
