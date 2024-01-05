 /*
  * This sample applies to the Tigris camera.
  * It shows how to exploit the on-board radiometric data to compute the temperature of observed objects.
  * The two available thermography modes are illustrated. 
  * In the temperature mode, temperatures are calculated via linear interpolation.
  * In the radiometric mode, temperatures can be obtained as a result of applying a dedicated analytic function.
  * The sample assumes that a radiometric calibration is loaded in the selected image correction source on board of the camera. 
  * Note that this sample does not apply to the Serval camera.
  */


#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.
#include "XFilters.h" // Xeneth SDK main header.
#include "math.h"

// Function declarations
double TemperatureInTemperatureMode(word adu, word maximumAdu, double minimalTemperature, double maximalTemperature);
double TemperatureInRadianceMode(word adu, double S1, double S2, double S3, double S4, double emissivity, double ambientTemperature);
double TempToRad(double temperatureK, double S1, double S2, double S3, double S4);
double TempFromRad(double radiance, double S1, double S2, double S3, double S4);

int main()
{
    // Variables
    XCHANDLE handle = 0;            // Handle to the camera.
    ErrCode errorCode = 0;          // Used to store returned errorCodes from the SDK functions.
    word *frameBuffer = 0;          // 16-bit buffer to store the capture frame.
    dword frameSize = 0;            // The size in bytes of the raw image.
    word maximalAdu = 0;
    dword width = 0, height = 0;

    //Radiometry parameters
    double emissivity                  = 0.95;    //emissivity of the observed object
    double ambientTemperature          = 300;     //temperature near the observed object



    // Open a connection to the first detected camera by using connection string cam://0
    printf("Opening connection to cam://0\n");
    handle = XC_OpenCamera("cam://0");


    // When the connection is initialised, ...
    if(XC_IsInitialised(handle))
    {
        frameSize = XC_GetFrameSize(handle);
        frameBuffer = new word[frameSize];
        maximalAdu = (word)XC_GetMaxValue(handle);
        width = XC_GetWidth(handle);
        height = XC_GetHeight(handle);


        if ((errorCode = XC_SetPropertyValueL(handle, "ThermographyEnable", 1, "")) != I_OK)
        {
            printf("Could not activate a radiomatric calibration, errorCode: %u\n", errorCode);
        }
        else
        {
            //Verify emissivity.
            if( emissivity < 0.1 ) emissivity = 0.1;
            if( emissivity > 1.0 ) emissivity = 1.0;

            //Temperature conversion in temperature mode.
            if( XC_SetPropertyValueE(handle, "ThermographyMode","Temperature") == I_OK )   //Temperature mode
            {
                printf("\nTemperature mode.\n");

                XC_SetPropertyValueF(handle, "ThermographyTemperatureEmissivity", emissivity, "");
                XC_SetPropertyValueF(handle, "ThermographyTemperatureAmbientTemperature", ambientTemperature, "");

                double minimalTemperature;
                XC_GetPropertyValueF(handle, "ThermographyTemperatureSelectionMinimalTemperature", &minimalTemperature);

                double maximalTemperature;
                XC_GetPropertyValueF(handle, "ThermographyTemperatureSelectionMaximalTemperature", &maximalTemperature);

                // .. start capturing.
                printf("Start capturing.\n");
                if ((errorCode = XC_StartCapture(handle)) != I_OK)
                {
                    printf("Could not start capturing, errorCode: %u\n", errorCode);
                }
                else if (XC_IsCapturing(handle)) // When the camera is capturing ...
                {
                    for(size_t i = 0; i < 100; ++i)
                    {
                       if ((errorCode = XC_GetFrame(handle, FT_16_BPP_GRAY, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                       {
                            printf("Problem while fetching frame, errorCode %u", errorCode);
                            break;
                        }
                        else
                        {
                            word adu = frameBuffer[width/2 + (height / 2 * width)];
                            double temperature = TemperatureInTemperatureMode(adu, maximalAdu, minimalTemperature, maximalTemperature);
                            printf("Temperature at center: %.1f Kelvin.\n\n",   temperature);
                        }
                    }
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
                }
            }


            //Temperature conversion in radiance mode.
            if( XC_SetPropertyValueE(handle, "ThermographyMode", "Radiometric") == I_OK)  //Radiance mode
            {
                printf("\nRadiance mode.\n");

                double minimalTemperature;
                XC_GetPropertyValueF(handle, "ThermographyTemperatureMinimalTemperature", &minimalTemperature);

                double maximalTemperature;
                XC_GetPropertyValueF(handle, "ThermographyTemperatureMaximalTemperature", &maximalTemperature);
            
            
                double S1, S2, S3, S4;
                XC_GetPropertyValueF(handle, "ThermographyRadiometryS1", &S1);
                XC_GetPropertyValueF(handle, "ThermographyRadiometryS2", &S2);
                XC_GetPropertyValueF(handle, "ThermographyRadiometryS3", &S3);
                XC_GetPropertyValueF(handle, "ThermographyRadiometryS4", &S4);

                // .. start capturing.
                printf("Start capturing.\n");
                if ((errorCode = XC_StartCapture(handle)) != I_OK)
                {
                    printf("Could not start capturing, errorCode: %u\n", errorCode);
                }
                else if (XC_IsCapturing(handle)) // When the camera is capturing ...
                {
                   for(size_t i = 0; i < 100; ++i)
                   {
                       if ((errorCode = XC_GetFrame(handle, FT_16_BPP_GRAY, XGF_Blocking, frameBuffer, frameSize)) != I_OK)
                       {
                            printf("Problem while fetching frame, errorCode %u", errorCode);
                            break;
                        }
                        else
                        {
                            word adu = frameBuffer[width/2 + (height / 2 * width)];
                            double temperature = TemperatureInRadianceMode(adu, S1, S2, S3, S4, emissivity, ambientTemperature);
                            if( temperature < minimalTemperature || temperature > maximalTemperature )
                            {
                                printf("Temperature at center: %.1f Kelvin. This is out of the calibration range.\n\n",   temperature);
                            }
                            else
                            {
                                printf("Temperature at center: %.1f Kelvin.\n\n",   temperature);
                            }
                        }
                    }
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
                }
            }
        }
    }
    else
    {
        printf("Initialization failed\n");
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



double TemperatureInTemperatureMode(word adu, word maximalAdu, double minimalTemperature, double maximalTemperature)
{
    //The temperature is computed using linear interpolation between the Selection minimal and the Selection maximal temperature.
    double fraction = (double)adu / maximalAdu;
    return minimalTemperature + fraction * (maximalTemperature - minimalTemperature);
}

double TemperatureInRadianceMode(word adu, double S1, double S2, double S3, double S4, double emissivity, double ambientTemperature)
{
    //The temperature is computer using the function Rad_BB = S1 + S2 * (exp(S3/(Temperature + S4)) - 1)^(-1)
    double rad = adu; 
    
    //If the object is not a perfect blackbody we take into account the emissivity and the ambient temperature
    //according to the following rule: Rad_object = emissivity * Rad_BB + (1 - emissivity) * Rad_ambient.
    if(emissivity < 1.0)
    {
        double ambientAdu = TempToRad(ambientTemperature, S1, S2, S3, S4);
        rad -= (1 - emissivity) * ambientAdu;
        rad /= emissivity;
    }

    return TempFromRad(rad, S1, S2, S3, S4);
}

double TempToRad(double temperature, double S1, double S2, double S3, double S4)
{
    //Rad_BB = S1 + S2 * (exp(S3/(Temperature + S4)) - 1)^(-1)
    double ret = exp(S3/(temperature + S4)) - 1;
    ret        = S1 + (S2 / ret);

    return ret;
}

double TempFromRad(double radiance, double S1, double S2, double S3, double S4)
{
    double ret = radiance - S1;
    ret        = S2 / ret;
    ret       += 1;
    ret        = log(ret);
    ret        = S3 / ret;
    ret       -= S4;

    return ret;
}