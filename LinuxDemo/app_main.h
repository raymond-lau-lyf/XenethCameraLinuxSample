#ifndef _X_APP_MAIN_H_
#define _X_APP_MAIN_H_

#include "wxincludes.h"

class XCamera;

class XApplication : public wxApp
{
    public:
        virtual bool    OnInit();
        virtual int     OnExit();

        void SetDeviceSelection     (wxString dev)  { m_deviceSelection     = dev; }
        void SetDeviceInformation   (wxString info) { m_deviceInformation   = info; }

        XCamera &Cam() { return *m_pCamera; }

    private:
        XCamera* ConnectDevice();

        XCamera* m_pCamera;

        wxString m_deviceSelection;
        wxString m_deviceInformation;
};

DECLARE_APP(XApplication);

#define XPDID_Timer 1050

#endif //_X_APP_MAIN_H_
