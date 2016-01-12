#pragma once
#ifndef CATHETER_GUI_APP_H
#define CATHETER_GUI_APP_H

#include <vector>

class wxString;
class wxApp;

class wxButton;
class wxCommandEvent;

class wxStaticText;

class CatheterGrid;

struct CatheterChannelCmd;
enum dir_t;


class CatheterGuiApp : public wxApp {
    public:
    bool OnInit();
};

class CatheterGuiFrame : public wxFrame {
    public:
    CatheterGuiFrame(const wxString& title);
    ~CatheterGuiFrame();

    // control buttons
    void OnRefreshSerialButtonClicked(wxCommandEvent& e);
    void OnSelectPlayfileButtonClicked(wxCommandEvent& e);
    void OnNewPlayfileButtonClicked(wxCommandEvent& e);
    void OnSavePlayfileButtonClicked(wxCommandEvent& e);
    void OnSendCommandsButtonClicked(wxCommandEvent& e);
    void OnSendResetButtonClicked(wxCommandEvent& e);

    enum {
        ID_SELECT_PLAYFILE_BUTTON = 1024,
        ID_NEW_PLAYFILE_BUTTON,
        ID_SAVE_PLAYFILE_BUTTON,
        ID_SEND_COMMANDS_BUTTON,
        ID_SEND_RESET_BUTTON, 
        ID_REFRESH_SERIAL_BUTTON
    };

    wxDECLARE_EVENT_TABLE();

    private:
    // status panel
    void setStatusText(const wxString& msg);
    // control buttons
    void warnSavePlayfile();
    wxString savePlayfile();
    wxString openPlayfile();
    void loadPlayfile(const wxString& path);
    void unloadPlayfile(const wxString& path);
    bool sendCommands(std::vector<CatheterChannelCmd> cmdVect);
    bool sendResetCommand();
    bool openSerialConnection();
    bool closeSerialConnection();

    wxPanel* parentPanel;
    CatheterGrid* grid;
    std::vector<CatheterChannelCmd> gridCmds;
    // status panel
    wxStaticText* statusText;
    // control buttons
    wxButton* selectPlayfileButton;
    wxButton* newPlayfileButton;
    wxButton* savePlayfileButton;
    wxButton* sendCommandsButton;
    wxButton* sendResetButton;
    wxButton* refreshSerialButton;
    bool serialConnected;
    bool playfileSaved;
    wxString playfilePath;
    wxString portName;
};

#endif