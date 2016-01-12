#pragma once
#ifndef CATHETER_GRID_H
#define CATHETER_GRID_H

#include <vector>

class wxString;

class wxPanel;

class wxGrid;
class wxGridEvent;

struct CatheterChannelCmd;
enum dir_t;

class CatheterGrid : public wxGrid {
    public:
    CatheterGrid(wxPanel* parent);
    ~CatheterGrid();

    void OnGridCellChanging(wxGridEvent& e);
    void OnGridTabbing(wxGridEvent& e);

    void GetCommands(std::vector<CatheterChannelCmd>& cmds);
    void SetCommands(const std::vector<CatheterChannelCmd>& cmds);
    void ResetDefault();

    wxDECLARE_EVENT_TABLE();

    private:
    void setRowReadOnly(int row, bool readOnly);
    void formatDefaultRow(int row);
    void formatDefaultGrid(int nrows);
    void resetDefaultGrid(int nrows);
    bool isGridRowNumValid(int row);
    bool isGridCellEmpty(int row, int col);
    bool isGridRowComplete(int row);
    CatheterChannelCmd parseGridRowCmd(int row);
    void addGridRow(bool readOnly);
    void setGridRowChannel(int row, int channel);
    void setGridRowChannel(int row, const wxString& channel);
    void setGridRowCurrentMA(int row, double currentMA);
    void setGridRowDirection(int row, dir_t direction);
    void setGridRowDelayMS(int row, int delayMS);
    int getGridRowChannel(int row);
    double getGridRowCurrentMA(int row);
    dir_t getGridRowDirection(int row);
    int getGridRowDelayMS(int row);

    unsigned int cmdCount;
};

#endif