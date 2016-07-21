/*
 * @file flv-repeater.c
 * @author sharpbai
 * @date 2016/07/20
 */

#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "flv-repeater.h"

// File-scope ("global") variables
const char *flv_signature = "FLV";

static FILE *g_infile;
static FILE *g_outfile;
static size_t g_start_index;
static size_t g_repeat_count;
static size_t g_repeat_remain_count;
static uint32_t g_timestamp_offset;
static uint32_t g_last_timestamp;
static uint8_t file_buffer[FILE_BUF_LENGTH];
static uint32_t g_prev_tag_size = 0;

void die(void) {
  printf("Error!\n");
  exit(-1);
}

size_t fread_1(uint8_t *ptr) {
  assert(NULL != ptr);
  return fread(ptr, 1, 1, g_infile);
}

size_t fread_3(uint32_t *ptr) {
  assert(NULL != ptr);
  size_t count = 0;
  uint8_t bytes[3] = {0};
  *ptr = 0;
  count = fread(bytes, 3, 1, g_infile);
  *ptr = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
  return count * 3;
}

size_t fread_4(uint32_t *ptr) {
  assert(NULL != ptr);
  size_t count = 0;
  uint8_t bytes[4] = {0};
  *ptr = 0;
  count = fread(bytes, 4, 1, g_infile);
  *ptr = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
  return count * 4;
}

/*
 * @brief skip 4 bytes in the file stream
 */
size_t fread_4s() {
  size_t count = 0;
  uint8_t bytes[4] = {0};
  count = fread(bytes, 4, 1, g_infile);
  return count * 4;
}

size_t fwrite_1(uint8_t *ptr) {
  assert(NULL != ptr);
  return fwrite(ptr, 1, 1, g_outfile);
}

size_t fwrite_3(uint32_t *ptr) {
  assert(NULL != ptr);
  size_t count = 0;
  uint8_t bytes[3] = {0};
  uint8_t *data = (uint8_t *)ptr;
  bytes[0] = data[2];
  bytes[1] = data[1];
  bytes[2] = data[0];
  count = fwrite(bytes, 3, 1, g_outfile);
  return count * 3;
}

size_t fwrite_4(uint32_t *ptr) {
  assert(NULL != ptr);
  size_t count = 0;
  uint8_t bytes[4] = {0};
  uint8_t *data = (uint8_t *)ptr;
  bytes[0] = data[3];
  bytes[1] = data[2];
  bytes[2] = data[1];
  bytes[3] = data[0];
  count = fwrite(bytes, 4, 1, g_outfile);
  return count * 4;
}

size_t file_read_write(size_t count) {
  size_t remain = count;
  while (remain > FILE_BUF_LENGTH) {
    fread(file_buffer, FILE_BUF_LENGTH, 1, g_infile);
    fwrite(file_buffer, FILE_BUF_LENGTH, 1, g_outfile);
    remain -= FILE_BUF_LENGTH;
  }
  fread(file_buffer, remain, 1, g_infile);
  fwrite(file_buffer, remain, 1, g_outfile);
  remain -= remain;
  return (count - remain);
}

void flv_repeater_init(FILE *in_file, FILE *out_file, size_t start_index,
                       size_t repeat_count) {
  g_timestamp_offset = 0;
  g_infile = in_file;
  g_outfile = out_file;
  g_start_index = start_index;
  g_repeat_count = repeat_count;
  g_repeat_remain_count = (repeat_count > 0) ? repeat_count : 1;
}

int flv_repeater_run() {
  flv_tag_t *tag;
  // 传递FLV头9个字节
  assert(file_read_write(FLV_FILE_HEADER_SIZE) == FLV_FILE_HEADER_SIZE);
  // 读取首个flv prev_tag_size
  assert(fread_4(&g_prev_tag_size) == 4);
  while (1) {
    tag = flv_read_tag(0); // read the tag
    if (!tag) {
      // jump to start_index
      assert(fseek(g_infile, g_start_index, SEEK_SET) == 0);
      // g_repeat_count为0时无限循环
      if (g_repeat_count > 0) {
        g_repeat_remain_count--;
      }
      // 时间戳偏移等于跳转前最后一个时间戳
      g_timestamp_offset = g_last_timestamp;
      if (g_repeat_remain_count <= 0) {
        return 0;
      }
    } else {
      // 写入上个prev_tag_size
      assert(fwrite_4(&g_prev_tag_size) == 4);
      // 时间戳增加偏移
      tag->timestamp += g_timestamp_offset;
      tag->timestamp_ext = tag->timestamp >> 24;
      // 保存最后一个时间戳
      g_last_timestamp = tag->timestamp;
      // 保存最后一个TAG长度
      g_prev_tag_size = tag->data_size + 11;
      // 写入FLV TAG without data
      assert(flv_write_tag(tag) == 11);
      // 写入TAG data
      assert(file_read_write(tag->data_size) == tag->data_size);
      uint32_t read_prev_tag_size;
      fread_4(&read_prev_tag_size);
      // 读取大于0才能算作有效长度
      if (read_prev_tag_size > 0) {
        g_prev_tag_size = read_prev_tag_size;
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
  printf("  Data size: %lu\n", (unsigned long)tag->data_size);
  printf("  Timestamp: %lu\n", (unsigned long)tag->timestamp);
  printf("  Timestamp extended: %u\n", tag->timestamp_ext);
  printf("  StreamID: %lu\n", (unsigned long)tag->stream_id);
  return;
}

flv_tag_t *flv_read_tag(int with_data) {

  size_t count = 0;
  flv_tag_t *tag = NULL;

  tag = malloc(sizeof(flv_tag_t));

  if (feof(g_infile)) {
    return NULL;
  }
  // Start reading next tag
  count += fread_1(&(tag->tag_type));
  count += fread_3(&(tag->data_size));
  count += fread_3(&(tag->timestamp));
  count += fread_1(&(tag->timestamp_ext));
  count += fread_3(&(tag->stream_id));

  tag->timestamp |= tag->timestamp_ext << 24;

  // printf("\n");

  if (with_data) {
    tag->data = malloc((size_t)tag->data_size);
    count += fread(tag->data, 1, (size_t)tag->data_size, g_infile);
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
  if (feof(g_infile)) {
    return NULL;
  }

  return tag;
}

size_t flv_write_tag(flv_tag_t *tag) {
  size_t count = 0;
  count += fwrite_1(&tag->tag_type);
  count += fwrite_3(&tag->data_size);
  count += fwrite_3(&tag->timestamp);
  // 强制转换成uint8_t *，即是访问高8位
  count += fwrite_1(&tag->timestamp_ext);
  count += fwrite_3(&tag->stream_id);
  if (tag->data) {
    count += fwrite(tag->data, tag->data_size, 1, g_outfile) * tag->data_size;
  }
  return count;
}
