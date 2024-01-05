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
        int propertyCount = XC_GetPropertyCount(handle);
        char propertyName[128];
        char propertyCategory[128];
        char propertyRange[128];
        char propertyUnit[128];
        XPropType propertyType;

        long lvalue = 0;
        char *cvalue = NULL;
        double fvalue = 0;

        // Iterate over each property and output details such as name, type, value
        for(int x = 0; x < propertyCount; x++)
        {
            XC_GetPropertyName(handle, x, &propertyName[0], 128);
            XC_GetPropertyCategory(handle, propertyName, &propertyCategory[0], 128);
            XC_GetPropertyType(handle, propertyName, &propertyType);
            XC_GetPropertyRange(handle, propertyName, propertyRange, 128);
            XC_GetPropertyUnit(handle, propertyName, propertyUnit, 128);

            printf("Property[%d]    Category: %s\n", x, propertyCategory);
            printf("Property[%d]        Name: %s\n", x, propertyName);
            printf("Property[%d]       Flags: %s%s%s%s%s%s\n", x,
                (propertyType & XType_Base_MinMax ? "MinMax | " : ""),
                (propertyType & XType_Base_ReadOnce ? "ReadOnce | " : ""),
                (propertyType & XType_Base_NoPersist ? "NoPersist | " : ""),
                (propertyType & XType_Base_NAI ? "NAI | " : ""),
                (propertyType & XType_Base_Writeable ? "Writeable | " : ""),
                (propertyType & XType_Base_Readable ? "Readable | " : "")
            );

            // The following output depends on the property type.
            switch(propertyType & XType_Base_Mask)
            {
                case XType_Base_Number:
                    // To check whether a value is of type float or long the camera specific property documentation should be referenced.
                    XC_GetPropertyValueL(handle, propertyName, &lvalue);
                    XC_GetPropertyValueF(handle, propertyName, &fvalue);

                    printf("Property[%d]        Type: Number\n", x);
                    printf("Property[%d]       Range: %s\n", x, propertyRange);
                    printf("Property[%d]        Unit: %s\n", x, propertyUnit);
                    printf("Property[%d]  Long value: %lu\n", x, lvalue);
                    printf("Property[%d] Float value: %f\n", x, fvalue);
                    break;

                case XType_Base_Enum:
                    XC_GetPropertyValueL(handle, propertyName, &lvalue);

                    printf("Property[%d]        Type: Enum\n", x);
                    printf("Property[%d]       Range: %s\n", x, propertyRange);
                    printf("Property[%d]       Value: %lu\n", x, lvalue);
                    break;

                case XType_Base_Bool:
                    XC_GetPropertyValueL(handle, propertyName, &lvalue);

                    printf("Property[%d]        Type: Bool\n", x);
                    printf("Property[%d]       Value: %s\n", x, lvalue == 1 ? "True" : "False");
                    break;

                case XType_Base_Blob:
                    // The size of the blob can be retrieved by calling the property with the GetPropertyL function.
                    XC_GetPropertyValueL(handle, propertyName, &lvalue); 

                    // Now that the size is known the data can be downloaded from the camera.
                    cvalue = new char[lvalue];
                    XC_GetPropertyBlob(handle, propertyName, cvalue, lvalue);

                    printf("Property[%d]        Type: Blob\n", x);
                    printf("Property[%d]        Size: %lu\n", x, lvalue);

                    // Output the contents of the blob
                    if (lvalue > 0)
                    {
                        printf("Property[%d]        Value:\n", x);

                        for(int x = 0; x < lvalue; x++)
                        {
                            printf("0x%X ", cvalue[x]);

                            if (x % 8 == 0 && x != 0)
                                printf("\n");
                        }
                        
                        printf("\n");
                    }

                    // clear buffer.
                    delete [] cvalue;
                    cvalue = NULL;
                    break;

                case XType_Base_String:
                    // Get the string.
                    cvalue = new char[128];
                    XC_GetPropertyValue(handle, propertyName, cvalue, 128);

                    printf("Property[%d]        Type: String\n", x);
                    printf("Property[%d]       Value: %s\n", x, cvalue);

                    delete [] cvalue;
                    cvalue = NULL;
                    break;
            }

            printf("\n");         
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
