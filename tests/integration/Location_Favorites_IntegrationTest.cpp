#include "test_common.h"
#include "location.h"
#include "userDataManager.h"

TEST(LocationFavoritesIntegration, SelectedUserOwnsFavorites) {
    testutil::resetDataFiles(); auto r=testutil::makeRecord("FAVORITE","alice","Madurai"); ASSERT_TRUE(UserDataManager::addFavorite("alice",r)); LocationManager m("alice"); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u); EXPECT_EQ(UserDataManager::favorites("bob").size(),0u);
}
TEST(LocationFavoritesIntegration, FiveFavoriteLimitIsEnforcedAcrossManagerBoundary) {
    testutil::resetDataFiles(); for(int i=0;i<5;i++) ASSERT_TRUE(UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","City"+std::to_string(i)))); EXPECT_FALSE(UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","City5")));
}
TEST(LocationFavoritesIntegration, RemovingFavoriteUpdatesPersistentHistory) {
    testutil::resetDataFiles(); ASSERT_TRUE(UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","Chennai"))); LocationManager m("alice"); ASSERT_TRUE(UserDataManager::removeFavorite("alice","Chennai")); EXPECT_TRUE(UserDataManager::favorites("alice").empty());
}
