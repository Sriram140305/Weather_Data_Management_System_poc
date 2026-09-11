#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(AlertsFunctional, AlertFeatureHandlesMissingAPIKey) { resetDataFiles(); unsetenv("WEATHER_API_KEY"); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.viewWeatherAlerts()); }
TEST(AlertsFunctional, AlertThresholdConfigurationPersists) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io("40\n10\n50\n45\n80\n"); ASSERT_NO_THROW(m.configureAlertThresholds()); EXPECT_NE(readText("alert_thresholds.conf").find("rain"),std::string::npos); }
TEST(AlertsFunctional, AlertThresholdFileCanBeReloadedBySubsequentOperation) { resetDataFiles(); writeText("alert_thresholds.conf","high_temperature 35\nlow_temperature 5\nrain 30\nwind 30\nhumidity 75\n"); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.viewWeatherAlerts()); }
TEST(AlertsFunctional, EmptyThresholdFileFallsBackSafely) { resetDataFiles(); writeText("alert_thresholds.conf",""); WeatherEnhancementManager m; ScopedIO io("Chennai\n"); EXPECT_NO_THROW(m.viewWeatherAlerts()); }
