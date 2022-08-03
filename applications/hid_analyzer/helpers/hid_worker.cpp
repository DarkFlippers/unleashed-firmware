#include "hid_worker.h"

HIDWorker::HIDWorker() {
}

HIDWorker::~HIDWorker() {
}

void HIDWorker::start_read() {
    reader.start();
}

bool HIDWorker::read() {
    static const uint8_t data_size = LFRFID_KEY_SIZE;
    uint8_t data[data_size] = {0};
    LfrfidKeyType type;

    bool result = reader.read(&type, data, data_size);

    if(result) {
        key.set_type(type);
        key.set_data(data, data_size);
    };

    return result;
}

bool HIDWorker::detect() {
    return reader.detect();
}

bool HIDWorker::any_read() {
    return reader.any_read();
}

void HIDWorker::stop_read() {
    reader.stop();
}
