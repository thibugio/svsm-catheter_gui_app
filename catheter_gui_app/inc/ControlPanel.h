#pragma once
#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

//#include "gui_common.h"
#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/button.h>

#define playfile_wildcard wxT("*.play")

class ControlPanel : public wxPanel {
    public:
    ControlPanel(wxPanel* parent);

    void OnSelectPlayfileButtonClicked(wxCommandEvent& e);
    void OnAddCommandButtonClicked(wxCommandEvent& e);

    wxPanel* parent;
    wxButton* selectPlayfileButton;
    wxButton* newPlayfileButton;
    wxButton* addCommandButton;
    wxButton* savePlayfileButton;
    wxButton* sendCommandsButton;
    wxButton* sendResetButton;

    bool serialConnected;
    bool playfileSaved;
    wxString playfilePath;
    wxString portName; 

    private:
    
};

enum {
    ID_SELECT_PLAYFILE_BUTTON = 1024,
    ID_NEW_PLAYFILE_BUTTON,
    ID_ADD_COMMAND_BUTTON,
    ID_SAVE_PLAYFILE_BUTTON,
    ID_SEND_COMMANDS_BUTTON,
    ID_SEND_RESET_BUTTON
};

#endif