#include <iostream>
#include "ffinclude.h"
#include <string>
#include <chrono>
#include <memory>

using namespace std;

AVFormatContext * inputContext = nullptr;
AVFormatContext * outputContext= nullptr;
int64_t  lastReadPacktTime ;


static int interrupt_cb(void * ctx) {
    int timeout = 3;
    if(av_gettime() - lastReadPacktTime > timeout * 1000 * 1000) {
        return -1;
    }
    return 0;
}




static int openInput(string inputUrl) {
    inputContext = avformat_alloc_context();
    lastReadPacktTime = av_gettime();
    inputContext->interrupt_callback.callback = interrupt_cb;
    int ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, nullptr);

    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Input file open input failed \n");
        return ret;
    }

    ret = avformat_find_stream_info(inputContext, nullptr);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Find input file stream inform failed\n");
    }
    else {
        av_log(NULL, AV_LOG_FATAL, "Open input file  %s success \n", inputUrl.c_str());
    }


    return ret;
}

int openOutput(string outUrl) {

    int ret = avformat_alloc_output_context2(&outputContext, nullptr, nullptr, outUrl.c_str());
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "open out put context failed");
        goto Error;
    }

    ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);

    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "open avio failed");
        goto Error;
    }

    for(int i = 0; i < inputContext->nb_streams; i++) {
        AVStream * stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
        ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "copy coddec context failed");
            goto Error;
        }
    }

    ret = avformat_write_header(outputContext, nullptr);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "format write header filed");
        goto Error;
    }

    av_log(NULL, AV_LOG_FATAL, "Open out put file seccess  %s  \n", outUrl.c_str());
    return ret;

Error:
    if(outputContext) {
        for(int i = 0; i < outputContext->nb_streams; i++) {
            avcodec_close(outputContext->streams[i]->codec);
        }
        avformat_close_input(&outputContext);
    }

}


shared_ptr<AVPacket> readPacketFromSource() {
    shared_ptr<AVPacket> packet(static_cast<AVPacket *>(av_malloc(sizeof(AVPacket))), [&](AVPacket * p) {av_packet_free(&p); av_freep(&p);});
    av_init_packet(packet.get());
    lastReadPacktTime = av_gettime();
    int ret = av_read_frame(inputContext, packet.get());
    if(ret >= 0) {
        return packet;
    } else {
        return nullptr;
    }
}

void av_packet_rescale_tss(AVPacket *pkt, AVRational src_tb, AVRational dst_tb) {

    if(pkt->pts != AV_NOPTS_VALUE) {
        pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
    }
    if(pkt->dts != AV_NOPTS_VALUE) {
        pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
    }
    if(pkt->duration > 0) {
        pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
    }
}

int writePacket(shared_ptr<AVPacket> packet) {
    auto inputStream = inputContext->streams[packet->stream_index];
    auto outputStreate = outputContext->streams[packet->stream_index];
    av_packet_rescale_tss(packet.get(), inputStream->time_base, outputStreate->time_base);
    return av_interleaved_write_frame(outputContext, packet.get());
}

void closeInput() {
    if(inputContext != nullptr) {
        avformat_close_input(&inputContext);
    }
}

void closeOutPut() {
    if(outputContext != nullptr) {
        for(int i = 0; i < outputContext->nb_streams; i++) {
            AVCodecContext * codecContext = outputContext->streams[i]->codec;
            avcodec_close(codecContext);
        }
        avformat_close_input(&inputContext);
    }
}

void FFMPEG_Callback(void * ptr, int level, const char * fmt, va_list vl) {
    char  buffer[1024];
    vsprintf(buffer, fmt, vl);
    if(level == AV_LOG_ERROR) {
        cout << "leve:" << level << "  : " << buffer;
    }
}

void init() {
    av_log_set_level(AV_LOG_VERBOSE);
    av_log_set_callback(&FFMPEG_Callback);
}

int main() {
    init();
    int ret = openInput("/home/a4004923/Develop/Code/Github/FFmpeg-sample/RtspSource-SaveFile/zjl.mp4");
    if(ret >= 0) {
        ret = openOutput("test.ts");
    }

    if(ret < 0) goto Error;

    while(true) {
        auto packet = readPacketFromSource();
        if(packet){
            auto packet = readPacketFromSource();
            if(packet) {
                ret = writePacket(packet);
                if(ret >= 0) {
                   // cout << "WritePacket Success!" << endl;
                } else {
                   // cout << "WritePacket failed" << endl;
                }
            } else {
                break;
            }
        }
    }

    av_write_trailer(outputContext);
    avio_close(outputContext->pb);



Error:
    closeInput();
    closeOutPut();
//    while(true) {
//        std::this_thread::sleep_for(chrono::seconds(100));
//    }
    return 0;
}