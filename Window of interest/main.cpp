#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.
      
    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to cam://0\n");
    handle = XC_OpenCamera("cam://0");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {      
        // ... start capturing
        printf("Start capturing.\n");
        if ((errorCode = XC_StartCapture(handle)) != I_OK)
        {
            printf("Could not start capturing, errorCode: %lu\n", errorCode);
        }
        else if (XC_IsCapturing(handle)) // When the camera is capturing ...
        {
            long woiSX = 0, woiEX = 0, woiSY = 0, woiEY = 0; // Holds the Window of interest (WOI) positions.
            
            // Build buffer before the window was set for demonstration purposes, this buffer can now hold a fullsized frame.
            frameSize = XC_GetFrameSize(handle);
            printf("Building buffer of %lu bytes\n", frameSize);
            frameBuffer = new word[frameSize / 2];

            // Make sure the camera is capable of setting a window of interest before continuing.
            if (I_OK == XC_GetPropertyValueL(handle, "CapWoiCount", NULL)) 
            {
                // Retrieve the current Woi values.
                XC_GetPropertyValueL(handle, "WoiSX(0)", &woiSX);
                XC_GetPropertyValueL(handle, "WoiEX(0)", &woiEX);
                XC_GetPropertyValueL(handle, "WoiSY(0)", &woiSY);
                XC_GetPropertyValueL(handle, "WoiEY(0)", &woiEY);
                printf("Current window of interest -> sx: %ld, ex: %ld, sy: %ld, ey: %ld\n", woiSX, woiEX, woiSY, woiEY);

                // We set a window which is half the size of the current window.
                XC_SetPropertyValueL(handle, "WoiSX(0)", woiSX, "");
                XC_SetPropertyValueL(handle, "WoiEX(0)", woiEX / 2, "");
                XC_SetPropertyValueL(handle, "WoiSY(0)", woiSY, "");
                XC_SetPropertyValueL(handle, "WoiEY(0)", woiEY / 2, "");

                // Retrieve the current Woi values.
                XC_GetPropertyValueL(handle, "WoiSX(0)", &woiSX);
                XC_GetPropertyValueL(handle, "WoiEX(0)", &woiEX);
                XC_GetPropertyValueL(handle, "WoiSY(0)", &woiSY);
                XC_GetPropertyValueL(handle, "WoiEY(0)", &woiEY);
                printf("Current window of interest after update -> sx: %ld, ex: %ld, sy: %ld, ey: %ld\n", woiSX, woiEX, woiSY, woiEY);

                // Now we can rebuild the buffer to accommodate the new framesize.
                if (frameBuffer != 0)
                    delete [] frameBuffer;

                frameSize = XC_GetFrameSize(handle);
                printf("Building new buffer of %lu bytes\n", frameSize);
                frameBuffer = new word[frameSize / 2];
            }
            else
            {
                printf("The connected camera does not support windows of interest.\n");
            }
        }
    }
    else
    {
        printf("Initialization failed\n");
    }

   // Cleanup.

   // When the camera is still capturing, ...
    if(XC_IsCapturing(handle))
    {
        // ... stop capturing.
        printf("Stop capturing.\n");
        if ((errorCode = XC_StopCapture(handle)) != I_OK)
        {
            printf("Could not stop capturing, errorCode: %lu\n", errorCode);
        }
    }

    // When the handle to the camera is still initialised ...
    if (XC_IsInitialised(handle))
    {
        printf("Closing connection to camera.\n");
        XC_CloseCamera(handle);
    }

    printf("Clearing buffers.\n");
    if (frameBuffer != 0)
    {
        delete [] frameBuffer;
        frameBuffer = 0;
    }

    return 0;
}
