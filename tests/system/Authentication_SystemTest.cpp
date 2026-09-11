#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(AuthenticationSystem, AdminLoginCreatesPersistentAdminAccount) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); EXPECT_NE(readText("users.csv").find("admin,"),std::string::npos); }
TEST(AuthenticationSystem, FullUserRegistrationLoginWorkflow) { resetDataFiles(); WeatherEnhancementManager m; {ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser());} {ScopedIO io("alice\nsecret1\n"); ASSERT_TRUE(m.loginAsUser());} EXPECT_EQ(m.getUsername(),"alice"); EXPECT_EQ(m.getRole(),"USER"); }
TEST(AuthenticationSystem, FailedLoginDoesNotAssignRole) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nwrong\n"); EXPECT_FALSE(m.loginAsAdmin()); EXPECT_TRUE(m.getRole().empty()); }
