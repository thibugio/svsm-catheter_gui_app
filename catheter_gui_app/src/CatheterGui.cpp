#include "CatheterGui.h"
#include <wx/frame.h>

CatheterGui::CatheterGui(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title) {

    parentPanel = new wxPanel(this, wxID_ANY);
    editPanel = new EditPanel(parentPanel);
    statusPanel = new StatusPanel(parentPanel);
    controlPanel = new ControlPanel(parentPanel);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(controlPanel, 1, wxEXPAND | wxALL, 5);
    vbox->Add(editPanel, 1, wxEXPAND | wxALL, 5);
    vbox->Add(statusPanel, 1, wxEXPAND | wxALL, 5);

    parentPanel->SetSizer(vbox);
    vbox->SetSizeHints(parentPanel);
    vbox->Fit(parentPanel);

    Fit();
    Center();
}