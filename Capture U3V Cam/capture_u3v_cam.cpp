
#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

#include <string.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "bitmap_image.hpp"

int main()
{
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    unsigned int deviceCount = 0;

    std::cout << "\nDiscovering USB3Vision cameras...\n" << std::endl;
    if ((errorCode = XCD_EnumerateDevices(NULL, &deviceCount, XEF_USB3Vision)) != I_OK) {
        printf("An error occurred while enumerating the devices. errorCode: %i\n", errorCode);
        return -1;
    }

    if (deviceCount == 0) {
        printf("Enumeration was a success but no devices were found!\n");
        return 0;
    }

    std::cout << "\nRetrieving cached USB3Vision cameras...\n" << std::endl;
    XDeviceInformation *devices = new XDeviceInformation[deviceCount];
    if ((errorCode = XCD_EnumerateDevices(devices, &deviceCount, XEF_UseCached)) != I_OK) {
        printf("Error while retrieving the cached device information structures. errorCode: %i\n", errorCode);
        delete [] devices;
        return -1;
    }

    /*  All discovered devices are now available in our local array and we are now able
     *  to iterate the list and output each item in the array */

    std::string camUrl = "";

    for(unsigned int i = 0; i < deviceCount; i++) {
        XDeviceInformation * dev = &devices[i];
        printf("device[%lu] %s @ %s (%s) \n", i, dev->name, dev->address, dev->transport);
        printf("PID: %4X\n", dev->pid);
        printf("Serial: %lu\n", dev->serial);
        printf("URL: %s\n", dev->url);
        printf("State: %s\n\n", dev->state == XDS_Available ? "Available" : dev->state == XDS_Busy ? "Busy" : "Unreachable");

        // Use the USB3Vision cam
        if (strcmp(dev->transport, "USB3Vision") == 0) {
            camUrl = dev->url;
        }
    }

    if (camUrl == "") {
        std::cout << "Could not find the Camera !?" << std::endl;
        return -1;
    }
    else {
        std::cout << "Found camera at: " << camUrl << std::endl;
    }


    XCHANDLE handle = 0; // Handle to the camera

    // Open a connection to the camera
    camUrl += "?bitsize=8";
    std::cout << "Opening connection to " << camUrl << std::endl;
    handle = XC_OpenCamera(camUrl.c_str());

    // When the connection is initialised, ...
    if (not XC_IsInitialised(handle)) {
        printf("Initialization failed\n");
        return -1;
    }

    std::cout << "Getting some image params" << std::endl;
    unsigned long maxWidth, maxHeight, width, height;
    maxWidth = XC_GetMaxWidth(handle);
    std::cout << "|- maxWidth = " << std::dec << maxWidth << std::endl;
    maxHeight = XC_GetMaxHeight(handle);
    std::cout << "|- maxHeight = " << std::dec << maxHeight << std::endl;
    width = XC_GetWidth(handle);
    std::cout << "|- width = " << std::dec << width << std::endl;
    height = XC_GetHeight(handle);
    std::cout << "|- height = " << std::dec << height << std::endl;

    // ... start capturing
    printf("Start capturing.\n");
    if ((errorCode = XC_StartCapture(handle)) != I_OK) {
        printf("Could not start capturing, errorCode: %lu\n", errorCode);
        return -1;
    }

    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0;   // The size in bytes of the raw image.

    if (XC_IsCapturing(handle)) // When the camera is capturing ...
    {
        // Determine native framesize.
        frameSize = XC_GetFrameSize(handle);
        std::cout << "frameSize = " << std::dec << frameSize << std::endl;

        // Initialize the 16-bit buffer.
        frameBuffer = new word[frameSize / 2];

        for (unsigned int i = 0; i<1000; i++) {
            // ... grab a frame from the camera.
            std::cout << "Grabbing frame #" << i << std::endl;
            if ((errorCode = XC_GetFrame(handle, FT_NATIVE, XGF_Blocking, frameBuffer, frameSize)) != I_OK) {
                std::cout << "Problem while fetching frame, errorCode " << std::dec << errorCode << std::endl;
            }
            else {

                // lambda function to format number to string with leading zeroes
                auto to_string_w_zeroes_lead = [](const int value, const unsigned precision) -> std::string {
                    std::ostringstream oss;
                    oss << std::setw(precision) << std::setfill('0') << value;
                    return oss.str();
                };

                BitmapImage bmi;
#ifdef _WIN32
                //windows specific code here !!!
                std::string imageFileName = "./bm_grab_" + to_string_w_zeroes_lead(act_cntr, 5) + ".bmp";
#else
                std::string imageFileName = "/tmp/bm_grab_" + to_string_w_zeroes_lead(i, 5) + ".bmp";
#endif // _WIN32
                bmi.generateBitmapFrom8bitImage((unsigned char *) frameBuffer, 480, 640, imageFileName);
                // YAXXX consider providing a method to get to u3vdb.payloadBufferInfo.ptr (dbhandler.u3vdb.payloadBufferInfo.ptr)
                std::cout << "Wrote to file: " << imageFileName << std::endl;
            }
        }
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
    delete [] frameBuffer;

    return 0;
}
