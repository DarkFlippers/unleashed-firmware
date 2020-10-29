#pragma once

CanvasApi* canvas_api_init();

void canvas_api_free(CanvasApi* api);

void canvas_reset(CanvasApi* api);

void canvas_commit(CanvasApi* api);

void canvas_frame_set(
    CanvasApi* api,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height);
