#include "test_common.h"
#include "weatherEnhancement.h"

using namespace testutil;

TEST(ValidationUnit, UsernameAtMaximumLengthIsAccepted) {
    resetDataFiles(); std::string u(32,'a'); WeatherEnhancementManager m; ScopedIO io(u+"\nsecret1\nsecret1\n"); EXPECT_TRUE(m.registerUser());
}
TEST(ValidationUnit, UsernameAboveMaximumLengthIsRejected) {
    resetDataFiles(); std::string u(33,'a'); WeatherEnhancementManager m; ScopedIO io(u+"\nsecret1\nsecret1\n"); EXPECT_FALSE(m.registerUser());
}
TEST(ValidationUnit, SixCharacterPasswordIsAccepted) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("sixpass\n123456\n123456\n"); EXPECT_TRUE(m.registerUser());
}
TEST(ValidationUnit, EmptyUsernameIsRejected) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("\nsecret1\nsecret1\n"); EXPECT_FALSE(m.registerUser());
}
TEST(ValidationUnit, PunctuationAllowedByUsernameRulesIsAccepted) {
    resetDataFiles(); WeatherEnhancementManager m; ScopedIO io("user_1.test-2\nsecret1\nsecret1\n"); EXPECT_TRUE(m.registerUser());
}
