#include "test_common.h"
#include "location.h"
#include "userDataManager.h"

using namespace testutil;

TEST(LocationUnit, ConstructorAcceptsEmptyUser) { LocationManager m; SUCCEED(); }
TEST(LocationUnit, SetCurrentUserCanBeCalledRepeatedly) { LocationManager m("alice"); m.setCurrentUser("bob"); m.setCurrentUser("carol"); SUCCEED(); }
TEST(LocationUnit, LoadingFavoritesEnsuresStorageFile) {
    resetDataFiles(); LocationManager m("alice"); m.loadFavoriteCities();
    EXPECT_TRUE(std::filesystem::exists("user_data.csv"));
    EXPECT_GE(lineCount("user_data.csv"), 1u);
}
TEST(LocationUnit, LoadingSearchHistoryEnsuresStorageFile) {
    resetDataFiles(); LocationManager m("alice"); m.loadSearchHistory();
    EXPECT_TRUE(std::filesystem::exists("user_data.csv"));
}
TEST(LocationUnit, FavoriteAndSearchStateIsUserScoped) {
    resetDataFiles();
    auto r=makeRecord("FAVORITE","alice","Madurai"); ASSERT_TRUE(UserDataManager::addFavorite("alice",r));
    LocationManager m("bob"); m.loadFavoriteCities();
    EXPECT_TRUE(UserDataManager::favorites("bob").empty()); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u);
}
TEST(LocationUnit, SearchHistoryKeepsLatestTen) {
    resetDataFiles();
    for(int i=0;i<12;++i){ auto r=makeRecord("SEARCH","alice","City"+std::to_string(i)); ASSERT_TRUE(UserDataManager::addSearch("alice",r)); }
    EXPECT_EQ(UserDataManager::recentSearches("alice").size(),10u);
    EXPECT_EQ(UserDataManager::recentSearches("alice").front().city,"City2");
}
