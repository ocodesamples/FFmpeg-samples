#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}
using namespace std;

int main()
{
    char  * infile = "/Users/linus/Movies/susheview2_640x480_rgb24.rgb";
    char * outFile = "/Users/linus/Movies/susheview2_640x480_rgb24.mp4";
    av_register_all();
    avcodec_register_all();
    cout << "config" <<avformat_configuration() << endl;
    FILE *fp = fopen(infile, "rb");
    if(!fp) {
        cout << infile << "open fiailed" << endl;
    }

    int width = 640;
    int height = 480;
    int fps = 25;

    //1 create codec
    AVCodec * codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec) {
        cout <<  "av_find_encoder AV_CODEC_ID_H264 fiailed!" << endl;
    }

   AVCodecContext *c = avcodec_alloc_context3(codec);
   if(!c) {
    cout << "avcodec_alloc_context3 failed " << endl;
   }
   //压缩比特率
   c->bit_rate = 400000000;

   c->width = width;
   c->height = height;
   c->time_base = {1, fps};
   c->framerate = {fps, 1};
   //画面组大小
   c->gop_size = 50;
   c->max_b_frames = 0;
   c->pix_fmt = AV_PIX_FMT_YUV420P;
   c->codec_id = AV_CODEC_ID_H264;
   //全局的编码信息
   c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

   int ret = avcodec_open2(c, codec, NULL);
   if(ret < 0) {
     cout << "avcodec_open2 failed !" << endl;
   }
    cout << "Hello World!" << endl;
    return 0;
}
