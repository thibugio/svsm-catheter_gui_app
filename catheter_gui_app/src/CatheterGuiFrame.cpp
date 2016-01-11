#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/frame.h>
#include <wx/panel.h>
// command grid
#include <wx/grid.h>
#include <wx/headerctrl.h>
#include <wx/generic/gridctrl.h>
#include <wx/generic/grideditors.h>
// status panel
#include <wx/stattext.h>
// control buttons
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>

#include "CatheterGuiFrame.h"
#include "common_utils.h"
#include "pc_utils.h"

#define CHANNEL_COL 0
#define CURRENT_COL 1
#define DIRECTION_COL 2
#define DELAY_COL 3

#define NFIELDS 4
#define NROWS_DEFAULT NCHANNELS

#define playfile_wildcard wxT("*.play")

CatheterGuiFrame::CatheterGuiFrame(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title) {

    wxPanel* parentPanel = new wxPanel(this, wxID_ANY);

    // command grid
    grid->Connect(wxEVT_GRID_CELL_CHANGING, wxGridEventHandler(CatheterGuiFrame::OnGridCellChanging));

    wxPanel* editPanel = new wxPanel(parentPanel, wxID_ANY);

    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(NROWS_DEFAULT, NFIELDS);

    grid->EnableDragGridSize(true);
    grid->SetTabBehaviour(wxGrid::Tab_Wrap);

    formatDefaultGrid(NROWS_DEFAULT);

    wxBoxSizer* hbox_grid = new wxBoxSizer(wxHORIZONTAL);
    hbox_grid->Add(grid);
    editPanel->SetSizer(hbox_grid);
    hbox_grid->SetSizeHints(editPanel);
    hbox_grid->Fit(editPanel);

    editPanel->Fit();
    editPanel->Center();

    dir_choices = new wxString[2];
    dir_choices[DIR_POS] = wxT("pos");
    dir_choices[DIR_NEG] = wxT("neg");

    cmdCount = 0;
    
    // status panel
    wxPanel* statusPanel = new wxPanel(parentPanel, wxID_ANY);
    
    statusText = new wxStaticText(statusPanel, wxID_ANY, wxEmptyString);
    
    wxBoxSizer* hbox_status = new wxBoxSizer(wxHORIZONTAL);
    hbox_status->Add(statusText);
    statusPanel->SetSizer(hbox_status);
    hbox_status->SetSizeHints(statusPanel);
    hbox_status->Fit(statusPanel);
    
    statusPanel->Fit();
    statusPanel->Center();

    // control buttons
    Connect(ID_SELECT_PLAYFILE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, 
        wxCommandEventHandler(CatheterGuiFrame::OnSelectPlayfileButtonClicked));
    Connect(ID_NEW_PLAYFILE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, 
        wxCommandEventHandler(CatheterGuiFrame::OnNewPlayfileButtonClicked));
    Connect(ID_SAVE_PLAYFILE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(CatheterGuiFrame::OnSavePlayfileButtonClicked));
    Connect(ID_SEND_COMMANDS_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, 
        wxCommandEventHandler(CatheterGuiFrame::OnSendCommandsButtonClicked));
    Connect(ID_SEND_RESET_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(CatheterGuiFrame::OnSendResetButtonClicked));

    wxPanel* controlPanel = new wxPanel(parentPanel, wxID_ANY);
    
    selectPlayfileButton = new wxButton(controlPanel, ID_SELECT_PLAYFILE_BUTTON, wxT("Select Playfile"));
    newPlayfileButton = new wxButton(controlPanel, ID_NEW_PLAYFILE_BUTTON, wxT("New Playfile"));    
    savePlayfileButton = new wxButton(controlPanel, ID_SAVE_PLAYFILE_BUTTON, wxT("Save Playfile"));
    sendCommandsButton = new wxButton(controlPanel, ID_SEND_COMMANDS_BUTTON, wxT("Send Commands"));
    sendResetButton = new wxButton(controlPanel, ID_SEND_RESET_BUTTON, wxT("Send Reset"));
    
    wxBoxSizer* hbox_control = new wxBoxSizer(wxHORIZONTAL);
    hbox_control->Add(selectPlayfileButton);
    hbox_control->Add(newPlayfileButton);    
    hbox_control->Add(savePlayfileButton);
    hbox_control->Add(sendCommandsButton);
    hbox_control->Add(sendResetButton);

    controlPanel->SetSizer(hbox_control);
    hbox_control->SetSizeHints(controlPanel);
    hbox_control->Fit(controlPanel);

    controlPanel->Fit();
    controlPanel->Center();
    
    serialConnected = false;
    playfileSaved = true;
    playfilePath = wxEmptyString;
    portName = wxEmptyString;


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

CatheterGuiFrame::~CatheterGuiFrame() {
    if (serialConnected) {
        // send reset command
        // close serial connection
    }
}

///////////////////////////
// event handler methods //
///////////////////////////

// command grid
void CatheterGuiFrame::OnGridCellChanging(wxGridEvent& e) {
    //called when edited cell loses focus
    int row = e.GetRow();
    int col = e.GetCol();

    switch (col) {
    case CHANNEL_COL:
        break;
    case CURRENT_COL:
        setGridRowCurrentMA(row, getGridRowCurrentMA(row));
        break;
    case DIRECTION_COL:
        setGridRowDirection(row, getGridRowDirection(row));
        break;
    case DELAY_COL:
        break;
    }

    if (isGridRowComplete(row)) {
        cmdCount++;
        //gridCmds.push_back(parseGridRowCmd(row));
        addGridRow();
        wxMessageBox(wxString::Format("command: channel=%d current=%3.3f delay=%d",
            gridCmds[row].channel, gridCmds[row].currentMA, gridCmds[row].delayMS));
    }
}

// control buttons
void CatheterGuiFrame::OnSelectPlayfileButtonClicked(wxCommandEvent& e) {
    warnSavePlayfile();

    wxString path = openPlayfile();

    if (!path.IsEmpty()) {
        playfileSaved = false;
        playfilePath = path;
        
        resetDefaultGrid(NROWS_DEFAULT);
        loadPlayfile(playfilePath);
        
        setStatusText(wxString::Format(wxT("Editing Existing Playfile %s\n"), playfilePath));
    }
}

void CatheterGuiFrame::OnNewPlayfileButtonClicked(wxCommandEvent& e) {
    warnSavePlayfile();
    
    // clear command grid
    resetDefaultGrid(NROWS_DEFAULT);

    playfileSaved = false;
    playfilePath = wxEmptyString;

    setStatusText(wxT("Editing New Playfile\n"));
}

void CatheterGuiFrame::OnSavePlayfileButtonClicked(wxCommandEvent& e) {
    wxString path = savePlayfile();
    if (!path.IsEmpty()) {
        playfileSaved = true;
        playfilePath = path;
        // save contents of edit panel to playfilePath
        unloadPlayfile(playfilePath);
        setStatusText(wxString::Format(wxT("Saved Playfile as %s"), playfilePath));
    }
}

void CatheterGuiFrame::OnSendCommandsButtonClicked(wxCommandEvent& e) {
    if (serialConnected) {
        setStatusText(wxT("Sending Commands...\n"));
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

void CatheterGuiFrame::OnSendResetButtonClicked(wxCommandEvent& e) {
    if (serialConnected) {
        setStatusText(wxT("Sending Reset Command...\n"));
        sendResetCommand();
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

//////////////////////////////////
// command grid private methods //
//////////////////////////////////

void CatheterGuiFrame::setGridRowChannel(int row, int channel) {
    if (isGridRowNumValid(row)) {
        if (channel > 0 && channel <= NCHANNELS) {
            grid->SetCellValue(wxGridCellCoords(row, CHANNEL_COL), wxString::Format("%d", channel));
        }
    }
}

void CatheterGuiFrame::setGridRowCurrentMA(int row, double currentMA) {
    if (isGridRowNumValid(row)) {
        grid->SetCellValue(wxGridCellCoords(row, CURRENT_COL), wxString::Format("%3.3f", currentMA));
        if (currentMA > 0)
            setGridRowDirection(row, DIR_POS);
        else if (currentMA < 0)
            setGridRowDirection(row, DIR_NEG);
    }
}

void CatheterGuiFrame::setGridRowDirection(int row, dir_t direction) {
    if (isGridRowNumValid(row)) {
        switch (direction) {
        case DIR_POS:
            grid->SetCellValue(wxGridCellCoords(row, DIRECTION_COL), dir_choices[DIR_POS]);
            if (!isGridCellEmpty(row, CURRENT_COL) && getGridRowCurrentMA(row) < 0)
                setGridRowCurrentMA(row, getGridRowCurrentMA(row) * -1);
            break;
        case DIR_NEG:
            grid->SetCellValue(wxGridCellCoords(row, DIRECTION_COL), dir_choices[DIR_NEG]);
            if (!isGridCellEmpty(row, CURRENT_COL) && getGridRowCurrentMA(row) > 0)
                setGridRowCurrentMA(row, getGridRowCurrentMA(row) * -1);
            break;
        }
    }
}

void CatheterGuiFrame::setGridRowDelayMS(int row, int delayMS) {
    if (isGridRowNumValid(row)) {
        if (delayMS >= 0) {
            grid->SetCellValue(wxGridCellCoords(row, DELAY_COL), wxString::Format("%d", delayMS));
        }
    }
}

int CatheterGuiFrame::getGridRowChannel(int row) {
    return wxAtoi(grid->GetCellValue(wxGridCellCoords(row, CHANNEL_COL)));
}

double CatheterGuiFrame::getGridRowCurrentMA(int row) {
    return wxAtof(grid->GetCellValue(wxGridCellCoords(row, CURRENT_COL)));
}

dir_t CatheterGuiFrame::getGridRowDirection(int row) {
    const wxString& dirStr = grid->GetCellValue(wxGridCellCoords(row, DIRECTION_COL));
    if (!wxStrcmp(dirStr, dir_choices[DIR_POS])) {
        return DIR_POS;
    } else {
        return DIR_NEG;
    }
}

int CatheterGuiFrame::getGridRowDelayMS(int row) {
    return wxAtoi(grid->GetCellValue(wxGridCellCoords(row, DELAY_COL)));
}

CatheterChannelCmd CatheterGuiFrame::parseGridRowCmd(int row) {
    CatheterChannelCmd c;
    c.channel = getGridRowChannel(row);
    c.currentMA = getGridRowCurrentMA(row);
    c.delayMS = getGridRowDelayMS(row);
    c.poll = false;
    return c;
}

bool CatheterGuiFrame::isGridRowNumValid(int row) {
    return (row < grid->GetNumberRows() && !grid->IsReadOnly(row, 0));
}
bool CatheterGuiFrame::isGridCellEmpty(int row, int col) {
    return grid->GetTable()->IsEmptyCell(row, col);
    //return grid->GetCellValue(row, col).IsEmpty();
}

bool CatheterGuiFrame::isGridRowComplete(int row) {
    bool row_complete = isGridRowNumValid(row);
    if (row_complete) {
        for (int i = 0; i < NFIELDS; i++) {
            if (isGridCellEmpty(row, i)) {
                row_complete = false;
                break;
            }
        }
    }
    return row_complete;
}

void CatheterGuiFrame::addGridRow() {
    grid->AppendRows(1);
    formatDefaultRow(grid->GetNumberRows() - 1);
    setRowReadOnly(grid->GetNumberRows() - 1, true);
}

void CatheterGuiFrame::formatDefaultGrid(int nrows) {
    grid->SetColLabelValue(CHANNEL_COL, wxT("Channel"));
    grid->SetColLabelValue(CURRENT_COL, wxT("Current (MA)"));
    grid->SetColLabelValue(DIRECTION_COL, wxT("Direction"));
    grid->SetColLabelValue(DELAY_COL, wxT("Delay (ms)"));
    //HideRowLabels();

    grid->SetColFormatNumber(CHANNEL_COL); //channel address
    grid->SetColFormatFloat(CURRENT_COL); // MA current
                                          //default is String for Direction
    grid->SetColFormatNumber(DELAY_COL); //delay

    for (int i = 0; i < nrows; i++)
        formatDefaultRow(i);

    setRowReadOnly(0, false);
}

void CatheterGuiFrame::resetDefaultGrid(int nrows) {
    gridCmds.clear();
    grid->ClearGrid();
    
    if (grid->GetNumberRows() > nrows)
        grid->DeleteRows(grid->GetNumberRows() - nrows);
    else 
        for (int i = 0; i < (nrows - grid->GetNumberRows()); i++)
            addGridRow();
    formatDefaultGrid(nrows);

    for (int i = 0; i < nrows; i++)
        setRowReadOnly(i, i != 0);
}

void CatheterGuiFrame::setRowReadOnly(int row, bool readOnly) {
    if (row >= grid->GetNumberRows())
        return;
    grid->SetReadOnly(row, CHANNEL_COL, readOnly);
    grid->SetReadOnly(row, CURRENT_COL, readOnly);
    grid->SetReadOnly(row, DIRECTION_COL, readOnly);
    grid->SetReadOnly(row, DELAY_COL, readOnly);
}

void CatheterGuiFrame::formatDefaultRow(int row) {
    if (row >= grid->GetNumberRows())
        return;
    grid->SetCellEditor(row, CHANNEL_COL, new wxGridCellNumberEditor(1, NCHANNELS));
    grid->SetCellEditor(row, CURRENT_COL, new wxGridCellFloatEditor(3, 3));
    grid->SetCellEditor(row, DIRECTION_COL, new wxGridCellEnumEditor(wxT("neg,pos")));
    grid->SetCellEditor(row, DELAY_COL, new wxGridCellNumberEditor(0, 3600));
    setRowReadOnly(row, true);
}

//////////////////////////////////
// status panel private methods //
//////////////////////////////////

void CatheterGuiFrame::setStatusText(const wxString& msg) {
    statusText->SetLabel(msg);
}

///////////////////////////////////
// control panel private methods //
///////////////////////////////////

wxString CatheterGuiFrame::openPlayfile() {
    wxString path = wxEmptyString;
    wxFileDialog openDialog(this, "Open Playfile", wxGetCwd(), "", playfile_wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDialog.ShowModal() != wxID_CANCEL) {
        wxFileInputStream playfile_stream(openDialog.GetPath());
        if (playfile_stream.IsOk()) {
            path = openDialog.GetPath();
        } else {
            wxLogError("Selected file could not be opened.");
        }
    }
    return path;
}

wxString CatheterGuiFrame::savePlayfile() {
    wxString path = wxEmptyString;
    wxFileDialog saveDialog(this, wxT("Save Playfile"), wxGetCwd(), "", playfile_wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDialog.ShowModal() != wxID_CANCEL) {
        wxFileOutputStream save_stream(saveDialog.GetPath());
        if (!save_stream.IsOk()) {
            wxLogError("Could not save to selected file");
        } else {
            path = saveDialog.GetPath();
        }
    }
    return path;
}

void CatheterGuiFrame::loadPlayfile(const wxString& path) {
    loadPlayFile(path.mb_str(), gridCmds);
    for (int i = 0; i < gridCmds.size(); i++) {
        cmdCount++;
        if (i > grid->GetNumberRows())
            addGridRow();
        setRowReadOnly(i, false);
        setGridRowChannel(i, gridCmds[i].channel);
        setGridRowCurrentMA(i, gridCmds[i].currentMA);
        setGridRowDelayMS(i, gridCmds[i].delayMS);
    }
    addGridRow();
}

void CatheterGuiFrame::unloadPlayfile(const wxString& path) {
    gridCmds.clear();
    for (int i = 0; i < cmdCount; i++) {
        gridCmds.push_back(parseGridRowCmd(i));
    }

    writePlayFile(path.mb_str(), gridCmds);
}

void CatheterGuiFrame::warnSavePlayfile() {
    if (!playfileSaved) {
        if (wxMessageBox(wxT("Current content has not been saved!"), wxT("Proceed?"),
            wxICON_QUESTION | wxYES_NO, this) == wxNO) {
            savePlayfile();
            return;
        }
    }
}

//TODO
bool CatheterGuiFrame::sendCommands(std::vector<CatheterChannelCmd> cmdVect) {
    bool success = serialConnected;
    if (serialConnected) {

    }
    return success;
}

bool CatheterGuiFrame::sendResetCommand() {
    std::vector<CatheterChannelCmd> resetVect;
    resetVect.push_back(resetCommand());

    return sendCommands(resetVect);
}