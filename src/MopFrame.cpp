#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <filesystem>
#include "MopFrame.h"
#include "Patcher.h"
#include "PreferenceManager.h"
#include <wx/choicdlg.h>
using namespace std;

// constant arrays
const wxArrayString version {
    "1.0", "1.01", "1.02", "1.1", "1.11", "1.2", "1.21", "1.22", "1.3", "1.4", "1.41", "1.5", "1.51", "1.6", "1.7", "1.71", "1.8", "1.81", "1.811"
};
const wxArrayString platform {
    "Android", "Windows", "Покед дешь", "я дибил :("
};
const wxArrayString library {
    "armeabi", "armeabi-v7a", "x86"
};
const bool lateVer[] {
    0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1
};

// MopFrame constructor
MopFrame::MopFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title) {
    // preferences
    prefMgr->LoadFromFile();

    // sizers
    mainSizer->Add(prefSizer, wxSizerFlags().Expand().Border(wxALL, 10));
        prefSizer->Add(topSizer, wxSizerFlags().Expand());
            topSizer->Add(pVerSizer);
                pVerSizer->Add(verSizer);
                    verSizer->Add(verStaticText, staticTextFlags);
                    verSizer->Add(verChoice);
                pVerSizer->Add(pfmSizer);
                    pfmSizer->Add(pfmStaticText, staticTextFlags);
                    pfmSizer->Add(pfmChoice);
            #ifdef DEBUG
            topSizer->AddStretchSpacer(1);
            topSizer->Add(testButton, wxSizerFlags().Border(wxLEFT, 10));
            #endif // DEBUG
        prefSizer->Add(pCheckSizer, wxSizerFlags().Border(wxTOP, 5));
            pCheckSizer->Add(autoVerCheck);
            pCheckSizer->Add(lateVerCheck);
    mainSizer->Add(objStaticText, staticTextFlags.CenterHorizontal());
    mainSizer->Add(objSpinCtrl, wxSizerFlags().CenterHorizontal());
    mainSizer->AddStretchSpacer(1);
    mainSizer->Add(patchButton, wxSizerFlags().CenterHorizontal().Border(wxALL, 10));

    // frame
    panel->SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    wxSize clientSize = GetClientSize();
    wxSize patchBtnSize = patchButton->GetSize();
    SetMinClientSize(wxSize(clientSize.GetWidth(), clientSize.GetHeight() + patchBtnSize.GetHeight()));
    SetClientSize(320, 320);

    // controls
    CreateStatusBar();
    pfmSizer->Show(false);
    patchButton->SetFocus();
    verChoice->Select(verChoice->FindString(prefMgr->GetVer()));
    pfmChoice->Select(verChoice->FindString(prefMgr->GetPfm()));
    autoVerCheck->SetValue(prefMgr->GetAutoVer());
    if(prefMgr->GetLateVer()) {
        prefMgr->SetLateVer(true);
        UpdateVerList();
    }
    Update();

    // events
    this->Bind(wxEVT_CLOSE_WINDOW, &MopFrame::OnCloseWindow, this);
    patchButton->Bind(wxEVT_BUTTON, &MopFrame::OnPatchClicked, this);
    autoVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnAutoVerChecked, this);
    verChoice->Bind(wxEVT_CHOICE, &MopFrame::OnVerChosen, this);
    objSpinCtrl->Bind(wxEVT_TEXT, &MopFrame::OnObjChanged, this);
    lateVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnLateVerChecked, this);
    lateVerCheck->Bind(wxEVT_CHECKBOX, &MopFrame::OnLateVerChecked, this);
    pfmChoice->Bind(wxEVT_CHOICE, &MopFrame::OnPfmChosen, this);
    #ifdef DEBUG
    testButton->Bind(wxEVT_BUTTON, &MopFrame::OnTestButton, this);
    #endif // DEBUG
}

// update functions
void MopFrame::Update() {
    verChoice->SetSelection(verChoice->FindString(prefMgr->GetVer()));
    verChoice->Enable(!prefMgr->GetAutoVer());
    objSpinCtrl->SetValue(prefMgr->GetObj());
    autoVerCheck->SetValue(prefMgr->GetAutoVer());
    lateVerCheck->SetValue(prefMgr->GetLateVer());
    pfmChoice->SetSelection(pfmChoice->FindString(prefMgr->GetPfm()));
}
void MopFrame::UpdateVerList() {
    if(prefMgr->GetLateVer() && verChoice->GetCount() == version.GetCount()) {
        int j = verChoice->GetSelection();
        while(!lateVer[j] && j < (int)verChoice->GetCount()) j++;
        verChoice->SetSelection(j);
        prefMgr->SetVer(verChoice->GetString(j));
        for(int i = version.GetCount() - 1; i >= 0; i--) {
            if(!lateVer[i]) verChoice->Delete(i);
        }
    } else if(verChoice->GetCount() < version.GetCount()) {
        for(int i = 0; i < (int)version.GetCount(); i++) {
            if(!lateVer[i]) verChoice->Insert(version.Item(i), i);
        }
    }
}

// event functions
void MopFrame::OnPatchClicked(wxCommandEvent& evt) {
    bool useAutoVer = autoVerCheck->GetValue();
    auto ver = useAutoVer ? -1 : version.Index(verChoice->GetStringSelection());
    if(ver == wxNOT_FOUND && !useAutoVer) {
        SetStatusText("You need to select a version.");
        return;
    }
    auto obj = objSpinCtrl->GetValue();
    Patcher* patcher = new Patcher;
    string path = "";
    int patch = 0, libsPatched = 0, libsNotFound = 0;
    bool local = false;
    string file = ver < 14 ? "libgame.so" : "libcocos2dcpp.so";
//    std::vector<int> returns;

    for(int i = 0; i < 3; i++) {
        switch(i) {
        case 0:
            path = ""; break;
        case 1:
            path = "lib/"; break;
        }
        if(i >= 2) break;

        int lib = useAutoVer ? -1 : 0;
        if(filesystem::exists(path + file)) {
            local = true;
            if(ver >= 11 && !useAutoVer) {
                wxSingleChoiceDialog dialog(this, "Select your library architecture", "Library selection", library);
                int answer = dialog.ShowModal();
                if(answer == wxID_CANCEL) {
                    SetStatusText("You have cancelled the operation.");
                    return;
                }
                lib = dialog.GetSelection();
            }
        }

        while(lib < 3) {
            string libPath = local ? "" : (string)library.Item(lib) + "/";
            bool stop = false, visual = false;
            int approx = 0;
            if(ver < 0) ver = patcher->GetAutoVer(path + libPath + "libgame.so");
            if(ver < 0) ver = patcher->GetAutoVer(path + libPath + "libcocos2dcpp.so");
            if(ver < 0) {lib++; continue;}
            file = ver % 100 < 14 ? "libgame.so" : "libcocos2dcpp.so";
//            wxLogMessage("%d", ver);

            do {
                stop = true;
//                wxLogMessage("%s, %d, %d, %d, %d, %d", path + libPath + file, ver % 100, obj, useAutoVer ? ver / 100 : lib, approx, visual);
                int result = patcher->Patch(path + libPath + file, ver % 100, obj, useAutoVer ? ver / 100 : lib, approx, visual);
//                wxLogMessage(wxString(path + libPath + file));
//                returns.push_back(result);
                switch(result) {
                case 1:
                    patch = 1;
//                    wxLogMessage("%d", patcher->GetAutoVer(path + libPath + file));
                    ++libsPatched;
                    break;
                case 2: {
                    int approx0 = patcher->Approx(obj, 0), approx1=patcher->Approx(obj, 1);
                    wxMessageDialog dialog(this, wxString::Format("This library can't have the limit of %d objects.\nWhat limit do you want to use instead?", obj),
                                           wxString::Format("Approximation problem in %s", library[lib]), wxYES_NO | wxICON_WARNING | wxCENTER);
                    dialog.SetYesNoLabels(wxString::Format("%d", approx0), wxString::Format("%d", approx1));
                    int answer1 = dialog.ShowModal();
                    approx = answer1 == wxYES ? approx1 : approx0;
                    int answer2 = wxMessageBox("In some cases the game may round the object limit (e.g., 516 -> 512, 4992 -> 5000).\nDo you want to use approximations in the object counter and popup as well?",
                                 wxString::Format("Approximation problem in %s", library[lib]), wxYES_NO | wxICON_WARNING | wxCENTER | wxNO_DEFAULT);
                    visual = answer2 == wxYES;
                    stop = false;
                    break;
                }
                case 3:
                    ++libsNotFound;
                    break;
                }
            } while(stop != true);

            if(local || (ver < 11 && !useAutoVer)) break;
            if(useAutoVer) ver = -1;
            lib++;
        }
        if(patch == 1) break;
    }

    int libsTotal = (ver < 11 || local) ? 1 : 3;
    if(libsPatched > 0) {
        if(useAutoVer) SetStatusText(wxString::Format("Automatically patched %d libraries.", libsPatched));
        else SetStatusText(wxString::Format("Successfully patched %d of %d libraries.", libsPatched, libsTotal));
    }
    else if(libsNotFound >= libsTotal * 2) SetStatusText("Could not find the libraries.");
    else SetStatusText("None of the libraries could be patched.");

//    string temp = "";
//    for(auto& x : returns) temp += to_string(x) + " ";
//    SetStatusText(wxString::Format("Patch result: %d, returns %s", patch, temp));

    delete patcher;

}
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
void MopFrame::OnPfmChosen(wxCommandEvent& evt) {
    prefMgr->SetPfm(evt.GetString());
}

// debug event functions
#ifdef DEBUG
void MopFrame::OnTestButton(wxCommandEvent& evt) {
    wxSingleChoiceDialog dialog(this, "Select your library architecture", "Library selection", library);
    int result = dialog.ShowModal();
    if(result == wxID_CANCEL) {wxLogMessage("You have cancelled the operation."); return;}
    wxLogMessage("%d", dialog.GetSelection());
}
#endif // DEBUG
