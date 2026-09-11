#include "test_common.h"
#include "weatherEnhancement.h"
#include "userDataManager.h"
using namespace testutil;
TEST(UserWorkflowSystem, RegisterLoginAndPersistFavorite) { resetDataFiles(); WeatherEnhancementManager m; {ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser());} {ScopedIO io("alice\nsecret1\n"); ASSERT_TRUE(m.loginAsUser());} ASSERT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","Chennai"))); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u); }
TEST(UserWorkflowSystem, UserHistoryIsIndependentFromOtherUser) { resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","A")); UserDataManager::append(makeRecord("WEATHER","bob","B")); EXPECT_EQ(UserDataManager::favorites("alice").size(),0u); EXPECT_EQ(UserDataManager::loadAll().size(),2u); }
TEST(UserWorkflowSystem, UserCanAccumulateSearchesAndWeatherData) { resetDataFiles(); for(int i=0;i<3;i++) ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","C"+std::to_string(i)))); ASSERT_TRUE(UserDataManager::append(makeRecord("WEATHER","alice","C0"))); EXPECT_EQ(UserDataManager::recentSearches("alice").size(),3u); EXPECT_EQ(UserDataManager::loadAll().size(),4u); }
