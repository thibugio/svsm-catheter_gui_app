#pragma once
#ifndef CATHETER_GUI_APP_H
#define CATHETER_GUI_APP_H

#include "CatheterGui.h"

class CatheterGuiApp : public wxApp {
    public:
    virtual bool OnInit();
    CatheterGui* gui;
};

#endif