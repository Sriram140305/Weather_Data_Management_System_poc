#include "test_common.h"
#include "weatherEnhancement.h"
#include "userDataManager.h"
using namespace testutil;
TEST(ExportFunctional, ExportWithoutSessionFailsGracefully) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.exportWeatherReport()); }
TEST(ExportFunctional, ExportFeatureIsCallableAfterStoredWeatherExists) { resetDataFiles(); UserDataManager::append(makeRecord()); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.exportWeatherReport()); }
TEST(ExportFunctional, ExistingReportFileIsNotDeletedByFeatureAttempt) { resetDataFiles(); writeText("weather_report.csv","existing\n"); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.exportWeatherReport()); EXPECT_NE(readText("weather_report.csv").find("existing"),std::string::npos); }
TEST(ExportFunctional, StoredLocationDataRemainsConsistent) { resetDataFiles(); auto r=makeRecord("WEATHER","alice","Madurai"); ASSERT_TRUE(UserDataManager::append(r)); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].city,"Madurai"); }
