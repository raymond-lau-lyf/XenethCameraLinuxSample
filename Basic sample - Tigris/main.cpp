#include "stdio.h"    // C Standard Input/Output library.
#include "XCamera.h"  // Xeneth SDK main header.
#include "XFooters.h" // Xeneth frame footer header.

/*
 * The sample applies to the XCO/Tigris camera.
 * It shows how an image and an image footer is captured for a selected pixel format.
 */

int main()
{

    // pixel formats supported for GigE Vision
    // const char* pixelFormat = "Mono8";
    // const char* pixelFormat = "Mono16";
    const char* pixelFormat = "RGB8";

    
    // Variables
    XCHANDLE handle = 0;   // handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.

    char   *frameBuffer;   // buffer to store the captured frame

    // Open a connection via GigE Vision - enter the correct ip address
    printf("Opening connection to gev://169.254.1.1\n");
    handle = XC_OpenCamera("gev://169.254.1.1");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {
        // Select pixel format. 
        printf("\nPixel format %s\n\n", pixelFormat);
        XC_SetPropertyValueE(handle, "PixelFormat", pixelFormat);

        // Enable in image footer.
        XC_SetPropertyValueL(handle, "FooterEnable", 1, "");
        
        // Determine the corresponding frame size.
        dword frameSize  = XC_GetFrameSize(handle);
        
        // Determine the footer size.
        dword footerSize = XC_GetFrameFooterLength(handle);

        frameBuffer = new char[frameSize + footerSize];

        // Start capturing.
        printf("Start capturing.\n");
        if ((errorCode = XC_StartCapture(handle)) != I_OK)
        {
            printf("Could not start capturing, errorCode: %lu\n", errorCode);
        }
        else if (XC_IsCapturing(handle)) // When the camera is capturing ...
        {
            // Determine native framesize.
            frameSize = XC_GetFrameSize(handle);

            // Grab a frame from the camera.
            printf("Grabbing a frame.\n");
            if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking|XGF_FetchPFF, frameBuffer, frameSize)) != I_OK)
            {
                printf("Problem while fetching frame, errorCode %lu", errorCode);
            }
            else
            {
                XPFF_GENERIC *perFrameFooter = (XPFF_GENERIC*)(frameBuffer + frameSize);

                // Software footer structure
                printf("\nSoftware footer contents:\n");

                printf("    Structure length:    %u\n", perFrameFooter->len);
                printf("    Version:             %x\n", perFrameFooter->ver);
                printf("    Start of capture:    %u\n", perFrameFooter->soc);
                printf("    Time of reception:   %u\n", perFrameFooter->tft);
                printf("    Frame count:         %u\n", perFrameFooter->tfc);
                printf("    Filter marker:       %u\n", perFrameFooter->fltref);
                printf("    Hardware footer len: %u\n", perFrameFooter->hfl);
                printf("    Hardware footer pid: %x\n", perFrameFooter->Common.pid);


                // Hardware footer structure
                if (perFrameFooter->Common.pid == 0xf090) // . 
                {
                    XPFF_F090 *hpff = &perFrameFooter->Common.Cameras.tigris;

                    // Hardware footer structure
                    printf("\nHardware footer contents:\n");

                    printf("    Time low:          %u\n", hpff->timelo);
                    printf("    Time high:         %x\n", hpff->timehi);
                    printf("    Frame counter:     %u\n", hpff->counter);
                    printf("    Sample counter:    %u\n", hpff->sample_counter);
                    printf("    Window offset X:   %u\n", hpff->offset_x);
                    printf("    Window offset Y:   %u\n", hpff->offset_y);
                }
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
