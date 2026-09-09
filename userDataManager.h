#ifndef USER_DATA_MANAGER_H
#define USER_DATA_MANAGER_H

#include <string>
#include <vector>

struct UserDataRecord
{
    std::string type;
    std::string username;
    std::string city;
    std::string region;
    std::string country;
    double latitude = 0.0;
    double longitude = 0.0;
    std::string timestamp;
    std::string detail;
    std::string value1;
    std::string value2;
    std::string value3;
};

class UserDataManager
{
public:
    static constexpr int MAX_FAVORITES = 5;
    static constexpr int MAX_RECENT_SEARCHES = 10;

    static void ensureFile();
    static std::vector<UserDataRecord> loadAll();
    static bool append(const UserDataRecord& record);
    static bool rewrite(const std::vector<UserDataRecord>& records);

    static std::vector<UserDataRecord> favorites(const std::string& username);
    static std::vector<UserDataRecord> recentSearches(const std::string& username);
    static int searchCount(const std::string& username, const std::string& city);

    static bool addFavorite(const std::string& username, const UserDataRecord& record);
    static bool removeFavorite(const std::string& username, const std::string& city);
    static bool addSearch(const std::string& username, const UserDataRecord& record);

    static void viewAllForAdmin();
    static void viewForUser(const std::string& username);

private:
    static std::string sanitize(const std::string& value);
    static std::vector<std::string> splitCsv(const std::string& line);
    static std::string csvEscape(const std::string& value);
};

#endif
