
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

            std::cout << "constructor" << std::endl;
        }

        SImageRGB( const SImageRGB& other )
        {
            this->width = other.width;
            this->height = other.height;
            this->size = other.size;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );
        }

        SImageRGB& operator= ( const SImageRGB& other )
        {
            // std::cout << "wtf - operator=" << std::endl;
            release();

            this->width = other.width;
            this->height = other.height;
            this->size = other.size;
            this->data = new cam::u8[ this->size ];

            memcpy( this->data, other.data, sizeof( cam::u8 ) * this->size );

            return *this;
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
            // std::cout << "trying to release" << std::endl;
            if ( this->data != NULL )
            {
                delete[] this->data;
                this->data = NULL;
                // std::cout << "release" << std::endl;
            }
        }

        cam::u8 get( int row, int col, int channel ) const
        {
            // if ( row > this->height - 1 || col > this->width - 1 )
            // {
            //     return 0;
            // }
            return this->data[ 3 * ( col + row * this->width ) + channel ];
        }
    };
}