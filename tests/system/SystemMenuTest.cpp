#include "test_common.h"
#include <fstream>
#include <string>

#ifndef WEATHER_PROJECT_SOURCE_DIR
#define WEATHER_PROJECT_SOURCE_DIR "."
#endif

TEST(SystemMenu, UserAutomaticRefreshOptionExists) {
    std::ifstream f(std::string(WEATHER_PROJECT_SOURCE_DIR)+"/main.cpp"); ASSERT_TRUE(f.is_open()); std::string text((std::istreambuf_iterator<char>(f)),{}); EXPECT_NE(text.find("case 15:"),std::string::npos); EXPECT_NE(text.find("automaticWeatherRefresh"),std::string::npos);
}
TEST(SystemMenu, AdminAutomaticRefreshOptionExists) {
    std::ifstream f(std::string(WEATHER_PROJECT_SOURCE_DIR)+"/main.cpp"); ASSERT_TRUE(f.is_open()); std::string text((std::istreambuf_iterator<char>(f)),{}); EXPECT_NE(text.find("case 7:"),std::string::npos); EXPECT_NE(text.find("automaticWeatherRefresh"),std::string::npos);
}
TEST(SystemMenu, AuthMenuContainsRequiredRoles) {
    std::ifstream f(std::string(WEATHER_PROJECT_SOURCE_DIR)+"/main.cpp"); ASSERT_TRUE(f.is_open()); std::string text((std::istreambuf_iterator<char>(f)),{}); EXPECT_NE(text.find("User Login"),std::string::npos); EXPECT_NE(text.find("User Registration"),std::string::npos); EXPECT_NE(text.find("Admin Login"),std::string::npos);
}
TEST(SystemMenu, LogoutCasesExist) {
    std::ifstream f(std::string(WEATHER_PROJECT_SOURCE_DIR)+"/main.cpp"); ASSERT_TRUE(f.is_open()); std::string text((std::istreambuf_iterator<char>(f)),{}); EXPECT_NE(text.find("Logout"),std::string::npos);
}
