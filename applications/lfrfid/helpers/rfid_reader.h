#pragma once
//#include "decoder_analyzer.h"
#include "decoder_gpio_out.h"
#include "decoder_emmarin.h"
#include "decoder_hid26.h"
#include "decoder_indala.h"
#include "key_info.h"

//#define RFID_GPIO_DEBUG 1

class RfidReader {
public:
    enum class Type : uint8_t {
        Normal,
        Indala,
    };

    RfidReader();
    void start();
    void start_forced(RfidReader::Type type);
    void stop();
    bool read(LfrfidKeyType* type, uint8_t* data, uint8_t data_size, bool switch_enable = true);

    bool detect();
    bool any_read();

private:
    friend struct RfidReaderAccessor;

    //DecoderAnalyzer decoder_analyzer;
#ifdef RFID_GPIO_DEBUG
    DecoderGpioOut decoder_gpio_out;
#endif
    DecoderEMMarin decoder_em;
    DecoderHID26 decoder_hid26;
    DecoderIndala decoder_indala;

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
