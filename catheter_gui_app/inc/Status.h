#pragma once
#ifndef STATUS_PANEL_H
#define STATUS_PANEL_H

class wxPanel;
class wxStaticText;

class StatusPanel : public wxPanel {
    public:
    StatusPanel(wxPanel* parent);

    wxPanel* parent;
    wxStaticText* statusText;
};

#endif
