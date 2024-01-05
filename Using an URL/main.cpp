#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    /* Let's assume we got two cameras on the network using a fixed IP.
     * We are aware of which IPs they have, the first one is on 192.168.1.87 and we know it is a Gobi-640 GigE camera. 
     * The second camera has IP address 192.168.1.210 and we know it's a Gobi384PDCLSCI50 using the Xenics network protocol (XNP) */

    printf("Opening connections.\n");
    XCHANDLE cam1 = XC_OpenCamera("cam://0");    /* following GigEVision URL scheme */
    XCHANDLE cam2 = XC_OpenCamera("cam://1");   /* following Xeneth network protocol URL scheme */
    
    if (!XC_IsInitialised(cam1)) { 
        printf("Could not open a connection to cam1.\n");
        return -1;
    }

    if (!XC_IsInitialised(cam2)) {
        XC_CloseCamera(cam1);
        printf("Could not open a connection to cam2.\n");
        return -1;
    }

    printf("Cameras initialized.\n");
    printf("Cleanup.\n");

    /* close handles */
    XC_CloseCamera(cam1);
    XC_CloseCamera(cam2);

    return 0;
}