#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(LoginFunctional, UserCanRegisterAndLogin) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); } { ScopedIO io("alice\nsecret1\n"); EXPECT_TRUE(m.loginAsUser()); } EXPECT_EQ(m.getRole(),"USER"); }
TEST(LoginFunctional, WrongPasswordIsRejected) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); } ScopedIO io("alice\nwrong99\n"); EXPECT_FALSE(m.loginAsUser()); }
TEST(LoginFunctional, AdminLoginUsesAdminRole) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); EXPECT_EQ(m.getRole(),"ADMIN"); }
TEST(LoginFunctional, UserCredentialsCannotAuthenticateAsAdmin) { resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); } ScopedIO io("alice\nsecret1\n"); EXPECT_FALSE(m.loginAsAdmin()); }
