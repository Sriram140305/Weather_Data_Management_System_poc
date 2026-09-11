#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(ForecastFunctional, ForecastFeatureHandlesMissingAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.showSevenDayForecast()); }
TEST(ForecastFunctional, ForecastFeatureDoesNotCrashWithEmptyInput) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("\n"); EXPECT_NO_THROW(m.showSevenDayForecast()); }
TEST(ForecastFunctional, ForecastFailureIsControlled) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Madurai\n"); EXPECT_NO_THROW(m.showSevenDayForecast()); }
TEST(ForecastFunctional, SevenDayFeatureIsExposedByManager) { WeatherEnhancementManager m; EXPECT_NO_THROW({ (void)&WeatherEnhancementManager::showSevenDayForecast; }); }
