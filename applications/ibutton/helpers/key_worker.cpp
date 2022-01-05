#include "key_worker.h"
#include <callback-connector.h>
#include <maxim_crc.h>

extern COMP_HandleTypeDef hcomp1;

KeyReader::Error KeyWorker::read(iButtonKey* key) {
    KeyReader::Error result = key_reader.read(key);

    return result;
}

void KeyWorker::start_read() {
    key_reader.start();
}

void KeyWorker::stop_read() {
    key_reader.stop();
}

bool KeyWorker::emulated() {
    return key_emulator.emulated();
}

void KeyWorker::start_emulate(iButtonKey* key) {
    key_emulator.start(key);
}

void KeyWorker::stop_emulate() {
    key_emulator.stop();
}

KeyWriter::Error KeyWorker::write(iButtonKey* key) {
    return key_writer.write(key);
}

void KeyWorker::start_write() {
    key_writer.start();
}

void KeyWorker::stop_write() {
    key_writer.stop();
}

KeyWorker::KeyWorker(const GpioPin* one_wire_gpio)
    : onewire_master{one_wire_gpio}
    , onewire_slave{one_wire_gpio}
    , key_reader{&onewire_master}
    , key_emulator{&onewire_slave}
    , key_writer{&onewire_master} {
}

KeyWorker::~KeyWorker() {
}