

message( "CONFIGURING TEST SAMPLES" )

include_directories( ../lib/include/ )

add_executable( testlib testlib.cpp )
target_link_libraries( testlib felixcam )