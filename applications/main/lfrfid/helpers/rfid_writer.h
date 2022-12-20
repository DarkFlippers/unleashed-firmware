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

typedef struct {
    uint8_t opcode_page_0;
    uint8_t opcode_page_1;
    uint8_t opcode_reset;
} T55xxCmd;

//static void writer_initialize(T55xxTiming* t55xxtiming, T55xxCmd* t55xxcmd);
void writer_start();
void writer_stop();
void write_gap(uint32_t gap_time);
void write_bit(T55xxTiming* t55xxtiming, bool value);
void write_byte(T55xxTiming* t55xxtiming, uint8_t value);
void write_block(
    T55xxTiming* t55xxtiming,
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool password_enable,
    uint32_t password);

void write_reset(T55xxTiming* t55xxtiming);