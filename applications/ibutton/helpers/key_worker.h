#pragma once
#include <furi.h>
#include "key_info.h"
#include "key_reader.h"
#include "key_emulator.h"
#include "key_writer.h"
#include "../ibutton_key.h"
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