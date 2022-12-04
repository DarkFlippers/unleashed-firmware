#pragma once

#include <stdint.h>

#include <storage/storage.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define VIDEO_WIDTH 104
#define VIDEO_HEIGHT 80
#define FILE_BUFFER_SIZE (1024 * 64)
#define VIDEO_PATH EXT_PATH("apps_data/bad_apple/video.bin")
#define VIDEO_X ((SCREEN_WIDTH - VIDEO_WIDTH) / 2)
#define VIDEO_Y 0

typedef struct {
    uint8_t framebuffer[VIDEO_HEIGHT * VIDEO_WIDTH / 8];
    uint8_t file_buffer[FILE_BUFFER_SIZE];
    uint32_t file_buffer_offset;
    uint32_t frame_write_offset; // bit index
    Storage* storage;
    File* video_file;
} BadAppleCtx;

uint8_t bad_apple_read_byte(BadAppleCtx* inst);
