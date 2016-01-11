#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "CatheterGuiApp.h"
#include "CatheterGuiFrame.h"

IMPLEMENT_APP(CatheterGuiApp)

bool CatheterGuiApp::OnInit() {
    CatheterGuiFrame* gui = new CatheterGuiFrame(wxT("Catheter Gui"));
    gui->Show(true);
    return (gui != NULL);
}