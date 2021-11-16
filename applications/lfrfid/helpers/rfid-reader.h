#pragma once
//#include "decoder-analyzer.h"
#include "decoder-gpio-out.h"
#include "decoder-emmarin.h"
#include "decoder-hid26.h"
#include "decoder-indala.h"
#include "key-info.h"

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

    LfrfidKeyType last_readed_type;
    uint8_t last_readed_data[LFRFID_KEY_SIZE];
    uint8_t last_readed_count;

    Type type = Type::Normal;
};
