// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
 #include <stdbool.h>
// #include <pthread.h>

#include "sample_comm.h"

#define RTSP_QUEUE_NUM_BUF 128
#define RTSP_PUSHER_BUF_LEN 2000000 

// 数据缓冲区结构体
typedef struct {
    char *data;
    int len;
    int capacity;
} DataBuf;

// 循环缓冲区结构体
typedef struct {
    DataBuf *buf;
    int size;
    int capacity;
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
    bool quit_flag;
} CircularBuffer;

// FFmpeg 推送器结构体
typedef struct {
    char *output_url;
    bool memory_copy_allowed;
    char *packet_buf;
    int buf_offset;
    CircularBuffer buf_queue;
    pthread_t pusher_thread;
    bool running;
} FFmpegPusher;

// void circular_buffer_init(CircularBuffer *cb, int capacity); // 初始化循环缓冲区

// void circular_buffer_destroy(CircularBuffer *cb); // 销毁循环缓冲区

// bool circular_buffer_put(CircularBuffer *cb, DataBuf data); // 向循环缓冲区写入数据

// bool circular_buffer_pop(CircularBuffer *cb, DataBuf *data);// 从循环缓冲区读取数据

// void circular_buffer_quit(CircularBuffer *cb); // 退出循环缓冲区

// static void *run_pushstream(void *arg); // 推送线程函数

FFmpegPusher *ffmpeg_pusher_create(const char *stream_url, bool allow_copy);// 创建 FFmpeg 推送器


void ffmpeg_pusher_destroy(FFmpegPusher *pusher);// 销毁 FFmpeg 推送器


bool ffmpeg_pusher_push(FFmpegPusher *pusher, char *data, int size, int flag);// 推送数据

// int ffmpeg_pusher_get_buf_queue_len(FFmpegPusher *pusher);// 获取缓冲区队列长度


// 使用方法：
// 创建 FFmpegPusher 对象：
// FFmpegPusher *pusher = ffmpeg_pusher_create("rtsp://example.com/live/stream", true);

// 推送数据：
// ffmpeg_pusher_push(pusher, data, size, flag);


// 销毁 FFmpegPusher 对象：
// ffmpeg_pusher_destroy(pusher);