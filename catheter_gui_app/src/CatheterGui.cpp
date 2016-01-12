#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/panel.h>
// command grid
#include <wx/grid.h>
#include <wx/headerctrl.h>
#include <wx/generic/gridctrl.h>
#include <wx/generic/grideditors.h>
// status panel
#include <wx/stattext.h>
// control buttons
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
// pyserial
#include <wx/numdlg.h>
#include <wx/utils.h>
#include <wx/process.h>
#include <wx/filefn.h>
#ifdef __WINDOWS__
#include "wx/dde.h"
#endif

#include "CatheterGui.h"
#include "CatheterGrid.h"
#include "common_utils.h"
#include "pc_utils.h"


// file definitions
#define playfile_wildcard wxT("*.play")
#define portfile wxT("ports.txt")


IMPLEMENT_APP(CatheterGuiApp)

bool CatheterGuiApp::OnInit() {
    CatheterGuiFrame* gui = new CatheterGuiFrame(wxT("Catheter Gui"));
    gui->Show(true);
    return (gui != NULL);
}

wxBEGIN_EVENT_TABLE(CatheterGuiFrame, wxFrame)
    EVT_BUTTON(CatheterGuiFrame::ID_REFRESH_SERIAL_BUTTON, CatheterGuiFrame::OnRefreshSerialButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SELECT_PLAYFILE_BUTTON, CatheterGuiFrame::OnSelectPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_NEW_PLAYFILE_BUTTON, CatheterGuiFrame::OnNewPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SAVE_PLAYFILE_BUTTON, CatheterGuiFrame::OnSavePlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_COMMANDS_BUTTON, CatheterGuiFrame::OnSendCommandsButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_RESET_BUTTON, CatheterGuiFrame::OnSendResetButtonClicked)
wxEND_EVENT_TABLE()

CatheterGuiFrame::CatheterGuiFrame(const wxString& title) :
    wxFrame(NULL, wxID_ANY, title) {

    parentPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);

    grid = new CatheterGrid(parentPanel);

    // status panel    
    statusText = new wxStaticText(parentPanel, wxID_ANY, wxEmptyString);

    // control buttons
    selectPlayfileButton = new wxButton(parentPanel, ID_SELECT_PLAYFILE_BUTTON, wxT("Select Playfile"));
    newPlayfileButton = new wxButton(parentPanel, ID_NEW_PLAYFILE_BUTTON, wxT("New Playfile"));
    savePlayfileButton = new wxButton(parentPanel, ID_SAVE_PLAYFILE_BUTTON, wxT("Save Playfile"));
    sendCommandsButton = new wxButton(parentPanel, ID_SEND_COMMANDS_BUTTON, wxT("Send Commands"));
    sendResetButton = new wxButton(parentPanel, ID_SEND_RESET_BUTTON, wxT("Send Reset"));
    refreshSerialButton = new wxButton(parentPanel, ID_REFRESH_SERIAL_BUTTON, wxT("Refresh Serial"));

    serialConnected = false;
    playfileSaved = false;
    playfilePath = wxEmptyString;
    portName = wxEmptyString;

    // frame
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(selectPlayfileButton);
    hbox->Add(newPlayfileButton);
    hbox->Add(savePlayfileButton);
    hbox->Add(sendCommandsButton);
    hbox->Add(sendResetButton);
    hbox->Add(refreshSerialButton);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(hbox, 1, wxEXPAND | wxALL, 5);
    vbox->Add(grid, 1, wxEXPAND | wxALL, 5);
    vbox->Add(statusText, 1, wxEXPAND | wxALL, 5);

    parentPanel->SetSizer(vbox);
    vbox->SetSizeHints(parentPanel);
    vbox->Fit(parentPanel);

    this->Fit();
    this->Center();

    setStatusText(wxT("Welcome to Catheter Gui"));

    // try to open serial connection
    if (openSerialConnection()) {
        setStatusText(wxString::Format("Serial Connected on port %s", portName));
    } else {
        setStatusText(wxString::Format("Serial Disconnected"));
    }
}

CatheterGuiFrame::~CatheterGuiFrame() {
    wxMessageBox(wxT("CatheterGuiFrame Destructor"));
    if (serialConnected) {
        sendResetCommand();
        closeSerialConnection();
    }
}

///////////////////////////
// event handler methods //
///////////////////////////

// control buttons
void CatheterGuiFrame::OnSelectPlayfileButtonClicked(wxCommandEvent& e) {
    warnSavePlayfile();

    wxString path = openPlayfile();

    if (!path.IsEmpty()) {
        playfileSaved = false;
        playfilePath = path;

        grid->ResetDefault();
        loadPlayfile(playfilePath);

        setStatusText(wxString::Format(wxT("Editing Existing Playfile %s\n"), playfilePath));
    }
}

void CatheterGuiFrame::OnNewPlayfileButtonClicked(wxCommandEvent& e) {
    warnSavePlayfile();

    // clear command grid
    grid->ResetDefault();

    playfileSaved = false;
    playfilePath = wxEmptyString;

    setStatusText(wxT("Editing New Playfile\n"));
}

void CatheterGuiFrame::OnSavePlayfileButtonClicked(wxCommandEvent& e) {
    wxString path = savePlayfile();
    if (!path.IsEmpty()) {
        playfileSaved = true;
        playfilePath = path;
        // save contents of edit panel to playfilePath
        unloadPlayfile(playfilePath);
        setStatusText(wxString::Format(wxT("Saved Playfile as %s"), playfilePath));
    }
}

void CatheterGuiFrame::OnSendCommandsButtonClicked(wxCommandEvent& e) {
    if (serialConnected) {
        setStatusText(wxT("Sending Commands...\n"));
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

void CatheterGuiFrame::OnSendResetButtonClicked(wxCommandEvent& e) {
    if (serialConnected) {
        setStatusText(wxT("Sending Reset Command...\n"));
        sendResetCommand();
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

void CatheterGuiFrame::OnRefreshSerialButtonClicked(wxCommandEvent& e) {
    if (openSerialConnection()) {
        setStatusText(wxString::Format("Serial Connected on port %s", portName));
    } else {
        setStatusText(wxString::Format("Serial Disconnected"));
    }
}

//////////////////////////////////
// status panel private methods //
//////////////////////////////////

void CatheterGuiFrame::setStatusText(const wxString& msg) {
    statusText->SetLabel(msg);
}

///////////////////////////////////
// control panel private methods //
///////////////////////////////////

wxString CatheterGuiFrame::openPlayfile() {
    wxString path = wxEmptyString;
    wxFileDialog openDialog(this, "Open Playfile", wxGetCwd(), "", playfile_wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openDialog.ShowModal() != wxID_CANCEL) {
        wxFileInputStream playfile_stream(openDialog.GetPath());
        if (playfile_stream.IsOk()) {
            path = openDialog.GetPath();
        } else {
            wxLogError("Selected file could not be opened.");
        }
    }
    return path;
}

wxString CatheterGuiFrame::savePlayfile() {
    wxString path = wxEmptyString;
    wxFileDialog saveDialog(this, wxT("Save Playfile"), wxGetCwd(), "", playfile_wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveDialog.ShowModal() != wxID_CANCEL) {
        wxFileOutputStream save_stream(saveDialog.GetPath());
        if (!save_stream.IsOk()) {
            wxLogError("Could not save to selected file");
        } else {
            path = saveDialog.GetPath();
        }
    }
    return path;
}

void CatheterGuiFrame::loadPlayfile(const wxString& path) {
    loadPlayFile(path.mb_str(), gridCmds);
    grid->SetCommands(gridCmds);
}

void CatheterGuiFrame::unloadPlayfile(const wxString& path) {
    grid->GetCommands(gridCmds);
    writePlayFile(path.mb_str(), gridCmds);
}

void CatheterGuiFrame::warnSavePlayfile() {
    if (!playfileSaved) {
        if (wxMessageBox(wxT("Current content has not been saved! Proceed?"), wxT("Warning!"),
            wxICON_QUESTION | wxYES_NO, this) == wxNO) {
            savePlayfile();
            return;
        }
    }
}

//TODO
bool CatheterGuiFrame::sendCommands(std::vector<CatheterChannelCmd> cmdVect) {
    bool success = serialConnected;
    if (serialConnected) {

    }
    return success;
}

bool CatheterGuiFrame::sendResetCommand() {
    std::vector<CatheterChannelCmd> resetVect;
    resetVect.push_back(resetCommand());

    return sendCommands(resetVect);
}

std::vector<wxString> getSerialPortsFromWxProcess() {
    std::vector<wxString> ports;
    /*
    wxString pythonexe = wxT("C:\\Users\\acceber\\AppData\\Local\\Programs\\Python\\Python35-32\\python.exe");
    wxString script = wxGetCwd() + wxT("\\find_serial_ports.py");

    wxString cmd_to_stdout = wxString::Format("%s %s", pythonexe, script);

    wxProcess* process = wxProcess::Open(cmd_to_stdout);
    if (process) {
        wxInputStream* std_out = process->GetInputStream();
        if (std_out) {
            wxString p = wxEmptyString;
            while (std_out->CanRead()) {
                char buf[64];
                std_out->Read(buf, 64);
                wxString temp(buf);
                if (!temp.IsEmpty()) {
                    p = p + temp;
                }
            }
            wxMessageBox(wxString::Format("Found Serial Ports: %s", p));
            wxString first = wxEmptyString;
            wxString rest;
            while (first.length < p.length) {
                first = p.BeforeFirst('\n', &rest);
                if (first.length < p.length) {
                    ports.push_back(first);
                    p = rest;
                }
            }
        } else {
            wxMessageBox(wxT("Could not open process stdout"));
        }
    } else {
        wxMessageBox(wxString::Format("Could not launch process %s", cmd_to_stdout));
    }
    if (wxProcess::Exists(process->GetPid()))
        wxProcess::Kill(process->GetPid());
    */
    return ports;
}

std::vector<wxString> getSerialPortsFromWxShell() {
    std::vector<wxString> ports;
    /*
    wxString pythonexe = wxT("C:\\Users\\acceber\\AppData\\Local\\Programs\\Python\\Python35-32\\python.exe");
    wxString script = wxGetCwd() + wxT("\\find_serial_ports.py");
    wxString fullPortfile = wxGetCwd() + "\\" + portfile;

    wxString cmd_to_file = wxString::Format("%s %s %s", pythonexe, script, fullPortfile);
    
    int exit_code = wxShell(cmd_to_file);

    if (exit_code == 0 && wxFileExists(portfile)) {
        wxFile* f = new wxFile(portfile);
        if (f->IsOpened()) {
            std::vector<wxString> ports;
            size_t ret = 1;
            while (ret > 0) {
                char buf[64];
                ret = f->Read(buf, 64);
                wxString p(buf);
                if (!p.IsEmpty()) {
                    ports.push_back(p);
                }
            }
        }
    }
    */
    return ports;
}

std::vector<wxString> getSerialPorts() {
    //return getSerialPortsFromWxProcess();
    return getSerialPortsFromWxShell();
}

bool CatheterGuiFrame::openSerialConnection() {
    if (wxFileExists(portfile)) {
        wxRemoveFile(portfile);
    }

    std::vector<wxString> ports = getSerialPorts();

    if (!ports.empty()) {
        for (int i = 0; i < ports.size(); i++) {
            wxMessageBox(wxString::Format("Found Serial Port: %s (%d/%d)", ports[i], i, ports.size()));
        }
        int which_port = wxGetNumberFromUser(wxEmptyString, wxT("Select Serial Port Number"), wxEmptyString, 0, 0, ports.size(), this);
        wxMessageBox(wxString::Format("Selected Serial Port: %s", ports[which_port]));
        portName = ports[which_port];
    }
    return (!portName.IsEmpty());
}

bool CatheterGuiFrame::closeSerialConnection() {
    if (wxFileExists(portfile)) {
        wxRemoveFile(portfile);
    }
    return !serialConnected;
}