// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
//  #include <stdbool.h>
// // #include <pthread.h>

// #include "sample_comm.h"

// #define RTSP_QUEUE_NUM_BUF 128
// #define RTSP_PUSHER_BUF_LEN 2000000 

// // 数据缓冲区结构体
// typedef struct {
//     char *data;
//     int len;
//     int capacity;
// } DataBuf;

// // 循环缓冲区结构体
// typedef struct {
//     DataBuf *buf;
//     int size;
//     int capacity;
//     int head;
//     int tail;
//     pthread_mutex_t mutex;
//     pthread_cond_t cond_full;
//     pthread_cond_t cond_empty;
//     bool quit_flag;
// } CircularBuffer;

// // FFmpeg 推送器结构体
// typedef struct {
//     char *output_url;
//     bool memory_copy_allowed;
//     char *packet_buf;
//     int buf_offset;
//     CircularBuffer buf_queue;
//     pthread_t pusher_thread;
//     bool running;
// } FFmpegPusher;

// // void circular_buffer_init(CircularBuffer *cb, int capacity); // 初始化循环缓冲区

// // void circular_buffer_destroy(CircularBuffer *cb); // 销毁循环缓冲区

// // bool circular_buffer_put(CircularBuffer *cb, DataBuf data); // 向循环缓冲区写入数据

// // bool circular_buffer_pop(CircularBuffer *cb, DataBuf *data);// 从循环缓冲区读取数据

// // void circular_buffer_quit(CircularBuffer *cb); // 退出循环缓冲区

// // static void *run_pushstream(void *arg); // 推送线程函数

// FFmpegPusher *ffmpeg_pusher_create(const char *stream_url, bool allow_copy);// 创建 FFmpeg 推送器


// void ffmpeg_pusher_destroy(FFmpegPusher *pusher);// 销毁 FFmpeg 推送器


// bool ffmpeg_pusher_push(FFmpegPusher *pusher, char *data, int size, int flag);// 推送数据

// // int ffmpeg_pusher_get_buf_queue_len(FFmpegPusher *pusher);// 获取缓冲区队列长度


// // 使用方法：
// // 创建 FFmpegPusher 对象：
// // FFmpegPusher *pusher = ffmpeg_pusher_create("rtsp://example.com/live/stream", true);

// // 推送数据：
// // ffmpeg_pusher_push(pusher, data, size, flag);


// // 销毁 FFmpegPusher 对象：
// // ffmpeg_pusher_destroy(pusher);






#ifndef RTSP_SERVER_H
#define RTSP_SERVER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <limits.h>
#include "sample_comm.h"

// #region rtsp.h 相关
#define __PACKED__

#define CC_test

typedef enum
{
    RTSP_VIDEO = 0,
    RTSP_VIDEOSUB = 1,
    RTSP_AUDIO = 2,
    RTSP_YUV422 = 3,
    RTSP_RGB = 4,
    RTSP_VIDEOPS = 5,
    RTSP_VIDEOSUBPS = 6
} enRTSP_MonBlockType;

struct _RTP_FIXED_HEADER
{
    /**/                              /* byte 0 */
    unsigned char csrc_len : 4; /**/  /* expect 0 */
    unsigned char extension : 1; /**/ /* expect 1, see RTP_OP below */
    unsigned char padding : 1; /**/   /* expect 0 */
    unsigned char version : 2; /**/   /* expect 2 */
    /**/                              /* byte 1 */
    unsigned char payload : 7; /**/   /* RTP_PAYLOAD_RTSP */
    unsigned char marker : 1; /**/    /* expect 1 */
    /**/                              /* bytes 2, 3 */
    unsigned short seq_no;
    /**/ /* bytes 4-7 */
    unsigned int timestamp;
    /**/                     /* bytes 8-11 */
    unsigned int ssrc; /**/ /* stream number is used here. */
} __PACKED__;
typedef struct _RTP_FIXED_HEADER RTP_FIXED_HEADER;


struct _H265_NALU_HEADER
{
    // byte 0
    unsigned char LayerId_h : 1;
    unsigned char TYPE : 6;
    unsigned char F : 1;

    // byte 1
    unsigned char TID : 3;
    unsigned char LayerId_l : 5;

} __PACKED__; /**/ /* 2 BYTES */
typedef struct _H265_NALU_HEADER H265_NALU_HEADER;

struct _H265_FU_INDICATOR
{

    // byte 0
    unsigned char LayerId_h : 1;
    unsigned char TYPE : 6;
    unsigned char F : 1;

    // byte 1
    unsigned char TID : 3;
    unsigned char LayerId_l : 5;

} __PACKED__; /**/ /* 2 BYTES */
typedef struct _H265_FU_INDICATOR H265_FU_INDICATOR;

struct _H265_FU_HEADER
{
    // byte 0
    unsigned char FuTYPE : 6;
    unsigned char E : 1;
    unsigned char S : 1;
} __PACKED__; /**/ /* 1 BYTES */
typedef struct _H265_FU_HEADER H265_FU_HEADER;





void InitRtspServer();// 初始化 RTSP 服务器
hi_s32 sample_comm_rtsp_sentjin(ot_venc_stream *pstStream,ot_venc_stream_buf_info *stream_buf_info);// RTSP 服务器推流







#endif

// #endregion


