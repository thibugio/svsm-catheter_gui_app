#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/app.h>
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

#include "CatheterGui.h"
#include "common_utils.h"
#include "pc_utils.h"

#define CHANNEL_COL 0
#define CURRENT_COL 1
#define DIRECTION_COL 2
#define DELAY_COL 3

#define NFIELDS 4
#define NROWS_DEFAULT 1//NCHANNELS

#define playfile_wildcard wxT("*.play")


IMPLEMENT_APP(CatheterGuiApp)

bool CatheterGuiApp::OnInit() {
    CatheterGuiFrame* gui = new CatheterGuiFrame(wxT("Catheter Gui"));
    gui->Show(true);
    return (gui != NULL);
}

wxBEGIN_EVENT_TABLE(CatheterGuiFrame, wxFrame)
    EVT_GRID_CELL_CHANGING(CatheterGuiFrame::OnGridCellChanging)
    EVT_BUTTON(CatheterGuiFrame::ID_SELECT_PLAYFILE_BUTTON, CatheterGuiFrame::OnSelectPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_NEW_PLAYFILE_BUTTON, CatheterGuiFrame::OnNewPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SAVE_PLAYFILE_BUTTON, CatheterGuiFrame::OnSavePlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_COMMANDS_BUTTON, CatheterGuiFrame::OnSendCommandsButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_RESET_BUTTON, CatheterGuiFrame::OnSendResetButtonClicked)
wxEND_EVENT_TABLE()

CatheterGuiFrame::CatheterGuiFrame(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title) {

    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(0, 0);

    grid->EnableDragGridSize(true);
    grid->SetTabBehaviour(wxGrid::Tab_Wrap);

    grid->AppendCols(NFIELDS);
    grid->AppendRows(NROWS_DEFAULT);

    formatDefaultGrid(NROWS_DEFAULT);

    dir_choices = new wxString[2];
    dir_choices[DIR_POS] = wxT("pos");
    dir_choices[DIR_NEG] = wxT("neg");

    cmdCount = 0;

    // status panel    
    statusText = new wxStaticText(this, wxID_ANY, wxEmptyString);

    // control buttons
    selectPlayfileButton = new wxButton(this, ID_SELECT_PLAYFILE_BUTTON, wxT("Select Playfile"));
    newPlayfileButton = new wxButton(this, ID_NEW_PLAYFILE_BUTTON, wxT("New Playfile"));
    savePlayfileButton = new wxButton(this, ID_SAVE_PLAYFILE_BUTTON, wxT("Save Playfile"));
    sendCommandsButton = new wxButton(this, ID_SEND_COMMANDS_BUTTON, wxT("Send Commands"));
    sendResetButton = new wxButton(this, ID_SEND_RESET_BUTTON, wxT("Send Reset"));

    serialConnected = false;
    playfileSaved = false;
    playfilePath = wxEmptyString;
    portName = wxEmptyString;

    // frame
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(selectPlayfileButton);
    hbox->Add(newPlayfileButton);
    hbox->Add(savePlayfileButton);
    hbox->Add(sendCommandsButton);
    hbox->Add(sendResetButton);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 1, wxEXPAND | wxALL, 5);
    vbox->Add(grid, 1, wxEXPAND | wxALL, 5);
    vbox->Add(statusText, 1, wxEXPAND | wxALL, 5);

    this->SetSizer(vbox);
    vbox->SetSizeHints(this);
    vbox->Fit(this);

    this->Fit();
    this->Center();

    setStatusText(wxT("Welcome to Catheter Gui"));
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
    //last value: grid->GetCellValue(row, col)
    //pending value: e.GetString()

    int row = e.GetRow();
    int col = e.GetCol();

    int channel;
    double currentMA;
    dir_t direction;
    int delayMS;
    switch (col) {
    case CHANNEL_COL:
        channel = wxAtoi(e.GetString());
        setGridRowChannel(row, channel);
        break;
    case CURRENT_COL:
        currentMA = wxAtof(e.GetString());
        setGridRowCurrentMA(row, currentMA);
        break;
    case DIRECTION_COL:
        direction = (wxStrcmp(dir_choices[DIR_POS], e.GetString()) ? DIR_NEG : DIR_POS);
        setGridRowDirection(row, direction);
        break;
    case DELAY_COL:
        delayMS = wxAtoi(e.GetString());
        setGridRowDelayMS(row, delayMS);
        break;
    }

    if (row == cmdCount && isGridRowComplete(row)) {
        cmdCount++;
        if (cmdCount < grid->GetNumberRows())
            setRowReadOnly(cmdCount, false);
        else
            addGridRow(false);
        wxMessageBox(wxString::Format("completed command %d", cmdCount));
    }

    e.Skip();
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

void CatheterGuiFrame::addGridRow(bool readOnly) {
    grid->AppendRows(1);
    formatDefaultRow(grid->GetNumberRows() - 1);
    setRowReadOnly(grid->GetNumberRows() - 1, readOnly);
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
            addGridRow(true);
    formatDefaultGrid(nrows);

    setRowReadOnly(0, false);
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
    const wxString choices[] = {wxT("neg"), wxT("pos")};
    grid->SetCellEditor(row, CHANNEL_COL, new wxGridCellNumberEditor(1, NCHANNELS));
    grid->SetCellEditor(row, CURRENT_COL, new wxGridCellFloatEditor(3, 3));
    grid->SetCellEditor(row, DIRECTION_COL, new wxGridCellChoiceEditor(WXSIZEOF(choices), choices));
    grid->SetCellEditor(row, DELAY_COL, new wxGridCellNumberEditor(0,3600));
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
        if (i >= grid->GetNumberRows())
            addGridRow(false);

        setGridRowChannel(i, gridCmds[i].channel);
        setGridRowCurrentMA(i, gridCmds[i].currentMA);
        setGridRowDelayMS(i, gridCmds[i].delayMS);
    }
    addGridRow(false);
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