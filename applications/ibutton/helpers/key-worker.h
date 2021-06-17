#pragma once
#include <furi.h>
#include "key-info.h"
#include "key-reader.h"
#include "key-emulator.h"
#include "key-writer.h"
#include "../ibutton-key.h"
#include <one_wire_master.h>
#include <one_wire_slave.h>

class KeyWorker {
public:
    KeyReader::Error read(iButtonKey* key);
    void start_read();
    void stop_read();

    bool emulated();
    void start_emulate(iButtonKey* key);
    void stop_emulate();

    KeyWriter::Error write(iButtonKey* key);
    void start_write();
    void stop_write();

    KeyWorker(const GpioPin* one_wire_gpio);
    ~KeyWorker();

private:
    // one wire
    OneWireMaster onewire_master;
    OneWireSlave onewire_slave;
    KeyReader key_reader;
    KeyEmulator key_emulator;
    KeyWriter key_writer;
};