// File: main.cpp
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <string>
#include <map>
#include <string>
#include "nlohmann/json.hpp"

using namespace std;

// Function to convert a string to wstring
wstring convertToWString(const string& str) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

// Function to load language resources from a JSON file
map<string, wstring> loadResources(const string& language) {
    map<string, wstring> resources;
    ifstream file("languages.json");
    if (!file.is_open()) {
        throw runtime_error("Could not open resources.json");
    }

    nlohmann::json j;
    file >> j;
    if (j.contains(language)) {
        auto languageResources = j[language];
        string setlocale = languageResources["setlocale"].get<string>();
        locale::global(locale(setlocale));
        for (auto& [key, value] : j[language].items()) {
            resources[key] = convertToWString(value);
        }
    } else {
        throw runtime_error("Language not found in resources.json");
    }

    return resources;
}    

class Logger {
public:

    Logger(const string& language) {
        resources = loadResources(language);
    }

    void log(const string& level, const string& message) {
        wcout << L"[" << resources.at(level) << L"] " << resources.at(message) << endl;
    }


private:
    map<string, wstring> resources;
};

int main() {
    string language;
    locale::global(locale("ja_JP.utf8"));
    wstring choices = L"Enter language (jp (日本語), fr (Français), es (Español)): ";
    wcout << choices;
    cin >> language;
    Logger logger(language);
    logger.log("INFO", "Test information");
    logger.log("WARNING", "Warning information");
    logger.log("ERROR", "Error information");
    loadResources(language);
    return 0;
}