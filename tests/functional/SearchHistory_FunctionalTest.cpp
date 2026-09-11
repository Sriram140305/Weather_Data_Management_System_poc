#include "test_common.h"
#include "userDataManager.h"
using namespace testutil;
TEST(SearchHistoryFunctional, SearchIsRecorded) { resetDataFiles(); ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","Chennai"))); EXPECT_EQ(UserDataManager::searchCount("alice","Chennai"),1); }
TEST(SearchHistoryFunctional, SearchCountIsCaseInsensitive) { resetDataFiles(); ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","Chennai"))); EXPECT_EQ(UserDataManager::searchCount("alice","chennai"),1); }
TEST(SearchHistoryFunctional, TenSearchesAreRetained) { resetDataFiles(); for(int i=0;i<10;i++) ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","C"+std::to_string(i)))); EXPECT_EQ(UserDataManager::recentSearches("alice").size(),10u); }
TEST(SearchHistoryFunctional, EleventhSearchDropsOldest) { resetDataFiles(); for(int i=0;i<11;i++) ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","C"+std::to_string(i)))); auto rows=UserDataManager::recentSearches("alice"); ASSERT_EQ(rows.size(),10u); EXPECT_EQ(rows.front().city,"C1"); }
TEST(SearchHistoryFunctional, DifferentUsersHaveIndependentHistory) { resetDataFiles(); ASSERT_TRUE(UserDataManager::addSearch("alice",makeRecord("SEARCH","alice","C"))); ASSERT_TRUE(UserDataManager::addSearch("bob",makeRecord("SEARCH","bob","C"))); EXPECT_EQ(UserDataManager::recentSearches("alice").size(),1u); EXPECT_EQ(UserDataManager::recentSearches("bob").size(),1u); }
