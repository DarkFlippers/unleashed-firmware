#pragma once

#include "subghz_txrx.h"

struct SubGhzTxRx {
    SubGhzWorker* worker;

    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzTransmitter* transmitter;
    SubGhzProtocolDecoderBase* decoder_result;
    FlipperFormat* fff_data;

    SubGhzRadioPreset* preset;
    SubGhzSetting* setting;

    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
    bool is_database_loaded;
    SubGhzHopperState hopper_state;

    SubGhzTxRxState txrx_state;
    SubGhzSpeakerState speaker_state;
    const SubGhzDevice* radio_device;
    SubGhzRadioDeviceType radio_device_type;

    SubGhzTxRxNeedSaveCallback need_save_callback;
    void* need_save_context;
    //True when the last TX transmitted the internal fff_data buffer (i.e. a
    //signal bound to subghz->file_path), as opposed to a history/RX signal.
    //Used to gate the post-TX dynamic save-back so it never writes an
    //unrelated file
    bool tx_from_internal_fff;

    bool debug_pin_state;
};
