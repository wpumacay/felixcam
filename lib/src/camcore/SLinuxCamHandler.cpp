
#include <camcore/SLinuxCamHandler.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cassert>
#include <cerrno>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
// #include <libv4l2.h>

#include <iostream>

using namespace std;

namespace cam { namespace handler {

    /****************************************************************
     * Taken from https://github.com/severin-lemaignan/webcam-v4l2
     */
    static int xioctl( int fh, unsigned long int request, void *arg )
    {
          int r;

          do {
                r = ioctl( fh, request, arg );
          } while ( -1 == r && EINTR == errno );

          return r;
    }

    /****************************************************************
     * Taken from libv4l2 (in v4l-utils)
     *
     * (C) 2008 Hans de Goede <hdegoede@redhat.com>
     *
     * Released under LGPL
     */
    #define CLIP( color ) ( unsigned char ) ( ( ( color ) > 0xFF ) ? 0xff : ( ( ( color ) < 0 ) ? 0 : ( color ) ) )

    static void v4lconvert_yuyv_to_rgb24( const unsigned char *src, 
                                          unsigned char *dest,
                                          int width, int height, 
                                          int stride )
    {
        int j;

        while ( --height >= 0 ) 
        {
            for ( j = 0; j + 1 < width; j += 2 ) 
            {
                int u = src[1];
                int v = src[3];
                int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
                int rg = (((u - 128) << 1) +  (u - 128) +
                        ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
                int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

                *dest++ = CLIP( src[0] + v1 );
                *dest++ = CLIP( src[0] - rg );
                *dest++ = CLIP( src[0] + u1 );

                *dest++ = CLIP( src[2] + v1 );
                *dest++ = CLIP( src[2] - rg );
                *dest++ = CLIP( src[2] + u1 );
                src += 4;
            }

            src += stride - (width * 2);
        }
    }
    /*******************************************************************/

    SLinuxCamHandler::SLinuxCamHandler( const string& device,
                                        int fWidth, int fHeight )
    {
        m_fWidth = fWidth;
        m_fHeight = fHeight;
        m_deviceId = device;

        m_fHandle = -1;
        m_buffers = NULL;
        m_nBuffers = 0;
    }


    SLinuxCamHandler::~SLinuxCamHandler()
    {
        if ( m_fHandle != -1 )
        {
            cout << "Don't forget to close the device" << endl;
            closeDevice();
        }

        m_fHandle = -1;
        m_fWidth = -1;
        m_fHeight = -1;

        m_deviceId = "None";
    }

    void SLinuxCamHandler::openDevice()
    {
        LOG_PROP( "Opening device", m_deviceId );

        struct stat _st;

        if ( -1 == stat( m_deviceId.c_str(), &_st ) )
        {
            cout << "SLinuxCamHandler::open> could not identify the requested device: " 
                 << m_deviceId << " - "
                 << to_string( errno ) << " : " << strerror( errno ) << endl;
            exit( -1 );
        }

        if ( !S_ISCHR( _st.st_mode ) )
        {
            cout << "SLinuxCamHandler::open> not a valid device: " << m_deviceId << endl;
            exit( -1 );
        }

        // Should O_NONBLOCK ??
        m_fHandle = open( m_deviceId.c_str(), O_RDWR | O_NONBLOCK, 0 );

        if ( m_fHandle == -1 )
        {
            cout << "SLinuxCamHandler::open> could not open " << m_deviceId
                 << " - " << to_string( errno ) << " - " << strerror( errno ) << endl;
            exit( -1 );
        }

        LOG_PROP( "Successfully opened device", m_deviceId );

        _deviceInit();
    }


    void SLinuxCamHandler::_deviceInit()
    {
        assert( m_fHandle != -1 );// Should have openend first

        struct v4l2_capability  _camCap;
        struct v4l2_cropcap     _camCropCap;
        struct v4l2_crop        _camCrop;
        struct v4l2_format      _camFormat;

        if ( xioctl( m_fHandle, VIDIOC_QUERYCAP, &_camCap ) == -1 ) 
        {
            if ( EINVAL == errno ) 
            {
                cout << "SLinuxCamHandler::initDevice> device: " << m_deviceId 
                     << " is not a valid v4l2 device" << endl;

                closeDevice();
                exit( -1 );
            } 
            else 
            {
                cout << "SLinuxCamHandler::initDevice> VIDIOC_QUERYCAP" << endl;
                closeDevice();
                exit( -1 );
            }
        }

        if ( !( _camCap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) ) 
        {
            cout << "SLinuxCamHandler::initDevice> device " << m_deviceId 
                 << "does not have video capture capabilities" << endl;
            closeDevice();
            exit( -1 );
        }

        if ( !( _camCap.capabilities & V4L2_CAP_STREAMING ) ) 
        {
            cout << "SLinuxCamHandler::initDevice> device " << m_deviceId 
                 << "does not support streaming io" << endl;
            closeDevice();
            exit( -1 );
        }

        /* Select video input, video standard and tune here. */

        CLEAR( _camCropCap );

        _camCropCap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if ( xioctl( m_fHandle, VIDIOC_CROPCAP, &_camCropCap ) == 0 ) 
        {
            _camCrop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            _camCrop.c = _camCropCap.defrect; /* reset to default */

            if ( xioctl( m_fHandle, VIDIOC_S_CROP, &_camCrop ) == -1 ) 
            {
                switch ( errno ) 
                {
                    case EINVAL:
                        cout << "SLinuxCamHandler::initDevice> warning, cropping not supported" << endl;
                        break;
                    default:
                        /* Errors ignored. */
                        break;
                }
            }
        } 
        else 
        {
            /* Errors ignored. */
        }


        CLEAR( _camFormat );

        _camFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        _camFormat.fmt.pix.width       = m_fWidth;
        _camFormat.fmt.pix.height      = m_fHeight;
        _camFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        _camFormat.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if ( xioctl( m_fHandle, VIDIOC_S_FMT, &_camFormat ) == -1 )
        {
            cout << "SLinuxCamHandler::initDevice> VIDIOC_S_FMT error" << endl;
            closeDevice();
            exit( -1 );
        }

        if (_camFormat.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV )
        {
            // note that libv4l2 (look for 'v4l-utils') provides helpers
            // to manage conversions
            cout << "SLinuxCamHandler::initDevice> Webcam does not support YUYV format ( only format supported for now )" << endl;
            closeDevice();
            exit( -1 );
        }

        /* Note VIDIOC_S_FMT may change width and height. */
        m_fWidth  = _camFormat.fmt.pix.width;
        m_fHeight = _camFormat.fmt.pix.height;
        m_fStride = _camFormat.fmt.pix.bytesperline;

        LOG_PROP( "Successfully initialized device", m_deviceId );

        _deviceMemoryMapping();

        m_rgbFrame.width  = m_fWidth;
        m_rgbFrame.height = m_fHeight;
        m_rgbFrame.size   = m_fWidth * m_fHeight * 3;
        m_rgbFrame.data   = ( cam::u8* ) malloc( m_rgbFrame.size * sizeof( cam::u8 ) );
    }

    void SLinuxCamHandler::_deviceMemoryMapping()
    {
        cout << "Initializing memory mapping" << endl;

        assert( m_fHandle != -1 ); // should have openend the device

        struct v4l2_requestbuffers _camReq;

        CLEAR( _camReq );

        _camReq.count   = 4; // YUYV ?
        _camReq.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        _camReq.memory  = V4L2_MEMORY_MMAP;

        if ( xioctl( m_fHandle, VIDIOC_REQBUFS, &_camReq ) ) 
        {
            if ( EINVAL == errno ) 
            {
                cout << "SLinuxCamHandler::deviceMemoryMapping> " << m_deviceId 
                     << " does not support memory mapping " << endl;
                closeDevice();
                exit( -1 );
            } 
            else 
            {
                cout << "SLinuxCamHandler::deviceMemoryMapping> VIDIOC_REQBUFS error" << endl;
                closeDevice();
                exit( -1 );
            }
        }

        if ( _camReq.count < 2 ) 
        {
            cout << "SLinuxCamHandler::deviceMemoryMapping> insufficient memory on " << m_deviceId << endl;
            closeDevice();
            exit( -1 );
        }

        m_nBuffers = _camReq.count;

        // use vector instead?
        m_buffers = ( SBuffer* ) calloc( m_nBuffers, sizeof( SBuffer ) ); // Warn??

        if ( m_buffers == NULL ) 
        {
            cout << "SLinuxCamHandler::deviceMemoryMapping> out of memory " << endl;
            closeDevice();
            exit( -1 );
        }

        for ( int q = 0; q < m_nBuffers; q++ ) 
        {
            struct v4l2_buffer _camBuff;

            CLEAR( _camBuff );

            _camBuff.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            _camBuff.memory = V4L2_MEMORY_MMAP;
            _camBuff.index  = q;

            if ( xioctl( m_fHandle, VIDIOC_QUERYBUF, &_camBuff ) == -1 )
            {
                cout << "SLinuxCamHandler::deviceMemoryMapping> VIDIOC_QUERYBUF error " << endl;
                closeDevice();
                exit( -1 );
            }

            m_buffers[q].size = _camBuff.length;
            m_buffers[q].data = mmap( NULL /* start anywhere */,
                                      _camBuff.length,
                                      PROT_READ | PROT_WRITE /* required */,
                                      MAP_SHARED /* recommended */,
                                      m_fHandle, _camBuff.m.offset );

            if ( MAP_FAILED == m_buffers[q].data )
            {
                cout << "SLinuxCamHandler::deviceMemoryMapping> mmap error " << endl;
            }
        }

        cout << "Successfully made memory mapping to device buffers" << endl;
    }

    void SLinuxCamHandler::startCapture()
    {
        assert( m_fHandle != -1 );

        enum v4l2_buf_type _type;

        for ( int q = 0; q < m_nBuffers; q++ )
        {
            struct v4l2_buffer _camBuff;

            CLEAR( _camBuff );
            _camBuff.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            _camBuff.memory = V4L2_MEMORY_MMAP;
            _camBuff.index  = q;

            if ( xioctl( m_fHandle, VIDIOC_QBUF, &_camBuff ) )
            {
                cout << "SLinuxCamHandler::startCapture> VIDIOC_QBUF error" << endl;
                closeDevice();
                exit( -1 );
            }
        }

        _type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if ( xioctl( m_fHandle, VIDIOC_STREAMON, &_type ) == -1 )
        {
                cout << "SLinuxCamHandler::startCapture> VIDIOC_STREAMON error" << endl;
                closeDevice();
                exit( -1 );
        }

        cout << "capture started" << endl;
    }

    void SLinuxCamHandler::stopCapture()
    {
        assert( m_fHandle != -1 );

        enum v4l2_buf_type _camType;

        _camType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if ( xioctl( m_fHandle, VIDIOC_STREAMOFF, &_camType ) )
        {
            cout << "SLinuxCamHandler::stopCapture> VIDIOC_STREAMOFF when trying to stop capture" << endl;
            closeDevice();
            exit( -1 );
        }

        cout << "Capture stopped" << endl;
    }

    void SLinuxCamHandler::closeDevice()
    {
        if ( close( m_fHandle ) == -1 )
        {
            cout << "SLinuxCamHandler::close> error when closing device: " 
                 << m_deviceId << " - with fHandle: " << m_fHandle << endl;
            exit( -1 );
        }

        LOG_PROP( "Closed device", m_deviceId );

        m_fHandle = -1;
    }

    bool SLinuxCamHandler::deviceSetProperty( cam::u32 propertyId, cam::i32 propertyValue )
    {
        assert( m_fHandle != -1 );

        // First, query for the control ( if it exists or not )
        struct v4l2_queryctrl _camQueryCtrl;

        CLEAR( _camQueryCtrl );
        _camQueryCtrl.id = propertyId;

        if ( v4l2_ioctl( m_fHandle, VIDIOC_QUERYCTRL, &_camQueryCtrl ) == -1 )
        {
            if ( errno != EINVAL )
            {
                cout << "Warning: couldn't query this control:  " << propertyId << endl;
                return false;
            }
            else
            {
                cout << "Warning: control " << propertyId << " is not supported" << endl;
                return false;
            }
        }
        else if ( _camQueryCtrl.flags & V4L2_CTRL_FLAG_DISABLED )
        {
            cout << "Warning: control " << propertyId << " is not supported" << endl;
            return false;
        }

        // If control is supported, then set its value property
        struct v4l2_control _camControl;

        CLEAR( _camControl );
        _camControl.id = propertyId;
        _camControl.value = propertyValue;

        if ( v4l2_ioctl( m_fHandle, VIDIOC_S_CTRL, &_camControl ) == -1 )
        {
            cout << "Warning: error while setting property: " 
                 << cam::PROPERTIES_MAP[ propertyId ] << " with value: " << propertyValue 
                 << " giving error: " << strerror( errno ) << endl;

            return false;
        }

        return true;
    }

    void SLinuxCamHandler::dumpCurrentProperties()
    {
        assert( m_fHandle != -1 );

        for ( auto _it = cam::PROPERTIES_MAP.begin(); _it != cam::PROPERTIES_MAP.end(); ++_it )
        {
            _dumpSingleProperty( _it->first, _it->second );
        }
    }

    void SLinuxCamHandler::_dumpSingleProperty( cam::u32 propertyId, string propName )
    {
        struct v4l2_control _camControl;
        _camControl.id = propertyId;

        if ( xioctl( m_fHandle, VIDIOC_G_CTRL, &_camControl ) == -1 )
        {
            cout << "Warning: error while reading property: " << propName << endl;
            return;
        }

        cout << "prop( " << propName << " ): " << _camControl.value << endl;
    }

    SImageRGB SLinuxCamHandler::takeFrame( int timeout )
    {
        SImageRGB _res;

        for ( ;; )  
        {
            fd_set _fds;
            struct timeval _camTimeVal;

            FD_ZERO( &_fds );
            FD_SET( m_fHandle, &_fds );

            /* Timeout. */
            _camTimeVal.tv_sec = timeout;
            _camTimeVal.tv_usec = 0;

            int r = select( m_fHandle + 1, &_fds, NULL, NULL, &_camTimeVal );

            if ( r == -1 ) 
            {
                if ( EINTR == errno )
                {
                    continue;
                }

                cout << "SLinuxCamHandler::takeFrame> select error " << endl;
                closeDevice();
                exit( -1 );
            }

            if ( r == 0 ) 
            {
                cout << "SLinuxCamHandler::takeFrame> select timeout error " << endl;
                closeDevice();
                exit( -1 );
            }

            if ( _deviceReadFrame() ) 
            {
                // just return the frame, the copy constructors will do the job
                return m_rgbFrame;
            }
            /* EAGAIN - continue select loop. */
        }

        return _res;
    }

    bool SLinuxCamHandler::_deviceReadFrame()
    {
        assert( m_fHandle != -1 );

        struct v4l2_buffer _camBuff;

        CLEAR( _camBuff );

        _camBuff.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        _camBuff.memory = V4L2_MEMORY_MMAP;

        if ( xioctl( m_fHandle, VIDIOC_DQBUF, &_camBuff ) == -1 )
        {
            switch( errno )
            {
                case EAGAIN :
                    return false;

                case EIO :

                default :
                    cout << "SLinuxCamHandler::_deviceReadFrame> VIDIOC_DQBUF error" << endl;
                    closeDevice();
                    exit( -1 );
            }
        }

        assert( _camBuff.index < m_nBuffers );

        v4lconvert_yuyv_to_rgb24( ( cam::u8 * ) m_buffers[ _camBuff.index ].data,
                                  m_rgbFrame.data,
                                  m_fWidth,
                                  m_fHeight,
                                  m_fStride );

        if ( xioctl( m_fHandle, VIDIOC_QBUF, &_camBuff ) == -1 )
        {
            cout << "SLinuxCamHandler::_deviceReadFrame> VIDIOC_QBUF error" << endl;
            closeDevice();
            exit( -1 );
        }

        return true;
    }

}}