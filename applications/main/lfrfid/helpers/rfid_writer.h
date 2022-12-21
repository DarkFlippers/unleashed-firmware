#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t wait_time;
    uint8_t start_gap;
    uint8_t write_gap;
    uint8_t data_0;
    uint8_t data_1;
    uint16_t program;
} T55xxTiming;

void writer_start();
void writer_stop();
void write_gap(uint32_t gap_time);
void write_bit(T55xxTiming* t55xxtiming, bool value);
void write_block(
    T55xxTiming* t55xxtiming,
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool password_enable,
    uint32_t password);

void write_reset(T55xxTiming* t55xxtiming);