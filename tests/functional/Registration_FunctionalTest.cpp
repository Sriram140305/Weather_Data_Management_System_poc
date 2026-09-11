#include "test_common.h"
#include "weatherEnhancement.h"
using namespace testutil;
TEST(RegistrationFunctional, ValidAccountIsCreated) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\nsecret1\nsecret1\n"); EXPECT_TRUE(m.registerUser()); }
TEST(RegistrationFunctional, DuplicateAccountIsRejected) { resetDataFiles(); WeatherEnhancementManager m; {ScopedIO io("alice\nsecret1\nsecret1\n"); ASSERT_TRUE(m.registerUser());} ScopedIO io("alice\nsecret2\nsecret2\n"); EXPECT_FALSE(m.registerUser()); }
TEST(RegistrationFunctional, MismatchedPasswordsAreRejected) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\nsecret1\nsecret2\n"); EXPECT_FALSE(m.registerUser()); }
TEST(RegistrationFunctional, InvalidUsernameIsRejected) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice user\nsecret1\nsecret1\n"); EXPECT_FALSE(m.registerUser()); }
TEST(RegistrationFunctional, ShortPasswordIsRejected) { resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("alice\n12345\n12345\n"); EXPECT_FALSE(m.registerUser()); }
