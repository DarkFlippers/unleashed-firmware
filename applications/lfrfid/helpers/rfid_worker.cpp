#include "rfid_worker.h"

RfidWorker::RfidWorker() {
}

RfidWorker::~RfidWorker() {
}

void RfidWorker::start_read() {
    reader.start();
}

bool RfidWorker::read() {
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

bool RfidWorker::detect() {
    return reader.detect();
}

bool RfidWorker::any_read() {
    return reader.any_read();
}

void RfidWorker::stop_read() {
    reader.stop();
}

void RfidWorker::start_write() {
    write_result = WriteResult::Nothing;
    write_sequence = new TickSequencer();
    validate_counts = 0;

    write_sequence->do_every_tick(1, std::bind(&RfidWorker::sq_write, this));
    write_sequence->do_after_tick(2, std::bind(&RfidWorker::sq_write_start_validate, this));
    write_sequence->do_every_tick(30, std::bind(&RfidWorker::sq_write_validate, this));
    write_sequence->do_every_tick(1, std::bind(&RfidWorker::sq_write_stop_validate, this));
}

RfidWorker::WriteResult RfidWorker::write() {
    write_sequence->tick();
    return write_result;
}

void RfidWorker::stop_write() {
    delete write_sequence;
    reader.stop();
}

void RfidWorker::start_emulate() {
    emulator.start(key.get_type(), key.get_data(), key.get_type_data_count());
}

void RfidWorker::stop_emulate() {
    emulator.stop();
}

void RfidWorker::sq_write() {
    for(size_t i = 0; i < 5; i++) {
        switch(key.get_type()) {
        case LfrfidKeyType::KeyEM4100:
            writer.start();
            writer.write_em(key.get_data());
            writer.stop();
            break;
        case LfrfidKeyType::KeyH10301:
            writer.start();
            writer.write_hid(key.get_data());
            writer.stop();
            break;
        case LfrfidKeyType::KeyI40134:
            writer.start();
            writer.write_indala(key.get_data());
            writer.stop();
            break;
        case LfrfidKeyType::KeyIoProxXSF:
            writer.start();
            writer.write_ioprox(key.get_data());
            writer.stop();
            break;
        }
    }
}

void RfidWorker::sq_write_start_validate() {
    switch(key.get_type()) {
    case LfrfidKeyType::KeyEM4100:
    case LfrfidKeyType::KeyH10301:
    case LfrfidKeyType::KeyIoProxXSF:
        reader.start_forced(RfidReader::Type::Normal);
        break;
    case LfrfidKeyType::KeyI40134:
        reader.start_forced(RfidReader::Type::Indala);
        break;
    }
}

void RfidWorker::sq_write_validate() {
    static const uint8_t data_size = LFRFID_KEY_SIZE;
    uint8_t data[data_size] = {0};
    LfrfidKeyType type;

    bool result = reader.read(&type, data, data_size);

    if(result && (write_result != WriteResult::Ok)) {
        if(validate_counts > (5 * 60)) {
            write_result = WriteResult::NotWritable;
        }

        if(type == key.get_type()) {
            if(memcmp(data, key.get_data(), key.get_type_data_count()) == 0) {
                write_result = WriteResult::Ok;
                validate_counts = 0;
            } else {
                validate_counts++;
            }
        } else {
            validate_counts++;
        }
    };
}

void RfidWorker::sq_write_stop_validate() {
    reader.stop();
}
