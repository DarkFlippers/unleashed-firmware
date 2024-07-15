#include "iso14443_3a_signal.h"

#include <digital_signal/digital_sequence.h>

#define BITS_IN_BYTE (8)

#define ISO14443_3A_SIGNAL_BIT_MAX_EDGES (10)
#define ISO14443_3A_SIGNAL_MAX_EDGES     (1350)

#define ISO14443_3A_SIGNAL_SEQUENCE_SIZE \
    (ISO14443_3A_SIGNAL_MAX_EDGES / (ISO14443_3A_SIGNAL_BIT_MAX_EDGES - 2))

#define ISO14443_3A_SIGNAL_F_SIG       (13560000.0)
#define ISO14443_3A_SIGNAL_T_SIG       7374 //73.746ns*100
#define ISO14443_3A_SIGNAL_T_SIG_X8    58992 //T_SIG*8
#define ISO14443_3A_SIGNAL_T_SIG_X8_X8 471936 //T_SIG*8*8
#define ISO14443_3A_SIGNAL_T_SIG_X8_X9 530928 //T_SIG*8*9

typedef enum {
    Iso14443_3aSignalIndexZero,
    Iso14443_3aSignalIndexOne,
    Iso14443_3aSignalIndexCount,
} Iso14443_3aSignalIndex;

typedef DigitalSignal* Iso14443_3aSignalBank[Iso14443_3aSignalIndexCount];

struct Iso14443_3aSignal {
    DigitalSequence* tx_sequence;
    Iso14443_3aSignalBank signals;
};

static void iso14443_3a_signal_add_byte(Iso14443_3aSignal* instance, uint8_t byte, bool parity) {
    for(size_t i = 0; i < BITS_IN_BYTE; i++) {
        digital_sequence_add_signal(
            instance->tx_sequence,
            FURI_BIT(byte, i) ? Iso14443_3aSignalIndexOne : Iso14443_3aSignalIndexZero);
    }
    digital_sequence_add_signal(
        instance->tx_sequence, parity ? Iso14443_3aSignalIndexOne : Iso14443_3aSignalIndexZero);
}

static void iso14443_3a_signal_encode(
    Iso14443_3aSignal* instance,
    const uint8_t* tx_data,
    const uint8_t* tx_parity,
    size_t tx_bits) {
    furi_assert(instance);
    furi_assert(tx_data);
    furi_assert(tx_parity);

    // Start of frame
    digital_sequence_add_signal(instance->tx_sequence, Iso14443_3aSignalIndexOne);

    if(tx_bits < BITS_IN_BYTE) {
        for(size_t i = 0; i < tx_bits; i++) {
            digital_sequence_add_signal(
                instance->tx_sequence,
                FURI_BIT(tx_data[0], i) ? Iso14443_3aSignalIndexOne : Iso14443_3aSignalIndexZero);
        }
    } else {
        for(size_t i = 0; i < tx_bits / BITS_IN_BYTE; i++) {
            bool parity = FURI_BIT(tx_parity[i / BITS_IN_BYTE], i % BITS_IN_BYTE);
            iso14443_3a_signal_add_byte(instance, tx_data[i], parity);
        }
    }
}

static inline void iso14443_3a_signal_set_bit(DigitalSignal* signal, bool bit) {
    digital_signal_set_start_level(signal, bit);

    if(bit) {
        for(uint32_t i = 0; i < 7; ++i) {
            digital_signal_add_period(signal, ISO14443_3A_SIGNAL_T_SIG_X8);
        }
        digital_signal_add_period(signal, ISO14443_3A_SIGNAL_T_SIG_X8_X9);
    } else {
        digital_signal_add_period(signal, ISO14443_3A_SIGNAL_T_SIG_X8_X8);
        for(uint32_t i = 0; i < 8; ++i) {
            digital_signal_add_period(signal, ISO14443_3A_SIGNAL_T_SIG_X8);
        }
    }
}

static inline void iso14443_3a_signal_bank_fill(Iso14443_3aSignalBank bank) {
    for(uint32_t i = 0; i < Iso14443_3aSignalIndexCount; ++i) {
        bank[i] = digital_signal_alloc(ISO14443_3A_SIGNAL_BIT_MAX_EDGES);
        iso14443_3a_signal_set_bit(bank[i], i % Iso14443_3aSignalIndexCount != 0);
    }
}

static inline void iso14443_3a_signal_bank_clear(Iso14443_3aSignalBank bank) {
    for(uint32_t i = 0; i < Iso14443_3aSignalIndexCount; ++i) {
        digital_signal_free(bank[i]);
    }
}

static inline void
    iso14443_3a_signal_bank_register(Iso14443_3aSignalBank bank, DigitalSequence* sequence) {
    for(uint32_t i = 0; i < Iso14443_3aSignalIndexCount; ++i) {
        digital_sequence_register_signal(sequence, i, bank[i]);
    }
}

Iso14443_3aSignal* iso14443_3a_signal_alloc(const GpioPin* pin) {
    furi_assert(pin);

    Iso14443_3aSignal* instance = malloc(sizeof(Iso14443_3aSignal));
    instance->tx_sequence = digital_sequence_alloc(ISO14443_3A_SIGNAL_SEQUENCE_SIZE, pin);

    iso14443_3a_signal_bank_fill(instance->signals);
    iso14443_3a_signal_bank_register(instance->signals, instance->tx_sequence);

    return instance;
}

void iso14443_3a_signal_free(Iso14443_3aSignal* instance) {
    furi_assert(instance);
    furi_assert(instance->tx_sequence);

    iso14443_3a_signal_bank_clear(instance->signals);
    digital_sequence_free(instance->tx_sequence);
    free(instance);
}

void iso14443_3a_signal_tx(
    Iso14443_3aSignal* instance,
    const uint8_t* tx_data,
    const uint8_t* tx_parity,
    size_t tx_bits) {
    furi_assert(instance);
    furi_assert(tx_data);
    furi_assert(tx_parity);

    FURI_CRITICAL_ENTER();
    digital_sequence_clear(instance->tx_sequence);
    iso14443_3a_signal_encode(instance, tx_data, tx_parity, tx_bits);
    digital_sequence_transmit(instance->tx_sequence);
    FURI_CRITICAL_EXIT();
}
