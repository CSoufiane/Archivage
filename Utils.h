#pragma once
#include <string>
#include <regex>
#include <memory>
#include <stdexcept>
#include <array>
#include <set>
#include <chrono>
#include <algorithm>
#include <filesystem>


namespace fs = std::filesystem;

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
    std::unique_ptr<FILE, decltype(&pclose) > pipe(popen(cmd, "r"), pclose);
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

bool HasSameContent(const std::vector<std::string>& v1, const std::vector<std::string>& v2){
    if(v1.size()!=v2.size()){
        return false;
    }

    std::set<std::string> hashes;
    for(const auto& value: v1){
        hashes.insert(value);
    }

    for(const auto& value: v2){
        auto isInserted = hashes.insert(value).second;
        if(isInserted){
            return false;
        }
    }

    return true;
}

void AddMatchingPattern(std::vector<std::string>& result, const std::string& st, const std::regex& r){
    for (std::sregex_iterator i = std::sregex_iterator(st.begin(), st.end(), r);
        i != std::sregex_iterator();
        ++i)
    {
        std::smatch m = *i;
        result.push_back(m.str());
    }
}

std::vector<std::string> GetFilesMatchingPattern(const std::string& filePattern){
    std::vector<std::string> result;
    std::filesystem::path path(filePattern);
    auto pattern = path.filename().string();
    ToUpper(pattern);

    for (const auto & entry : fs::directory_iterator(path.parent_path()))
    {
        const auto& pathSrc = entry.path();
        if(fs::is_regular_file(pathSrc)){
            auto currentFile = pathSrc.filename().string();
            if(!Match(currentFile, pattern))
            {
                continue;
            }
            result.emplace_back(std::filesystem::relative(pathSrc).string());
        } else if(fs::is_directory(pathSrc)){
            std::string target = pathSrc.filename().string();
            auto allDigit = true;
            for(char c : target){
                allDigit = std::isdigit(c);
                if(allDigit == false) break;
            }

            if(!allDigit){
                continue;
            }
            
            auto newFilePatern = std::filesystem::path(std::filesystem::relative(pathSrc).string()).append(pattern).string();
            auto newResults = GetFilesMatchingPattern(newFilePatern);
            newResults.reserve(result.size() + newResults.size());
            for(auto& cfile : newResults){
                result.emplace_back(std::filesystem::relative(cfile).string());
            }
        }
    }
    return result;
}


bool IsLower(const std::string& p1, const std::string& p2){
    auto n1 = std::filesystem::path(p1).filename().string();
    auto n2 = std::filesystem::path(p1).filename().string();
    return n1.compare(n2) < 0;
}


std::string GetLatestFileMatchingPattern(const std::string& filePattern){
    std::string result;
    auto files = GetFilesMatchingPattern(filePattern);
    for(const auto& f : files){
        if(result.empty() || IsLower(result, f)){
            result = f;
        }
    }

    return result;
}
