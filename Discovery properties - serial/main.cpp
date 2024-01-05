#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
    int COMPortLow = 0, COMPortHigh = 0;

    /* retrieve the current default values */
    if (XCD_GetPropertyValueL("COMPortLow", &COMPortLow) != I_OK || XCD_GetPropertyValueL("COMPortHigh", &COMPortHigh) != I_OK) {
        printf("An issue occurred while trying to retrieve the default discovery properties.\n");
        return -1;
    }

    printf("The current default values are %i to %i\n", COMPortLow, COMPortHigh);

    /* Set new serial port range, for example 128 to 255 */
    if (XCD_SetPropertyValueL("COMPortLow", 128) != I_OK || XCD_SetPropertyValueL("COMPortHigh", 255) != I_OK) {
        printf("An issue occurred while trying to set the new discovery properties.\n");
        return -1;
    }

    /* Retrieve the new COM port values */
    if (XCD_GetPropertyValueL("COMPortLow", &COMPortLow) != I_OK || XCD_GetPropertyValueL("COMPortHigh", &COMPortHigh) != I_OK) {
        printf("An issue occurred while trying to retrieve the new discovery properties.\n");
        return -1;
    }

    printf("The new values are %i to %i\n", COMPortLow, COMPortHigh);
        
    return 0;
}