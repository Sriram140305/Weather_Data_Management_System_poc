#include "test_common.h"
#include "location.h"
using namespace testutil;
TEST(TravelFunctional, TravelRecommendationFeatureCanBeInvoked) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); LocationManager m("alice"); ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.travelRecommendation()); }
TEST(TravelFunctional, TravelRecommendationHandlesEmptyInput) { resetDataFiles(); LocationManager m("alice"); ScopedIO io("\n"); EXPECT_NO_THROW(m.travelRecommendation()); }
TEST(TravelFunctional, TravelFeatureHasNoEffectOnOtherUsersData) { resetDataFiles(); UserDataManager::addFavorite("bob",testutil::makeRecord("FAVORITE","bob","Chennai")); LocationManager m("alice"); ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.travelRecommendation()); EXPECT_EQ(UserDataManager::favorites("bob").size(),1u); }
