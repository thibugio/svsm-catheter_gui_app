#pragma once
#ifndef STATUS_PANEL_H
#define STATUS_PANEL_H

//#include "gui_common.h"
#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/stattext.h>

class StatusPanel : public wxPanel {
    public:
    StatusPanel(wxPanel* parent);

    wxPanel* parent;
    wxStaticText* statusText;
};

#endif
