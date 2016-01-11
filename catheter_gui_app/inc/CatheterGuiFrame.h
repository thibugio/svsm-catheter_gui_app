#pragma once
#ifndef CATHETER_GUI_H
#define CATHETER_GUI_H

class wxPanel;
class EditPanel;
class StatusPanel;
class ControlPanel;
//#include "EditPanel.h"
//#include "StatusPanel.h"
//#include "ControlPanel.h"

class CatheterGuiFrame : public wxFrame {
    public:
        CatheterGuiFrame(const wxString& title);
        ~CatheterGuiFrame();

        wxPanel* parentPanel;
        EditPanel* editPanel;
        StatusPanel* statusPanel;
        ControlPanel* controlPanel;
};

#endif