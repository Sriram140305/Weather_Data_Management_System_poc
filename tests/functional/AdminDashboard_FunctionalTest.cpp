#include "test_common.h"
#include "weatherEnhancement.h"
#include "weatherStorageManager.h"
using namespace testutil;
TEST(AdminDashboardFunctional, APIConfigurationFeatureIsCallable) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io("TEST_KEY\n"); EXPECT_NO_THROW(m.configureApi()); }
TEST(AdminDashboardFunctional, ThresholdConfigurationFeatureIsCallable) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io("40\n10\n50\n45\n80\n"); EXPECT_NO_THROW(m.configureAlertThresholds()); }
TEST(AdminDashboardFunctional, SystemLogMonitoringFeatureIsCallable) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); } ScopedIO io(""); EXPECT_NO_THROW(m.monitorSystemLogs()); }
TEST(AdminDashboardFunctional, StoredDataCanBeViewedByAdminBoundary) { resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","Chennai")); WeatherStorageManager s; ScopedIO io(""); EXPECT_NO_THROW(s.viewAllUserData()); EXPECT_NE(io.output().find("alice"),std::string::npos); }
