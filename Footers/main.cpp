#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include "XFooters.h" // Xeneth frame footer header.

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.
    XPFF_GENERIC *perFrameFooter = 0; // The generic structure for the footer.
    dword frameFooterSize = 0;

    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to gev://169.254.1.1\n");
    handle = XC_OpenCamera("gev://169.254.1.1");

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

            // Determine frame footer size
            frameFooterSize = XC_GetFrameFooterLength(handle);

            // Initialize the 16-bit buffer, making sure there is also room for the extra footer data.
            frameBuffer = new word[(frameSize + frameFooterSize) / 2];

            // ... grab a frame from the camera. A new flag is passed as an argument to the GetFrame method named XGF_FetchPFF.
            printf("Grabbing a frame.\n");
            
            if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking|XGF_FetchPFF, frameBuffer, frameSize)) != I_OK)
            {
                printf("Problem while fetching frame, errorCode %lu", errorCode);
            }
            else
            {
                perFrameFooter = (XPFF_GENERIC*)(frameBuffer + (frameSize / 2));

                // Software footer structure
                printf("\nSoftware footer contents:\n");

                printf("     Structure length: %u\n", perFrameFooter->len);
                printf("              Version: %x\n", perFrameFooter->ver);
                printf("     Start of capture: %u\n", perFrameFooter->soc);
                printf("    Time of reception: %u\n", perFrameFooter->tft);
                printf("          Frame count: %u\n", perFrameFooter->tfc);
                printf("        Filter marker: %u\n", perFrameFooter->fltref);
                printf("  Hardware footer len: %u\n", perFrameFooter->hfl);
                printf("  Hardware footer pid: %x\n", perFrameFooter->Common.pid);

                // Hardware footer structure
                // if (perFrameFooter->Common.pid == 0xf003) // GOBI class cameras. 
                if (1)
                {
                    XPFF_F003 *hpff = &perFrameFooter->Common.Cameras.gobi;

                    printf("\nHardware footer contents:\n");

                    printf("  Integration time: %u\n", hpff->tint);
                    printf("          time low: %u\n", hpff->timelo);
                    printf("         time high: %u\n", hpff->timehi);
                    printf("   Die temperature: %u\n", hpff->temp_die);
                    printf("               Tag: %u\n", hpff->tag);
                    printf("      Image offset: %u\n", hpff->image_offset);
                    printf("        Image gain: %u\n", hpff->image_gain);

                    printf("\nStatus bits:\n");
                    printf("      Trigger ext.: %u\n", hpff->status.statusbits.trig_ext);
                }
                else if (perFrameFooter->Common.pid == 0xf040) // ONCA class cameras
                {
                    XPFF_F040 *hpff = &perFrameFooter->Common.Cameras.onca;

                    printf("\nFrame footer contents:\n");

                    printf("  Integration time: %u\n", hpff->tint);
                    printf("          time low: %u\n", hpff->timelo);
                    printf("         time high: %u\n", hpff->timehi);
                    printf("   Die temperature: %u\n", hpff->temp_die);
                    printf("  Case temperature: %u\n", hpff->temp_case);

                    printf("\nStatus bits:\n");
                    printf("      Trigger ext.: %u\n", hpff->status.statusbits.trig_ext);
                    printf("       Trigger cl.: %u\n", hpff->status.statusbits.trig_cl);
                    printf("      Trigger soft: %u\n", hpff->status.statusbits.trig_soft);
                    printf(" Line cam fixed SH: %u\n", hpff->status.statusbits.linecam_fixedSH);
                    printf("Line cam SHB first: %u\n", hpff->status.statusbits.linecam_SHBfirst);
                    printf("      Filter wheel: %u\n", hpff->status.statusbits.filterwheel);
                }

                printf("\n");
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
