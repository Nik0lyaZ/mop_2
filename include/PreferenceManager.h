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
#define DIR PATH + "\\" + DIRNAME
#define SAVE DIR + "\\" + SAVENAME
#else
#define DIR PATH + "/" + DIRNAME
#define SAVE DIR + "/" + SAVENAME
#endif // __WIN32__

struct Preferences
{
    std::string ver;
    std::string pfm;
    int obj = 16384;
    bool autoVer = false,
    lateVer = false;
};

class PreferenceManager
{
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
        wxString GetVer() {return prefs.ver;}
        void SetVer(wxString value) {prefs.ver = value;}

        // Object count
        int GetObj() {return prefs.obj;}
        void SetObj(int value) {prefs.obj = value;}

        // Auto-detect
        bool GetAutoVer() {return prefs.autoVer;}
        void SetAutoVer(bool value) {prefs.autoVer = value;}

        // Show only the latest releases
        bool GetLateVer() {return prefs.lateVer;}
        void SetLateVer(bool value) {prefs.lateVer = value;}

        // Platform
        wxString GetPfm() {return prefs.pfm;}
        void SetPfm(wxString value) {prefs.pfm = value;}
};

#endif // PREFERENCEMANAGER_H