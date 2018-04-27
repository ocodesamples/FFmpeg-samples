#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}
using namespace std;

int main(int argc, char *argv[])
{

    char inputUrl[] = "/Users/linus/Movies/sv.mp4";
    char outputUrl[] = "/Users/linus/Movies/ssd.wmv";

    //muxer, demuters
    //avcodec_register_all();

    //1.open input file
    AVFormatContext *ic  = NULL;
    avformat_open_input(&ic, inputUrl, NULL, NULL);
    cout << "ss" << ic->filename << endl;
    if(!ic) {
        cout << "avformat_open_input failed!" <<endl;
    }
    cout << " priv_data: " << ic->format_probesize << endl;
    avformat_find_stream_info(ic, NULL);

    //2.create output context
    AVFormatContext * oc = NULL;
    avformat_alloc_output_context2(&oc, NULL,NULL, outputUrl);
    if(!oc)
    {
        cerr << "avformat_alloc_output_context2 failed!"  << endl;
    }

    //3 add the stream
    AVStream * videoStream = avformat_new_stream(oc, NULL);
    AVStream * audioStream = avformat_new_stream(oc, NULL);

    //5. copy para
    avcodec_parameters_copy(videoStream->codecpar, ic->streams[0]->codecpar);
    avcodec_parameters_copy(audioStream->codecpar, ic->streams[1]->codecpar);
    videoStream->codecpar->codec_tag = 0;
    audioStream->codecpar->codec_tag = 0;
 //   av_dump_format(oc, 0, outputUrl, 1);
 //   cout << "==============" << endl;
 //   av_dump_format(ic, 0, inputUrl, 0);

    //5 open out file io, write head
    int ret = avio_open(&oc->pb, outputUrl, AVIO_FLAG_WRITE);
    if(ret < 0) {
        cout << "avio open failed!" << endl;
    }

    ret = avformat_write_header(oc, NULL);
    if(ret < 0) {
        cout << "avformat_write_header failed !" << endl;
    }

    AVPacket pkt;
    for(;;) {
       int re = av_read_frame(ic,&pkt);
       if(re < 0) break;
       pkt.pts = av_rescale_q_rnd(pkt.pts, ic->streams[pkt.stream_index]->time_base, oc->streams[pkt.stream_index]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
       pkt.dts = av_rescale_q_rnd(pkt.dts, ic->streams[pkt.stream_index]->time_base, oc->streams[pkt.stream_index]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
       pkt.pos = -1;
       pkt.duration = av_rescale_q_rnd(pkt.duration, ic->streams[pkt.stream_index]->time_base, oc->streams[pkt.stream_index]->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
       av_write_frame(oc, &pkt);
        av_packet_unref(&pkt);
        cout << ".";
    }

    //写尾部信息
    av_write_trailer(oc);

    avio_close(oc->pb);

    cout << "========end" << endl;
    return 0;
}
