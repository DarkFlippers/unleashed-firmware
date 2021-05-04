#pragma once
#include "decoder-analyzer.h"
#include "decoder-emmarine.h"
#include "decoder-hid26.h"
#include "decoder-indala.h"
#include "key-info.h"

class RfidReader {
public:
    enum class Type : uint8_t {
        Normal,
        Indala,
    };

    RfidReader();
    void start(Type type);
    void stop();
    bool read(LfrfidKeyType* type, uint8_t* data, uint8_t data_size);

private:
    friend struct RfidReaderAccessor;

    //DecoderAnalyzer decoder_analyzer;
    DecoderEMMarine decoder_em;
    DecoderHID26 decoder_hid26;
    DecoderIndala decoder_indala;

    uint32_t last_dwt_value;

    void start_comparator(void);
    void start_timer(void);
    void start_timer_indala(void);
    void start_gpio(void);
    void stop_comparator(void);
    void stop_timer(void);
    void stop_gpio(void);

    void decode(bool polarity);

    Type type = Type::Normal;
};
