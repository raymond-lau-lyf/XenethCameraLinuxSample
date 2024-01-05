#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    // Variables
    XCHANDLE handle = 0; // Handle to the camera
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.

    // Open a connection to the first detected camera by using connection string cam://0
    // Note the fg parameter that is passed in the query part of the connection string.
    printf("Opening connection to cam://0?fg=none\n");
    handle = XC_OpenCamera("cam://0?fg=none");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {   
        long pid;
        long ser;

        // Once the camera is initialized you can get and set properties.
        XC_GetPropertyValueL(handle, "_CAM_PID", &pid);
        XC_GetPropertyValueL(handle, "_CAM_SER", &ser);

        // Output the product id and serial.
        printf("Controlling camera with PID: %lx, SER: %ld\n", pid, ser);

        // A call to the getframe functions will not return an error, but the image will be black since no thread is running to grab images.
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
