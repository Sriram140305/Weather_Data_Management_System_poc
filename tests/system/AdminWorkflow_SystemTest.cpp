#include "test_common.h"
#include "weatherEnhancement.h"
#include "weatherStorageManager.h"
using namespace testutil;
TEST(AdminWorkflowSystem, AdminCanConfigureAPI) { resetDataFiles(); WeatherEnhancementManager m; {ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin());} {ScopedIO io("KEY\n"); EXPECT_NO_THROW(m.configureApi());} EXPECT_TRUE(std::filesystem::exists("api_config.conf")); }
TEST(AdminWorkflowSystem, AdminCanConfigureThresholds) { resetDataFiles(); WeatherEnhancementManager m; {ScopedIO login("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin());} ScopedIO io("40\n10\n50\n45\n80\n"); EXPECT_NO_THROW(m.configureAlertThresholds()); EXPECT_TRUE(std::filesystem::exists("alert_thresholds.conf")); }
TEST(AdminWorkflowSystem, AdminCanViewAllUserWeather) { resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","alice","A")); UserDataManager::append(makeRecord("WEATHER","bob","B")); WeatherStorageManager s; ScopedIO io(""); s.viewAllUserData(); EXPECT_NE(io.output().find("alice"),std::string::npos); EXPECT_NE(io.output().find("bob"),std::string::npos); }
