
#pragma once


#include <string>
#include <cstring>
#include <iostream>


#define LOG_PROP( msg, prop ) std::cout << msg << ": " << prop << std::endl


#define CLEAR( x ) memset( &( x ), 0, sizeof( x ) )

namespace cam
{

    typedef unsigned char u8;
    typedef unsigned int u32;
    typedef int i32;

    struct SBuffer
    {
        void* data;
        size_t size;
    };


    struct SImageRGB
    {
        cam::u8* data;
        int width;
        int height;
        size_t size;

        SImageRGB()
        {
            this->data = NULL;
            this->width = 0;
            this->height = 0;
            this->size = 0;
        }

        SImageRGB( int width, int height )
        {
            this->width = width;
            this->height = height;
            this->size = width * height * 3;
            this->data = new cam::u8[ this->size ];
        }

        SImageRGB( const SImageRGB& other )
        {
            this->width = other.width;
            this->height = other.height;
            this->size = other.size;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );
        }

        void operator= ( const SImageRGB& other )
        {
            release();

            this->width = other.width;
            this->height = other.height;
            this->size = other.size;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );
        }

        ~SImageRGB()
        {
            release();

            this->width = 0;
            this->height = 0;
            this->size = 0;
        }

        void release()
        {
            if ( this->data != NULL )
            {
                delete[] this->data;
                this->data = NULL;
            }
        }
    };

    // TODO: Check if this struct is necessary. Only used in the video recording ...
    // attempt using a library from github

    struct SImageRGBX
    {
        cam::u8* data;
        int width;
        int height;
        size_t size;

        SImageRGBX()
        {
            this->data = NULL;
            this->width = 0;
            this->height = 0;
            this->size = 0;
        }

        SImageRGBX( const SImageRGBX& other )
        {
            this->width = other.width;
            this->height = other.height;
            this->size = this->width * this->height * 4;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );
        }

        SImageRGBX( const SImageRGB& imgRGB )
        {
            this->width = imgRGB.width;
            this->height = imgRGB.height;
            this->size = this->width * this->height * 4;
            this->data = new cam::u8[ this->size ];

            for ( int x = 0; x < this->width; x++ )
            {
                for ( int y = 0; y < this->height; y++ )
                {
                    int _pixIndx = x + y * this->width;
                    this->data[ 4 * _pixIndx + 0 ] = imgRGB.data[ 3 * _pixIndx + 0 ];
                    this->data[ 4 * _pixIndx + 1 ] = imgRGB.data[ 3 * _pixIndx + 1 ];
                    this->data[ 4 * _pixIndx + 2 ] = imgRGB.data[ 3 * _pixIndx + 2 ];
                    this->data[ 4 * _pixIndx + 3 ] = 255;
                }
            }
        }

        void operator= ( const SImageRGBX& other )
        {
            release();

            this->width = other.width;
            this->height = other.height;
            this->size = this->width * this->height * 4;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );
        }

        ~SImageRGBX()
        {
            release();

            this->width = 0;
            this->height = 0;
            this->size = 0;
        }

        void release()
        {
            if ( this->data != NULL )
            {
                delete[] this->data;
                this->data = NULL;
            }
        }
    };

}