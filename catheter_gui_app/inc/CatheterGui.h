#pragma once
#ifndef CATHETER_GUI_APP_H
#define CATHETER_GUI_APP_H

#include <vector>

class wxString;
class wxApp;

class wxGrid;
class wxGridEvent;

class wxButton;
class wxCommandEvent;

class wxStaticText;

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

    // command grid
    void OnGridCellChanging(wxGridEvent& e);
    // control buttons
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
        ID_SEND_RESET_BUTTON
    };

    wxDECLARE_EVENT_TABLE();

    private:
    // command grid
    void setRowReadOnly(int row, bool readOnly);
    void formatDefaultRow(int row);
    void formatDefaultGrid(int nrows);
    void resetDefaultGrid(int nrows);
    bool isGridRowNumValid(int row);
    bool isGridCellEmpty(int row, int col);
    bool isGridRowComplete(int row);
    CatheterChannelCmd parseGridRowCmd(int row);
    void addGridRow();
    void setGridRowChannel(int row, int channel);
    void setGridRowCurrentMA(int row, double currentMA);
    void setGridRowDirection(int row, dir_t direction);
    void setGridRowDelayMS(int row, int delayMS);
    int getGridRowChannel(int row);
    double getGridRowCurrentMA(int row);
    dir_t getGridRowDirection(int row);
    int getGridRowDelayMS(int row);
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

    // command grid
    wxGrid* grid;
    unsigned int cmdCount;
    wxString* dir_choices;
    std::vector<CatheterChannelCmd> gridCmds;
    // status panel
    wxStaticText* statusText;
    // control buttons
    wxButton* selectPlayfileButton;
    wxButton* newPlayfileButton;
    wxButton* savePlayfileButton;
    wxButton* sendCommandsButton;
    wxButton* sendResetButton;
    bool serialConnected;
    bool playfileSaved;
    wxString playfilePath;
    wxString portName;
};

#endif