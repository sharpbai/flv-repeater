//
// Created by sharpbai on 16/8/4.
//
#include "flv-repeater.h"
#include "assert.h"

size_t fread_1(FILE* fs, uint8_t *ptr) {
    assert(NULL != ptr);
    return fread(ptr, 1, 1, fs);
}

size_t fread_3(FILE* fs, uint32_t *ptr) {
    assert(NULL != ptr);
    size_t count = 0;
    uint8_t bytes[3] = {0};
    *ptr = 0;
    count = fread(bytes, 3, 1, fs);
    *ptr = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
    return count * 3;
}

size_t fread_4(FILE* fs, uint32_t *ptr) {
    assert(NULL != ptr);
    size_t count = 0;
    uint8_t bytes[4] = {0};
    *ptr = 0;
    count = fread(bytes, 4, 1, fs);
    *ptr = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    return count * 4;
}

/*
 * @brief skip 4 bytes in the file stream
 */
size_t fread_4s(FILE* fs) {
    size_t count = 0;
    uint8_t bytes[4] = {0};
    count = fread(bytes, 4, 1, fs);
    return count * 4;
}

size_t fwrite_1(FILE* fs, uint8_t *ptr) {
    assert(NULL != ptr);
    return fwrite(ptr, 1, 1, fs);
}

size_t fwrite_3(FILE* fs, uint32_t *ptr) {
    assert(NULL != ptr);
    size_t count = 0;
    uint8_t bytes[3] = {0};
    uint8_t *data = (uint8_t *) ptr;
    bytes[0] = data[2];
    bytes[1] = data[1];
    bytes[2] = data[0];
    count = fwrite(bytes, 3, 1, fs);
    return count * 3;
}

size_t fwrite_4(FILE* fs, uint32_t *ptr) {
    assert(NULL != ptr);
    size_t count = 0;
    uint8_t bytes[4] = {0};
    uint8_t *data = (uint8_t *) ptr;
    bytes[0] = data[3];
    bytes[1] = data[2];
    bytes[2] = data[1];
    bytes[3] = data[0];
    count = fwrite(bytes, 4, 1, fs);
    return count * 4;
}

size_t file_read_write(FILE* fs_read, FILE* fs_write, size_t count, uint8_t* buffer, size_t buffer_length) {
    size_t remain = count;
    while (remain > buffer_length) {
        fread(buffer, buffer_length, 1, fs_read);
        fwrite(buffer, buffer_length, 1, fs_write);
        remain -= buffer_length;
    }
    fread(buffer, remain, 1, fs_read);
    fwrite(buffer, remain, 1, fs_write);
    fflush(fs_write);
    remain -= remain;
    return (count - remain);
}