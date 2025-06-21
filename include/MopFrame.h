#ifndef MOPFRAME_H
#define MOPFRAME_H
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <fstream>
#include "PreferenceManager.h"

extern const wxArrayString version, platform;

class MopFrame : public wxFrame {
public:
    MopFrame(const wxString& title);

private:
// Preferences
    PreferenceManager* prefMgr = new PreferenceManager(version[0], platform[0]);

// Functions
    void Update();
    void UpdateVerList();
    void OnCloseWindow(wxCloseEvent& evt);
    void OnPatchClicked(wxCommandEvent& evt);
    void OnVerChosen(wxCommandEvent& evt);
    void OnObjChanged(wxCommandEvent& evt);
    void OnAutoVerChecked(wxCommandEvent& evt);
    void OnLateVerChecked(wxCommandEvent& evt);
//    void OnPfmChosen(wxCommandEvent& evt);
    void OnPathChanged(wxCommandEvent& evt);
    #ifdef DEBUG
    void OnTestButton(wxCommandEvent& evt);
    #endif // DEBUG

// Controls
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxStaticText* verStaticText = new wxStaticText(panel, wxID_ANY, "Game version");
    wxChoice* verChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, version);
    wxCheckBox* autoVerCheck = new wxCheckBox(panel, wxID_ANY, "Auto-detect");
    wxCheckBox* lateVerCheck = new wxCheckBox(panel, wxID_ANY, "Latest subversion");
    wxStaticText* objStaticText = new wxStaticText(panel, wxID_ANY, "Object count");
    wxSpinCtrl* objSpinCtrl = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, -1),
                                               wxSP_ARROW_KEYS | wxSP_WRAP, INT_MIN, INT_MAX, prefMgr->GetObj());
    wxButton* patchButton = new wxButton(panel, wxID_ANY, "Apply patches!", wxDefaultPosition, wxSize(125, 35));
//    wxStaticText* pfmStaticText = new wxStaticText(panel, wxID_ANY, "Platform");
//    wxChoice* pfmChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, platform);
    wxStaticText* pathStaticText = new wxStaticText(panel, wxID_ANY, "Path to libraries");
    wxTextCtrl* pathTextCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(140, -1));
    wxGauge* patchGauge = new wxGauge(panel, wxID_ANY, 160, wxDefaultPosition, wxSize(160, -1));
    wxStaticText* patchStaticText = new wxStaticText(panel, wxID_ANY, "0.0\%");
    #ifdef DEBUG
    wxButton* testButton = new wxButton(panel, wxID_ANY, "Test");
    #endif // DEBUG

// Sizers
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* prefSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* verSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* pfmSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxGridSizer* pVerSizer = new wxGridSizer(1, 0, wxSize(5, 0));
    wxGridSizer* pCheckSizer = new wxGridSizer(0, 1, wxSize(0, 2));
    wxSizerFlags staticTextFlags = wxSizerFlags().Border(wxBOTTOM, 2);
};

#endif // MOPFRAME_H
