#ifndef PCMA_DEMUXER_
#define PCMA_DEMUXER_
#include <iostream>
#include "rtpdemuxer.h"

class PCMADemuxer : public RTPDemuxer{
public:
    void InputData(const uint8_t* data, size_t size);
private:
};
#endif