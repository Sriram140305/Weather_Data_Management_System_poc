#include "test_common.h"
#include "weatherEnhancement.h"
#include "userDataManager.h"

TEST(AuthenticationDataIntegration, RegistrationThenLoginWorks) {
    testutil::resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); } { ScopedIO io("alice\nsecret1\n"); ASSERT_TRUE(m.loginAsUser()); } EXPECT_EQ(m.getUsername(),"alice"); EXPECT_EQ(m.getRole(),"USER");
}
TEST(AuthenticationDataIntegration, RegisteredPasswordIsNotStoredAsPlaintext) {
    testutil::resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); EXPECT_EQ(readText("users.csv").find("secret1"),std::string::npos);
}
TEST(AuthenticationDataIntegration, AdminAccountIsCreatedAndPersisted) {
    testutil::resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nAdmin@123\n"); ASSERT_TRUE(m.loginAsAdmin()); EXPECT_NE(readText("users.csv").find("admin,"),std::string::npos);
}
