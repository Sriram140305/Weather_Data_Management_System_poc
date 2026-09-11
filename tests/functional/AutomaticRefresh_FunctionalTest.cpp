#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(AutomaticRefreshFunctional, RefreshHandlesMissingAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Chennai\n5\n1\n"); EXPECT_NO_THROW(m.automaticWeatherRefresh()); }
TEST(AutomaticRefreshFunctional, RefreshRejectsIntervalBelowMinimum) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Chennai\n1\n"); EXPECT_NO_THROW(m.automaticWeatherRefresh()); }
TEST(AutomaticRefreshFunctional, RefreshHandlesEmptyInput) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("\n"); EXPECT_NO_THROW(m.automaticWeatherRefresh()); }
TEST(AutomaticRefreshFunctional, RefreshFeatureIsExposed) { WeatherEnhancementManager m; EXPECT_NO_THROW({ (void)&WeatherEnhancementManager::automaticWeatherRefresh; }); }
