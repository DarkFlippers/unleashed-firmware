#pragma once
#include "key_info.h"
#include "rfid_key.h"
#include "hid_reader.h"

class HIDWorker {
public:
    HIDWorker();
    ~HIDWorker();

    void start_read();
    bool read();
    bool detect();
    bool any_read();
    void stop_read();

    RfidKey key;

private:
    HIDReader reader;
};
