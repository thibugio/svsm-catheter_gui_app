#pragma once
#ifndef EDIT_PANEL_H
#define EDIT_PANEL_H

#include "com/common_utils.h"
//#include "gui_common.h"
#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <vector>

#define CHANNEL_COL 0
#define CURRENT_COL 1
#define DIR_COL 2
#define DELAY_COL 3

#define NFIELDS 4

class EditPanel : public wxPanel {
    public:
    EditPanel(wxPanel* parent);

    void OnGridCellChanging(wxGridEvent& e);

    void addCommandRow();

    //wxPanel* parent;
    wxGrid* cmdGrid;
    unsigned int cmdCount;
    wxString* dir_choices;
    std::vector<CatheterChannelCmd> gridCmds;

    private:
    void formatRow(int row);
    void formatGrid(int nrows);    
    void setRowReadOnly(int row, bool readOnly);
    bool isCmdNumValid(int row);
    bool isGridCellEmpty(int row, int col);
    bool isRowComplete(int row);
    void addCommand();
    bool isCommandComplete(int n);
    void setCommandChannel(int row, wxString& channel);
    void setCommandCurrentMA(int row, wxString& currentMA);
    void setCommandDirection(int row, wxString& dir);
    void setCommandDelayMS(int row, wxString& delay);
};

#endif