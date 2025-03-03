cmake_minimum_required(VERSION 3.10)
project (RTSPSERVER)
set(CMAKE_CXX_STANDARD 11)
include_directories( ./ ./rtp ./3rdparty)
aux_source_directory(./ SRC)
aux_source_directory(./rtp RTP)
aux_source_directory(./3rdparty 3RD)


set(EXECUTABLE_OUTPUT_PATH ./)
add_compile_options(-g -std=c++11)
set(CMAKE_BUILD_TYPE Release)

add_executable(rtsp_client ${SRC} ${RTP} ${3RD})
if(WIN32)
    # Set the function stack size to 6MB
    add_compile_options(-Wl,--stack,6000000)
    target_link_libraries(rtsp_client ws2_32)
elseif(UNIX)
    target_link_libraries(rtsp_client pthread)
endif()

