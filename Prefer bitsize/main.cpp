#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.
    long bitsize = 0;

    // Open a connection to the first detected camera by using connection string cam://0?bitsize=8
    printf("Opening connection to cam://0?bitsize=8\n");
    handle = XC_OpenCamera("cam://0?bitsize=8");

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
            // Determine native framesize.
            frameSize = XC_GetFrameSize(handle);

            // Retrieve bitsize (tested with a Gobi F020, 
            // for the correct property name of your camera refer to the 
            // Camera specific\properties in the documentaion.
            errorCode = XC_GetPropertyValueL(handle, "BitsPerPixel", &bitsize);

            if (errorCode != E_NOT_SUPPORTED)
                printf("Current bitsize of the camera is %d-bit\n", bitsize == 0 ? 8 : 16);

            // Initialize the 16-bit buffer.
            frameBuffer = new word[frameSize / 2];

            // ... grab a frame from the camera.
            printf("Grabbing a frame.\n");
            if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
            {
                printf("Problem while fetching frame, errorCode %lu", errorCode);
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
