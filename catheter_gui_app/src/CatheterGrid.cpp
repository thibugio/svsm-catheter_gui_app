#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/headerctrl.h>
#include <wx/generic/gridctrl.h>
#include <wx/generic/grideditors.h>

#include "CatheterGrid.h"
#include "com/common_utils.h"

#define CHANNEL_COL 0
#define CURRENT_COL 1
#define DIRECTION_COL 2
#define DELAY_COL 3

#define NFIELDS 4
#define NROWS_DEFAULT 1//NCHANNELS

// file definitions
#define playfile_wildcard wxT("*.play")
#define portfile wxT("ports.txt")

// cell choice string definitions
#define DIRNEGSTR wxT("neg")
#define DIRPOSSTR wxT("pos")
#define GLOBALSTR wxT("global")

wxBEGIN_EVENT_TABLE(CatheterGrid, wxGrid)
    EVT_GRID_CELL_CHANGING(CatheterGrid::OnGridCellChanging)
    EVT_GRID_TABBING(CatheterGrid::OnGridTabbing)
wxEND_EVENT_TABLE()

CatheterGrid::CatheterGrid(wxPanel* parentPanel) :
    wxGrid(parentPanel, wxID_ANY) {

    this->CreateGrid(0, 0);

    this->EnableDragGridSize(true);

    this->AppendCols(NFIELDS);
    this->AppendRows(NROWS_DEFAULT);

    formatDefaultGrid(NROWS_DEFAULT);

    cmdCount = 0;
}

CatheterGrid::~CatheterGrid() { }

///////////////////////////
// event handler methods //
///////////////////////////

void CatheterGrid::OnGridCellChanging(wxGridEvent& e) {
    //called when edited cell loses focus
    //last value: GetCellValue(row, col)
    //pending value: e.GetString()

    int row = e.GetRow();
    int col = e.GetCol();

    switch (col) {
    case CHANNEL_COL:
        //setGridRowChannel(row, wxAtoi(e.GetString()));
        setGridRowChannel(row, e.GetString());
        break;
    case CURRENT_COL:
        setGridRowCurrentMA(row, wxAtof(e.GetString()));
        break;
    case DIRECTION_COL:
        setGridRowDirection(row, (wxStrcmp(DIRPOSSTR, e.GetString()) ? DIR_NEG : DIR_POS));
        break;
    case DELAY_COL:
        setGridRowDelayMS(row, wxAtoi(e.GetString()));
        break;
    }

    if (row == cmdCount && isGridRowComplete(row)) {
        cmdCount++;
        if (cmdCount < GetNumberRows())
            setRowReadOnly(cmdCount, false);
        else
            addGridRow(false);
        wxMessageBox(wxString::Format("completed command %d", cmdCount));
    }

    e.Skip();
}

void CatheterGrid::OnGridTabbing(wxGridEvent& e) {
    //SetFocus();
}

////////////////////
// public methods //
////////////////////

void CatheterGrid::GetCommands(std::vector<CatheterChannelCmd>& cmds) {
    cmds.clear();
    for (int i = 0; i < cmdCount; i++) {
        cmds.push_back(parseGridRowCmd(i));
    }
}

void CatheterGrid::SetCommands(const std::vector<CatheterChannelCmd>& cmds) {
    resetDefaultGrid(NROWS_DEFAULT);

    for (int i = 0; i < cmds.size(); i++) {
        cmdCount++;
        if (i >= GetNumberRows())
            addGridRow(false);

        setGridRowChannel(i, cmds[i].channel);
        setGridRowCurrentMA(i, cmds[i].currentMA);
        setGridRowDelayMS(i, cmds[i].delayMS);
    }
    addGridRow(false);
}

void CatheterGrid::ResetDefault() {
    resetDefaultGrid(NROWS_DEFAULT);
}

//////////////////////////////////
// command grid private methods //
//////////////////////////////////

void CatheterGrid::setGridRowChannel(int row, int channel) {
    if (isGridRowNumValid(row)) {
        if (channel > 0 && channel <= NCHANNELS) {
            SetCellValue(wxGridCellCoords(row, CHANNEL_COL), wxString::Format("%d", channel));
        } else if (channel == GLOBAL_ADDR) {
            SetCellValue(wxGridCellCoords(row, CHANNEL_COL), GLOBALSTR);
        }
    }
}

void CatheterGrid::setGridRowChannel(int row, const wxString& channel) {
    if (isGridRowNumValid(row)) {
        SetCellValue(wxGridCellCoords(row, CHANNEL_COL), channel);
    }
}

void CatheterGrid::setGridRowCurrentMA(int row, double currentMA) {
    if (isGridRowNumValid(row)) {
        SetCellValue(wxGridCellCoords(row, CURRENT_COL), wxString::Format("%3.3f", currentMA));
        if (currentMA > 0)
            setGridRowDirection(row, DIR_POS);
        else if (currentMA < 0)
            setGridRowDirection(row, DIR_NEG);
    }
}

void CatheterGrid::setGridRowDirection(int row, dir_t direction) {
    if (isGridRowNumValid(row)) {
        switch (direction) {
        case DIR_POS:
            SetCellValue(wxGridCellCoords(row, DIRECTION_COL), DIRPOSSTR);
            if (!isGridCellEmpty(row, CURRENT_COL) && getGridRowCurrentMA(row) < 0)
                setGridRowCurrentMA(row, getGridRowCurrentMA(row) * -1);
            break;
        case DIR_NEG:
            SetCellValue(wxGridCellCoords(row, DIRECTION_COL), DIRNEGSTR);
            if (!isGridCellEmpty(row, CURRENT_COL) && getGridRowCurrentMA(row) > 0)
                setGridRowCurrentMA(row, getGridRowCurrentMA(row) * -1);
            break;
        }
    }
}

void CatheterGrid::setGridRowDelayMS(int row, int delayMS) {
    if (isGridRowNumValid(row)) {
        if (delayMS >= 0) {
            SetCellValue(wxGridCellCoords(row, DELAY_COL), wxString::Format("%d", delayMS));
        }
    }
}

int CatheterGrid::getGridRowChannel(int row) {
    const wxString& channel = GetCellValue(wxGridCellCoords(row, CHANNEL_COL));
    if (!wxStrcmp(channel, "global"))
        return GLOBAL_ADDR;
    else
        return wxAtoi(channel);
}

double CatheterGrid::getGridRowCurrentMA(int row) {
    return wxAtof(GetCellValue(wxGridCellCoords(row, CURRENT_COL)));
}

dir_t CatheterGrid::getGridRowDirection(int row) {
    const wxString& dirStr = GetCellValue(wxGridCellCoords(row, DIRECTION_COL));
    if (!wxStrcmp(dirStr, DIRPOSSTR)) {
        return DIR_POS;
    } else {
        return DIR_NEG;
    }
}

int CatheterGrid::getGridRowDelayMS(int row) {
    return wxAtoi(GetCellValue(wxGridCellCoords(row, DELAY_COL)));
}

CatheterChannelCmd CatheterGrid::parseGridRowCmd(int row) {
    CatheterChannelCmd c;
    c.channel = getGridRowChannel(row);
    c.currentMA = getGridRowCurrentMA(row);
    c.delayMS = getGridRowDelayMS(row);
    c.poll = false;
    return c;
}

bool CatheterGrid::isGridRowNumValid(int row) {
    return (row < GetNumberRows() && !IsReadOnly(row, 0));
}
bool CatheterGrid::isGridCellEmpty(int row, int col) {
    return GetTable()->IsEmptyCell(row, col);
    //return GetCellValue(row, col).IsEmpty();
}

bool CatheterGrid::isGridRowComplete(int row) {
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

void CatheterGrid::addGridRow(bool readOnly) {
    AppendRows(1);
    formatDefaultRow(GetNumberRows() - 1);
    setRowReadOnly(GetNumberRows() - 1, readOnly);
}

void CatheterGrid::formatDefaultGrid(int nrows) {
    SetColLabelValue(CHANNEL_COL, wxT("Channel"));
    SetColLabelValue(CURRENT_COL, wxT("Current (MA)"));
    SetColLabelValue(DIRECTION_COL, wxT("Direction"));
    SetColLabelValue(DELAY_COL, wxT("Delay (ms)"));
    //HideRowLabels();

    SetColFormatNumber(CHANNEL_COL); //channel address
    SetColFormatFloat(CURRENT_COL); // MA current
                                          //default is String for Direction
    SetColFormatNumber(DELAY_COL); //delay

    for (int i = 0; i < nrows; i++)
        formatDefaultRow(i);

    setRowReadOnly(0, false);
}

void CatheterGrid::resetDefaultGrid(int nrows) {
    cmdCount = 0;

    DeleteRows(0, GetNumberRows());
    for (int i = 0; i < nrows; i++)
        addGridRow(true);

    formatDefaultGrid(nrows);

    setRowReadOnly(0, false);
}

void CatheterGrid::setRowReadOnly(int row, bool readOnly) {
    if (row >= GetNumberRows())
        return;
    SetReadOnly(row, CHANNEL_COL, readOnly);
    SetReadOnly(row, CURRENT_COL, readOnly);
    SetReadOnly(row, DIRECTION_COL, readOnly);
    SetReadOnly(row, DELAY_COL, readOnly);
}

void CatheterGrid::formatDefaultRow(int row) {
    if (row >= GetNumberRows())
        return;
    const wxString direction_opts[] = { DIRNEGSTR, DIRPOSSTR };

    wxString channel_opts[NCHANNELS + 1];
    channel_opts[0] = GLOBALSTR;
    for (int i = 1; i <= NCHANNELS; i++)
        channel_opts[i] = wxString::Format("%d", i);

    SetCellEditor(row, CHANNEL_COL, new wxGridCellChoiceEditor(WXSIZEOF(channel_opts), (const wxString*)channel_opts));
    SetCellEditor(row, CURRENT_COL, new wxGridCellFloatEditor(3, 3));
    SetCellRenderer(row, CURRENT_COL, new wxGridCellFloatRenderer());
    SetCellEditor(row, DIRECTION_COL, new wxGridCellChoiceEditor(WXSIZEOF(direction_opts), direction_opts));
    SetCellEditor(row, DELAY_COL, new wxGridCellNumberEditor(0, 3600));
    SetCellRenderer(row, DELAY_COL, new wxGridCellNumberRenderer());
    setRowReadOnly(row, true);
}