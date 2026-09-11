#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(InsightsFunctional, InsightsHandlesMissingAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.showWeatherInsights()); }
TEST(InsightsFunctional, InsightsFeatureDoesNotCrashWithEmptyInput) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("\n"); EXPECT_NO_THROW(m.showWeatherInsights()); }
TEST(InsightsFunctional, HistoricalDataCanExistBeforeInsights) { resetDataFiles(); UserDataManager::append(testutil::makeRecord("WEATHER","alice","Chennai")); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.showWeatherInsights()); }
