#pragma once
#include "one_wire_master.h"
#include "maxim_crc.h"

typedef enum {
    KEY_DS1990, /**< DS1990 */
    KEY_CYFRAL, /**< CYFRAL*/
    KEY_METAKOM, /**< METAKOM */
} KeyType;

typedef enum {
    WR_OK,
    WR_SAME_KEY,
    WR_ERROR,
} WriterResult;

class BlanksWriter {
private:
    const GpioPin* gpio;
    OneWireMaster* onewire;

    void onewire_release(void);
    void onewire_write_one_bit(bool value, uint32_t delay);

    bool write_TM2004(const uint8_t* key, uint8_t key_length);
    bool write_1990_1(const uint8_t* key, uint8_t key_length);
    bool write_1990_2(const uint8_t* key, uint8_t key_length);
    bool write_TM01(KeyType type, const uint8_t* key, uint8_t key_length);

    void write_byte_ds1990(uint8_t data);
    bool compare_key_ds1990(const uint8_t* key, uint8_t key_length);

public:
    BlanksWriter(const GpioPin* one_wire_gpio);
    ~BlanksWriter();

    WriterResult write(KeyType type, const uint8_t* key, uint8_t key_length);
    void start();
    void stop();
};
