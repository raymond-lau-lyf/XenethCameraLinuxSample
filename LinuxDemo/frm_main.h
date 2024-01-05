#ifndef _X_FRM_MAIN_H_
#define _X_FRM_MAIN_H_

#include "app_main.h"
#include "XCamera.h"

// events
enum
{
    XMFID_Quit = 1000,
    XMFID_About,
    XMFID_LoadCP,
    XMFID_Start,
    XMFID_Stop,
    XMFID_SaveImage,
    XMFID_AutoGain,
    XMFID_LoadCalibration,
    XMFID_ToggleCalibration,
    XMFID_Timer,
    XMFID_ChangeHistoBal,
    XMFID_ChangeIntegTime,
    XMFID_ChangeVDetcom,
    XMFID_ChangeVidGain,
    XMFID_ChangeVidGainP,
    XMFID_ChangeVidGainI,
    XMFID_ChangeMaxGain
};

#define START_MENU_STRING               "Start capture\tF2"
#define STOP_MENU_STRING                "Stop capture\tF3"
#define SAVEIMG_MENU_STRING             "Save image\tF4"
#define AUTOGAIN_MENU_STRING            "Auto Gain\tALT-R"
#define TOGGLECALIBRATION_MENU_STRING   "Calibration\tALT-C"

class XMainFrame : public wxFrame
{
    public:
        XMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
        virtual ~XMainFrame();

        void OnClose                (wxCloseEvent& event);
        void OnQuit                 (wxCommandEvent& event);
        void OnAbout                (wxCommandEvent& event);
        void OnStart                (wxCommandEvent& event);
        void OnStop                 (wxCommandEvent& event);
        void OnSaveImage            (wxCommandEvent& event);
        void OnAutoGain             (wxCommandEvent& event);
        void OnLoadCalibration      (wxCommandEvent& event);
        void OnToggleCalibration    (wxCommandEvent& event);
        void OnTimer                (wxTimerEvent& event);
        void OnLoadColorProfile     (wxCommandEvent& event);
        void OnChangeHistoBal       (wxCommandEvent& event);
        void OnChangeIntegTime      (wxCommandEvent& event);
        void OnChangeVDetcom        (wxCommandEvent& event);
        void OnChangeVidGain        (wxCommandEvent& event);
        void OnChangeVidGainP       (wxCommandEvent& event);
        void OnChangeVidGainI       (wxCommandEvent& event);
        void OnChangeMaxGain        (wxCommandEvent& event);

    private:
        XCamera*    m_pCam;
        wxClientDC* m_pDC;

        bool    m_bCapturing;
        bool    m_bCalibLoaded;

        FilterID m_iFID;

        wxString    m_InstallDir;

        wxTextCtrl* m_pHistoBal;
        wxTextCtrl* m_pIntegTime;
        wxTextCtrl* m_pVDetcom;
        wxTextCtrl* m_pVidGain;
        wxTextCtrl* m_pVidGainP;
        wxTextCtrl* m_pVidGainI;
        wxTextCtrl* m_pMaxGain;

        wxMenu* mMenuFilters;

        // App icon
        wxBitmap    m_IconBmp;
        wxIcon      m_Icon;

        // Image properties
        int m_ImageWidth;
        int m_ImageHeight;
        int m_FrameSize;

        unsigned char*  m_pFrameBuffer;

        wxTimer m_Timer;

        void    CreateMenus();
        void    CreateToolBar();
        void    StartCapture();
        void    StopCapture();
        void    AddPropertyControl(wxToolBar *tb, const char *prop, const char *desc, int ctrlID, wxTextCtrl*& ctrl);
        void    ChangePropertyVal(const char *prop, wxTextCtrl* ctrl);

        DECLARE_EVENT_TABLE()
};

#endif //_X_FRM_MAIN_H_
