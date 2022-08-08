#pragma once
#include "decoder_hid.h"
#include "key_info.h"

//#define RFID_GPIO_DEBUG 1

class HIDReader {
public:
    enum class Type : uint8_t {
        Normal,
        Indala,
    };

    HIDReader();
    void start();
    void start_forced(HIDReader::Type type);
    void stop();
    bool read(LfrfidKeyType* _type, uint8_t* data, uint8_t data_size, bool switch_enable = true);

    bool detect();
    bool any_read();

private:
    friend struct HIDReaderAccessor;

    DecoderHID decoder_hid;

    uint32_t last_dwt_value;

    void start_comparator(void);
    void stop_comparator(void);

    void decode(bool polarity);

    uint32_t detect_ticks;

    uint32_t switch_os_tick_last;
    bool switch_timer_elapsed();
    void switch_timer_reset();
    void switch_mode();

    LfrfidKeyType last_read_type;
    uint8_t last_read_data[LFRFID_KEY_SIZE];
    uint8_t last_read_count;

    Type type = Type::Normal;
};
