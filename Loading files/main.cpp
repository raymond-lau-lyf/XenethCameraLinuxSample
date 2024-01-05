#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    const char * url = "cam://0";       // The url of the camera to which a connection will be established.
    XCHANDLE handle = 0;                // Handle of the camera session.

    /*
    *  FileAccessCorrectionFile is a blob typed property used to upload a Xenics calibration pack (XCA) to the camera.
    *  Do note that not all cameras have the same property name exposed to upload the pack to. 
    *  For example, in the interface of a Gobi (F020) the property is called LoadAndStoreCalibration. 
    *  It is advised to check the Camera Specific documentation for the correct property name.
    *  You should make sure that the propertyName-variable contains the correct value or uploading a calibration pack will fail.
    */

    const char * propertyName = "FileAccessCorrectionFile";

    /*
    *  The calibration pack which will be uploaded to the camera. This can be a relative or absolute path to the source file.
    *  Prior to running this sample, make sure the pack variable contains a the path to a XCA file for the camera to which a connection will be established.
    */
    
    const char * pack = "pack.xca";

    // Open connection to the camera
    printf("Open connection to '%s'\r\n", url);
    handle = XC_OpenCamera(url);
    
    // Check if initialisation was successful
    if (XC_IsInitialised(handle)) {
        printf("Connection to '%s' initialised.\r\n", url);

        /*
        *  It is possible to upload data to a blob property using the XC_SetPropertyValue instead of XC_SetPropertyBlob.
        *  This is mainly useful when uploading data that is stored as a file on the computer.
        *  Xeneth will handle all the necessary file operations to open and read the contents.
        *  The pValue-argument must contain a string with a relative or absolute path to the calibration pack (XCA-file).
        */

        printf("Uploading '%s' to '%s': ", pack, propertyName);
        ErrCode errorCode = XC_SetPropertyValue(handle, propertyName, pack, "");

        // Validate return value
        if (errorCode != I_OK) printf("Error (%i)\r\n", errorCode, pack, propertyName);
        else printf("OK\r\n");

        // Close the connection
        XC_CloseCamera(handle);
    }
    else {
        printf("Could not initialise connection to '%s'.\r\n", url);
    }

    return 0;
}