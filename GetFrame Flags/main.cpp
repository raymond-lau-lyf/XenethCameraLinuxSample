#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The width of the camera's image.

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
            // Determine native framesize.
            frameSize = XC_GetFrameSize(handle);

            // Initialize the 16-bit buffer.
            frameBuffer = new word[frameSize / 2];

            // Grab 5 frames in non-blocking mode.
            printf("Grabbing 5 frames in non-blocking mode (default).\n");
            for(unsigned int i = 1; i <= 5; i++)
            {
                // Loop until we got a frame ready.
                while((errorCode = XC_GetFrame(handle, FT_NATIVE, 0, frameBuffer, frameSize)) == E_NO_FRAME)
                {
                    // Do some intermediate work.
                }

                if (errorCode == I_OK)
                {
                    printf("Got frame %u\n", i);
                }
                else
                {
                    printf("A problem occurred while grabbing frame %u, errorCode: %lu\n", i, errorCode);
                }
            }

            // Grab 5 frames using XGF_Blocking
            printf("Grabbing 5 frames in blocking mode.\n");
            for(unsigned int i = 1; i <= 5; i++)
            {      
                if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking, frameBuffer, frameSize)) == I_OK)
                {
                    printf("Got frame %u\n", i);
                }
                else
                {
                    printf("A problem occurred while grabbing frame %u, errorCode: %lu\n", i, errorCode);
                }
            }

            // Grab 5 frames using XGF_Blocking and XGF_NoConversion
            printf("Grabbing 5 frames in blocking and no conversion mode.\n");
            for(unsigned int i = 1; i <= 5; i++)
            {      
                if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking | XGF_NoConversion, frameBuffer, frameSize)) == I_OK)
                {
                    printf("Got frame %u\n", i);
                }
                else
                {
                    printf("A problem occurred while grabbing frame %u, errorCode: %lu\n", i, errorCode);
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
