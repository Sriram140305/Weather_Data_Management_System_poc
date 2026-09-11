#include "test_common.h"
#include "location.h"
using namespace testutil;
TEST(CompareCitiesFunctional, CompareFeatureHandlesMissingAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); LocationManager m("alice"); ScopedIO io("Chennai\nMadurai\n"); EXPECT_NO_THROW(m.compareCities()); }
TEST(CompareCitiesFunctional, CompareFeatureHandlesIncompleteInput) { resetDataFiles(); LocationManager m("alice"); ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.compareCities()); }
TEST(CompareCitiesFunctional, CompareFeatureDoesNotModifyFavorites) { resetDataFiles(); ASSERT_TRUE(UserDataManager::addFavorite("alice",testutil::makeRecord("FAVORITE","alice","Chennai"))); LocationManager m("alice"); ScopedIO io("Chennai\nMadurai\n"); EXPECT_NO_THROW(m.compareCities()); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u); }
