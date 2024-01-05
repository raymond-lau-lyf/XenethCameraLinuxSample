#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

#include <string.h>

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.

    // Open a connection to the first detected camera.
    printf("Opening connection to cam://0?fg=none\n");
    handle = XC_OpenCamera("cam://0?fg=none");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {   
        long pid = 0;
        XC_GetPropertyValueL(handle, "_CAM_PID", &pid);
        printf("Connected to a %lx camera\n", pid);

        if (pid == 0xF020) // Tested on a Gobi, check the camera specific documentation.
        {
            FILE *file;
            char *buffer = 0;
            long bufferlen = 0;

            // Open file in mode binary read!
            file = fopen("pack.xca", "rb"); 
            if (file != 0)
            {
                // Determine the filesize.
                fseek(file, 0, SEEK_END);
                bufferlen = ftell(file);
                rewind(file);

                // Fill buffer with file data.
                buffer = new char[bufferlen];
                fread(buffer, sizeof(char), bufferlen, file);

                // Upload the blob.
                printf("Uploading Blob to camera.\n");
                errorCode = XC_SetPropertyBlob(handle, "LoadAndStoreCalibration", buffer, bufferlen);

                if (errorCode != I_OK)
                {
                    printf("An error state was returned when loading the blob to the camera, code: %lu", errorCode);
                }
                else
                {
                    printf("Upload complete!\n");
                }
            }
            else
            {
            printf("pack.xca not found.\n");
            }

            if (file != 0)
            fclose(file);

            if (buffer != 0)
            delete [] buffer;
        }
    }
    else
    {
        printf("Initialization failed\n");
    }

    // Cleanup.

    // When the handle to the camera is still initialised ...
    if (XC_IsInitialised(handle))
    {
        printf("Closing connection to camera.\n");
        XC_CloseCamera(handle);
    }

    return 0;
}
