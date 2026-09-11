#include "test_common.h"
#include "weatherEnhancement.h"

using namespace testutil;

TEST(AuthenticationUnit, DefaultAdminLoginSucceeds) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nAdmin@123\n"); EXPECT_TRUE(m.loginAsAdmin()); EXPECT_EQ(m.getUsername(),"admin"); EXPECT_EQ(m.getRole(),"ADMIN");
}
TEST(AuthenticationUnit, WrongAdminPasswordFails) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("admin\nWrong123\n"); EXPECT_FALSE(m.loginAsAdmin()); EXPECT_TRUE(m.getRole().empty());
}
TEST(AuthenticationUnit, UserRegistrationStoresSHA256LengthHash) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser());
    const auto text=readText("users.csv"); EXPECT_NE(text.find("alice,"),std::string::npos); EXPECT_NE(text.find(",USER"),std::string::npos); EXPECT_EQ(text.find("secret1"),std::string::npos);
}
TEST(AuthenticationUnit, RegistrationRejectsShortPassword) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\n123\n123\n"); EXPECT_FALSE(m.registerUser()); EXPECT_FALSE(std::filesystem::exists("users.csv"));
}
TEST(AuthenticationUnit, RegistrationRejectsInvalidUsername) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("bad user\nsecret1\nsecret1\n"); EXPECT_FALSE(m.registerUser());
}
TEST(AuthenticationUnit, RegistrationRejectsPasswordMismatch) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\nsecret1\nsecret2\n"); EXPECT_FALSE(m.registerUser());
}
TEST(AuthenticationUnit, DuplicateUsernameIsRejected) {
    resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); }
    { ScopedIO io("alice\nsecret2\nsecret2\n"); EXPECT_FALSE(m.registerUser()); }
}
TEST(AuthenticationUnit, UserCannotLoginAsAdmin) {
    resetDataFiles(); WeatherEnhancementManager m; { ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser()); }
    ScopedIO io("alice\nsecret1\n"); EXPECT_FALSE(m.loginAsAdmin());
}
