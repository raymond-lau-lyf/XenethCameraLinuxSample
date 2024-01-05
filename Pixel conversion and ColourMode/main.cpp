#include "stdio.h" // C Standard Input/Output library.
#include "XCamera.h" // Xeneth SDK main header.

int main()
{
   // Variables
   XCHANDLE handle = 0; // Handle to the camera
   ErrCode errorCode = 0; // Used to store returned errorCodes from the SDK functions.
   dword *frameBuffer = 0; // 32-bit buffer to store the capture frame.
   dword frameSize = 0; // The width of the camera's image.

   // Open a connection to the first detected camera by using connection string cam://0
   printf("Opening connection to cam://0\n");
   handle = XC_OpenCamera("cam://0");

   // When the connection is initialised, ...
   if(XC_IsInitialised(handle))
   {      
      // ... start capturing
      printf("Start capturing.\n");
      if ((errorCode = XC_StartCapture(handle)) != I_OK)
      {
         printf("Could not start capturing, errorCode: %lu\n", errorCode);
      }
      else if (XC_IsCapturing(handle)) // When the camera is capturing ...
      {
         // Load the color profile delivered with this sample.
         if ((errorCode = XC_LoadColourProfile(handle, "colorprofile.png")) != I_OK)
         {
            printf("Problem while loading the desired colorprofile, errorCode: %lu", errorCode);
         }

         // Set the colourmode so that the last loaded colorprofile is used.
         XC_SetColourMode(handle, ColourMode_Profile);

         //Select which part of the dynamical range needs to be mapped on the image.
         //XMsgSetTROIParms msg;
         //msg.pctlo = 0.;  // between 0 and 1
         //msg.pcthi = 1.0;  // between 0 and 1
         //XC_MsgImageFilter(handle, -1, XMsgSetTROI, &msg);

         // Determine framesize for a 32-bit buffer.
         frameSize = XC_GetWidth(handle) * XC_GetHeight(handle);

         // Initialize the 32-bit buffer.
         frameBuffer = new dword[frameSize];

         // ... grab a frame from the camera - FT_32_BPP_BGRA
         printf("Grabbing a frame - FT_32_BPP_BGRA.\n");
         if ((errorCode = XC_GetFrame(handle, FT_32_BPP_BGRA, XGF_Blocking, frameBuffer, frameSize * 4 /* bytes per pixel */)) != I_OK)
         {
            printf("Problem while fetching frame, errorCode %lu\n", errorCode);
         }
         else
         {
             dword pixel = frameBuffer[0];
             byte r = (pixel >> 0)  & 0xff;
             byte g = (pixel >> 8)  & 0xff;
             byte b = (pixel >> 16) & 0xff;
             byte a = (pixel >> 24) & 0xff;

             printf("Pixel value: r = %u, g = %u, b = %u\n", r, g, b);
         }
         
         // ... grab a frame from the camera - FT_32_BPP_RGBA
         printf("Grabbing a frame - FT_32_BPP_RGBA.\n");
         if ((errorCode = XC_GetFrame(handle, FT_32_BPP_RGBA, XGF_Blocking, frameBuffer, frameSize * 4 /* bytes per pixel */)) != I_OK)
         {
            printf("Problem while fetching frame, errorCode %lu\n", errorCode);
         }
         else
         {
             dword pixel = frameBuffer[0];
             byte b = (pixel >> 0)  & 0xff;
             byte g = (pixel >> 8)  & 0xff;
             byte r = (pixel >> 16) & 0xff;
             byte a = (pixel >> 24) & 0xff;

             printf("Pixel value: r = %u, g = %u, b = %u\n", r, g, b);
         }
      }
   }
   else
   {
      printf("Initialization failed\n");
   }

   // Cleanup.

   // When the camera is still capturing ...
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
      // .. close the connection.
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
