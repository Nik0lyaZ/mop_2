#include "MopFrame.h"
#include "Patcher.h"
#include "PreferenceManager.h"
#include <filesystem>
using namespace std;

#ifdef DEBUG
void MopFrame::Test() {
    Patcher* patcher = new Patcher();
    patcher->Test();
    delete patcher;
}
#endif // DEBUG

// Constant arrays
const wxArrayString version {
    "1.0", "1.01", "1.02", "1.1", "1.11", "1.2", "1.21", "1.22", "1.3", "1.4", "1.41", "1.5", "1.51", "1.6", "1.7", "1.71", "1.8", "1.81", "1.811"
};
const wxArrayString platform {
    "Android"//, "iOS", "Windows", "MacOS", "Windows phone"
};
const wxArrayString library {
    "armeabi", "armeabi-v7a", "x86"
};
const bool lateVer[] {
    0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1
};

// MopFrame constructor
MopFrame::MopFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title) {
    // Preferences
    prefMgr->LoadFromFile();

    // Sizers
    mainSizer->Add(prefSizer, wxSizerFlags().Expand().Border(wxALL, 10));
        prefSizer->Add(topSizer, wxSizerFlags().Expand());
            topSizer->Add(pVerSizer);
                pVerSizer->Add(verSizer);
                    verSizer->Add(verStaticText, staticTextFlags);
                    verSizer->Add(verChoice);
//                pVerSizer->Add(pfmSizer);
//                    pfmSizer->Add(pfmStaticText, staticTextFlags);
//                    pfmSizer->Add(pfmChoice);
            #ifdef DEBUG
            topSizer->AddStretchSpacer(1);
            topSizer->Add(testButton, wxSizerFlags().Border(wxLEFT, 10));
            #endif // DEBUG
        prefSizer->AddSpacer(5);
        prefSizer->Add(pCheckSizer, wxSizerFlags());
            pCheckSizer->Add(autoVerCheck);
            pCheckSizer->Add(lateVerCheck);
    mainSizer->Add(objStaticText, staticTextFlags.Align(wxALIGN_CENTER));
    mainSizer->Add(objSpinCtrl, wxSizerFlags().Align(wxALIGN_CENTER));
    mainSizer->Add(pathStaticText, staticTextFlags.Align(wxALIGN_CENTER).Border(wxTOP, 5));
    mainSizer->Add(pathTextCtrl, wxSizerFlags().Align(wxALIGN_CENTER));
    mainSizer->AddStretchSpacer(1);
    mainSizer->Add(patchStaticText, staticTextFlags.Align(wxALIGN_CENTER));
    mainSizer->Add(patchGauge, wxSizerFlags().Align(wxALIGN_CENTER));
    mainSizer->Add(patchButton, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 10));

    // Frame
    panel->SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    wxSize clientSize = GetClientSize();
    wxSize patchBtnSize = patchButton->GetSize();
    SetMinClientSize(wxSize(clientSize.GetWidth(), clientSize.GetHeight() + patchBtnSize.GetHeight()));
    SetClientSize(320, 320);

    // Controls
    CreateStatusBar();
//    pfmSizer->Show(false);
    patchButton->SetFocus();
    verChoice->Select(verChoice->FindString(prefMgr->GetVer()));
//    pfmChoice->Select(verChoice->FindString(prefMgr->GetPfm()));
    autoVerCheck->SetValue(prefMgr->GetAutoVer());
    if (prefMgr->GetLateVer()) {
        prefMgr->SetLateVer(true);
        UpdateVerList();
    }
    Update();

    // Tooltips
    patchButton->SetToolTip("Drink some eeffoc and become Cirno"); // <-- peak humor // Approved! -21/06/2025
    autoVerCheck->SetToolTip("Automatically detect the version and library");
    lateVerCheck->SetToolTip("Show only the latest subversions in version list");
    #ifdef DEBUG
    testButton->SetToolTip("Call an unnecessary window");
    #endif // DEBUG

    // Events
    this->Bind(wxEVT_CLOSE_WINDOW, &MopFrame::OnCloseWindow, this);
    patchButton->Bind(wxEVT_BUTTON, &MopFrame::OnPatchClicked, this);
    autoVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnAutoVerChecked, this);
    verChoice->Bind(wxEVT_CHOICE, &MopFrame::OnVerChosen, this);
    objSpinCtrl->Bind(wxEVT_TEXT, &MopFrame::OnObjChanged, this);
    lateVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnLateVerChecked, this);
    lateVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnLateVerChecked, this);
    //pfmChoice->Bind(wxEVT_CHOICE, &MopFrame::OnPfmChosen, this);
    pathTextCtrl->Bind(wxEVT_TEXT, &MopFrame::OnPathChanged, this);
    #ifdef DEBUG
    testButton->Bind(wxEVT_BUTTON, &MopFrame::OnTestButton, this);
    #endif // DEBUG
}

// Update methods
void MopFrame::Update() {
    verChoice->SetSelection(verChoice->FindString(prefMgr->GetVer()));
    verChoice->Enable(!prefMgr->GetAutoVer());
    objSpinCtrl->SetValue(prefMgr->GetObj());
    autoVerCheck->SetValue(prefMgr->GetAutoVer());
    lateVerCheck->SetValue(prefMgr->GetLateVer());
    //pfmChoice->SetSelection(pfmChoice->FindString(prefMgr->GetPfm()));
    pathTextCtrl->SetValue(prefMgr->GetPath());
}
void MopFrame::UpdateVerList() {
    if (prefMgr->GetLateVer() && verChoice->GetCount() == version.GetCount()) {
        int j = verChoice->GetSelection();
        while (!lateVer[j] && j < (int)verChoice->GetCount()) j++;
        verChoice->SetSelection(j);
        prefMgr->SetVer(verChoice->GetString(j));
        for (int i = version.GetCount() - 1; i >= 0; i--) {
            if (!lateVer[i]) verChoice->Delete(i);
        }
    } else if (verChoice->GetCount() < version.GetCount()) {
        for (int i = 0; i < (int)version.GetCount(); i++) {
            if (!lateVer[i]) verChoice->Insert(version.Item(i), i);
        }
    }
}

// Event methods
void MopFrame::OnPatchClicked(wxCommandEvent& evt) {
    patchGauge->SetValue(0);
    patchStaticText->SetLabel("0.0\%");
    mainSizer->Layout();
    SetStatusText("");
    bool useAutoVer = autoVerCheck->GetValue();
    int ver = useAutoVer ? -1 : version.Index(verChoice->GetStringSelection());
    if (ver == wxNOT_FOUND && !useAutoVer) {
        SetStatusText("You need to select a version.");
        return;
    }
    unsigned obj = objSpinCtrl->GetValue();
    Patcher* patcher = new Patcher();
    string path = "";
    int patch = 0, libsPatched = 0, libsNotFound = 0;
    bool local = false;
    string file = ver < 14 ? "libgame.so" : "libcocos2dcpp.so";
//    vector<int> returns;

    for (int i = 0; i < 2; i++) {
        path = i ? "lib" + PATH_SLASH : "";
        path = prefMgr->GetPath() + path;

        int lib = 0;
        if (filesystem::exists(path + file)) {
            local = true;
            if (ver >= 11 && !useAutoVer) {
                wxSingleChoiceDialog dialog(this, "Select your library architecture:", "Library selection", library);
                int answer = dialog.ShowModal();
                if (answer == wxID_CANCEL) {
                    SetStatusText("You have cancelled the operation.");
                    return;
                }
                lib = dialog.GetSelection();
            }
        }

        while (lib < 3) {
            string libPath = local ? "" : (string)library.Item(lib) + PATH_SLASH;
            bool stop = false;
            patcher->m_approx = 0;
            if (ver < 0) ver = patcher->GetAutoVer(path + libPath + "libgame.so");
            if (ver < 0) ver = patcher->GetAutoVer(path + libPath + "libcocos2dcpp.so");
            if (ver < 0) {lib++; continue;}
            file = ver % 100 < 14 ? "libgame.so" : "libcocos2dcpp.so";
//            wxLogMessage("%d", ver);

            do {
                stop = true;
//                wxLogMessage("%s, %d, %d, %d, %d, %d", path + libPath + file, ver % 100, obj, useAutoVer ? ver / 100 : lib, approx, visual);
//                Result result = patcher->Patch(path + libPath + file, ver % 100, obj, useAutoVer ? ver / 100 : lib, approx, visual);
                patcher->Prepare(path + libPath + file, obj, ver % 100, useAutoVer ? ver / 100 : lib);
                Result result = patcher->Patch();
//                wxLogMessage(wxString(path + libPath + file));
//                returns.push_back(static_cast<int>(result));
                switch(result) {
                    case Result::OK:
                        patch = 1;
    //                    wxLogMessage("%d", patcher->GetAutoVer(path + libPath + file));
                        ++libsPatched;
                        break;
                    case Result::ApproxReq: {
                        int approx0 = patcher->Approx(obj, 0), approx1=patcher->Approx(obj, 1);
                        wxMessageDialog dialog(this, wxString::Format("This library can't have the limit of %d objects.\nWhat limit do you want to use instead?", obj),
                                               wxString::Format("Approximation problem in %s", useAutoVer ? library[ver / 100] : library[lib]), wxYES_NO | wxICON_WARNING | wxCENTER | wxNO_DEFAULT);
                        dialog.SetYesNoLabels(wxString::Format("%d", approx0), wxString::Format("%d", approx1));
                        int answer1 = dialog.ShowModal();
                        patcher->m_approx = answer1 == wxID_YES ? approx0 : approx1;
                        stop = false;
                        break;
                    }
                    case Result::FileError:
                        ++libsNotFound;
                        break;
                    default:
                        break;
                }
            } while (stop != true);

            if (local || (ver < 11 && !useAutoVer)) break;
            if (useAutoVer) ver = -1;
            lib++;
            float percentage = float(3 * i + lib) / 6.0 * 100.0;
            patchGauge->SetValue(percentage * 1.6);
            patchStaticText->SetLabel(wxString::Format("%.1f\%", percentage));
            mainSizer->Layout();
        }
        if (patch == 1) break;
    }

    delete patcher;

    patchGauge->SetValue(160);
    patchStaticText->SetLabel("100.0%");
    mainSizer->Layout();

    int libsTotal = (ver < 11 || local) ? 1 : 3;
    if (libsPatched > 0) {
        if (useAutoVer) SetStatusText(wxString::Format("Automatically patched %d %s.", libsPatched, libsPatched == 1 ? "library" : "libraries"));
        else SetStatusText(wxString::Format("Successfully patched %d of %d libraries.", libsPatched, libsTotal));
    }
    else if (libsNotFound >= libsTotal * 2) SetStatusText("Could not find the libraries.");
    else SetStatusText("None of the libraries could be patched.");

//    string temp = "";
//    for (auto& x : returns) temp += to_string(x) + " ";
//    wxLogStatus("Patch result: %d, returns %s", patch, temp);
}

// Control callbacks
void MopFrame::OnCloseWindow(wxCloseEvent& evt) {
    prefMgr->SaveToFile();
    evt.Skip();
}
void MopFrame::OnVerChosen(wxCommandEvent& evt) {
    prefMgr->SetVer(evt.GetString());
}
void MopFrame::OnObjChanged(wxCommandEvent& evt) {
    prefMgr->SetObj(evt.GetInt());
}
void MopFrame::OnAutoVerChecked(wxCommandEvent& evt) {
    prefMgr->SetAutoVer(evt.GetInt());
    Update();
}
void MopFrame::OnLateVerChecked(wxCommandEvent& evt) {
    prefMgr->SetLateVer(evt.GetInt());
    UpdateVerList();
}
//void MopFrame::OnPfmChosen(wxCommandEvent& evt) {
//    prefMgr->SetPfm(evt.GetString());
//}
void MopFrame::OnPathChanged(wxCommandEvent& evt) {
    prefMgr->SetPath(evt.GetString());
}
#ifdef DEBUG // Debug control callbacks
void MopFrame::OnTestButton(wxCommandEvent& evt) {
    // No more offensive jokes here.
    wxMessageBox("This is a debug pop-up. You're welcome!", "Hello", wxICON_INFORMATION);
    Test();
}
#endif // DEBUG
