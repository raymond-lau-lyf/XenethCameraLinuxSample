#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include "XFilters.h" // Xeneth SDK main header.

int main()
{
    // Variables
    
    /*
     *  The calibration pack.
     *  Calibration packs are camera specific. 
     *  Copy the calibration pack that comes with the camera onto your system and modify the packname variable accordingly.
     */
    const char *packname = "C:\\Program Files\\Xeneth\\Sdk\\Samples\\Thermography - basic\\pack.xca"; 
    XCHANDLE handle = 0; // Handle to the camera.
    FilterID fltThermography = 0; // Handle to the thermography filter.
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.
    double *tempLUT = 0; // Temperature lookup table (ADU to temperature)
    dword width = 0, height = 0;
      
    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to cam://0\n");
    handle = XC_OpenCamera("cam://0");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {
        if (I_OK == XC_LoadCalibration(handle, packname, XLC_StartSoftwareCorrection))
        {
            fltThermography = XC_FLT_Queue(handle, "Thermography", "celsius");

            if (fltThermography > 0)
            {
                //When a TrueThermal calibration pack is loaded, it is allowed to change the integration time.
                //XC_SetPropertyValueL(handle,"IntegrationTime", 100, "");
                
                // Build the look-up table and ..
                dword mv = XC_GetMaxValue(handle);
                tempLUT = new double[mv+1];

                for(dword x = 0; x < mv + 1; x++)
                {
                    XC_FLT_ADUToTemperature(handle, fltThermography, x, &tempLUT[x]);
                }

                // .. start capturing.
                printf("Start capturing.\n");
                if ((errorCode = XC_StartCapture(handle)) != I_OK)
                {
                    printf("Could not start capturing, errorCode: %u\n", errorCode);
                }
                else if (XC_IsCapturing(handle)) // When the camera is capturing ...
                {
                    frameSize = XC_GetFrameSize(handle);
                    frameBuffer = new word[(frameSize + XC_GetFrameFooterLength(handle)) / 2];
                    width = XC_GetWidth(handle);
                    height = XC_GetHeight(handle);

                    while (true)
                    {
                        if ((errorCode = XC_GetFrame(handle, FT_16_BPP_GRAY, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                        {
                            printf("Problem while fetching frame, errorCode %u", errorCode);
                            break;
                        }
                        else
                        {
                            // Use the table to lookup the temperature by passing the digital values as index.
                            printf("Temperature at center: %f.2lf degrees Celsius       \r", tempLUT[frameBuffer[width/2 + (height / 2 * width)]]);
                        }
                    }
                }
            }
            else
            {
                printf("Could not start thermography filter, closing connection.\n");
            }
        }
        else
        {
            printf("Could not load calibration pack '%s', closing connection\n", packname);
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
            printf("Could not stop capturing, errorCode: %u\n", errorCode);
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

    if (tempLUT != 0)
    {
        delete [] tempLUT;
        tempLUT = 0;
    }

    return 0;
}
