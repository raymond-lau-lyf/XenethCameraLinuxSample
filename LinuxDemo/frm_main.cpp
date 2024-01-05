#include <stdio.h>

#include "frm_main.h"

#include "XFilters.h"

#define _MAX_PROP_LEN (2048)

BEGIN_EVENT_TABLE(XMainFrame, wxFrame)
    EVT_CLOSE(XMainFrame::OnClose)
    EVT_MENU(XMFID_Quit, XMainFrame::OnQuit)
    EVT_MENU(XMFID_About, XMainFrame::OnAbout)
    EVT_MENU(XMFID_LoadCP, XMainFrame::OnLoadColorProfile)
    EVT_MENU(XMFID_Start, XMainFrame::OnStart)
    EVT_MENU(XMFID_Stop, XMainFrame::OnStop)
    EVT_MENU(XMFID_SaveImage, XMainFrame::OnSaveImage)
    EVT_MENU(XMFID_AutoGain, XMainFrame::OnAutoGain)
    EVT_MENU(XMFID_LoadCalibration, XMainFrame::OnLoadCalibration)
    EVT_MENU(XMFID_ToggleCalibration, XMainFrame::OnToggleCalibration)
    EVT_TIMER(XMFID_Timer, XMainFrame::OnTimer)
    EVT_TEXT_ENTER(XMFID_ChangeHistoBal, XMainFrame::OnChangeHistoBal)
    EVT_TEXT_ENTER(XMFID_ChangeIntegTime, XMainFrame::OnChangeIntegTime)
    EVT_TEXT_ENTER(XMFID_ChangeVDetcom, XMainFrame::OnChangeVDetcom)
    EVT_TEXT_ENTER(XMFID_ChangeVidGain, XMainFrame::OnChangeVidGain)
    EVT_TEXT_ENTER(XMFID_ChangeVidGainP, XMainFrame::OnChangeVidGainP)
    EVT_TEXT_ENTER(XMFID_ChangeVidGainI, XMainFrame::OnChangeVidGainI)
    EVT_TEXT_ENTER(XMFID_ChangeMaxGain, XMainFrame::OnChangeMaxGain)
END_EVENT_TABLE()

XMainFrame::XMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
:wxFrame((wxFrame *)NULL, -1, title, pos, size),
m_Timer(this, XMFID_Timer),
m_pDC(NULL)
{
    m_bCapturing    = false;
    m_bCalibLoaded  = false;

    // Retreive camera object
    m_pCam = &wxGetApp().Cam();

    // Retrieve install dir
    char installdir[PATH_MAX];
    m_pCam->GetPath(XDir_InstallDir, installdir, PATH_MAX);
    m_InstallDir = installdir;

    // Allocate a frame buffer, for optional image processing
    m_ImageWidth    = 0;
    m_ImageHeight   = 0;
    m_FrameSize     = 0;
    m_pFrameBuffer  = NULL;
    if(m_pCam && m_pCam->IsInitialised())
    {
        m_ImageWidth    = m_pCam->GetWidth();
        m_ImageHeight   = m_pCam->GetHeight();
        m_FrameSize     = m_pCam->GetFrameSize();
        m_pFrameBuffer  = (unsigned char*)calloc(sizeof(char), m_FrameSize);
    }

    // Create menus
    CreateMenus();

    // Create status bar
    CreateStatusBar();
    if(m_pCam && m_pCam->IsInitialised())
        SetStatusText("Welcome to the XenICs Demonstrator!");
    else
        SetStatusText("Camera initialisation failed!");

    // Create tool bar
    CreateToolBar();
    
    // Load icon
    if (m_IconBmp.LoadFile(m_InstallDir+"/Resources/icon.png", wxBITMAP_TYPE_PNG))
    {
        m_IconBmp.SetMask(new wxMask(m_IconBmp, wxColour(255, 255, 255)));
        m_Icon.CopyFromBitmap(m_IconBmp);
        SetIcon(m_Icon);
    }

    // Centre the frame on the display
    Centre();

    // Start timer
    m_Timer.Start(1, true);

    // Load Thermal blue color profile
    m_pCam->LoadColourProfile(m_InstallDir+"/Resources/Colour profiles/Thermal Blue.png");
    m_pCam->SetColourMode(ColourMode_Profile);

    // Start acquisition
    //StartCapture();
}

XMainFrame::~XMainFrame()
{
    if(m_pFrameBuffer)
        free(m_pFrameBuffer);

    if(m_pDC)
    {
        delete(m_pDC);
        m_pDC = NULL;
    }
}

void XMainFrame::CreateMenus()
{
    wxMenu *menuFile = new wxMenu();
    menuFile->Append(XMFID_LoadCP, "Load colour profile");
    menuFile->AppendSeparator();
    menuFile->Append(XMFID_Quit, "Exit");

    wxMenu *menuImage = new wxMenu();
    menuImage->Append(XMFID_Start, START_MENU_STRING);
    menuImage->Append(XMFID_Stop, STOP_MENU_STRING);
    menuImage->AppendSeparator();
    menuImage->Append(XMFID_SaveImage, SAVEIMG_MENU_STRING);

    mMenuFilters = new wxMenu();
    mMenuFilters->Append(XMFID_LoadCalibration, "Load Calibration");
    mMenuFilters->AppendCheckItem(XMFID_ToggleCalibration, TOGGLECALIBRATION_MENU_STRING);
    mMenuFilters->Enable(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), false);	// don't enable untill a calibration is loaded
    mMenuFilters->AppendCheckItem(XMFID_AutoGain, AUTOGAIN_MENU_STRING);
    if(!m_pCam || !m_pCam->IsInitialised())
    {
        menuImage->Enable(menuImage->FindItem(START_MENU_STRING), false);
        menuImage->Enable(menuImage->FindItem(STOP_MENU_STRING), false);
        menuImage->Enable(menuImage->FindItem(SAVEIMG_MENU_STRING), false);
        mMenuFilters->Enable(mMenuFilters->FindItem("Load Calibration"), false);
        mMenuFilters->Enable(mMenuFilters->FindItem(AUTOGAIN_MENU_STRING), false);
    }

    wxMenu *menuHelp = new wxMenu();
    menuHelp->Append(XMFID_About, "About...");

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(menuFile, "File");
    menuBar->Append(menuImage, "Image");
    menuBar->Append(mMenuFilters, "Filters");
    menuBar->Append(menuHelp, "Help");

    SetMenuBar(menuBar);
}

void XMainFrame::CreateToolBar()
{
    wxToolBar *tb = new wxToolBar(this, -1, wxPoint(-1, -1), wxSize(-1, -1), wxTB_VERTICAL);
    tb->AddControl(new wxStaticText(tb, -1, "Settings", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER));
    tb->AddSeparator();

    // // Histogram balance
    // AddPropertyControl(tb, "HistogramBalance", "Histogram Balance: ", XMFID_ChangeHistoBal, m_pHistoBal);

    // Integration time 
    AddPropertyControl(tb, "IntegrationTime", "Integration Time: ", XMFID_ChangeIntegTime, m_pIntegTime);

    // Maximal Gain 
    AddPropertyControl(tb, "AutoModeMaximalGain", "Maximal Gain: ", XMFID_ChangeMaxGain, m_pMaxGain);

    // // VDetcom
    // AddPropertyControl(tb, "VDETCOM", "VDetcom: ", XMFID_ChangeVDetcom, m_pVDetcom);

    // // Video gain
    // AddPropertyControl(tb, "VideoGain",  "Video Gain: ",   XMFID_ChangeVidGain,  m_pVidGain);
    // AddPropertyControl(tb, "VideoGainP", "Video Gain P: ", XMFID_ChangeVidGainP, m_pVidGainP);
    // AddPropertyControl(tb, "VideoGainI", "Video Gain I: ", XMFID_ChangeVidGainI, m_pVidGainI);
 
    tb->AddSeparator();
    tb->AddControl(new wxStaticText(tb, -1, "\nPress Enter to validate \nthe new property values "));

    SetToolBar(tb);
}

void XMainFrame::AddPropertyControl(wxToolBar *tb, const char *prop, const char *desc, int ctrlID, wxTextCtrl*& ctrl)
{
    char propVal_str[_MAX_PROP_LEN];
    if(m_pCam->GetPropertyValue(prop, propVal_str, _MAX_PROP_LEN) != E_NOT_SUPPORTED)
    {
        tb->AddControl(new wxStaticText(tb, -1, desc));
        ctrl = new wxTextCtrl(tb, ctrlID, propVal_str, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        tb->AddControl(ctrl);
    }
    else
    {
        std::cout<<"error in AddPropertyControl"<<std::endl;
    }
}

void XMainFrame::OnQuit(wxCommandEvent&)
{
    Close(true);
}

void XMainFrame::OnClose(wxCloseEvent&)
{
    StopCapture();
    m_Timer.Stop();
    Destroy();
}

void XMainFrame::OnAbout(wxCommandEvent&)
{
    wxMessageBox("Xeneth On Linux sample program\n\n © 2009-2012 XenICs N.V.", "About", wxOK|wxICON_INFORMATION);
}

void XMainFrame::OnStart(wxCommandEvent&)
{
    StartCapture();
}

void XMainFrame::OnStop(wxCommandEvent&)
{
    StopCapture();
}

void XMainFrame::OnAutoGain(wxCommandEvent &evt)
{
    static FilterID histoFlt = 0;
    if(evt.IsChecked())
    {
        histoFlt = QueueFilter(m_pCam, "AutoGain", "");
    }
    else
    {
        m_pCam->RemImageFilter(histoFlt);
    }
}

void XMainFrame::OnLoadCalibration(wxCommandEvent&)
{
    wxFileDialog dlg(this,
                     "Choose a calibration file",
                     wxGetCwd(),
                     "",
                     "Calibration Files (*.xca)|*.xca",
                     wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if(dlg.ShowModal() == wxID_OK)
    {
        long errCode = m_pCam->LoadCalibration(dlg.GetPath().c_str(), 0);
        if(errCode != I_OK)
        {
            m_bCalibLoaded = false;
            mMenuFilters->Enable(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), false);
        	mMenuFilters->Check(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), false);
        	char errString[50];
        	sprintf(errString, "Could not load calibration. Error: %ld", errCode);
        	wxMessageBox(errString, _("Error"), wxICON_ERROR|wxOK);
        }
        else
        {
        	m_bCalibLoaded = true;
        	m_iFID = QueueFilter(m_pCam, "SoftwareCorrection", "");
        	m_pCam->PriImageFilter(m_iFID, 0);
        	mMenuFilters->Enable(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), true);
        	mMenuFilters->Check(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), true);
        }
    }
}

void XMainFrame::OnToggleCalibration(wxCommandEvent& evt)
{
    if(!m_bCalibLoaded)
    {
        m_iFID = QueueFilter(m_pCam, "SoftwareCorrection", "");  // negative fID = failure to load, perhaps try correction instead of thermal
        if(m_iFID < 0) wxMessageBox(_("Could not load the software correction filter."), _("Error"), wxICON_ERROR|wxOK); // negative fID again, report
        else {
            m_bCalibLoaded = true;
            m_pCam->PriImageFilter(m_iFID, 0);
            mMenuFilters->Check(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), true);
        }

    }
    else
    {
        m_pCam->RemImageFilter(m_iFID);
        m_bCalibLoaded = false;
        mMenuFilters->Check(mMenuFilters->FindItem(TOGGLECALIBRATION_MENU_STRING), false);
    }
}

void XMainFrame::OnSaveImage(wxCommandEvent&)
{
    wxFileDialog dlg(this,
                     "Choose an output filename",
                     wxGetCwd(),
                     "",
                     "Image files (8-bit) (*.png;*.jpg;*.tif;*.bmp)|*.png;*.jpg;*.tif;*.bmp|Image files (16-bit) (*.png;*.tif;*.csv;*.bin)|*.png;*.tif;*.csv;*.bin",
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    FilterID freeze = QueueFilter(m_pCam, "Freeze", "");; // freeze frame;
    
    if(dlg.ShowModal() == wxID_OK)
    {
        wxFileName filename(dlg.GetPath());
        if(filename.GetExt() == "csv")
        {
            m_pCam->SaveData(dlg.GetPath().c_str());
        }
        else
        {
            unsigned long force8or16 = (dlg.GetFilterIndex() == 0)? XSD_Force8 : XSD_Force16;
            m_pCam->SaveData(dlg.GetPath().c_str(), force8or16);
        }
    }
    m_pCam->RemImageFilter(freeze);
}

void XMainFrame::OnTimer(wxTimerEvent&)
{
    int period = 1;
    int iFrameDropMax = 9;
    int dc_width, dc_height;
    int blit_width, blit_height;
    int blit_x, blit_y;

    if (!m_pDC) m_pDC = new wxClientDC(this);

    // Resize and center the image
    m_pDC->GetSize(&dc_width, &dc_height);
    double ratio = (double)m_ImageWidth/(double)m_ImageHeight;
    if((double)dc_width/(double)dc_height > ratio)
    {
        blit_width = dc_height * ratio;
        blit_height = dc_height;
        blit_x = (dc_width-blit_width) / 2;
        blit_y = 0;
    }
    else
    {
        blit_width = dc_width;
        blit_height = dc_width/ratio;
        blit_x = 0;
        blit_y = (dc_height-blit_height) / 2;
    }

    if(m_bCapturing && m_pCam->GetFrame(FT_16_BPP_GRAY, 0, m_pFrameBuffer, m_FrameSize) == I_OK)
    {
        m_pCam->Blit(m_pDC, blit_x, blit_y, blit_width, blit_height, DeviceContext);
        
        // Eat the rest of the frames
        while(m_pCam->GetFrame(FT_NATIVE, XGF_NoConversion, NULL, m_FrameSize) == I_OK && iFrameDropMax--);
    }
    else
    {
        m_pCam->Blit(m_pDC, blit_x, blit_y, blit_width, blit_height, DeviceContext);
    }

    if(m_bCapturing)
    {
        SetStatusText(wxString::Format(_("Capturing frame #%ld \t @ %.0lf fps")
                                       , m_pCam->GetFrameCount(), m_pCam->GetFrameRate()));
        period = 10;
    }
    else
    {
        period = 100;
    }
    m_Timer.Start(period, true);
}

void XMainFrame::OnLoadColorProfile(wxCommandEvent&)
{
    wxFileDialog fd(this, 
                    "Select a colour profile",
                    m_InstallDir+"/Resources/Colour profiles",
                    "",
                    "Colour profile files (*.png)|*.png;*.PNG");
    if (fd.ShowModal() == wxID_OK)
    {
        wxString ColourProfile = fd.GetPath();
        m_pCam->LoadColourProfile(ColourProfile);
        m_pCam->SetColourMode(ColourMode_Profile);
    }
}

void XMainFrame::StartCapture()
{
    if(!m_bCapturing)
    {
        m_pCam->StartCapture();
        m_bCapturing = true;
    }
}

void XMainFrame::StopCapture()
{
    if(m_bCapturing)
    {
        m_pCam->StopCapture();
        m_bCapturing = false;
    }
}

void XMainFrame::OnChangeHistoBal(wxCommandEvent&)
{
    ChangePropertyVal("HistogramBalance", m_pHistoBal);
}

void XMainFrame::OnChangeIntegTime(wxCommandEvent&)
{
    ChangePropertyVal("IntegrationTime", m_pIntegTime);
}

void XMainFrame::OnChangeVDetcom(wxCommandEvent&)
{
    ChangePropertyVal("VDETCOM", m_pVDetcom);
}

void XMainFrame::OnChangeVidGain(wxCommandEvent&)
{
    ChangePropertyVal("VideoGain", m_pVidGain);
}

void XMainFrame::OnChangeVidGainP(wxCommandEvent&)
{
    ChangePropertyVal("VideoGainP", m_pVidGainP);
}

void XMainFrame::OnChangeVidGainI(wxCommandEvent&)
{
    ChangePropertyVal("VideoGainI", m_pVidGainI);
}

void XMainFrame::OnChangeMaxGain(wxCommandEvent&)
{
    ChangePropertyVal("AutoModeMaximalGain", m_pMaxGain);
}

void XMainFrame::ChangePropertyVal(const char *prop, wxTextCtrl* ctrl)
{
    double propVal = 0;
    char propVal_str[_MAX_PROP_LEN];

    ctrl->GetLineText(0).ToDouble(&propVal);
    m_pCam->SetPropertyValueF(prop, propVal);
    m_pCam->GetPropertyValue(prop, propVal_str, _MAX_PROP_LEN);
    ctrl->SetValue(propVal_str);
}
