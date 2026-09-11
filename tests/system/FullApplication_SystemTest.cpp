#include "test_common.h"
#include <cstdlib>
#include <string>

#ifndef WEATHER_SYSTEM_EXE
#define WEATHER_SYSTEM_EXE "weather_system"
#endif

static int runApp(const std::string& input) {
    const std::string in="system_test_input.txt"; const std::string out="system_test_output.txt";
    testutil::writeText(in,input);
    std::string cmd=std::string("\"")+WEATHER_SYSTEM_EXE+"\" < \""+in+"\" > \""+out+"\" 2>&1";
    int rc=std::system(cmd.c_str());
    testutil::removeFile(in); return rc;
}

TEST(FullApplicationSystem, LaunchAndExit) { testutil::resetDataFiles(); EXPECT_EQ(runApp("0\n"),0); }
TEST(FullApplicationSystem, InvalidMainMenuChoiceDoesNotCrash) { testutil::resetDataFiles(); EXPECT_EQ(runApp("99\n0\n"),0); }
TEST(FullApplicationSystem, AdminLoginCanEnterDashboardAndLogout) { testutil::resetDataFiles(); EXPECT_EQ(runApp("3\nadmin\nAdmin@123\n0\n0\n"),0); }
TEST(FullApplicationSystem, UserRegistrationThenExitCompletes) { testutil::resetDataFiles(); EXPECT_EQ(runApp("2\nalice\nsecret1\nsecret1\n0\n"),0); }
