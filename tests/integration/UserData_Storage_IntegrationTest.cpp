#include "test_common.h"
#include "userDataManager.h"
#include "weatherStorageManager.h"

TEST(UserDataStorageIntegration, WeatherStorageWritesThroughUserDataManager) {
    testutil::resetDataFiles(); WeatherStorageManager s; s.setCurrentUser("alice"); s.saveWeatherData("Chennai",30,70,1008,"TN","India",13.08,80.27); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].type,"WEATHER");
}
TEST(UserDataStorageIntegration, FavoriteSearchAndWeatherRecordsCanCoexist) {
    testutil::resetDataFiles(); ASSERT_TRUE(UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","Chennai"))); ASSERT_TRUE(UserDataManager::addSearch("alice",testutil::makeRecord("SEARCH","alice","Madurai"))); ASSERT_TRUE(UserDataManager::append(testutil::makeRecord("WEATHER","alice","Coimbatore"))); EXPECT_EQ(UserDataManager::loadAll().size(),3u);
}
TEST(UserDataStorageIntegration, UserDataPersistsAfterManagerReconstruction) {
    testutil::resetDataFiles(); { WeatherStorageManager s; s.setCurrentUser("alice"); s.saveWeatherData("A",1,2,3); } WeatherStorageManager s2; s2.setCurrentUser("alice"); EXPECT_EQ(UserDataManager::favorites("alice").size(),0u); EXPECT_EQ(UserDataManager::loadAll().size(),1u);
}
TEST(UserDataStorageIntegration, ClearUserWeatherDoesNotDeleteFavorites) {
    testutil::resetDataFiles(); UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","Fav")); UserDataManager::append(testutil::makeRecord("WEATHER","alice","Weather")); WeatherStorageManager s; s.setCurrentUser("alice"); s.clearStorage(); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].type,"FAVORITE");
}
