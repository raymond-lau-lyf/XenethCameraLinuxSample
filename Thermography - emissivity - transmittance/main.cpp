#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include "XFilters.h" // Xeneth SDK main header.
#include "math.h"


// Function declarations
dword CorrectReponse( dword aduIn, dword maxValue, double emissivity,             double ambientAdu
                                                 , double atmosphereTransmission, double atmosphereAdu
                                                 , double windowTransmission,     double windowAdu          
                                                 , double windowReflection,       double windowReflectedAdu );

dword GetBlackbodyResponse(double temperature, double* tempLUT, int sizeOfLUT);


int main()
{
    /*
     * The calibration pack.
     * Calibration packs are camera specific. 
     * Copy the calibration pack that comes with the camera onto your system and modify the packname variable accordingly.
     */
    
    const char *packname = "C:\\Program Files\\Xeneth\\Sdk\\Samples\\Thermography - emissivity - transmittance\\pack.xca"; // The calibration pack, change this to the correct path.
    
    // Variables
    
    XCHANDLE handle = 0; // Handle to the camera.
    FilterID fltThermography = 0; // Handle to the thermography filter.
    ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0; // 16-bit buffer to store the capture frame.
    dword frameSize = 0; // The size in bytes of the raw image.
    double *tempLUT = 0; // Temperature lookup table (ADU to temperature)
    dword width = 0, height = 0;

    //Radiometry parameters
    double emissivity                  = 0.95;    //emissivity of the observed object
    double ambientTemperature          = 25.0;    //temperature near the observed object
    double atmosphereTransmission      = 1.0;     //transmission coefficient of the atmosphere between the object and the camera
    double atmosphereTemperature       = 25.0;    //temperature of the atmosphere between the object and the camera
    double windowTransmission          = 1.0;     //transmission coefficient of a window in front of the camera
    double windowTemperature           = 25.0;    //temperature of a window in front of the camera
    double windowReflection            = 0.0;     //reflection coefficient of a window in front of the camera
    double windowReflectedTemperature  = 25.0;    //reflected temperature (onto the camera) via the window in front of the camera

    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to cam://0\n");
    handle = XC_OpenCamera("cam://0");

    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {
        if (I_OK == XC_LoadCalibration(handle, packname, XLC_StartSoftwareCorrection))
        {
            fltThermography = XC_FLT_Queue(handle, "Thermography", "celsius");

            if (fltThermography > 0)
            {
                // Build the look-up table
                dword mv = XC_GetMaxValue(handle);
                tempLUT = new double[mv+1];

                for(dword x = 0; x < mv; x++)
                {
                    XC_FLT_ADUToTemperature(handle, fltThermography, x, &tempLUT[x]);
                }

                //Find the camera response when looking to an object of emissivity 1 and whose temperature equals ambientTemperature.
                int ambientAdu    = GetBlackbodyResponse(ambientTemperature, tempLUT, mv + 1);

                //Find the camera response when looking to an object of emissivity 1 and whose temperature equals atmosphereTemperature.
                int atmosphereAdu = GetBlackbodyResponse(atmosphereTemperature, tempLUT, mv + 1);
                
                //Find the camera response when looking to an object of emissivity 1 and whose temperature equals windowTemperature.
                int windowAdu = GetBlackbodyResponse(windowTemperature, tempLUT, mv + 1);
                
                //Find the camera response when looking to an object of emissivity 1 and whose temperature equals windowReflectedTemperature.
                int windowReflectedAdu = GetBlackbodyResponse(windowReflectedTemperature, tempLUT, mv + 1);
                

                //Verify emissivity.
                if( emissivity < 0.1 ) emissivity = 0.1;
                if( emissivity > 1.0 ) emissivity = 1.0;

                //Verify atmosphereTransmission.
                if( atmosphereTransmission < 0.1 ) atmosphereTransmission = 0.1;
                if( atmosphereTransmission > 1.0 ) atmosphereTransmission = 1.0;
                
                //Verify windowTransmission.
                if( windowTransmission < 0.1 ) windowTransmission = 0.1;
                if( windowTransmission > 1.0 ) windowTransmission = 1.0;
                
                //Verify windowReflection.
                if( windowReflection < 0.0 ) windowReflection = 0.0;
                if( windowReflection > 0.9 ) windowReflection = 0.9;
                
                //Make sure that windowTransmission + windowReflection < 1

                // .. start capturing.
                printf("Start capturing.\n");
                if ((errorCode = XC_StartCapture(handle)) != I_OK)
                {
                    printf("Could not start capturing, errorCode: %u\n", errorCode);
                }
                else if (XC_IsCapturing(handle)) // When the camera is capturing ...
                {
                    frameSize = XC_GetFrameSize(handle);
                    frameBuffer = new word[(frameSize + XC_GetFrameFooterLength(handle)) / 2];
                    width = XC_GetWidth(handle);
                    height = XC_GetHeight(handle);

                    while (true)
                    {
                        if ((errorCode = XC_GetFrame(handle, FT_16_BPP_GRAY, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                        {
                            printf("Problem while fetching frame, errorCode %u", errorCode);
                            break;
                        }
                        else
                        {
                            dword adu = frameBuffer[width/2 + (height / 2 * width)];
                            
                            adu       = CorrectReponse( adu, mv, emissivity, ambientAdu, atmosphereTransmission, atmosphereAdu, windowTransmission, windowAdu, windowReflection, windowReflectedAdu );

                            printf("----\n");
                            printf("Emissivity of the object: %.2f.\n",                 emissivity);
                            printf("Ambient temperature near the object = %.2f. \n\n",  ambientTemperature);

                            printf("Transmittance of the atomosphere: %.2f.\n",         atmosphereTransmission);
                            printf("Temperature of the atmosphere = %.2f. \n\n",        atmosphereTemperature);
                            
                            printf("Transmittance of the window: %.2f.\n",              windowTransmission);
                            printf("Temperature of the window = %.2f. \n\n",            windowTemperature);
                            
                            printf("Reflection on the window: %.2f.\n",                 windowReflection);
                            printf("Reflected temperature on the window = %.2f. \n\n",  windowTemperature);

                            // Use the table to lookup the temperature by passing the digital values as index.
                            printf("Temperature at center: %.1f degrees Celsius.\n\n",   tempLUT[adu]);
                            printf("----\n");
                        }
                    }
                }
            }
            else
            {
                printf("Could not start thermography filter, closing connection.\n");
            }
        }
        else
        {
            printf("Could not load calibration pack '%s', closing connection\n", packname);
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
            printf("Could not stop capturing, errorCode: %u\n", errorCode);
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

    if (tempLUT != 0)
    {
        delete [] tempLUT;
        tempLUT = 0;
    }

    return 0;
}

dword GetBlackbodyResponse(double temperature, double* tempLUT, int sizeOfLUT)
{
    int index = 0; 
    while( (tempLUT[index] < temperature) && (index < sizeOfLUT - 1) )
    {
        index++;
    }
    return index;

}

dword CorrectReponse( dword aduIn, dword maxValue, double emissivity,             double ambientAdu
                                                 , double atmosphereTransmission, double atmosphereAdu
                                                 , double windowTransmission,     double windowAdu          
                                                 , double windowReflection,       double windowReflectedAdu )
{

    /*
    //   observed radiation = radiation from the object + radiation reflected on the object + radiation from the atmosphere + radiation from the window + radiation reflected on the window
    //
    //   radiation from the object            = blackbody radiation at temperature object T       * emissivity         * atmosphereTransmission  * windowTransmission
    //   radiation reflected on the object    = blackbody radiation at ambientTemperature         * ( 1 - emissivity ) * atmosphereTransmission  * windowTransmission
    //   radiation from the atmosphere        = blackbody radiation at atmosphereTemperature      * ( 1 - atmosphereTransmission )               * windowTransmission
    //   radiation from the window            = blackbody radiation at windowTemperature          * ( 1 - windowTransmission - windowReflection )
    //   radiation reflected on the window    = blackbody radiation at windowReflectedTemperature * windowReflection
    */
    
    double aduOut = aduIn;
    //The short version.
    if( atmosphereTransmission == 1.0 &&  windowTransmission == 1.0 && windowReflection == 0.0 ) 
    {
        aduOut -= ambientAdu * ( 1 - emissivity );
        aduOut /= emissivity;
    }
    //The longer version
    else 
    {
        aduOut -= ambientAdu         * ( 1 - emissivity )              * atmosphereTransmission * windowTransmission;
        aduOut -= atmosphereAdu      * ( 1 - atmosphereTransmission )                           * windowTransmission;
        aduOut -= windowAdu          * ( 1 - windowTransmission - windowReflection ); 
        aduOut -= windowReflectedAdu * windowReflection;     
        aduOut /= emissivity * atmosphereTransmission * windowTransmission;

    }
    
    if ( aduOut < 0 )        aduOut = 0;
    if ( aduOut > maxValue)  aduOut = maxValue;

    //Round to the nearest dword
    return (dword)floor( aduOut + 0.5 );
}
