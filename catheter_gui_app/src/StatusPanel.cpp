#include "StatusPanel.h"

StatusPanel::StatusPanel(wxPanel* parentPanel) : 
    wxPanel(parentPanel, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN) {

    parent = parentPanel;

    statusText = new wxStaticText(this, wxID_ANY, wxEmptyString);

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(statusText);
    this->SetSizer(hbox);
    hbox->SetSizeHints(this);
    hbox->Fit(this);

    Fit();
    Center();
}