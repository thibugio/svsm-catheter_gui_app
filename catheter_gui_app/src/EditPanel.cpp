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

#include "EditPanel.h"
#include "com/common_utils.h"

EditPanel::EditPanel(wxPanel* parentPanel) :
    wxPanel(parentPanel, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN) {

    //parent = parentPanel;

    dir_choices = new wxString[2];
    dir_choices[DIR_POS] = wxT("pos");
    dir_choices[DIR_NEG] = wxT("neg");

    cmdCount = 0;

    //gridCmds
    addCommand();

    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(NCHANNELS, NFIELDS); 

    grid->GetTable()->SetAttrProvider(new wxGridCellAttrProvider());  
    for (int i = 0; i < NFIELDS; i++)
        grid->GetTable()->GetAttrProvider()->SetColAttr(new wxGridCellAttr(), i);

    grid->EnableDragGridSize(true);
    grid->SetTabBehaviour(wxGrid::Tab_Wrap);

    formatGrid(NCHANNELS);

    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(grid);
    this->SetSizer(hbox);
    hbox->SetSizeHints(this);
    hbox->Fit(this);
    
    grid->Connect(wxEVT_GRID_CELL_CHANGING, wxGridEventHandler(EditPanel::OnGridCellChanging));

    Fit();
    Center();
}

void EditPanel::formatGrid(int nrows) {
    grid->SetColLabelValue(CHANNEL_COL, wxT("Channel"));
    grid->SetColLabelValue(CURRENT_COL, wxT("Current (MA)"));
    grid->SetColLabelValue(DIR_COL, wxT("Direction"));
    grid->SetColLabelValue(DELAY_COL, wxT("Delay (ms)"));
    //HideRowLabels();

    grid->SetColFormatNumber(CHANNEL_COL); //channel address
    grid->SetColFormatFloat(CURRENT_COL); // MA current
    //default is String for Direction
    grid->SetColFormatNumber(DELAY_COL); //delay

    for (int i = 0; i < nrows; i++)
        formatRow(i);

    setRowReadOnly(0, false);
}

void EditPanel::OnGridCellChanging(wxGridEvent& e) {
    //called when edited cell loses focus
    int row = e.GetRow();
    int col = e.GetCol();
    wxString s = e.GetString();

    wxMessageBox(wxString::Format(wxT("Grid Cell (%d, %d): %s"), row, col, s));
    wxMessageBox(wxString::Format("gridCmds.size()=%d, cmdCount=%d", gridCmds.size(), cmdCount));
    if (isCommandComplete(row)) {
        cmdCount++;
        addCommand();
        addCommandRow();
        wxMessageBox(wxString::Format("command: channel=%d current=%3.3f delay=%d", 
                                      gridCmds[row].channel, gridCmds[row].currentMA, gridCmds[row].delayMS));
    }
    switch (col) {
    case CHANNEL_COL:
        setCommandChannel(row, s);
        break;
    case CURRENT_COL:
        setCommandCurrentMA(row, s);
        break;
    case DIR_COL:
        setCommandDirection(row, s);
        break;
    case DELAY_COL:
        setCommandDelayMS(row, s);
        break;
    }
    //if (isRowComplete(row)) {
    //    cmdCount++;
    //    addCommandRow();
    //    wxMessageBox(wxString::Format("Grid cell (%d, %d) changing!\nCmdCount=%d", row, col, cmdCount));
    //}
}

void EditPanel::setCommandChannel(int row, wxString& s) {
    gridCmds[row].channel = wxAtoi(s);
}

void EditPanel::setCommandCurrentMA(int row, wxString& s) {
    double currentMA = wxAtof(s);
    gridCmds[row].currentMA = currentMA;

    if (currentMA < 0)
        grid->SetCellValue(wxGridCellCoords(row, DIR_COL), dir_choices[DIR_NEG]);
    else if (currentMA > 0)
        grid->SetCellValue(wxGridCellCoords(row, DIR_COL), dir_choices[DIR_POS]);
}

void EditPanel::setCommandDirection(int row, wxString& s) {
    if (gridCmds[row].currentMA != NAN) {
        if ((!wxStrcmp(s, dir_choices[DIR_POS]) && gridCmds[row].currentMA < 0) || 
            (!wxStrcmp(s, dir_choices[DIR_NEG]) && gridCmds[row].currentMA > 0)) {

            gridCmds[row].currentMA *= -1;
            grid->SetCellValue(wxGridCellCoords(row, CURRENT_COL), 
                                  wxString::Format("3.3f", gridCmds[row].currentMA));
        }
    }
}

void EditPanel::setCommandDelayMS(int row, wxString& s) {
    gridCmds[row].delayMS = wxAtoi(s);
}

void EditPanel::addCommandRow() {
    if (cmdCount >= grid->GetNumberRows()) {
        grid->AppendRows(1);
        formatRow(grid->GetNumberRows() - 1);
        Fit();
    }
    setRowReadOnly(cmdCount, false);
}

void EditPanel::formatRow(int row) {
    grid->SetCellEditor(row, CHANNEL_COL, new wxGridCellNumberEditor(1, NCHANNELS));
    grid->SetCellEditor(row, CURRENT_COL, new wxGridCellFloatEditor(3, 3));
    grid->SetCellEditor(row, DIR_COL, new wxGridCellEnumEditor(wxT("neg,pos")));
    grid->SetCellEditor(row, DELAY_COL, new wxGridCellNumberEditor(0, 3600));
    //setRowReadOnly(row, true);
}

void EditPanel::setRowReadOnly(int row, bool readOnly) {
    grid->SetReadOnly(row, CHANNEL_COL, readOnly);
    grid->SetReadOnly(row, CURRENT_COL, readOnly);
    grid->SetReadOnly(row, DIR_COL, readOnly);
    grid->SetReadOnly(row, DELAY_COL, readOnly);
}

bool EditPanel::isCmdNumValid(int row) {
    return (row < cmdCount);
}

bool EditPanel::isGridCellEmpty(int row, int col) {
    return grid->GetTable()->IsEmptyCell(row, col);
    //return grid->GetCellValue(row, col).IsEmpty();
}

bool EditPanel::isRowComplete(int row) {
    bool row_complete = isCmdNumValid(row);
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

void EditPanel::addCommand() {
    CatheterChannelCmd c;
    c.channel = -1;
    c.currentMA = NAN;
    c.delayMS = NAN;
    c.poll = false;

    gridCmds.push_back(c);
    wxMessageBox(wxString::Format("gridCmds.size()=%d, cmdCount=%d", gridCmds.size(), cmdCount));
}

bool EditPanel::isCommandComplete(int n) {
    bool complete = false;
    if (n < cmdCount) {
        CatheterChannelCmd c = gridCmds[n];
        complete = (c.channel != -1 && c.currentMA != NAN && c.delayMS != NAN);
    }
    return complete;
}