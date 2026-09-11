#include "test_common.h"
#include "userDataManager.h"

using namespace testutil;

class UserDataManagerUnit : public ::testing::Test { protected: void SetUp() override { resetDataFiles(); UserDataManager::ensureFile(); } };

TEST_F(UserDataManagerUnit, EnsureFileCreatesExpectedHeader) {
    EXPECT_EQ(readText("user_data.csv"), "Type,Username,City,Region,Country,Latitude,Longitude,DateTime,Detail,Value1,Value2,Value3\n");
}
TEST_F(UserDataManagerUnit, AppendAndLoadRoundTripPreservesFields) {
    auto r=makeRecord(); ASSERT_TRUE(UserDataManager::append(r)); auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u);
    EXPECT_EQ(rows[0].username,"alice"); EXPECT_EQ(rows[0].city,"Chennai"); EXPECT_DOUBLE_EQ(rows[0].latitude,13.0827); EXPECT_EQ(rows[0].value3,"1008");
}
TEST_F(UserDataManagerUnit, MalformedRowsAreIgnored) {
    writeText("user_data.csv", "Type,Username,City,Region,Country,Latitude,Longitude,DateTime,Detail,Value1,Value2,Value3\nBAD,ROW\n");
    EXPECT_TRUE(UserDataManager::loadAll().empty());
}
TEST_F(UserDataManagerUnit, DuplicateFavoriteIsRejectedCaseInsensitively) {
    auto r=makeRecord("FAVORITE","alice","Chennai"); EXPECT_TRUE(UserDataManager::addFavorite("alice",r));
    r.city="chEnNaI"; EXPECT_FALSE(UserDataManager::addFavorite("alice",r)); EXPECT_EQ(UserDataManager::favorites("alice").size(),1u);
}
TEST_F(UserDataManagerUnit, SixthFavoriteIsRejected) {
    for(int i=0;i<5;++i){auto r=makeRecord("FAVORITE","alice","City"+std::to_string(i)); ASSERT_TRUE(UserDataManager::addFavorite("alice",r));}
    auto sixth=makeRecord("FAVORITE","alice","City5"); EXPECT_FALSE(UserDataManager::addFavorite("alice",sixth));
}
TEST_F(UserDataManagerUnit, FavoriteRemovalOnlyAffectsMatchingUserAndCity) {
    auto a=makeRecord("FAVORITE","alice","Chennai"); auto b=makeRecord("FAVORITE","bob","Chennai");
    ASSERT_TRUE(UserDataManager::addFavorite("alice",a)); ASSERT_TRUE(UserDataManager::addFavorite("bob",b));
    EXPECT_TRUE(UserDataManager::removeFavorite("alice","chennai")); EXPECT_TRUE(UserDataManager::favorites("alice").empty()); EXPECT_EQ(UserDataManager::favorites("bob").size(),1u);
}
TEST_F(UserDataManagerUnit, RemovingMissingFavoriteReturnsFalse) { EXPECT_FALSE(UserDataManager::removeFavorite("alice","Nowhere")); }
TEST_F(UserDataManagerUnit, RecentSearchesAreLimitedToTen) {
    for(int i=0;i<12;++i){auto r=makeRecord("SEARCH","alice","City"+std::to_string(i)); ASSERT_TRUE(UserDataManager::addSearch("alice",r));}
    auto rows=UserDataManager::recentSearches("alice"); ASSERT_EQ(rows.size(),10u); EXPECT_EQ(rows.front().city,"City2"); EXPECT_EQ(rows.back().city,"City11");
}
TEST_F(UserDataManagerUnit, SearchCountIsCaseInsensitiveAndUserScoped) {
    auto r=makeRecord("SEARCH","alice","Chennai"); ASSERT_TRUE(UserDataManager::addSearch("alice",r)); r.city="chennai"; ASSERT_TRUE(UserDataManager::addSearch("alice",r));
    ASSERT_TRUE(UserDataManager::addSearch("bob",makeRecord("SEARCH","bob","Chennai")));
    EXPECT_EQ(UserDataManager::searchCount("alice","CHENNAI"),2); EXPECT_EQ(UserDataManager::searchCount("bob","CHENNAI"),1);
}
TEST_F(UserDataManagerUnit, CSVUnsafeCharactersDoNotCreateExtraColumns) {
    auto r=makeRecord(); r.detail="line1,line2\nline3"; r.value1="10,20"; ASSERT_TRUE(UserDataManager::append(r));
    auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].detail,"line1 line2 line3"); EXPECT_EQ(rows[0].value1,"10 20");
}
TEST_F(UserDataManagerUnit, RewriteReplacesExistingRecords) {
    ASSERT_TRUE(UserDataManager::append(makeRecord("WEATHER","alice","A"))); ASSERT_TRUE(UserDataManager::append(makeRecord("WEATHER","bob","B")));
    std::vector<UserDataRecord> only{makeRecord("WEATHER","carol","C")}; ASSERT_TRUE(UserDataManager::rewrite(only));
    auto rows=UserDataManager::loadAll(); ASSERT_EQ(rows.size(),1u); EXPECT_EQ(rows[0].username,"carol");
}
TEST_F(UserDataManagerUnit, AdminViewUsesMetricNamesForWeatherRows) {
    ASSERT_TRUE(UserDataManager::append(makeRecord())); ScopedIO io(""); UserDataManager::viewAllForAdmin();
    EXPECT_NE(io.output().find("Temperature"),std::string::npos); EXPECT_NE(io.output().find("Humidity"),std::string::npos); EXPECT_NE(io.output().find("Pressure"),std::string::npos);
}
