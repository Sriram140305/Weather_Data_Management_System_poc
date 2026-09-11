#ifndef WEATHER_TEST_COMMON_H
#define WEATHER_TEST_COMMON_H

#include <gtest/gtest.h>
#include "userDataManager.h"
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testutil {

inline void removeFile(const std::string& path) { std::error_code ec; std::filesystem::remove(path, ec); }
inline void resetDataFiles() {
    removeFile("user_data.csv");
    removeFile("users.csv");
    removeFile("user_data_backup.csv");
    removeFile("weather_report.csv");
    removeFile("api_config.conf");
    removeFile("alert_thresholds.conf");
    removeFile("system.log");
    removeFile("error.log");
    removeFile("api.log");
    removeFile("api_performance.log");
}
inline void writeText(const std::string& path, const std::string& text) {
    std::ofstream f(path, std::ios::trunc); ASSERT_TRUE(f.is_open()) << path; f << text;
}
inline std::string readText(const std::string& path) {
    std::ifstream f(path); std::ostringstream s; s << f.rdbuf(); return s.str();
}
inline std::size_t lineCount(const std::string& path) {
    std::ifstream f(path); std::size_t n=0; std::string line; while (std::getline(f,line)) ++n; return n;
}

class ScopedIO {
public:
    explicit ScopedIO(const std::string& input) : in_(std::cin.rdbuf()), out_(std::cout.rdbuf()) {
        iss_.str(input); std::cin.rdbuf(iss_.rdbuf()); std::cout.rdbuf(oss_.rdbuf());
    }
    ~ScopedIO() { std::cin.rdbuf(in_); std::cout.rdbuf(out_); }
    std::string output() const { return oss_.str(); }
private:
    std::istringstream iss_;
    std::ostringstream oss_;
    std::streambuf* in_;
    std::streambuf* out_;
};

inline UserDataRecord makeRecord(const std::string& type="WEATHER", const std::string& user="alice", const std::string& city="Chennai") {
    UserDataRecord r; r.type=type; r.username=user; r.city=city; r.region="Tamil Nadu"; r.country="India";
    r.latitude=13.0827; r.longitude=80.2707; r.timestamp="2026-09-09 10:00:00"; r.detail="Current Weather";
    r.value1="30.5"; r.value2="70"; r.value3="1008"; return r;
}
inline std::string defaultAdminHash() { return "e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7"; }

} // namespace testutil

#endif
