/*
 * @file flv-repeater.c
 * @author sharpbai
 * @date 2016/07/20
 */

#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "flv-repeater.h"

// File-scope ("global") variables
const char *flv_signature = "FLV";

typedef struct RepeaterState {
    FILE *infile;
    FILE *outfile;
    size_t start_index;
    size_t repeat_count;
    size_t repeat_remain_count;
    uint32_t timestamp_offset;
    uint32_t last_timestamp;
    uint8_t file_buffer[FILE_BUF_LENGTH];
    uint32_t prev_tag_size;
    uint32_t sleep_interval;
    uint32_t timeline;
    int read_in_framerate;

} RepeaterState;
RepeaterState _repeaterState;
RepeaterState *rs = &_repeaterState;

/**
 * Returns the current time in microseconds.
 */
int64_t getMillitime(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (currentTime.tv_sec * (int)1e6 + currentTime.tv_usec) / 1000;
}

void die(void) {
    printf("Error!\n");
    exit(-1);
}

void flv_repeater_init(globalArgs_t *globalArgs) {
    rs->timestamp_offset = 0;
    rs->infile = globalArgs->infile;
    rs->outfile = globalArgs->outfile;
    rs->start_index = globalArgs->startIndex;
    rs->repeat_count = globalArgs->repeatCount;
    rs->repeat_remain_count = (rs->repeat_count > 0) ? rs->repeat_count : 1;
    rs->sleep_interval = 10;
    rs->timeline = 0;
    rs->read_in_framerate = globalArgs->readInFramerate;
}

int flv_repeater_run() {
    flv_tag_t *tag;
    // 传递FLV头9个字节
    assert(file_read_write(rs->infile, rs->outfile, FLV_FILE_HEADER_SIZE,
                           rs->file_buffer, FILE_BUF_LENGTH) == FLV_FILE_HEADER_SIZE);
    // 读取首个flv prev_tag_size
    assert(fread_4(rs->infile, &rs->prev_tag_size) == 4);

    int64_t lastReadTime = getMillitime();
    int64_t currentTime = getMillitime();

    while (1) {
        tag = flv_read_tag(0); // read the tag
        if (!tag) {
            // jump to start_index
            assert(fseek(rs->infile, rs->start_index, SEEK_SET) == 0);
            // g_repeat_count为0时无限循环
            if (rs->repeat_count > 0) {
                rs->repeat_remain_count--;
            }
            // 时间戳偏移等于跳转前最后一个时间戳
            rs->timestamp_offset = rs->last_timestamp;
            if (rs->repeat_remain_count <= 0) {
                return 0;
            }
        } else {
            // 时间戳增加偏移
            tag->timestamp += rs->timestamp_offset;
            tag->timestamp_ext = (uint8_t)(tag->timestamp >> 24);
            // 保存最后一个时间戳
            rs->last_timestamp = tag->timestamp;

            // 这里插入时间控制逻辑
            while(rs->read_in_framerate) {
                currentTime = getMillitime();
                rs->timeline += currentTime - lastReadTime;
                lastReadTime = currentTime;
                // 如果时间戳不超过timeline,则继续发送;
                // 否则等待
                //fprintf(stderr, "ts: %u tl: %u\n", rs->timeline, tag->timestamp);
                if(tag->timestamp <= rs->timeline) {
                    break;
                } else {
                    usleep(rs->sleep_interval);
                }
            }
            // 写入上个prev_tag_size
            assert(fwrite_4(rs->outfile, &rs->prev_tag_size) == 4);
            // 保存最后一个TAG长度
            rs->prev_tag_size = tag->data_size + 11;
            // 写入FLV TAG without data
            assert(flv_write_tag(tag) == 11);
            // 写入TAG data
            assert(file_read_write(rs->infile, rs->outfile, tag->data_size,
                                   rs->file_buffer, FILE_BUF_LENGTH) == tag->data_size);
            uint32_t read_prev_tag_size;
            fread_4(rs->infile, &read_prev_tag_size);
            // 读取大于0才能算作有效长度
            if (read_prev_tag_size > 0) {
                rs->prev_tag_size = read_prev_tag_size;
            }
            flv_free_tag(tag); // and free it
        }
    }
}

void flv_free_tag(flv_tag_t *tag) {
    free(tag->data);
    free(tag);
}

void print_general_tag_info(flv_tag_t *tag) {
    assert(NULL != tag);
    printf("  Data size: %lu\n", (unsigned long) tag->data_size);
    printf("  Timestamp: %lu\n", (unsigned long) tag->timestamp);
    printf("  Timestamp extended: %u\n", tag->timestamp_ext);
    printf("  StreamID: %lu\n", (unsigned long) tag->stream_id);
    return;
}

flv_tag_t *flv_read_tag(int with_data) {

    size_t count = 0;
    flv_tag_t *tag = NULL;

    tag = malloc(sizeof(flv_tag_t));

    if (feof(rs->infile)) {
        return NULL;
    }
    // Start reading next tag
    count += fread_1(rs->infile, &(tag->tag_type));
    count += fread_3(rs->infile, &(tag->data_size));
    count += fread_3(rs->infile, &(tag->timestamp));
    count += fread_1(rs->infile, &(tag->timestamp_ext));
    count += fread_3(rs->infile, &(tag->stream_id));

    tag->timestamp |= tag->timestamp_ext << 24;

    // printf("\n");

    if (with_data) {
        tag->data = malloc((size_t) tag->data_size);
        count += fread(tag->data, 1, (size_t) tag->data_size, rs->infile);
        if (count != tag->data_size + 11) {
            return NULL;
        }
    } else {
        tag->data = NULL;
        if (count != 11) {
            return NULL;
        }
    }

    // printf("Tag type: %u - ", tag->tag_type);
    // print_general_tag_info(tag);
    // printf("data size: %d\n", tag->data_size);

    // Did we reach end of file?
    if (feof(rs->infile)) {
        return NULL;
    }

    return tag;
}

size_t flv_write_tag(flv_tag_t *tag) {
    size_t count = 0;
    count += fwrite_1(rs->outfile, &tag->tag_type);
    count += fwrite_3(rs->outfile, &tag->data_size);
    count += fwrite_3(rs->outfile, &tag->timestamp);
    // 强制转换成uint8_t *，即是访问高8位
    count += fwrite_1(rs->outfile, &tag->timestamp_ext);
    count += fwrite_3(rs->outfile, &tag->stream_id);
    if (tag->data) {
        count += fwrite(tag->data, tag->data_size, 1, rs->outfile) * tag->data_size;
    }
    return count;
}
