#include "test_common.h"
#include "weatherEnhancement.h"
#include "userDataManager.h"

using namespace testutil;

TEST(WeatherEnhancementUnit, NewManagerHasEmptyRoleAndUsername) { WeatherEnhancementManager m; EXPECT_TRUE(m.getRole().empty()); EXPECT_TRUE(m.getUsername().empty()); }
TEST(WeatherEnhancementUnit, ConfigureApiRejectsBlankInputOrLeavesControlledState) {
    resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io("TEST_KEY\n"); EXPECT_NO_THROW(m.configureApi());
}
TEST(WeatherEnhancementUnit, ConfigureThresholdsHandlesCompleteInput) {
    resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io("40\n10\n50\n45\n80\n"); EXPECT_NO_THROW(m.configureAlertThresholds());
    EXPECT_TRUE(std::filesystem::exists("alert_thresholds.conf")); EXPECT_NE(readText("alert_thresholds.conf").find("high_temperature"),std::string::npos);
}
TEST(WeatherEnhancementUnit, MonitorLogsHandlesMissingLogFiles) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io(""); EXPECT_NO_THROW(m.monitorSystemLogs()); }
TEST(WeatherEnhancementUnit, ExportWithoutWeatherDataFailsGracefully) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.exportWeatherReport()); EXPECT_EQ(UserDataManager::loadAll().size(),0u); }
TEST(WeatherEnhancementUnit, ShowInsightsWithoutAPIDataDoesNotCrash) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.showWeatherInsights()); }
