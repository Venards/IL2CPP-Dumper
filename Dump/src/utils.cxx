#include "../include/utils.hxx"
#include <iostream>
#include <direct.h>
#include <algorithm>
#include <map>
#include <string>

std::ofstream logFile("C:\\IL2CPPDump_Log.txt");

void Log(const std::string& msg) {
    std::cout << msg << "\n";
    if (logFile.is_open()) {
        logFile << msg << "\n";
        logFile.flush();
    }
}

void EnsureDirectory(const std::string& path) {
    int result = _mkdir(path.c_str());
    if (result == -1 && errno != EEXIST) {
        Log("[WARN] Could not create directory (it might already exist or access is denied)");
    }
}

std::string GetAccessModifier(uint32_t flags) {
    static const std::map<uint32_t, std::string> accessMap = {
        {0x0006, "public"},
        {0x0001, "private"},
        {0x0004, "protected"},
        {0x0005, "internal"}
    };

    uint32_t access = flags & 0x0007;
    auto it = accessMap.find(access);
    return (it != accessMap.end()) ? it->second : "private";
}