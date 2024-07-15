#include "iso15693_signal.h"

#include <digital_signal/digital_sequence.h>

#define BITS_IN_BYTE (8U)

#define ISO15693_SIGNAL_COEFF_HI (1U)
#define ISO15693_SIGNAL_COEFF_LO (4U)

#define ISO15693_SIGNAL_ZERO_EDGES (16U)
#define ISO15693_SIGNAL_ONE_EDGES  (ISO15693_SIGNAL_ZERO_EDGES + 1U)
#define ISO15693_SIGNAL_EOF_EDGES  (64U)
#define ISO15693_SIGNAL_SOF_EDGES  (ISO15693_SIGNAL_EOF_EDGES + 1U)
#define ISO15693_SIGNAL_EDGES      (1350U)

#define ISO15693_SIGNAL_FC     (13.56e6)
#define ISO15693_SIGNAL_FC_16  (16.0e11 / ISO15693_SIGNAL_FC)
#define ISO15693_SIGNAL_FC_256 (256.0e11 / ISO15693_SIGNAL_FC)
#define ISO15693_SIGNAL_FC_768 (768.0e11 / ISO15693_SIGNAL_FC)

typedef enum {
    Iso15693SignalIndexSof,
    Iso15693SignalIndexEof,
    Iso15693SignalIndexOne,
    Iso15693SignalIndexZero,
    Iso15693SignalIndexNum,
} Iso15693SignalIndex;

typedef DigitalSignal* Iso15693SignalBank[Iso15693SignalIndexNum];

struct Iso15693Signal {
    DigitalSequence* tx_sequence;
    Iso15693SignalBank banks[Iso15693SignalDataRateNum];
};

// Add an unmodulated signal for the length of Fc / 256 * k (where k = 1 or 4)
static void iso15693_add_silence(DigitalSignal* signal, Iso15693SignalDataRate data_rate) {
    const uint32_t k = data_rate == Iso15693SignalDataRateHi ? ISO15693_SIGNAL_COEFF_HI :
                                                               ISO15693_SIGNAL_COEFF_LO;
    digital_signal_add_period_with_level(signal, ISO15693_SIGNAL_FC_256 * k, false);
}

// Add 8 * k subcarrier pulses of Fc / 16 (where k = 1 or 4)
static void iso15693_add_subcarrier(DigitalSignal* signal, Iso15693SignalDataRate data_rate) {
    const uint32_t k = data_rate == Iso15693SignalDataRateHi ? ISO15693_SIGNAL_COEFF_HI :
                                                               ISO15693_SIGNAL_COEFF_LO;
    for(uint32_t i = 0; i < ISO15693_SIGNAL_ZERO_EDGES * k; ++i) {
        digital_signal_add_period_with_level(signal, ISO15693_SIGNAL_FC_16, !(i % 2));
    }
}

static void iso15693_add_bit(DigitalSignal* signal, Iso15693SignalDataRate data_rate, bool bit) {
    if(bit) {
        iso15693_add_silence(signal, data_rate);
        iso15693_add_subcarrier(signal, data_rate);
    } else {
        iso15693_add_subcarrier(signal, data_rate);
        iso15693_add_silence(signal, data_rate);
    }
}

static inline void iso15693_add_sof(DigitalSignal* signal, Iso15693SignalDataRate data_rate) {
    // Not adding silence since it only increases response time

    for(uint32_t i = 0; i < ISO15693_SIGNAL_FC_768 / ISO15693_SIGNAL_FC_256; ++i) {
        iso15693_add_subcarrier(signal, data_rate);
    }

    iso15693_add_bit(signal, data_rate, true);
}

static inline void iso15693_add_eof(DigitalSignal* signal, Iso15693SignalDataRate data_rate) {
    iso15693_add_bit(signal, data_rate, false);

    for(uint32_t i = 0; i < ISO15693_SIGNAL_FC_768 / ISO15693_SIGNAL_FC_256; ++i) {
        iso15693_add_subcarrier(signal, data_rate);
    }

    // Not adding silence since it does nothing here
}

static inline uint32_t
    iso15693_get_sequence_index(Iso15693SignalIndex index, Iso15693SignalDataRate data_rate) {
    return index + data_rate * Iso15693SignalIndexNum;
}

static inline void
    iso15693_add_byte(Iso15693Signal* instance, Iso15693SignalDataRate data_rate, uint8_t byte) {
    for(size_t i = 0; i < BITS_IN_BYTE; i++) {
        const uint8_t bit = byte & (1U << i);
        digital_sequence_add_signal(
            instance->tx_sequence,
            iso15693_get_sequence_index(
                bit ? Iso15693SignalIndexOne : Iso15693SignalIndexZero, data_rate));
    }
}

static inline void iso15693_signal_encode(
    Iso15693Signal* instance,
    Iso15693SignalDataRate data_rate,
    const uint8_t* tx_data,
    size_t tx_data_size) {
    digital_sequence_add_signal(
        instance->tx_sequence, iso15693_get_sequence_index(Iso15693SignalIndexSof, data_rate));

    for(size_t i = 0; i < tx_data_size; i++) {
        iso15693_add_byte(instance, data_rate, tx_data[i]);
    }

    digital_sequence_add_signal(
        instance->tx_sequence, iso15693_get_sequence_index(Iso15693SignalIndexEof, data_rate));
}

static void iso15693_signal_bank_fill(Iso15693Signal* instance, Iso15693SignalDataRate data_rate) {
    const uint32_t k = data_rate == Iso15693SignalDataRateHi ? ISO15693_SIGNAL_COEFF_HI :
                                                               ISO15693_SIGNAL_COEFF_LO;
    DigitalSignal** bank = instance->banks[data_rate];

    bank[Iso15693SignalIndexSof] = digital_signal_alloc(ISO15693_SIGNAL_SOF_EDGES * k);
    bank[Iso15693SignalIndexEof] = digital_signal_alloc(ISO15693_SIGNAL_EOF_EDGES * k);
    bank[Iso15693SignalIndexOne] = digital_signal_alloc(ISO15693_SIGNAL_ONE_EDGES * k);
    bank[Iso15693SignalIndexZero] = digital_signal_alloc(ISO15693_SIGNAL_ZERO_EDGES * k);

    iso15693_add_sof(bank[Iso15693SignalIndexSof], data_rate);
    iso15693_add_eof(bank[Iso15693SignalIndexEof], data_rate);
    iso15693_add_bit(bank[Iso15693SignalIndexOne], data_rate, true);
    iso15693_add_bit(bank[Iso15693SignalIndexZero], data_rate, false);
}

static void
    iso15693_signal_bank_clear(Iso15693Signal* instance, Iso15693SignalDataRate data_rate) {
    DigitalSignal** bank = instance->banks[data_rate];

    for(uint32_t i = 0; i < Iso15693SignalIndexNum; ++i) {
        digital_signal_free(bank[i]);
    }
}

static void
    iso15693_signal_bank_register(Iso15693Signal* instance, Iso15693SignalDataRate data_rate) {
    for(uint32_t i = 0; i < Iso15693SignalIndexNum; ++i) {
        digital_sequence_register_signal(
            instance->tx_sequence,
            iso15693_get_sequence_index(i, data_rate),
            instance->banks[data_rate][i]);
    }
}

Iso15693Signal* iso15693_signal_alloc(const GpioPin* pin) {
    furi_assert(pin);

    Iso15693Signal* instance = malloc(sizeof(Iso15693Signal));

    instance->tx_sequence = digital_sequence_alloc(BITS_IN_BYTE * 255 + 2, pin);

    for(uint32_t i = 0; i < Iso15693SignalDataRateNum; ++i) {
        iso15693_signal_bank_fill(instance, i);
        iso15693_signal_bank_register(instance, i);
    }

    return instance;
}

void iso15693_signal_free(Iso15693Signal* instance) {
    furi_assert(instance);

    digital_sequence_free(instance->tx_sequence);

    for(uint32_t i = 0; i < Iso15693SignalDataRateNum; ++i) {
        iso15693_signal_bank_clear(instance, i);
    }

    free(instance);
}

void iso15693_signal_tx(
    Iso15693Signal* instance,
    Iso15693SignalDataRate data_rate,
    const uint8_t* tx_data,
    size_t tx_data_size) {
    furi_assert(instance);
    furi_assert(data_rate < Iso15693SignalDataRateNum);
    furi_assert(tx_data);

    FURI_CRITICAL_ENTER();
    digital_sequence_clear(instance->tx_sequence);
    iso15693_signal_encode(instance, data_rate, tx_data, tx_data_size);
    digital_sequence_transmit(instance->tx_sequence);

    FURI_CRITICAL_EXIT();
}

void iso15693_signal_tx_sof(Iso15693Signal* instance, Iso15693SignalDataRate data_rate) {
    furi_assert(instance);
    furi_assert(data_rate < Iso15693SignalDataRateNum);

    FURI_CRITICAL_ENTER();
    digital_sequence_clear(instance->tx_sequence);
    digital_sequence_add_signal(
        instance->tx_sequence, iso15693_get_sequence_index(Iso15693SignalIndexSof, data_rate));
    digital_sequence_transmit(instance->tx_sequence);

    FURI_CRITICAL_EXIT();
}
