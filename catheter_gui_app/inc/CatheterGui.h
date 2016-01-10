#pragma once
#ifndef CATHETER_GUI_H
#define CATHETER_GUI_H

#include "EditPanel.h"
#include "StatusPanel.h"
#include "ControlPanel.h"

class CatheterGui : public wxFrame {
    public:
        CatheterGui(const wxString& title);

        wxPanel* parentPanel;
        EditPanel* editPanel;
        StatusPanel* statusPanel;
        ControlPanel* controlPanel;
};

#endif