
#pragma once

#include <linux/videodev2.h>
#include <libv4l2.h>

#include <unordered_map>
#include <string>


#define CAMPROP_BACKLIGHT_COMPENSATION      V4L2_CID_BACKLIGHT_COMPENSATION
#define CAMPROP_BRIGHTNESS                  V4L2_CID_BRIGHTNESS
#define CAMPROP_CONTRAST                    V4L2_CID_CONTRAST
#define CAMPROP_SATURATION                  V4L2_CID_SATURATION
#define CAMPROP_FOCUS_ABSOLUTE              V4L2_CID_FOCUS_ABSOLUTE
#define CAMPROP_EXPOSURE_ABSOLUTE           V4L2_CID_EXPOSURE_ABSOLUTE
#define CAMPROP_WHITE_BALANCE_TEMPERATURE   V4L2_CID_WHITE_BALANCE_TEMPERATURE
#define CAMPROP_AUTO_BALANCE_WHITE          V4L2_CID_AUTO_WHITE_BALANCE
#define CAMPROP_AUTO_FOCUS                  V4L2_CID_FOCUS_AUTO
// #define CAMPROP_AUTO_HUE                    V4L2_CID_HUE_AUTO
#define CAMPROP_AUTO_EXPOSURE               V4L2_CID_EXPOSURE_AUTO

namespace cam
{

    typedef std::unordered_map< int, std::string > SPropertiesMap;

    static SPropertiesMap PROPERTIES_MAP( { { CAMPROP_AUTO_FOCUS, "Focus Auto" }, 
                                           { CAMPROP_BRIGHTNESS, "Brightness" },
                                           { CAMPROP_CONTRAST, "Contrast" },
                                           { CAMPROP_SATURATION, "Saturation" },
                                           { CAMPROP_FOCUS_ABSOLUTE, "Focus Absolute" },
                                           { CAMPROP_WHITE_BALANCE_TEMPERATURE, "White balance temperature" },
                                           { CAMPROP_AUTO_BALANCE_WHITE, "White balance auto" },
                                           { CAMPROP_BACKLIGHT_COMPENSATION, "Backlight compensation" },
                                           { CAMPROP_EXPOSURE_ABSOLUTE, "Exposure Absolute" },
                                           // { CAMPROP_AUTO_HUE, "Hue Auto" },
                                           { CAMPROP_AUTO_EXPOSURE, "Exposure Auto" } } );

}