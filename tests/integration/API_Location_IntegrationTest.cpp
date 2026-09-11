#include "test_common.h"
#include "APIManager.h"
#include "location.h"

TEST(APILocationIntegration, APIKeyConfigurationFeedsLocationManagerDependency) {
    testutil::resetDataFiles(); setenv("WEATHER_API_KEY","KEY",1); APIManager api; EXPECT_EQ(api.getApiKey(),"KEY"); LocationManager manager("alice"); manager.loadSearchHistory(); EXPECT_TRUE(std::filesystem::exists("user_data.csv")); unsetenv("WEATHER_API_KEY");
}
TEST(APILocationIntegration, SearchEndpointURLIsCompatibleWithLocationQueries) {
    testutil::resetDataFiles(); setenv("WEATHER_API_KEY","KEY",1); APIManager api; EXPECT_NE(api.buildSearchURL("Madurai").find("q=Madurai"),std::string::npos); unsetenv("WEATHER_API_KEY");
}
TEST(APILocationIntegration, SearchWithoutKeyProducesControlledFailure) { testutil::resetDataFiles(); unsetenv("WEATHER_API_KEY"); APIManager api; EXPECT_FALSE(api.searchLocations("Madurai").empty()); }
