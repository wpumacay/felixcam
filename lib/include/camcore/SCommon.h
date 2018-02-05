
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

        void copyFrom( const SImageRGB &src )
        {
            if ( this->data != NULL )
            {
                free( this->data );
            }

            this->width = src.width;
            this->height = src.height;
            this->size = src.size;
            this->data = ( u8* ) malloc( this->size * sizeof( u8 ) );

            memcpy( this->data, src.data, sizeof( u8 ) * this->size );
        }
    };

    struct SCameraProps
    {
        int fWidth;
        int fHeight;

    };


}