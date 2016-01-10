#include "CatheterGuiApp.h"

IMPLEMENT_APP(CatheterGuiApp)

bool CatheterGuiApp::OnInit() {
    gui = new CatheterGui(wxT("Catheter Gui"));
    gui->Show(true);
    return (gui != NULL);
    return true;
}