#pragma once
#include "key-info.h"
#include "rfid-reader.h"
#include "rfid-writer.h"
#include "rfid-timer-emulator.h"
#include "rfid-key.h"
#include "state-sequencer.h"

class RfidWorker {
public:
    RfidWorker();
    ~RfidWorker();

    void start_read();
    bool read();
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
    uint8_t validate_counts;
    void sq_write_stop_validate();
};