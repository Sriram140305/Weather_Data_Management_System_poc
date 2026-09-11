#include "test_common.h"
#include "userDataManager.h"
#include "location.h"
using namespace testutil;
TEST(FavoriteCitiesFunctional, AddFavoriteSucceeds) { resetDataFiles(); EXPECT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","Chennai"))); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u); }
TEST(FavoriteCitiesFunctional, DuplicateFavoriteFails) { resetDataFiles(); auto r=makeRecord("FAVORITE","alice","Chennai"); ASSERT_TRUE(UserDataManager::addFavorite("alice",r)); EXPECT_FALSE(UserDataManager::addFavorite("alice",r)); }
TEST(FavoriteCitiesFunctional, FifthFavoriteSucceeds) { resetDataFiles(); for(int i=0;i<4;i++) ASSERT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","C"+std::to_string(i)))); EXPECT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","C4"))); }
TEST(FavoriteCitiesFunctional, SixthFavoriteFails) { resetDataFiles(); for(int i=0;i<5;i++) ASSERT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","C"+std::to_string(i)))); EXPECT_FALSE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","C5"))); }
TEST(FavoriteCitiesFunctional, RemoveFavoriteSucceeds) { resetDataFiles(); ASSERT_TRUE(UserDataManager::addFavorite("alice",makeRecord("FAVORITE","alice","Chennai"))); EXPECT_TRUE(UserDataManager::removeFavorite("alice","Chennai")); }
