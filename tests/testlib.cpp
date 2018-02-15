
#include <camcore/SLinuxCamHandler.h>

#include <iostream>
#include <fstream>

#define WIDTH 640
#define HEIGHT 480

using namespace std;

int main()
{

    auto _camHandler = new cam::handler::SLinuxCamHandler( "/dev/video0",
                                                           WIDTH, HEIGHT );

    _camHandler->openDevice();

    cout << "**************************************" << endl;
    cout << "Previous props: " << endl;
    _camHandler->dumpCurrentProperties();
    cout << "**************************************" << endl;
    _camHandler->deviceSetProperty( CAMPROP_AUTO_FOCUS, 0 );
    _camHandler->deviceSetProperty( CAMPROP_FOCUS_ABSOLUTE, 10 );
    _camHandler->deviceSetProperty( CAMPROP_BACKLIGHT_COMPENSATION, 1 );
    _camHandler->deviceSetProperty( CAMPROP_BRIGHTNESS, 0 );
    _camHandler->deviceSetProperty( CAMPROP_CONTRAST, 10 );
    cout << "New props: " << endl;
    _camHandler->dumpCurrentProperties();
    cout << "**************************************" << endl;

    _camHandler->startCapture();

    // auto _frame = _camHandler->takeFrame();
    cam::SImageRGB _frame;
    _camHandler->takeFrame( _frame );

    ofstream _image;
    _image.open( "frame.ppm" );
    _image << "P6\n" << WIDTH << " " << HEIGHT << " 255\n";
    _image.write( ( char *) _frame.data, _frame.size );
    _image.close();    

    _camHandler->stopCapture();
    _camHandler->closeDevice();

    delete _camHandler;

    return 0;
}