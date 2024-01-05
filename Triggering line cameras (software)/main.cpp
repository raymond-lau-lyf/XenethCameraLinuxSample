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

bool SetupSoftwareTriggeredMode_F083_F085(XCHANDLE handle);
bool ExecuteSoftwareLineTrigger_F083_F085(XCHANDLE handle, unsigned int count = 1, unsigned int delay = 0);
bool ReArmTriggerMode_F083_F085(XCHANDLE handle);


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
 *  Set the height of one frame.
 */

const unsigned int height = 10;


/*
 *  The product id of the connected device
 */
long pid = 0;


// Entry point ///////////////////////////////////////////////////////////

int main() {    
    ErrCode errCode = I_OK;
    

    printf("Open connection to '%s'" CRLF, url);
    handle = XC_OpenCamera(url);

    if (XC_IsInitialised(handle)) {     
        printf("Successfully established a connection to '%s'." CRLF, url);
        
        /* retrieve camera product id (PID) */
        errCode = XC_GetPropertyValueL(handle, "_CAM_PID", &pid);
        if (!HandleError(errCode, "Retrieving the camera PID")) AbortSession();        
        printf("Connected to camera with product id (PID) 0x%X" CRLF CRLF, pid);

        /* Check for the Lynx-CL and Lynx-GigE family (F083 and F085) */
        if (pid == 0xF083 || pid == 0xF085) {

            /* configure camera in software triggered mode */
            if (!SetupSoftwareTriggeredMode_F083_F085(handle)) AbortSession();

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
            printf("Could not allocate frame buffer memory." CRLF);
            AbortSession();
        }
         
        /* start capture */
        XC_StartCapture(handle);

        /* main loop */
        while (true) {

            if (!ReArmTriggerMode_F083_F085(handle)) AbortSession();

            printf("Waiting for %i line triggers to generate one frame" CRLF, XC_GetHeight(handle));
            if (!ExecuteSoftwareLineTrigger_F083_F085(handle, height, 200)) AbortSession();

            /* 
             *  Grab a frame from the internal buffers if one is available.
             *  If no frames were captured yet, or we are polling faster than 
             *  frames are arriving, GetFrame will return E_NO_FRAME.
             */

            do {
                
                errCode = XC_GetFrame(handle, FT_NATIVE, 0, framebuffer, framesize);

                if (I_OK == errCode) {
                    printf("Received a frame" CRLF);
                }
                else if (E_NO_FRAME != errCode) {
                    printf(CRLF);
                    HandleError(errCode, "while grabbing frame");
                    AbortSession();
                }

            } while (errCode == E_NO_FRAME);            

            /* Ask the user to perform another trigger burst cycle */
            printf(CRLF "Enter 'y' to perform another burst: ");
            char ch = (char)fgetc(stdin);
            for (int c = getc(stdin); c != '\n'; c = getc(stdin));

            /* any other answer except for 'y' breaks the main loop */
            if (ch != 'y') break;
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
 *  In SetupSoftwareTriggeredMode_F083_F085 we configure the camera in software triggered mode.
 */

bool SetupSoftwareTriggeredMode_F083_F085(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    printf("Configuring camera in external triggered mode with rising edge activation" CRLF);
    
    /*
     *  TriggerMode = 
     *
     *  -- F083 --------------------------------------------------------
     *
     *      Line: Free running - Frame: Free running (0),
     *      Line: Free running - Frame: CC1 (1), 
     *      Line: Free running - Frame: SMA (2),
     *      Line: CC1 - Frame: Free running (3),
     *      Line: CC1 - Frame: SMA (4),
     *      Line: SMA - Frame: Free running (5),
     *      Line: SMA - Frame: CC1 (6),
     *
     *  -- F085 --------------------------------------------------------
     *
     *      Line: Free running - Frame: Free running (0), 
     *      Line: Free running - Frame: SMA (1), 
     *      Line: SMA - Frame: Free running (2) 
     *
     * -----------------------------------------------------------------
     *  The trigger mode configures which trigger signal is triggered
     *  on activation, the line or the frame trigger. For CameraLink
     *  devices the CC1 signal can also be used as a source.
     */

    if (pid == 0xF083) {
        errCode = XC_SetPropertyValueL(handle, "TriggerMode", 5, "");
        if (!HandleError(errCode, " * Configure trigger mode 'Line: SMA - Frame: Free running'"))
            return false;

    }
    else if (pid == 0xF085) {
        errCode = XC_SetPropertyValueL(handle, "TriggerMode", 2, "");
        if (!HandleError(errCode, " * Configure trigger mode 'Line: SMA - Frame: Free running'"))
            return false;

    }


    /*
     *  LineTriggerEnable = Off (0), On (1)
     *  Disable the line trigger block, these signal are overridden by the software trigger
     */

    errCode = XC_SetPropertyValueL(handle, "LineTriggerEnable", 0, "");
    if (!HandleError(errCode, " * Enable line trigger"))
        return false;
    

    /*
     *  FrameTriggerEnable = Off (0), On (1)
     *  Disable the frame trigger block, these signal are overridden by the software trigger
     */

    errCode = XC_SetPropertyValueL(handle, "FrameTriggerEnable", 0, "");
    if (!HandleError(errCode, " * Disable frame trigger"))
        return false;


    printf(CRLF);
    return true;
}


/*
 *  Execute the software line trigger.
 */

bool ExecuteSoftwareLineTrigger_F083_F085(XCHANDLE handle, unsigned int count /* = 1 */, unsigned int delayMs /* = 0 */) {

    ErrCode errCode = I_OK;

    printf("Executing %i software line triggers with %ims delay." CRLF, count, delayMs);

    /* generate one or more software line trigger */

    for (unsigned int i = 0; i < count; ++i) {
        if (delayMs) Sleep(delayMs);
        errCode = XC_SetPropertyValueL(handle, "SoftwareLineTrigger", 1, "");
        if (I_OK != errCode) {
            HandleError(errCode, CRLF " * While executing software line trigger");
            return false;
        }

        printf(".");
    }

    printf(CRLF);
    return true;
}


/*
 *  Re-arm the current trigger mode.
 */

bool ReArmTriggerMode_F083_F085(XCHANDLE handle) {

    ErrCode errCode = I_OK;

    printf("Re-arming camera in triggered mode." CRLF);


    /* make sure we are no longer capturing frames */
    if (XC_IsCapturing(handle)) XC_StopCapture(handle);


    /* obtain the current trigger mode */
    long prevTriggerMode = 0;
    errCode = XC_GetPropertyValueL(handle, "TriggerMode", &prevTriggerMode);
    if (errCode != I_OK) {
        HandleError(errCode, " * While retrieving the current trigger mode");
        return false;
    }


    /*  No need to re-arm if both line and frame triggers are already configured for free running */
    if (prevTriggerMode == 0) 
        return true;


    /* set camera in free-running */ 
    errCode = XC_SetPropertyValueL(handle, "TriggerMode", 0, "");
    if (!HandleError(errCode, " * Set free-running for both line and frame triggers"))
        return false;


    /* start capturing */
    XC_StartCapture(handle);


    /* make sure we collect more than one frame */
    while (XC_GetFrameCount(handle) < 2) Sleep(1);


    /* revert to the triggered mode */    
    errCode = XC_SetPropertyValueL(handle, "TriggerMode", prevTriggerMode, "");
    if (errCode != I_OK) {
        HandleError(errCode, " * While setting back the configured trigger mode");
        return false;
    }
    
    /* when performing line triggers, make sure we start 
        generating the triggers on the start of a new image */        
    
    if ((pid == 0xF083 && prevTriggerMode == 5) || (pid == 0xF085 && prevTriggerMode == 2)) {
        dword fc = XC_GetFrameCount(handle);
        while(fc == XC_GetFrameCount(handle)) {
            errCode = XC_SetPropertyValueL(handle, "SoftwareLineTrigger", 1, "");
            if (I_OK != errCode) {
                HandleError(errCode, CRLF " * While executing software line trigger");
                return false;
            }
        }
    }

    /* eat data already available in the buffers */
    while(XC_GetFrame(handle, FT_NATIVE, 0, 0, XC_GetFrameSize(handle)) != E_NO_FRAME) Sleep(1);


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
    XC_StopCapture(handle);

    printf("Setup image format and acquisition parameters" CRLF);
    
    /* 
     *  ExposureTimeAbs = Integer (1 ... 1000000)
     * -----------------------------------------------------------------
     */

    errCode = XC_SetPropertyValueL(handle, "ExposureTime", 250, "");
    if (!HandleError(errCode, " * Set exposure time"))
        return false;


    /* 
     *  Height = Integer (1 .. 1024)
     * -----------------------------------------------------------------
     */

    errCode = XC_SetPropertyValueL(handle, "Height", height, "");
    if (!HandleError(errCode, " * Set height"))
        return false;


    /* enable acquisition if it was already running when entering this function */
    if (wasCapturing) XC_StartCapture(handle);


    printf(CRLF);
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
