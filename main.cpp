// File: main.cpp
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <string>
#include <map>
#include <string>
#include <chrono>
#include <mutex>
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
        throw runtime_error("Could not open languages.json");
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

    void checkCompressedLogs() {
        const std::string archiveDir = "logs/archived_logs";
        const int maxFiles = 20;

        std::vector<filesystem::path> files;
        for (const auto& entry : filesystem::directory_iterator(archiveDir)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }

        if (files.size() > maxFiles) {
            std::sort(files.begin(), files.end(), [](const filesystem::path& a, const filesystem::path& b) {
                return filesystem::last_write_time(a) < filesystem::last_write_time(b);
            });

            for (int i = 0; i < files.size() - maxFiles; ++i) {
                filesystem::remove(files[i]);
            }
        }
    }

    string createTextFile() {
        auto now = chrono::system_clock::now();
        auto now_time_t = chrono::system_clock::to_time_t(now);
        auto now_tm = *localtime(&now_time_t);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y.%m.%d.%H.%M.%S", &now_tm);
        string filename = string(timestamp) + ".txt";
        ofstream file("logs/" + filename);
        if (!file.is_open()) {
            throw runtime_error("Could not create log file");
        }
        file.close();
        return filename;
    }

    string getFileName() {
       for (const auto& entry : filesystem::directory_iterator("logs")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                return entry.path().filename().string();
            }
        }
        string filename = createTextFile();
        return filename;
    }

    void compressLog(string filename) {
        string sourceFile = "logs/" + filename;
        string destinationFile = "logs/archived_logs/" + filename + ".zip";
        string command = "zip -r " + destinationFile + " " + sourceFile + " && rm " + sourceFile;
        int result = system(command.c_str());
        if (result != 0) {
            throw runtime_error("Failed to compress log file");
        }
    }

    bool checkLogSize(string filename) {
        ifstream file("logs/" + filename);
        if (!file.is_open()) {
            throw runtime_error("Could not open log file");
        }
        file.seekg(0, ios::end);
        size_t size = file.tellg();
        file.close();
        return size >= 1024;
    }

    void writeToLog(const wstring& entry, const string& filename) {
        wofstream outputFile("logs/" + filename, ios::app);
        if (outputFile.is_open()) {
            auto now = chrono::system_clock::now();
            auto now_time_t = chrono::system_clock::to_time_t(now);
            auto now_tm = *localtime(&now_time_t);
            auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
            char timestamp[30];
            strftime(timestamp, sizeof(timestamp), "[%Y:%m:%d %H:%M:%S.", &now_tm);
            sprintf(timestamp + strlen(timestamp), "%03ld]", now_ms.count());
            outputFile << entry << " " << timestamp << endl;
            outputFile.close();
        } else {
            throw runtime_error("Could not open log file");
        }
    }

    void log(const string& level, const string& message) {
        lock_guard<mutex> guard(logMutex);
        string filename = getFileName();
        wstring entry = L"[" + resources.at(level) + L"] " + resources.at(message);
        wcout << "log (L), print (P), both (B), or niether (N)?: ";
        string choice;
        cin >> choice;
        if (choice == "L" || choice == "l") {
            writeToLog(entry, filename);
        } else if (choice == "P" || choice == "p") {
            wcout << entry << endl;
        } else if (choice == "B" || choice == "b") {
            writeToLog(entry, filename);
            wcout << entry << endl;
        } else {
            wcout << "No action taken" << endl;
        }
        if(checkLogSize(filename)) {
            compressLog(filename);
            checkCompressedLogs();
        }
    }

private:
    map<string, wstring> resources;
    mutex logMutex;
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
    logger.log("INFO", "Test information");
    logger.log("WARNING", "Warning information");
    logger.log("ERROR", "Error information");
    loadResources(language);
    return 0;
}