# simple-rtsp-client
* RTSP1.0
* H264/H265/AAC/PCMA(G711A)
* Support rtp over udp、rtp over tcp, support authentication
* MD5：https://github.com/talent518/md5

# Compile
1. linux
   * mkdir build
   * cd build
   * cmake ..
   * make -j
2. Windows(MinGW + cmake)
   * mkdir build
   * cd build
   * cmake -G "MinGW Makefiles" ..
   * mingw32-make

# Test
* ./rtsp_client rtsp_url
* H264/H265 is written to test_out.h26x, AAC is written to test_out.aac, PCMA is written to test_out.pcma

# Email
* kxsun617@163.com
