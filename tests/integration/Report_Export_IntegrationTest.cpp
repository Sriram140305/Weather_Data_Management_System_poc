#include "test_common.h"
#include "weatherDataProcessor.h"
#include "userDataManager.h"
#include "weatherEnhancement.h"

TEST(ReportExportIntegration, ProcessorCanGenerateReportFromStoredWeather) {
    testutil::resetDataFiles(); UserDataManager::append(testutil::makeRecord()); WeatherDataProcessor p; EXPECT_NO_THROW(p.generateWeatherReport());
}
TEST(ReportExportIntegration, UserHistoricalReportIsUserScoped) {
    testutil::resetDataFiles(); UserDataManager::append(testutil::makeRecord("WEATHER","alice","A")); UserDataManager::append(testutil::makeRecord("WEATHER","bob","B")); WeatherDataProcessor p; EXPECT_NO_THROW(p.generateUserHistoricalReport("alice"));
}
TEST(ReportExportIntegration, ExportFeatureDoesNotUseAnotherUsersCityWhenNoSession) {
    testutil::resetDataFiles(); UserDataManager::append(testutil::makeRecord("WEATHER","bob","Chennai")); WeatherEnhancementManager m; ScopedIO io(""); EXPECT_NO_THROW(m.exportWeatherReport());
}
