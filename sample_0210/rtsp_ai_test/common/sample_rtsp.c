#include "sample_rtsp.h"

// 初始化循环缓冲区
static void circular_buffer_init(CircularBuffer *cb, int capacity) {
    cb->buf = (DataBuf *)malloc(capacity * sizeof(DataBuf));
    cb->size = 0;
    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->cond_full, NULL);
    pthread_cond_init(&cb->cond_empty, NULL);
    cb->quit_flag = false;
}

// 销毁循环缓冲区
static void circular_buffer_destroy(CircularBuffer *cb) {
    free(cb->buf);
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->cond_full);
    pthread_cond_destroy(&cb->cond_empty);
}

// 向循环缓冲区写入数据
static bool circular_buffer_put(CircularBuffer *cb, DataBuf data) {
    pthread_mutex_lock(&cb->mutex);

    while (cb->size == cb->capacity && !cb->quit_flag) {
        pthread_cond_wait(&cb->cond_full, &cb->mutex);
    }

    if (cb->quit_flag) {
        pthread_mutex_unlock(&cb->mutex);
        return false; // 退出
    }

    cb->buf[cb->tail] = data;
    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->size++;

    pthread_cond_signal(&cb->cond_empty);
    pthread_mutex_unlock(&cb->mutex);
    return true;
}

// 从循环缓冲区读取数据
static bool circular_buffer_pop(CircularBuffer *cb, DataBuf *data) {
    pthread_mutex_lock(&cb->mutex);

    while (cb->size == 0 && !cb->quit_flag) {
        pthread_cond_wait(&cb->cond_empty, &cb->mutex);
    }

    if (cb->quit_flag && cb->size == 0) {
        pthread_mutex_unlock(&cb->mutex);
        return false; // 退出
    }

    *data = cb->buf[cb->head];
    cb->head = (cb->head + 1) % cb->capacity;
    cb->size--;

    pthread_cond_signal(&cb->cond_full);
    pthread_mutex_unlock(&cb->mutex);
    return true;
}

// 退出循环缓冲区
static void circular_buffer_quit(CircularBuffer *cb) {
    pthread_mutex_lock(&cb->mutex);
    cb->quit_flag = true;
    pthread_cond_broadcast(&cb->cond_full);
    pthread_cond_broadcast(&cb->cond_empty);
    pthread_mutex_unlock(&cb->mutex);
}

// 推送线程函数
static void *run_pushstream(void *arg) {
    FFmpegPusher *pusher = (FFmpegPusher *)arg;

    char command[512];
    snprintf(command, sizeof(command), "ffmpeg -re -y -i - -vcodec copy -an -f %s %s", 
             (strncmp(pusher->output_url, "rtsp", 4) == 0) ? "rtsp" : 
             (strncmp(pusher->output_url, "rtp", 3) == 0) ? "rtp_mpegts" : 
             (strncmp(pusher->output_url, "rtmp", 4) == 0) ? "flv" : "rtsp", 
             pusher->output_url);

    printf("FFmpeg cmd: %s\n", command);

    FILE *fp = popen(command, "w");
    if (fp == NULL) {
        fprintf(stderr, "Open stream: %s failed!\n", pusher->output_url);
        pusher->running = false;
        return NULL;
    }

    DataBuf buf;
    while (pusher->running) {
        if (circular_buffer_pop(&pusher->buf_queue, &buf)) {
            if (buf.len > 0) {
                fwrite(buf.data, 1, buf.len, fp);
            } else {
                break; // 数据读取完毕
            }
        }
    }

    pclose(fp);
    return NULL;
}

// 创建 FFmpeg 推送器
FFmpegPusher *ffmpeg_pusher_create(const char *stream_url, bool allow_copy) {
    FFmpegPusher *pusher = (FFmpegPusher *)malloc(sizeof(FFmpegPusher));
    if (pusher == NULL) {
        return NULL;
    }

    pusher->output_url = strdup(stream_url);
    pusher->memory_copy_allowed = allow_copy;
    if (pusher->memory_copy_allowed) {
        pusher->packet_buf = (char *)malloc(RTSP_PUSHER_BUF_LEN);
        if (pusher->packet_buf == NULL) {
            free(pusher->output_url);
            free(pusher);
            return NULL;
        }
    } else {
        pusher->packet_buf = NULL;
    }
    pusher->buf_offset = 0;
    circular_buffer_init(&pusher->buf_queue, RTSP_QUEUE_NUM_BUF);
    pusher->running = true;

    pthread_create(&pusher->pusher_thread, NULL, run_pushstream, pusher);

    return pusher;
}

// 销毁 FFmpeg 推送器
void ffmpeg_pusher_destroy(FFmpegPusher *pusher) {
    if (pusher == NULL) {
        return;
    }

    pusher->running = false;
    circular_buffer_quit(&pusher->buf_queue);
    pthread_join(pusher->pusher_thread, NULL);

    free(pusher->output_url);
    if (pusher->memory_copy_allowed) {
        free(pusher->packet_buf);
    }
    circular_buffer_destroy(&pusher->buf_queue);
    free(pusher);
}

// 推送数据
bool ffmpeg_pusher_push(FFmpegPusher *pusher, char *data, int size, int flag) {
    if (size <= 0 || pusher == NULL) {
        return false;
    }

    DataBuf buf;
    if (pusher->memory_copy_allowed) {
        if (pusher->buf_offset + size >= RTSP_PUSHER_BUF_LEN) {
            pusher->buf_offset = 0;
        }
        memcpy(pusher->packet_buf + pusher->buf_offset, data, size);
        buf.data = pusher->packet_buf + pusher->buf_offset;
        pusher->buf_offset += size;
    } else {
        buf.data = data;
    }
    buf.len = size;
    buf.capacity = size;

    return circular_buffer_put(&pusher->buf_queue, buf);
}

int ffmpeg_pusher_get_buf_queue_len(FFmpegPusher *pusher) {
    if (pusher == NULL) {
        return -1;
    }
    return pusher->buf_queue.size;
}