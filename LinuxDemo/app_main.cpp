#include <stdio.h>

#include "XCamera.h"

#include "app_main.h"
#include "frm_main.h"
#include "dlg_camera.h"

#define X_APP_NAME "Xeneth On Linux sample program"

#define APPDLG_W 900
#define APPDLG_H 600

IMPLEMENT_APP(XApplication)

bool XApplication::OnInit()
{
    SetVendorName("Xenics");
    SetAppName(X_APP_NAME);

    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxBMPHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxTIFFHandler);

    m_pCamera = ConnectDevice();
    if(!m_pCamera)
        return false;

    wxString frameTitle = "Xeneth";
    frameTitle += ": ";
    frameTitle += m_deviceInformation;

    XMainFrame *frame = new XMainFrame(frameTitle, wxDefaultPosition, wxSize(APPDLG_W,APPDLG_H));
    frame->Show(true);
    SetTopWindow(frame);

    return true;
}

int XApplication::OnExit()
{
    delete m_pCamera;
    return 0;
}

XCamera* XApplication::ConnectDevice()
{
    char cDeviceList[1024]="";

    // Get a list of available devices.
    XCamera::GetDeviceList(cDeviceList, sizeof(cDeviceList));

    XCameraDlg cameraDlg;
    cameraDlg.Create(cDeviceList);
    if (cameraDlg.ShowModal() != wxID_OK) return NULL;

    XCamera* cam = XCamera::Create(m_deviceSelection, NULL, NULL);

    if(cam == NULL)
    {
        wxMessageBox(_("Unable to create API instance"), _("Error"), wxICON_ERROR|wxOK);
        return NULL;
    }

    if(cam->IsInitialised() == false)
    {
        wxMessageBox(_("Unable to establish a connection"), _("Error"), wxICON_ERROR|wxOK);
        delete cam;
        return NULL;
    }

    return (cam);
}
