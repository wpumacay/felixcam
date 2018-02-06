

message( "CONFIGURING felixcam LIBRARY" )

include_directories( include/ )

add_library( felixcam 
             src/camcore/SLinuxCamHandler.cpp
             src/ext/jo_mpeg.cpp )
target_link_libraries( felixcam v4l2 )
