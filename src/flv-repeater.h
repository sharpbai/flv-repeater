/*
 * @file flv-repeater.h
 * @author sharpbai
 * @date 2016/07/20
 */

#ifndef FLV_REPEATER_H_
#define FLV_REPEATER_H_ (1)
#define FILE_BUF_LENGTH 4096
#define FLV_FILE_HEADER_SIZE 9

#include <stdint.h>
#include <stdio.h>

/*
 * @brief flv tag general header 11 bytes
 */
typedef struct flv_tag {
  uint8_t tag_type;
  uint32_t data_size;
  uint32_t timestamp;
  uint8_t timestamp_ext;
  uint32_t stream_id;
  void *data; // will point to an audio_tag or video_tag
} flv_tag_t;

typedef struct globalArgs_t {
    FILE *infile;
    FILE *outfile;
    int verbosity;     /* -v option */
    size_t startIndex; /* -s option */
    size_t repeatCount;    /* -r option */
    int readInFramerate; /* -re option */
    size_t timestampBase; /* -f option */
} globalArgs_t;

size_t fread_1(FILE* fs, uint8_t *ptr);
size_t fread_3(FILE* fs, uint32_t *ptr);
size_t fread_4(FILE* fs, uint32_t *ptr);
size_t fread_4s(FILE* fs);
size_t fwrite_1(FILE* fs, uint8_t *ptr);
size_t fwrite_3(FILE* fs, uint32_t *ptr);
size_t fwrite_4(FILE* fs, uint32_t *ptr);

size_t file_read_write(FILE* fs_read, FILE* fs_write, size_t count, uint8_t* buffer, size_t buffer_length);

flv_tag_t *flv_read_tag(int with_data);
size_t flv_write_tag(flv_tag_t *tag);
void flv_free_tag(flv_tag_t *tag);
void flv_repeater_init(globalArgs_t* globalArgs);
int flv_repeater_run(void);

#endif // FLV_REPEATER_H_
