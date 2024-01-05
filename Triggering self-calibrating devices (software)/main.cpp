// Header inclusions /////////////////////////////////////////////////////

#include <stdlib.h>     // C Standard Common Library
#include <stdio.h>      // C Standard Input/Output library.

#include "XCamera.h"    // Xeneth SDK main header.


// Utility ///////////////////////////////////////////////////////////////

#define CRLF    "\r\n"

#if defined(_WIN32)
#include <Windows.h>
#else 
#include <unistd.h>    
#define Sleep(t) usleep((t) * 1000)
#endif


// Function prototypes ///////////////////////////////////////////////////

bool HandleError(ErrCode errCode, const char * msg);
void CleanupSession();
void AbortSession();

bool SetupImageFormatAndAcquisition(XCHANDLE handle);

bool SetupSoftwareTriggeredMode_F027(XCHANDLE handle);
bool ExecuteSoftwareTrigger_F027(XCHANDLE handle);
bool SetupShutterControl_F027(XCHANDLE handle);
bool ExecuteCalibration_F027(XCHANDLE handle);
void WaitForCalibration_F027(XCHANDLE handle);


// Global variables //////////////////////////////////////////////////////

/*
 *  Set the URL of the camera you want to connect to.
 *  By using "cam://0" as the URL a connection will be established
 *  to the first discovered camera. It is also possible to make a
 *  direct connection to a GigE Vision camera when the IP is known.
 *  The format of the URL for a GigE Vision camera will be "gev://<ip-address>"
 *  Make sure to check out the "Enumerate devices" to learn how device
 *  discovery is performed.
 */

const char * url = "cam://0";


/*
 *  Handle to the XCamera-object
 */

XCHANDLE handle = 0;


/*
 *  Variables to hold the frame size and buffer
 */

unsigned int framesize = 0;
unsigned short * framebuffer = 0;


/*
 *  In this set-up the burst size of the function generator is set to 100 frames.
 */

const unsigned int n_burst = 100;


// Entry point ///////////////////////////////////////////////////////////

int main() {    
    ErrCode errCode = I_OK;
    long pid = 0;

    printf("Open connection to '%s'" CRLF, url);
    handle = XC_OpenCamera(url);

    if (XC_IsInitialised(handle)) {     
        printf("Successfully established a connection to '%s'." CRLF, url);
        
        /* retrieve camera product id (PID) */
        errCode = XC_GetPropertyValueL(handle, "_CAM_PID", &pid);
        if (!HandleError(errCode, "Retrieving the camera PID")) AbortSession();        
        printf("Connected to camera with product id (PID) 0x%X" CRLF CRLF, pid);

        /* Check for the Gobi-640-GigE (F027) */
        if (pid == 0xF027 || pid == 0xF122) {

            /* configure camera in software triggered mode */
            if (!SetupSoftwareTriggeredMode_F027(handle)) AbortSession();

            /* configure camera to disable the automatic shutter calibration process */
            if (!SetupShutterControl_F027(handle)) AbortSession();

        }
        else {
            printf("Connected to an unsupported camera." CRLF);
            AbortSession();
        }

        /* Setup image format and acquisition parameters */
        if (!SetupImageFormatAndAcquisition(handle)) AbortSession();

        /* assign frame buffer */
        framesize = XC_GetFrameSize(handle);
        framebuffer = (unsigned short *)malloc(framesize);

        if (0 == framebuffer) {
            printf("Could not allocate framebuffer memory." CRLF);
            AbortSession();
        }
        
        /* start capture */
        XC_StartCapture(handle);
        WaitForCalibration_F027(handle);
        
        /* main loop */
        while (true) {

            /* acquire n frames */
            unsigned int n = 0;
            printf("Generating %i software triggers." CRLF, n_burst);
                        
            /*
             *  Generate initial software trigger.
             */

            if (!ExecuteSoftwareTrigger_F027(handle)) AbortSession();

            do {               

                /* 
                 *  Grab a frame from the internal buffers if one is available.
                 *  If no frames were captured yet, or we are polling faster than 
                 *  frames are arriving, GetFrame will return E_NO_FRAME.
                 */

                ErrCode errCode = XC_GetFrame(handle, FT_NATIVE, 0, framebuffer, framesize);

                if (I_OK == errCode) { 
                    
                    n++;
                    printf("Received frame %i of %i                \r", n, n_burst, framebuffer[0]);
                    fflush(stdout);

                    
                    /*
                     *  Generate another software trigger.
                     */

                    if (!ExecuteSoftwareTrigger_F027(handle)) AbortSession();
                    

                }
                else if (E_NO_FRAME != errCode) {
                    printf(CRLF);
                    HandleError(errCode, "while grabbing frame");
                    AbortSession();
                }

            } while (n < n_burst);
            printf(CRLF);
            
            /* Ask the user to perform another trigger burst cycle */
            printf("Enter 'y' to perform another burst: ");
            char ch = (char)fgetc(stdin);
            for (int c = getc(stdin); c != '\n'; c = getc(stdin));

            /* any other answer except for 'y' breaks the main loop */
            if (ch == 'y') ExecuteCalibration_F027(handle);
            else break;

        }

        printf(CRLF);
        
        /* clean-up */
        printf("Closing session" CRLF);        
        CleanupSession();

    }
    else {
        printf("Initialization failed" CRLF);
    }

    return 0;
}


// Function implementations //////////////////////////////////////////////

/*
 *  Utility function to handle error messages 
 */

bool HandleError(ErrCode errCode, const char * msg) {
    const int sz = 2048;
    char err_msg[sz];

    XC_ErrorToString(errCode, err_msg, sz);
    printf("%s: %s (%i)" CRLF, msg, err_msg, errCode);

    return I_OK == errCode;
}


/*
 *  In SetupSoftwareTriggeredMode_F027 we configure the camera in  triggered mode. 
 *  
 */

bool SetupSoftwareTriggeredMode_F027(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    printf("Configuring camera in software triggered mode." CRLF);

    /*
     *  TriggerInEnable = Disabled (0), Enabled (1)
     * -----------------------------------------------------------------
     *  Enable the trigger input block
     */

    errCode = XC_SetPropertyValueL(handle, "TriggerInEnable", 0, "");
    if (!HandleError(errCode, " * Disable trigger input"))
        return false;


    /*
     *  TriggerOutEnable = Disabled (0), Enabled (1)
     * -----------------------------------------------------------------
     *  Although the TriggerDirection works as a multiplexer for the 
     *  trigger connection on the camera, it is still good practice to 
     *  make sure the trigger output block is completely disabled by setting
     *  its value to 0.
     */

    errCode = XC_SetPropertyValueL(handle, "TriggerOutEnable", 0, "");
    if (!HandleError(errCode, " * Disable trigger output"))
        return false;

    
    /*
     *  TriggerInMode = FreeRunning (0), ExternalTriggered (1)
     * -----------------------------------------------------------------
     *  Set the TriggerInMode to ExternalTriggered to put the camera 
     *  in triggered mode. FreeRunning-mode means the camera
     *  is continuously triggered by an internal source.
     *  Software triggers work independent to how the sensitivity and 
     *  are configured polarity.
     */

    errCode = XC_SetPropertyValueL(handle, "TriggerInMode", 1, "");
    if (!HandleError(errCode, " * Set trigger input mode")) 
        return false;

       
    /*
     *  TriggerInTiming = Optimal (0), Custom (1)
     * -----------------------------------------------------------------
     *  In the optimal mode, the sensor is constantly running and acquiring images preventing the
     *  sensor to cool down due to an irregular or too slow external trigger rate. If an external trigger
     *  is active, the next frame from the sensor is sent out on the interface. The latency between a
     *  trigger pulse and the actual acquisition start is in the worst case 1 frame time. In this mode,
     *  the trigger input delay should be considered as a minimum trigger input delay. As soon as an
     *  external trigger pulse is detected, a timer is started to implement the trigger input delay.
     *  When the timer expires, a trigger pulse is generated. The next frame will be sent out.
     *
     *  In the custom mode, the sensor only acquires frames when an external trigger is active. 
     *  This might degrade the image quality if the frequency of the external triggers is not high 
     *  enough or when irregular external triggers are applied.
     */

    errCode = XC_SetPropertyValueL(handle, "TriggerInTiming", 0, "");
    if (!HandleError(errCode, " * Set trigger input timing"))
        return false;

    printf(CRLF);
    return true;
}


/*
 *  In ExecuteSoftwareTrigger_F027 the software trigger is executed by writing
 *  a 1 to the SoftwareTrigger register.  
 */

bool ExecuteSoftwareTrigger_F027(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    /*
     *  SoftwareTrigger = Execute (1)
     * -----------------------------------------------------------------
     *  Execute a software trigger.
     */

    errCode = XC_SetPropertyValueL(handle, "SoftwareTrigger", 1, "");
    if (errCode != I_OK) return HandleError(errCode, "Execute software trigger");

    return true;
}


/*
 *  In SetupShutterControl_F027 we disable the automatic shutter correction.
 *  When this is set to enabled it is possible that triggers being received 
 *  during a calibrate cycle are not processed by the camera.
 *  To make sure the image does not drift and stays corrected the camera has
 *  to be occasionally calibrated by stopping / starting the acquisition or
 *  executing the "Calibrate"-property by setting its value to 1.
 */

bool SetupShutterControl_F027(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    printf("Configuring camera to disable the automatic shutter control: " CRLF);

    /* 
     *  AutoCorrectionEnabled = Disabled (0), Enabled (1)
     * -----------------------------------------------------------------
     *  Disable the automatic shutter control, 
     */

    errCode = XC_SetPropertyValueL(handle, "AutoCorrectionEnabled", 0, "");
    if (!HandleError(errCode, " * Disable auto correction"))
        return false;
    
    printf(CRLF);
    return true;
}


/*
 *  Setup image format and acquisition parameters
 */
bool SetupImageFormatAndAcquisition(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    /* Acquisition should be halted when the image format will change */
    boole wasCapturing = XC_IsCapturing(handle);
    if (wasCapturing) XC_StopCapture(handle);

    printf("Setup image format and acquisition parameters" CRLF);
    

    /* 
     *  ExposureTimeAbs = Integer (1 ... 80)
     * -----------------------------------------------------------------
     */

    errCode = XC_SetPropertyValueL(handle, "ExposureTimeAbs", 25, "");
    if (!HandleError(errCode, " * Set integration time"))
        return false;


    /* 
     *  Width = Integer (160 ... 640)
     * -----------------------------------------------------------------
     */

    errCode = XC_SetPropertyValueL(handle, "Width", XC_GetMaxWidth(handle), "");
    if (!HandleError(errCode, " * Set maximum width"))
        return false;


    /* 
     *  Height = Integer (120 ... 480)
     * -----------------------------------------------------------------
     */

    errCode = XC_SetPropertyValueL(handle, "Height", XC_GetMaxHeight(handle), "");
    if (!HandleError(errCode, " * Set maximum height"))
        return false;


    /* enable acquisition if it was already running when entering this function */
    if (wasCapturing) XC_StartCapture(handle);

    printf(CRLF);
    return true;
}


/*
 *  To make sure the image does not drift and stays corrected between bursts 
 *  the camera has to be occasionally calibrated by stopping / starting the 
 *  acquisition or executing the "Calibrate"-property between bursts by setting 
 *  its value to 1. When automatic correction is enabled the calibration occurs 
 *  after 150 seconds or when the devices temperature deviates 0.5 degrees from 
 *  the last measured point.
 *
 *  Because automatic correction while we are expecting triggers can cause events to 
 *  be missed we have to control the calibration progress.
 */

void WaitForCalibration_F027(XCHANDLE handle) {
    Sleep(2500);
}


bool ExecuteCalibration_F027(XCHANDLE handle) {
    ErrCode errCode = XC_SetPropertyValueL(handle, "Calibrate", 1, "");
    if (!HandleError(errCode, "Perform calibration"))
        return false;
    
    /* wait for calibrate */
    WaitForCalibration_F027(handle);
        
    printf("Calibration done." CRLF);
    return true;
}


void CleanupSession() {
    /* cleanup frame buffer */
    if (framebuffer != 0) free(framebuffer);
    framebuffer = 0;

    /* make sure capturing has stopped */
    if (XC_IsCapturing(handle)) XC_StopCapture(handle);

    /* close the session */
    XC_CloseCamera(handle);
}


void AbortSession() {
    printf("Aborting session." CRLF);
    CleanupSession();
    exit(-1);
}
