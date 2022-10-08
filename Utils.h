#pragma once
#include <string>
#include <regex>
#include <memory>
#include <stdexcept>
#include <array>

#include <chrono>

void ToUpper(std::string& chain)
{
    for(auto& c : chain) c = std::toupper(c);
}

bool Match(std::string filename, const std::string& pattern)
{
    std::regex rLaforet(pattern);
    std::smatch match;
    ToUpper(filename);
    std::regex_match (filename, match, rLaforet);
    return !match.empty();
}

std::string Exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string GetTodayForFileName(){
    auto time = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(time);
    auto tm = *std::localtime(&tt);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
}

bool Replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}