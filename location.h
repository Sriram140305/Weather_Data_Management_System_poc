#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <vector>

struct LocationResult
{
    std::string name;
    std::string region;
    std::string country;
    double latitude = 0.0;
    double longitude = 0.0;
};

class LocationManager
{
public:
    explicit LocationManager(const std::string& username = "");
    void setCurrentUser(const std::string& username);

    bool selectLocation(LocationResult& selected, const std::string& prompt = "Search Location: ", bool recordSearch = true);
    void searchLocation();

    void addFavoriteCity();
    void viewFavoriteCities();
    void removeFavoriteCity();
    void showRecentSearches();
    void showMostSearchedCity();
    void findHottestCity();
    void weatherComfortScore();
    void travelRecommendation();
    void compareCities();

    void loadFavoriteCities();
    void loadSearchHistory();

private:
    std::string currentUsername;
};

#endif
