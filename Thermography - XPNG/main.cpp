#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include "XFilters.h" // Xeneth SDK main header.

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

            /*
            *  The calibration pack.
            *  Calibration packs are camera specific. 
            *  Copy the calibration pack that comes with the camera onto your system and modify the packname variable accordingly.
            */
            XC_LoadCalibration(handle, "pack.xca", XLC_StartSoftwareCorrection);

            // Load thermography filter
            int thermalFilterId = XC_FLT_Queue(handle, "Thermography", 0);

            // Determine native framesize.
            frameSize = XC_GetFrameSize(handle);

            // Initialize the 16-bit buffer.
            frameBuffer = new word[frameSize / 2];

            // ... grab a frame from the camera.

            for(int cnt = 0; cnt < 100; cnt++) {
                if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                {
                    printf("Problem while fetching frame, errorCode %lu", errorCode);
                }
            }

            errorCode = XC_SaveData(handle, "output.xpng", XSD_SaveThermalInfo | XSD_Force16);

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
