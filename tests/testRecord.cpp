
#include <camcore/SLinuxCamHandler.h>
#include <ext/jo_mpeg.h>

#include <iostream>
#include <fstream>

#define WIDTH 640
#define HEIGHT 480

using namespace std;

#define FPS 50
#define TIME_MS 20
#define NUM_FRAMES 100

#include <chrono>
#include <thread>

cam::SImageRGBX _frames[ NUM_FRAMES ];


int main()
{

    auto _camHandler = new cam::handler::SLinuxCamHandler( "/dev/video0",
                                                           WIDTH, HEIGHT );

    _camHandler->openDevice();

    // cout << "**************************************" << endl;
    // cout << "Previous props: " << endl;
    // _camHandler->dumpCurrentProperties();
    // cout << "**************************************" << endl;
    // _camHandler->deviceSetProperty( CAMPROP_AUTO_FOCUS, 0 );
    // _camHandler->deviceSetProperty( CAMPROP_FOCUS_ABSOLUTE, 10 );
    // _camHandler->deviceSetProperty( CAMPROP_BACKLIGHT_COMPENSATION, 1 );
    // _camHandler->deviceSetProperty( CAMPROP_BRIGHTNESS, 0 );
    // _camHandler->deviceSetProperty( CAMPROP_CONTRAST, 10 );
    // cout << "New props: " << endl;
    // _camHandler->dumpCurrentProperties();
    // cout << "**************************************" << endl;

    _camHandler->startCapture();


    for ( int q = 0; q < NUM_FRAMES; q++ )
    {
        cam::SImageRGB _frame = _camHandler->takeFrame();

        cam::SImageRGBX _sFrame( _frame );

        _frames[q] = _sFrame;

        cout << "saved frame: " << q << endl;

        // jo_write_mpeg( _fp, _sFrame.data, _sFrame.width, _sFrame.height, FPS );

        std::this_thread::sleep_for( std::chrono::milliseconds( TIME_MS ) );

        // free( _sFrame.data );
        free( _frame.data );
    }

    FILE* _fp = fopen( "testCamRecord.mpg", "wb" );

    for ( int q = 0; q < NUM_FRAMES; q++ )
    {
        jo_write_mpeg( _fp, _frames[q].data, _frames[q].width, _frames[q].height, FPS );
    }

    fclose( _fp );

    for ( int q = 0; q < NUM_FRAMES; q++ )
    {
        free( _frames[q].data );
    }

    _camHandler->stopCapture();
    _camHandler->closeDevice();

    delete _camHandler;

    return 0;
}