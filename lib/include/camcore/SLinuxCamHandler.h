
/**
* Based on https://github.com/severin-lemaignan/webcam-v4l2 wrapper
*/

#pragma once

#include <camcore/SCommon.h>
#include <camcore/SProperties.h>


using namespace std;


namespace cam { namespace handler {



    class SLinuxCamHandler
    {

        private :

        string m_deviceId;

        int m_fWidth;
        int m_fHeight;
        int m_fStride;

        int m_fHandle;

        SBuffer* m_buffers;// array of buffers
        int m_nBuffers;

        SImageRGB m_rgbFrame;

        void _deviceMemoryMapping();
        bool _deviceReadFrame();
        void _deviceInit();

        void _dumpSingleProperty( cam::u32 propertyId, std::string propName );

        public :

        SLinuxCamHandler( const string& device,
                          int fWidth, int fHeight );
        ~SLinuxCamHandler();

        void openDevice();
        void closeDevice();
        
        void startCapture();
        void stopCapture();

        bool deviceSetProperty( cam::u32 propertyId, cam::i32 propertyValue );
        bool deviceSetStreamingProperty( cam::u32 propertyId, cam::i32 propertyValue );
        
        void dumpCurrentProperties();

        SImageRGB takeFrame( int timeout = 1 );

    };


}}