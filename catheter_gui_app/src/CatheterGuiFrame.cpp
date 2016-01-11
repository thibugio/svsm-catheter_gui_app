#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/frame.h>

#include "CatheterGuiFrame.h"
#include "EditPanel.h"
#include "StatusPanel.h"
#include "ControlPanel.h"

CatheterGuiFrame::CatheterGuiFrame(const wxString& title) :
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

CatheterGuiFrame::~CatheterGuiFrame() {}