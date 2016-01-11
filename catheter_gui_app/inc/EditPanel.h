#pragma once
#ifndef EDIT_PANEL_H
#define EDIT_PANEL_H

#include <vector>

class wxGrid;
class wxGridEvent;
struct CatheterChannelCmd;

class EditPanel : public wxPanel {
    public:
    EditPanel(wxPanel* parent);

    void OnGridCellChanging(wxGridEvent& e);

    void addCommandRow();

    //wxPanel* parent;
    wxGrid* grid;
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