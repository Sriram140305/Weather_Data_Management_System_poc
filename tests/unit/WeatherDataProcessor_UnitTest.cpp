#include "test_common.h"
#include "weatherDataProcessor.h"
#include "userDataManager.h"

using namespace testutil;

TEST(WeatherDataProcessorUnit, EmptyStorageLoadsWithoutCrash) { resetDataFiles(); WeatherDataProcessor p; EXPECT_NO_THROW(p.loadStoredWeatherData()); }
TEST(WeatherDataProcessorUnit, ReportWithNoDataDoesNotCrash) { resetDataFiles(); WeatherDataProcessor p; EXPECT_NO_THROW(p.generateWeatherReport()); }
TEST(WeatherDataProcessorUnit, UserHistoricalReportWithNoRecordsDoesNotCrash) { resetDataFiles(); WeatherDataProcessor p; EXPECT_NO_THROW(p.generateUserHistoricalReport("alice")); }
TEST(WeatherDataProcessorUnit, HistoricalReportIgnoresOtherUsers) {
    resetDataFiles(); UserDataManager::append(makeRecord("WEATHER","bob","Chennai")); WeatherDataProcessor p; EXPECT_NO_THROW(p.generateUserHistoricalReport("alice"));
}
TEST(WeatherDataProcessorUnit, HistoricalReportHandlesMultipleRecords) {
    resetDataFiles(); auto r=makeRecord("WEATHER","alice","Chennai"); ASSERT_TRUE(UserDataManager::append(r)); r.value1="32"; r.value2="75"; ASSERT_TRUE(UserDataManager::append(r));
    WeatherDataProcessor p; EXPECT_NO_THROW(p.generateUserHistoricalReport("alice"));
}
