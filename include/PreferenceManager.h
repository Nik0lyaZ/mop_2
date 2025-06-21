#ifndef PREFERENCEMANAGER_H
#define PREFERENCEMANAGER_H
#include <wx/wx.h>

#define DIRNAME "MaxObjectPatcher"
#define SAVENAME "Preferences.txt"

#ifdef __WIN32__
#define PATH (std::string)std::getenv("APPDATA")
#elif defined __APPLE__
#define PATH (std::string)std::getenv("HOME") + "/Library/Application Support"
#else
#define PATH (std::string)std::getenv("HOME") + "/.config"
#endif // __WIN32__

#ifdef __WIN32__
#define PATH_SLASH (std::string)"\\"
#else
#define PATH_SLASH (std::string)"/"
#endif // __WIN32__

#define DIR PATH + PATH_SLASH + DIRNAME
#define SAVE DIR + PATH_SLASH + SAVENAME

struct Preferences {
    std::string ver;
    int obj = 16384;
    bool autoVer = false,
    lateVer = false;
    std::string pfm;
    std::string path = "";
};

class PreferenceManager {
private:
    Preferences prefs;

public:
    PreferenceManager(wxString ver, wxString pfm) {
        prefs.ver = ver;
        prefs.pfm = pfm;
    }
    bool SaveToFile();
    bool LoadFromFile();

    // Game version
    inline wxString GetVer() {return prefs.ver;}
    inline void SetVer(wxString value) {prefs.ver = value;}

    // Object count
    inline int GetObj() {return prefs.obj;}
    inline void SetObj(int value) {prefs.obj = value;}

    // Auto-detect
    inline bool GetAutoVer() {return prefs.autoVer;}
    inline void SetAutoVer(bool value) {prefs.autoVer = value;}

    // Show only the latest releases
    inline bool GetLateVer() {return prefs.lateVer;}
    inline void SetLateVer(bool value) {prefs.lateVer = value;}

    // Platform
    inline wxString GetPfm() {return prefs.pfm;}
//    void SetPfm(wxString value) {prefs.pfm = value;}

    // Path to libraries
    inline wxString GetPath() {
        if (!prefs.path.size()) return "";
        if (prefs.path.front() == '/' || prefs.path.front() == '\\')
            return prefs.path;
        return prefs.path + PATH_SLASH;
    }
    inline void SetPath(wxString value) {prefs.path = value;}
};

#endif // PREFERENCEMANAGER_H
