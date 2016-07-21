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

size_t fread_1(uint8_t *ptr);
size_t fread_3(uint32_t *ptr);
size_t fread_4(uint32_t *ptr);
size_t fread_4s();
size_t fwrite_1(uint8_t *ptr);
size_t fwrite_3(uint32_t *ptr);
size_t fwrite_4(uint32_t *ptr);

size_t file_read_write(size_t count);

flv_tag_t *flv_read_tag(int with_data);
size_t flv_write_tag(flv_tag_t *tag);
void flv_free_tag(flv_tag_t *tag);
void flv_repeater_init(FILE *in_file, FILE *out_file, size_t start_index,
                       size_t repeat_count);
int flv_repeater_run(void);

#endif // FLV_REPEATER_H_
