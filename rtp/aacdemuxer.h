#ifndef AAC_DEMUXER_
#define AAC_DEMUXER_
#include <iostream>
#include "rtpdemuxer.h"

class AACDemuxer : public RTPDemuxer{
public:
    void InputData(const uint8_t* data, size_t size);
private:
};
#endif