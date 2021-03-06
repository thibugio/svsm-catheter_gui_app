#include "CatheterGui.h"

ControlPanel::ControlPanel(wxPanel* parentPanel) :
    wxPanel(parentPanel, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN) {

    parent = parentPanel;
    serialConnected = false;
    playfileSaved = true;
    playfilePath = wxEmptyString;
    portName = wxEmptyString;

    selectPlayfileButton = new wxButton(this, ID_SELECT_PLAYFILE_BUTTON, wxT("Select Playfile"));
    newPlayfileButton = new wxButton(this, ID_NEW_PLAYFILE_BUTTON, wxT("New Playfile"));
    addCommandButton = new wxButton(this, ID_ADD_COMMAND_BUTTON, wxT("Add Command"));
    savePlayfileButton = new wxButton(this, ID_SAVE_PLAYFILE_BUTTON, wxT("Save Playfile"));
    sendCommandsButton = new wxButton(this, ID_SEND_COMMANDS_BUTTON, wxT("Send Commands"));
    sendResetButton = new wxButton(this, ID_SEND_RESET_BUTTON, wxT("Send Reset"));

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(selectPlayfileButton);
    hbox->Add(newPlayfileButton);
    hbox->Add(addCommandButton);
    hbox->Add(savePlayfileButton);
    hbox->Add(sendCommandsButton);
    hbox->Add(sendResetButton);

    this->SetSizer(hbox);
    hbox->SetSizeHints(this);
    hbox->Fit(this);
    
    Fit();
    Center();

    Connect(ID_SELECT_PLAYFILE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ControlPanel::OnSelectPlayfileButtonClicked));
    Connect(ID_ADD_COMMAND_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ControlPanel::OnAddCommandButtonClicked));
}

void ControlPanel::OnSelectPlayfileButtonClicked(wxCommandEvent& WXUNUSED(e)) {
    CatheterGui* gui = (CatheterGui*)(parent->GetParent());
    gui->statusPanel->statusText->SetLabel(wxT("Selecting Playfile"));
}

void ControlPanel::OnAddCommandButtonClicked(wxCommandEvent& WXUNUSED(e)) {
    CatheterGui* gui = (CatheterGui*)(parent->GetParent());
    gui->statusPanel->statusText->SetLabel(wxT("Adding Channel Command"));
    gui->editPanel->addCommandRow();
}