#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H
#include <QApplication>
#include <stdio.h>
#include <QDebug>
#include <fstream>
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
}

#define DATASIZE 800*1280

using namespace std;


class VideoStream
{
public:
    VideoStream();
    ~VideoStream();
    bool avWriterInit(const char* out_filename);
    void avWtiter(char* buffer);
    bool avWriterRelease();

private:
    AVStream *addVideoStream(AVFormatContext *oc, enum AVCodecID codec_id);

private:
    AVFormatContext *ofmt_ctx;
    AVPacket pkt;
    int frameCount;
    unsigned char *mydata;

};

#endif // VIDEOSTREAM_H
