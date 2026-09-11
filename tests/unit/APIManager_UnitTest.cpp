#include "test_common.h"
#include "APIManager.h"

using namespace testutil;

TEST(APIManagerUnit, MissingKeyReturnsEmptyWhenNoConfiguration) {
    resetDataFiles(); unsetenv("WEATHER_API_KEY");
    APIManager api; EXPECT_TRUE(api.getApiKey().empty());
}
TEST(APIManagerUnit, ReadsPersistedConfiguration) {
    resetDataFiles(); writeText("api_config.conf", "WEATHER_API_KEY=TEST_KEY_123\n");
    unsetenv("WEATHER_API_KEY"); APIManager api; EXPECT_EQ(api.getApiKey(), "TEST_KEY_123");
}
TEST(APIManagerUnit, EnvironmentKeyHasPriority) {
    resetDataFiles(); writeText("api_config.conf", "WEATHER_API_KEY=FILE_KEY\n"); setenv("WEATHER_API_KEY", "ENV_KEY", 1);
    APIManager api; EXPECT_EQ(api.getApiKey(), "ENV_KEY"); unsetenv("WEATHER_API_KEY");
}
TEST(APIManagerUnit, CurrentWeatherURLContainsEncodedQueryAndKey) {
    resetDataFiles(); setenv("WEATHER_API_KEY", "KEY", 1); APIManager api;
    const auto url=api.buildURL("New York");
    EXPECT_NE(url.find("key=KEY"), std::string::npos); EXPECT_NE(url.find("q=New%20York"), std::string::npos);
    unsetenv("WEATHER_API_KEY");
}
TEST(APIManagerUnit, SearchURLEncodesSpecialCharacters) {
    resetDataFiles(); setenv("WEATHER_API_KEY", "KEY", 1); APIManager api;
    const auto url=api.buildSearchURL("C++ & Co");
    EXPECT_NE(url.find("C%2B%2B%20%26%20Co"), std::string::npos); unsetenv("WEATHER_API_KEY");
}
TEST(APIManagerUnit, EmptyQueryStillProducesWellFormedURL) {
    resetDataFiles(); setenv("WEATHER_API_KEY", "KEY", 1); APIManager api;
    EXPECT_NE(api.buildURL("" ).find("q="), std::string::npos); unsetenv("WEATHER_API_KEY");
}
TEST(APIManagerUnit, SearchLocationsReturnsErrorWithoutKey) {
    resetDataFiles(); unsetenv("WEATHER_API_KEY"); APIManager api;
    EXPECT_NE(api.searchLocations("Chennai").find("API"), std::string::npos);
}
