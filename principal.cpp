#include <string>
#include <iostream>
#include <filesystem>

#include "Config.h"
#include "Utils.h"

namespace fs = std::filesystem;


void DisplayHelp(const Config& config){
    std::cout << "Display possible actions:";
    for(auto& keyDesc : config.GetConfig()){
        std::cout << "\t" << keyDesc.first <<  " : " << keyDesc.second << "\n";
    }    
}


void CopySrcToTempDir(const std::string& source ,const std::string& temporaryDir, const std::string& pattern);

void ExtractToLocation(const std::string& locationPath, const Config& config);

std::vector<std::string> BuildHash(const std::string& filePath, std::string hashCommandBuilder, const std::vector<std::string>& hashLineSelectors);

std::string GetLatestFileMatchingPattern(const std::string& filePattern);

int main(int c, char ** argc)
{
    Config config(c, argc);

    //7z l -slt

    if(config.GetHelp())
    {
        DisplayHelp(config);
        return 0;
    }

    auto& temporaryDir = config.GetTemp();
    auto& source = config.GetSource();
    auto pattern = config.GetPattern();
    if(temporaryDir.empty() || source.empty() || pattern.empty())
    {
        std::cout << "--TEMP et --SOURCE sont obligatoires";
        std::cout << "--PATTERN si il est renseigné,il ne doit pas être vide";
        DisplayHelp(config);
        return 0;
    }
    ToUpper(pattern);
    std::cout << "Reset du repertoire temporaire '" << temporaryDir << "'" << "\n";
    std::filesystem::remove_all(temporaryDir);
    std::filesystem::create_directories(temporaryDir);

    CopySrcToTempDir(source, temporaryDir, pattern);

    //
    auto archiveLocation = config.GetOut();
    auto today = GetTodayForFileName();
    Replace(archiveLocation, "_DATE_", today);
    auto makeHashCommand = config.GetArchiveHash();
    
    if(makeHashCommand.empty()){
        ExtractToLocation(archiveLocation, config);
    }
    else{
        std::filesystem::path archivePath(archiveLocation);
        auto temporaryOut = std::filesystem::path(temporaryDir).append(archivePath.filename().string()).string();
        ExtractToLocation(temporaryOut, config);

        auto outPattern = config.GetOut();
        Replace(outPattern, "_DATE_", ".*");
        auto latestFilePath = GetLatestFileMatchingPattern(outPattern);

        if(latestFilePath.empty()){
            ExtractToLocation(archiveLocation, config);
            return 0;
        }

        auto hashLatest = BuildHash(latestFilePath, makeHashCommand, config.GetHashLineSelectors());
        auto hashNew = BuildHash(temporaryOut, makeHashCommand, config.GetHashLineSelectors());

        auto isSameContent= HasSameContent(hashLatest, hashNew);
        if(isSameContent){
            std::cout << "L'etat actuel des fichiers correspondent à la sauvegarde : '" << latestFilePath << "'\n";
            return 0;
        }

        fs::copy(temporaryOut, archiveLocation, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        //fs::rename(temporaryOut, archiveLocation);
        std::cout << "Archivage fait dans: '" << archiveLocation << "'\n";
    }


    return 0;

}

std::vector<std::string> BuildHash(const std::string& filePath, std::string hashCommandBuilder, const std::vector<std::string>& hashLineSelectors){
    std::vector<std::string> result;

    if(!hashCommandBuilder.empty()){
        Replace(hashCommandBuilder, "__FILE__", filePath);
        auto output = Exec(hashCommandBuilder.c_str());

        std::vector<std::regex> matchingLines;
        matchingLines.reserve(hashLineSelectors.size());
        for(const auto& pattern : hashLineSelectors){
            std::regex match(pattern.c_str(), std::regex::multiline);
            AddMatchingPattern(result, output, match);
        }
    }

    return result;
}

void ExtractToLocation(const std::string& locationPath, const Config& config){
    auto command = config.GetCommand();
        Replace(command, "_OUT_", locationPath);
        Replace(command, "_TEMP_", config.GetTemp());

        std::cout << command << "\n";

        if(command.empty())
        {
            std::cout << "Pas d'archivage à faire car aucune commande d'archivage fournie (--COMMAND)";
            return;
        }
        std::cout << Exec(command.c_str()) << "\n";

}



void CopySrcToTempDir(const std::string& source ,const std::string& temporaryDir, const std::string& pattern){
    std::filesystem::path path(source);
    for (const auto & entry : fs::directory_iterator(path))
    {
        const auto& pathSrc = entry.path();
        if(fs::is_directory(pathSrc))
        {
            auto currentSource = pathSrc.filename().string();
            if(!Match(currentSource, pattern))
            {
                continue;
            }
            std::filesystem::path newOut(temporaryDir);
            newOut.append(pathSrc.filename().string());
            std::cout << "Selection du repertoire: '" << currentSource << "' pour sauvegarde car il correspond au pattern '" << pattern << "'\n";
            fs::copy(pathSrc, newOut, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
    }
}
