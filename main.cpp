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
        const std::string en_archiveDir = "en_logs/en_archived_logs";
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

        std::vector<filesystem::path> en_files;
        for (const auto& entry : filesystem::directory_iterator(en_archiveDir)) {
            if (entry.is_regular_file()) {
                en_files.push_back(entry.path());
            }
        }

        if (en_files.size() > maxFiles) {
            std::sort(en_files.begin(), en_files.end(), [](const filesystem::path& a, const filesystem::path& b) {
                return filesystem::last_write_time(a) < filesystem::last_write_time(b);
            });

            for (int i = 0; i < en_files.size() - maxFiles; ++i) {
                filesystem::remove(en_files[i]);
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
        // also add to en_logs
        ofstream en_file("en_logs/" + filename);
        if (!en_file.is_open()) {
            throw runtime_error("Could not create en_log file");
        } 
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
            throw runtime_error("Failed to compress log file for archived_logs");
        }
        
        string en_sourceFile = "en_logs/" + filename;
        string en_destinationFile = "en_logs/en_archived_logs/" + filename + ".zip";
        string en_command = "zip -r " + en_destinationFile + " " + en_sourceFile + " && rm " + en_sourceFile;
        int en_result = system(en_command.c_str());
        if (en_result != 0) {
            throw runtime_error("Failed to compress log file for en_archived_logs");
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
        return size >= 100;
    }

    void writeToLog(const wstring& entry, const wstring& english, const string& filename) {
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
        
        wofstream en_outputFile("en_logs/" + filename, ios::app);
        if (en_outputFile.is_open()) {
            auto now = chrono::system_clock::now();
            auto now_time_t = chrono::system_clock::to_time_t(now);
            auto now_tm = *localtime(&now_time_t);
            auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
            char timestamp[30];
            strftime(timestamp, sizeof(timestamp), "[%Y:%m:%d %H:%M:%S.", &now_tm);
            sprintf(timestamp + strlen(timestamp), "%03ld]", now_ms.count());
            en_outputFile << english << " " << timestamp << endl;
            en_outputFile.close();
        } else {
            throw runtime_error("Could not open en_log file");
        }
    }

    void log(const string& level, const string& message) {
        lock_guard<mutex> guard(logMutex);
        string filename = getFileName();
        wstring entry = L"[" + resources.at(level) + L"] " + resources.at(message);
        wstring english = L"[" + convertToWString(level) + L"] " + convertToWString(message);
        wcout << "log (L), print (P), both (B), or niether (N)?: ";
        string choice;
        cin >> choice;
        if (choice == "L" || choice == "l") {
            writeToLog(entry, english, filename);
        } else if (choice == "P" || choice == "p") {
            wcout << entry << endl;
        } else if (choice == "B" || choice == "b") {
            writeToLog(entry, english, filename);
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
    return 0;
}