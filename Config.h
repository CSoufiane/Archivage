#pragma once
#include <map>
#include "Utils.h"


class Config {
    std::map<std::string, std::string> _Values;
    std::map<std::string, std::string> _Descriptions;
    static const std::string& GetEmpty(){
        static std::string empty;
        return empty;
    }
    const std::string* Get(const std::string& key) const {
        auto it = _Values.find(key);
        if(it == _Values.end()) return nullptr;
        return &(it->second);
    }
    const std::string& GetOrEmpty(const std::string& key) const {
        auto pValue = Get(key);
        if(pValue == nullptr) return GetEmpty();
        return *pValue;
    }
    public:
    void Init(){
        _Descriptions["--SOURCE"] = "Repertoire ou se trouve les repertoires à sauvegarder";
        _Descriptions["--TEMP"] = "Repertoire temporaire pour traitement";
        _Descriptions["--PATTERN"] = "Expression pour identifier les repertoires a sauver (example: \"^LA FOR.*\", identifie tous les repertoire dans le nom commence par \"la for\")";
        _Values["--PATTERN"] = "^LA FOR.*";
        _Descriptions["--OUT"] = "chemin de l'archivage ex: c:\archivage-_DATE_.zip, va se transformer en c:\archivage-2022-10-04.zip";
        _Descriptions["--COMMAND"] = "Commande à executer pour archiver ex: 'C:\\Program Files\\7-Zip\\7z.exe' a -tzip _OUT_ _TEMP_";
        _Descriptions["--HELP"] = "Display this help";
    }
    Config(int c, char ** argc)
    {
        Init();
        std::string value;
        std::string* pValue {nullptr};
        for(int i = 0; i < c; ++i)
        {
            value = argc[i];
            if(value.empty()) continue;
            if(value[0] == '-'){
                ToUpper(value);
                if(_Descriptions.find(value)!=_Descriptions.end())
                {
                    pValue = &(_Values[value]);
                    continue;
                }
            }
            else if(pValue != nullptr){
                *pValue = value;
                pValue = nullptr;
            }

        }
    }
    const std::map<std::string, std::string>& GetConfig() const
    {
        return _Descriptions;
    }
    const std::map<std::string, std::string>& GetValues() const
    {
        return _Values;
    }
    const std::string& GetSource() const {
        return GetOrEmpty("--SOURCE");
    }
    const std::string& GetTemp() const {
        return GetOrEmpty("--TEMP");
    }
    const std::string& GetPattern() const {
        return GetOrEmpty("--PATTERN");
    }
    const std::string& GetOut() const {
        return GetOrEmpty("--OUT");
    }
    const std::string& GetCommand() const {
        return GetOrEmpty("--COMMAND");
    }
    const bool GetHelp() const {
        return Get("--HELP") != nullptr;
    }
};