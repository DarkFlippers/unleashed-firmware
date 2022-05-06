#pragma once
#include "key_info.h"
#include "rfid_reader.h"
#include "rfid_writer.h"
#include "rfid_timer_emulator.h"
#include "rfid_key.h"
#include "state_sequencer.h"

class RfidWorker {
public:
    RfidWorker();
    ~RfidWorker();

    void start_read();
    bool read();
    bool detect();
    bool any_read();
    void stop_read();

    enum class WriteResult : uint8_t {
        Ok,
        NotWritable,
        Nothing,
    };

    void start_write();
    WriteResult write();
    void stop_write();

    void start_emulate();
    void stop_emulate();

    RfidKey key;

private:
    RfidWriter writer;
    RfidReader reader;
    RfidTimerEmulator emulator;

    WriteResult write_result;
    TickSequencer* write_sequence;

    void sq_write();
    void sq_write_start_validate();
    void sq_write_validate();
    uint16_t validate_counts;
    void sq_write_stop_validate();
};
