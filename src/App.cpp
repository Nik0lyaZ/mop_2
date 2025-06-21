#include "App.h"
#include "MopFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    MopFrame* mopFrame = new MopFrame("Max Object Patcher");
    mopFrame->SetIcon(wxICON(aaaa));
    mopFrame->Center();
    mopFrame->Show();
    return true;
}
