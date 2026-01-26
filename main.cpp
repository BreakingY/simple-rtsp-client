#include <iostream>
#include <thread>
#include <chrono>
#include "rtsp_client.h"
static void AdtsHeader(char *adts_header_buffer, int data_len, int profile, int sample_rate_index, int channels){
    int adts_len = data_len + 7;

    adts_header_buffer[0] = 0xff;         //syncword:0xfff                          high 8bits
    adts_header_buffer[1] = 0xf0;         //syncword:0xfff                          low 4bits
    adts_header_buffer[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    adts_header_buffer[1] |= (0 << 1);    //Layer:0                                 2bits
    adts_header_buffer[1] |= 1;           //protection absent:1                     1bit

    adts_header_buffer[2] = (profile)<<6;            //profile:audio_object_type - 1                      2bits
    adts_header_buffer[2] |= (sample_rate_index & 0x0f)<<2; //sampling frequency index:sampling_frequency_index  4bits
    adts_header_buffer[2] |= (0 << 1);                             //private bit:0                                      1bit
    adts_header_buffer[2] |= (channels & 0x04)>>2;           //channel configuration:channel_config               高1bit

    adts_header_buffer[3] = (channels & 0x03)<<6;     //channel configuration:channel_config      low 2bits
    adts_header_buffer[3] |= (0 << 5);                      //original：0                               1bit
    adts_header_buffer[3] |= (0 << 4);                      //home：0                                   1bit
    adts_header_buffer[3] |= (0 << 3);                      //copyright id bit：0                       1bit
    adts_header_buffer[3] |= (0 << 2);                      //copyright id start：0                     1bit
    adts_header_buffer[3] |= ((adts_len & 0x1800) >> 11);           //frame length：value   high 2bits

    adts_header_buffer[4] = (uint8_t)((adts_len & 0x7f8) >> 3);     //frame length:value    middle 8bits
    adts_header_buffer[5] = (uint8_t)((adts_len & 0x7) << 5);       //frame length:value    low 3bits
    adts_header_buffer[5] |= 0x1f;                                 //buffer fullness:0x7ff high 5bits
    adts_header_buffer[6] = 0xfc;
    return;
}
class RtspClientProxyDemo:public RtspMediaInterface{
public:
    RtspClientProxyDemo(char *rtsp_url){
        rtsp_url_ = rtsp_url;
        client_ =  new RtspClient(transport_); 
        client_->Connect(rtsp_url); 
        client_->SetCallBack(this);
        tid_=std::thread(ReconnectThread,this);
    }
    ~RtspClientProxyDemo(){
        run_flag_ = false;
        tid_.join();
        delete client_;
        if(h26x_fd_){
            fclose(h26x_fd_);
        }
        if(aac_fd_){
            fclose(aac_fd_);
        }
        if(pcma_fd_){
            fclose(pcma_fd_);
        }
        std::cout << "~RtspClientProxyDemo" << std::endl;
    }
    static void *ReconnectThread(void *arg){
        RtspClientProxyDemo *self = (RtspClientProxyDemo*)arg;
        while(self->run_flag_){
           bool stat = self->client_->GetOpenStat();
           if(stat == false){
                std::cout << self->rtsp_url_ << " Reconnect" << std::endl;
                delete self->client_;
                self->client_ =  new RtspClient(self->transport_); 
                self->client_->Connect(self->rtsp_url_.c_str()); 
                self->client_->SetCallBack(self);

           }
           std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return NULL;
    }
    void RtspVideoData(int64_t pts, const uint8_t* data, size_t size){
        if(h26x_fd_ == NULL){
            h26x_fd_ = fopen(h26x_filename_, "wb");
        }
        fwrite(data, 1, size, h26x_fd_);
        if(client_->GetVideoType() == MediaEnum::H264){
            // std::cout << "video type:" << (data[4] & 0x1f) << std::endl;
        }
        else if(client_->GetVideoType() == MediaEnum::H265){
            // std::cout << "video type:" << ((data[4] >> 1) & 0x3f) << std::endl;
        }
        return;
    }
    void RtspAudioData(int64_t pts,  const uint8_t* data, size_t size){
        if(client_->GetAudioType() == MediaEnum::AAC){
            if(aac_fd_ == NULL){
                aac_fd_ = fopen(aac_filename_, "wb");
            }

            char adts_header_buf[7] = {0};
            int profile, sample_rate_index, channels;
            client_->GetAudioInfo(sample_rate_index, channels, profile);
            AdtsHeader(adts_header_buf, size,
                        profile,
                        sample_rate_index,
                        channels);
            fwrite(adts_header_buf, 1, 7, aac_fd_);
            fwrite(data, 1, size, aac_fd_);
        }
        else if(client_->GetAudioType() == MediaEnum::PCMA){
            if(pcma_fd_ == NULL){
                pcma_fd_ = fopen(pcma_filename_, "wb");
            }
            fwrite(data, 1, size, pcma_fd_);
        }
        return;
    }
private:
    std::string rtsp_url_;
    enum TRANSPORT transport_ = TRANSPORT::RTP_OVER_TCP;
    RtspClient *client_ = NULL;
    const char *aac_filename_ = "test_out.aac"; 
    FILE *aac_fd_ = NULL;
    const char *pcma_filename_ = "test_out.pcma";
    FILE *pcma_fd_ = NULL;


    const char *h26x_filename_ = "test_out.h26x"; 
    FILE *h26x_fd_ = NULL;

    std::thread tid_;
    bool run_flag_ = true;
};
int main(int argc, char **argv){
    if(argc < 2){
        printf("./rtsp_client rtsp_url\n");
        return -1;
    }
    RtspClientProxyDemo *rtsp_client_proxy_demo = new RtspClientProxyDemo(argv[1]);
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 10));
        // break;
    }
    delete rtsp_client_proxy_demo;
}