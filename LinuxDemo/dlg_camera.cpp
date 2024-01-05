#include "dlg_camera.h"

#define CAMDLG_W 640
#define CAMDLG_H 125

BEGIN_EVENT_TABLE(XCameraDlg, wxDialog)
    EVT_CLOSE(XCameraDlg::OnClose)
    EVT_BUTTON(XDLGCAM_OK_ID, XCameraDlg::OnOK)
    EVT_BUTTON(XDLGCAM_CANCEL_ID, XCameraDlg::OnCancel)
    EVT_COMBOBOX(XDLGCAM_CB_ID, XCameraDlg::OnSelectCamera)
END_EVENT_TABLE()

XCameraDlg::XCameraDlg()
:wxDialog()
{
	m_pCameraCB = 0;
}

XCameraDlg::~XCameraDlg()
{
}

void XCameraDlg::Create(char* devList)
{
    wxDialog::Create(NULL, -1, "Camera selection", wxPoint(-1,-1), wxSize(CAMDLG_W,CAMDLG_H), wxDEFAULT_DIALOG_STYLE, "camera_selection");

    wxButton *pOKButton = new wxButton(this, XDLGCAM_OK_ID, _("OK"), wxPoint(CAMDLG_W/4+5, CAMDLG_H-40), wxSize(CAMDLG_W/4-10, -1));
    new wxButton(this, XDLGCAM_CANCEL_ID, _("Cancel"), wxPoint(CAMDLG_W/2+5, CAMDLG_H-40), wxSize(CAMDLG_W/4-10, -1));

    new wxStaticText(this, -1, "Please select a camera in the following list", wxPoint(10,5), wxSize(CAMDLG_W-15, -1));
    m_pCameraCB = new wxComboBox(this, XDLGCAM_CB_ID, "", wxPoint(5, 35), wxSize(CAMDLG_W-10, -1), 0, NULL, wxCB_DROPDOWN|wxCB_READONLY);

    int devCount = 0;
    wxStringTokenizer tok(devList, "|");
    while(tok.HasMoreTokens())
    {
        wxString spec = tok.GetNextToken();
        wxString desc = tok.GetNextToken();

        m_DevSpecs.Add(spec);
        m_DevDescs.Add(desc);

        m_pCameraCB->Append(desc);
        devCount++;
    }
    if(devCount == 0)
    {
        m_pCameraCB->Append("No camera detected...");
        pOKButton->Enable(false);
    }
    m_pCameraCB->SetSelection(0);
}

void XCameraDlg::OnClose(wxCloseEvent&)
{
    EndModal(wxID_CANCEL);
}

void XCameraDlg::OnOK(wxCommandEvent&)
{
    int index = m_pCameraCB->GetSelection();
    wxGetApp().SetDeviceSelection(m_DevSpecs[index]);
    wxGetApp().SetDeviceInformation(m_DevDescs[index]);
    EndModal(wxID_OK);
}

void XCameraDlg::OnCancel(wxCommandEvent&)
{
    EndModal(wxID_CANCEL);
}

void XCameraDlg::OnSelectCamera(wxCommandEvent& event) {}
