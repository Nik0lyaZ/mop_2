#include "PreferenceManager.h"
#include <filesystem>
#include <fstream>

bool PreferenceManager::SaveToFile() {
    if (!std::filesystem::exists(DIR) && !std::filesystem::create_directories(DIR))
        return false;
    std::ofstream file(SAVE);

    // Preferences to save
    std::replace(prefs.pfm.begin(), prefs.pfm.end(), ' ', '_');
    file << prefs.ver << '\n';
    file << prefs.obj << '\n';
    file << prefs.autoVer << '\n';
    file << prefs.lateVer << '\n';
    file << prefs.pfm << '\n';
    file << '"' << prefs.path << '"' << '\n';

    file.close();
    return true;
}

bool PreferenceManager::LoadFromFile() {
    if (!std::filesystem::exists(DIR)) return false;
    std::ifstream file(SAVE);

    // Preferences to load
    file >> prefs.ver;
    file >> prefs.obj;
    file >> prefs.autoVer;
    file >> prefs.lateVer;
    file >> prefs.pfm;
    file >> prefs.path;
    if (prefs.path.front() == '"' && prefs.path.back() != '"') {
        std::string s = "";
        while (s.back() != '"' && file >> s) {
            prefs.path += ' ' + s;
        }
    }
    std::replace(prefs.pfm.begin(), prefs.pfm.end(), '_', ' ');
    while (prefs.path.find('"') != std::string::npos) prefs.path.erase(prefs.path.find('"'), 1);

    file.close();
    return true;
}
