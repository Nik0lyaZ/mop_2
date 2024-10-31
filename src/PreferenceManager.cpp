#include <vector>
#include <fstream>
#include <filesystem>
#include "PreferenceManager.h"

bool PreferenceManager::SaveToFile()
{
    if(!std::filesystem::exists(DIR) && !std::filesystem::create_directories(DIR))
        return false;
    std::ofstream file(SAVE);

    // Preferences to save
    std::replace(prefs.pfm.begin(), prefs.pfm.end(), ' ', '_');
    file << prefs.ver << '\n';
    file << prefs.obj << '\n';
    file << prefs.autoVer << '\n';
    file << prefs.lateVer << '\n';
    file << prefs.pfm << '\n';

    file.close();
    return true;
}

bool PreferenceManager::LoadFromFile()
{
    if(!std::filesystem::exists(DIR)) return false;
    std::ifstream file(SAVE);

    // Preferences to load
    file >> prefs.ver;
    file >> prefs.obj;
    file >> prefs.autoVer;
    file >> prefs.lateVer;
    file >> prefs.pfm;
    std::replace(prefs.pfm.begin(), prefs.pfm.end(), '_', ' ');

    file.close();
    return true;
}
