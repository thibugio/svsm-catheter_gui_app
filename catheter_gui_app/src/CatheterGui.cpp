
#include "ser/SerialSender.h"


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
#include <wx/numdlg.h>
#include <wx/grid.h>
// status panel
#include <wx/stattext.h>
// control buttons
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>

#include "CatheterGui.h"
#include "CatheterGrid.h"
#include "common_utils.h"
#include "pc_utils.h"


// file definitions
#define playfile_wildcard wxT("*.play")


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

    playfileSaved = false;
    playfilePath = wxEmptyString;

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
    ss = new SerialSender();
    if ((ss->getSerialPath()).empty()) {
        setStatusText(wxString::Format("Serial Disconnected"));
    } else {
        if (ss->connect(SerialSender::BR_9600)) {
            SetStatusText(wxString::Format("Serial Connected on Port %s", ss->getSerialPath()));
        }
    }
}

CatheterGuiFrame::~CatheterGuiFrame() {
    wxMessageBox(wxT("CatheterGuiFrame Destructor"));
    if (ss->isOpen()) {
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
    if (ss->isOpen()) {
        setStatusText(wxT("Sending Commands...\n"));
        if (sendGridCommands()) {
            wxMessageBox(wxT("Commands Successfully Sent"));
        } else {
            wxMessageBox(wxT("Error Sending Commands"));
        }
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

void CatheterGuiFrame::OnSendResetButtonClicked(wxCommandEvent& e) {
    if (ss->isOpen()) {
        setStatusText(wxT("Sending Reset Command...\n"));
        if (sendResetCommand()) {
            wxMessageBox(wxT("Reset Command Successfully Sent"));
        } else {
            wxMessageBox(wxT("Error Sending Reset Command"));
        }
    } else {
        wxMessageBox("Serial Disconnected!");
    }
}

void CatheterGuiFrame::OnRefreshSerialButtonClicked(wxCommandEvent& e) {
    if (refreshSerialConnection()) {
        setStatusText(wxString::Format("Serial Connected on port %s", ss->getSerialPath()));
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
    std::vector<CatheterChannelCmd> gridCmds;
    loadPlayFile(path.mb_str(), gridCmds);
    grid->SetCommands(gridCmds);
}

void CatheterGuiFrame::unloadPlayfile(const wxString& path) {
    std::vector<CatheterChannelCmd> gridCmds;
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

bool CatheterGuiFrame::sendCommands(std::vector<CatheterChannelCmd> cmdVect) {
    if (ss->isOpen()) {        
        std::vector<CatheterChannelCmd> cmds;
        std::vector<std::vector<uint8_t>> cmdBytes;
        std::vector<int> delays;
        std::vector<std::vector<uint8_t>> bytesRead;
        CatheterPacket packet;
        // pad list of commands so that there is a command for each channel
        cmds = padChannelCmds(cmdVect);
        // convert commands to byte and delay vectors
        getPacketBytes(cmds, cmdBytes, delays);
        // send packet bytes and read bytes returned
        ss->sendByteVectorsAndRead(cmdBytes, bytesRead, delays);
        // verify success of returned bytes for each packet
        if (bytesRead.size()) {
            for (int i = 0; i < bytesRead.size(); i++) {
                packet = validateBytesRcvd(bytesRead[i]);
                for (int j = 0; j < NCHANNELS; j++) {
                    if (packet.cmds[j].currentMA == -1) {
                        return false;
                    } else {
                        dir_t dir = ((cmds[(i*NCHANNELS) + j].currentMA > 0) ? DIR_POS : DIR_NEG);
                        double currentMA = convert_current_by_channel(packet.cmds[j].currentMA, dir, j);
                        // check if the returned current matches the original current
                        if (!abs(cmds[(i*NCHANNELS) + j].currentMA - currentMA) < 0.01) {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool CatheterGuiFrame::sendGridCommands() {
    // get list of CatheterChannelCmds
    std::vector<CatheterChannelCmd> cmds;
    grid->GetCommands(cmds);
    return sendCommands(cmds);
}

bool CatheterGuiFrame::sendResetCommand() {
    std::vector<CatheterChannelCmd> resetVect;
    resetVect.push_back(resetCommand());
    return sendCommands(resetVect);
}

bool CatheterGuiFrame::refreshSerialConnection() {
    std::vector<std::string> ports = ss->findAvailableSerialPorts();

    if (!ports.empty()) {
        for (int i = 0; i < ports.size(); i++) {
            wxMessageBox(wxString::Format("Found Serial Port: %s (%d/%d)", wxString(ports[i]), i, ports.size()));
        }
        int which_port = wxGetNumberFromUser(wxEmptyString, wxT("Select Serial Port Number"), wxEmptyString, 0, 0, ports.size(), this);
        wxMessageBox(wxString::Format("Selected Serial Port: %s", wxString(ports[which_port])));
        ss->setSerialPath(ports[which_port]);
        return ss->connect();
    }
    return false;
}

bool CatheterGuiFrame::closeSerialConnection() {
    return ss->disconnect();
}