#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    ErrCode errorCode = I_OK;
    unsigned int deviceCount = 0;

    /*  We start by retrieving the number of discovered devices. In this sample discovery is performed on all available protocols.
     *  For this simply pass the XEF_EnableAll flag with the XCD_EnumerateDevices function.
     *  To only retrieve the device count the first argument must be set to NULL.
     *  Note that when no flags are passed at all, meaning the flags argument value is 0, no enumeration will happen. */

    if ((errorCode = XCD_EnumerateDevices(NULL, &deviceCount, XEF_EnableAll)) != I_OK) {
        printf("An error occurred while enumerating the devices. errorCode: %i\n", errorCode);
        return -1;
    }

    if (deviceCount == 0) {
        printf("Enumeration was a success but no devices were found!\n");
        return 0;
    }

    /*  At this point we know how much devices are present in the environment.
     *  Now allocate the XDeviceInformation array to accommodate all the discovered devices using
     *  the device count to determine the size needed. Once allocated we call the enumerate
     *  devices method again but now instead of passing null as the initial argument use the new
     *  allocated buffer. For the flags argument we no longer use a protocol enable flag but make use
     *  of the XEF_UseCached flag. On discovery devices are cached internally and as such we are able to 
     *  retrieve the device information structures instantly when calling XCD_EnumerateDevices for a second time.
     *  Note that it is not required to first check device count, allocate structure and retrieve cached devices.
     *  A user could allocate one or more device structure and immediately pass this with the initial call to XCD_EnumerateDevices.
     *  XCD_EnumerateDevices will not exceed the supplied deviceCount and when less devices were discovered than the initial deviceCount 
     *  this argument is updated with the new count. */

    XDeviceInformation *devices = new XDeviceInformation[deviceCount];
    if ((errorCode = XCD_EnumerateDevices(devices, &deviceCount, XEF_UseCached)) != I_OK) {
        printf("Error while retrieving the cached device information structures. errorCode: %i\n", errorCode);
        delete [] devices;
        return -1;
    }

    /*  All discovered devices are now available in our local array and we are now able 
     *  to iterate the list and output each item in the array */

    for(unsigned int i = 0; i < deviceCount; i++) {
        XDeviceInformation * dev = &devices[i];
        printf("device[%lu] %s @ %s (%s) \n", i, dev->name, dev->address, dev->transport);
        printf("PID: %4X\n", dev->pid); 
        printf("Serial: %lu\n", dev->serial);
        printf("URL: %s\n", dev->url);
        printf("State: %s\n\n", dev->state == XDS_Available ? "Available" : dev->state == XDS_Busy ? "Busy" : "Unreachable");
    }

    delete [] devices;
    return 0;
}