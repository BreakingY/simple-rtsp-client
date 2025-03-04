#include <iostream>
#include <thread>
#include <chrono>
#include "rtsp_client.h"
class RtspClientProxy:public RtspMediaInterface{
public:
    RtspClientProxy(char *rtsp_url){
        rtsp_url_ = rtsp_url;
        client_ =  new RtspClient(transport_); 
        client_->Connect(rtsp_url); 
        client_->SetCallBack(this);
        tid_=std::thread(ReconnectThread,this);
    }
    ~RtspClientProxy(){
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
        std::cout << "~RtspClientProxy" << std::endl;
    }
    static void *ReconnectThread(void *arg){
        RtspClientProxy *self = (RtspClientProxy*)arg;
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
    RtspClientProxy *rtsp_client_proxy = new RtspClientProxy(argv[1]);
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 10));
        // break;
    }
    delete rtsp_client_proxy;
}