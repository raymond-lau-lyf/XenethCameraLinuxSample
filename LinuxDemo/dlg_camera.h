#ifndef _X_DLG_CAMERA_H_
#define _X_DLG_CAMERA_H_

#include "app_main.h"

enum
{
    XDLGCAM_OK_ID = 2000,
    XDLGCAM_CANCEL_ID,
    XDLGCAM_CB_ID
};

class XCameraDlg : public wxDialog
{
    public:
        XCameraDlg();
        virtual ~XCameraDlg();

        void Create         (char *deviceList);
        void OnClose        (wxCloseEvent& event);
        void OnOK           (wxCommandEvent& event);
        void OnCancel       (wxCommandEvent& event);
        void OnSelectCamera (wxCommandEvent& event);

    private:
        wxComboBox*     m_pCameraCB;

        wxArrayString   m_DevSpecs;
        wxArrayString   m_DevDescs;

        DECLARE_EVENT_TABLE()
};

#endif //_X_DLG_CAMERA_H_
