#include <string>
#include <iostream>
#include <filesystem>

#include "Config.h"

namespace fs = std::filesystem;


void DisplayHelp(const Config& config){
    std::cout << "Display possible actions:";
    for(auto& keyDesc : config.GetConfig()){
        std::cout << "\t" << keyDesc.first <<  " : " << keyDesc.second << "\n";
    }    
}

int main(int c, char ** argc)
{
    Config config(c, argc);

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

    std::filesystem::path path(source);
    for (const auto & entry : fs::directory_iterator(path))
    {
        const auto& pathSrc = entry.path();
        if(fs::is_directory(pathSrc))
        {
            if(!Match(pathSrc.filename().string(), pattern))
            {
                continue;
            }
            std::filesystem::path newOut(temporaryDir + "\\" + pathSrc.filename().string());
            std::cout << "Selection du repertoire: '" << newOut.string() << "' car il correspond au pattern '" << pattern << "'\n";
            fs::copy(pathSrc, newOut, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
    }
    //
    auto archivePath = config.GetOut();
    auto today = GetTodayForFileName();
    Replace(archivePath, "_DATE_", today);

    auto command = config.GetCommand();
    Replace(command, "_OUT_", archivePath);
    Replace(command, "_TEMP_", config.GetTemp());
    std::cout << command << "\n";

    if(command.empty())
    {
        std::cout << "Pas d'archivage à faire";
    }
    std::cout << Exec(command.c_str()) << "\n";
    return 0;

}