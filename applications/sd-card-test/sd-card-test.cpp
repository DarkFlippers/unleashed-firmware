#include "app-template.h"
#include "stm32_adafruit_sd.h"
#include "fnv1a-hash.h"
#include "filesystem-api.h"
#include "cli/cli.h"
#include "callback-connector.h"
#include <notification/notification-messages.h>

// event enumeration type
typedef uint8_t event_t;

class SdTestState {
public:
    // state data
    static const uint8_t lines_count = 6;
    const char* line[lines_count];

    // state initializer
    SdTestState() {
        for(uint8_t i = 0; i < lines_count; i++) {
            line[i] = "";
        }
    }
};

// events class
class SdTestEvent {
public:
    // events enum
    static const event_t EventTypeTick = 0;
    static const event_t EventTypeKey = 1;

    // payload
    union {
        InputEvent input;
    } value;

    // event type
    event_t type;
};

// our app derived from base AppTemplate class
// with template variables <state, events>
class SdTest : public AppTemplate<SdTestState, SdTestEvent> {
public:
    // vars
    const uint32_t benchmark_data_size = 4096;
    uint8_t* benchmark_data;
    FS_Api* fs_api;
    NotificationApp* notification;

    // consts
    static const uint32_t BENCHMARK_ERROR = UINT_MAX;

    // funcs
    void run();
    void render(Canvas* canvas);
    template <class T> void set_text(std::initializer_list<T> list);
    template <class T> void set_error(std::initializer_list<T> list);
    void wait_for_button(InputKey input_button);
    bool ask(InputKey input_button_cancel, InputKey input_button_ok);
    void blink_red();
    void blink_green();

    // "tests"
    void detect_sd_card();
    void show_warning();
    void get_sd_card_info();

    bool prepare_benchmark_data();
    void free_benchmark_data();
    void write_benchmark();
    uint32_t
    write_benchmark_internal(const uint32_t size, const uint32_t tcount, bool silent = false);

    void read_benchmark();
    uint32_t read_benchmark_internal(
        const uint32_t size,
        const uint32_t count,
        File* file,
        bool silent = false);

    void hash_benchmark();

    // cli tests
    void cli_read_benchmark(Cli* cli, string_t args, void* _ctx);
    void cli_write_benchmark(Cli* cli, string_t args, void* _ctx);
};

// start app
void SdTest::run() {
    app_ready();

    fs_api = static_cast<FS_Api*>(furi_record_open("sdcard"));
    notification = static_cast<NotificationApp*>(furi_record_open("notification"));

    if(fs_api == NULL) {
        set_error({"cannot get sdcard api"});
        exit();
    }

    Cli* cli = static_cast<Cli*>(furi_record_open("cli"));

    // read_benchmark and write_benchmark signatures are same. so we must use tags
    auto cli_read_cb = cbc::obtain_connector<0>(this, &SdTest::cli_read_benchmark);
    cli_add_command(cli, "sd_read_test", cli_read_cb, this);

    auto cli_write_cb = cbc::obtain_connector<1>(this, &SdTest::cli_write_benchmark);
    cli_add_command(cli, "sd_write_test", cli_write_cb, this);

    detect_sd_card();
    get_sd_card_info();
    show_warning();

    set_text({"preparing benchmark data"});
    bool data_prepared = prepare_benchmark_data();
    if(data_prepared) {
        set_text({"benchmark data prepared"});
    } else {
        set_error({"cannot allocate buffer", "for benchmark data"});
    }

    write_benchmark();
    read_benchmark();
    hash_benchmark();
    free_benchmark_data();

    set_text({
        "test complete",
        "",
        "",
        "",
        "",
        "press BACK to exit",
    });
    wait_for_button(InputKeyBack);

    furi_record_close("notification");
    exit();
}

// detect sd card insertion
void SdTest::detect_sd_card() {
    const uint8_t str_buffer_size = 40;
    const uint8_t dots_animation_size = 4;
    char str_buffer[str_buffer_size];
    const char dots[dots_animation_size][4] = {"", ".", "..", "..."};
    uint8_t i = 0;

    // detect sd card pin
    while(fs_api->common.get_fs_info(NULL, NULL) == FSE_NOT_READY) {
        delay(100);

        snprintf(str_buffer, str_buffer_size, "Waiting%s", dots[i]);
        set_text({static_cast<const char*>(str_buffer), "Please insert sd card"});

        if(i < (dots_animation_size - 1)) {
            i++;
        } else {
            i = 0;
        }
    }

    blink_green();
}

// show warning about test
void SdTest::show_warning() {
    set_text(
        {"!!Warning!!",
         "during the tests",
         "files can be overwritten",
         "or data on card may be lost",
         "",
         "press UP DOWN OK to continue"});

    wait_for_button(InputKeyUp);
    wait_for_button(InputKeyDown);
    wait_for_button(InputKeyOk);
}

// get info about sd card, label, sn
// sector, cluster, total and free size
void SdTest::get_sd_card_info() {
    const uint8_t str_buffer_size = 26;
    char str_buffer[2][str_buffer_size];
    FS_Error result;
    uint64_t bytes_total, bytes_free;
    int __attribute__((unused)) snprintf_count = 0;

    result = fs_api->common.get_fs_info(&bytes_total, &bytes_free);
    if(result != FSE_OK) set_error({"get_fs_info error", fs_api->error.get_desc(result)});

    snprintf(
        str_buffer[0], str_buffer_size, "%lu KB total", static_cast<uint32_t>(bytes_total / 1024));
    snprintf(
        str_buffer[1], str_buffer_size, "%lu KB free", static_cast<uint32_t>(bytes_free / 1024));

    set_text(
        {static_cast<const char*>(str_buffer[0]),
         static_cast<const char*>(str_buffer[1]),
         "",
         "",
         "",
         "press OK to continue"});

    blink_green();

    wait_for_button(InputKeyOk);
}

// prepare benchmark data (allocate data in ram)
bool SdTest::prepare_benchmark_data() {
    bool result = true;
    benchmark_data = static_cast<uint8_t*>(malloc(benchmark_data_size));

    if(benchmark_data == NULL) {
        result = false;
    }

    for(size_t i = 0; i < benchmark_data_size; i++) {
        benchmark_data[i] = static_cast<uint8_t>(i);
    }

    return result;
}

void SdTest::free_benchmark_data() {
    free(benchmark_data);
}

// write speed test
void SdTest::write_benchmark() {
    const uint32_t b1_size = 1;
    const uint32_t b8_size = 8;
    const uint32_t b32_size = 32;
    const uint32_t b256_size = 256;
    const uint32_t b4096_size = 4096;

    const uint32_t benchmark_data_size = 16384 * 4;

    uint32_t benchmark_bps = 0;

    const uint8_t str_buffer_size = 32;
    char str_buffer[6][str_buffer_size] = {"", "", "", "", "", ""};
    auto string_list = {
        static_cast<const char*>(str_buffer[0]),
        static_cast<const char*>(str_buffer[1]),
        static_cast<const char*>(str_buffer[2]),
        static_cast<const char*>(str_buffer[3]),
        static_cast<const char*>(str_buffer[4]),
        static_cast<const char*>(str_buffer[5])};

    set_text({"write speed test", "procedure can be lengthy", "please wait"});
    delay(100);

    // 1b test
    benchmark_bps = write_benchmark_internal(b1_size, benchmark_data_size / b1_size);
    snprintf(str_buffer[0], str_buffer_size, "1-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 8b test
    benchmark_bps = write_benchmark_internal(b8_size, benchmark_data_size / b8_size);
    snprintf(str_buffer[1], str_buffer_size, "8-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 32b test
    benchmark_bps = write_benchmark_internal(b32_size, benchmark_data_size / b32_size);
    snprintf(str_buffer[2], str_buffer_size, "32-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 256b test
    benchmark_bps = write_benchmark_internal(b256_size, benchmark_data_size / b256_size);
    snprintf(str_buffer[3], str_buffer_size, "256-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 4096b test
    benchmark_bps = write_benchmark_internal(b4096_size, benchmark_data_size / b4096_size);
    snprintf(str_buffer[4], str_buffer_size, "4096-byte: %lu bps", benchmark_bps);
    snprintf(str_buffer[5], str_buffer_size, "press OK to continue");
    set_text(string_list);

    blink_green();

    wait_for_button(InputKeyOk);
}

uint32_t SdTest::write_benchmark_internal(const uint32_t size, const uint32_t count, bool silent) {
    uint32_t start_tick, stop_tick, benchmark_bps = 0, benchmark_time, bytes_written;
    File file;

    const uint8_t str_buffer_size = 32;
    char str_buffer[str_buffer_size];

    if(!fs_api->file.open(&file, "write.test", FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        if(!silent) {
            snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
            set_error({"cannot open file ", static_cast<const char*>(str_buffer)});
        } else {
            benchmark_bps = BENCHMARK_ERROR;
        }
    }

    start_tick = osKernelGetTickCount();
    for(size_t i = 0; i < count; i++) {
        bytes_written = fs_api->file.write(&file, benchmark_data, size);
        if(bytes_written != size || file.error_id != FSE_OK) {
            if(!silent) {
                snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
                set_error({"cannot write to file ", static_cast<const char*>(str_buffer)});
            } else {
                benchmark_bps = BENCHMARK_ERROR;
                break;
            }
        }
    }
    stop_tick = osKernelGetTickCount();

    if(!fs_api->file.close(&file)) {
        if(!silent) {
            snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
            set_error({"cannot close file ", static_cast<const char*>(str_buffer)});
        } else {
            benchmark_bps = BENCHMARK_ERROR;
        }
    }

    if(benchmark_bps != BENCHMARK_ERROR) {
        benchmark_time = stop_tick - start_tick;
        benchmark_bps = (count * size) * osKernelGetTickFreq() / benchmark_time;
    }

    return benchmark_bps;
}

// read speed test
void SdTest::read_benchmark() {
    const uint32_t benchmark_data_size = 16384 * 8;
    uint32_t bytes_written;
    uint32_t benchmark_bps = 0;

    const uint8_t str_buffer_size = 32;
    char str_buffer[6][str_buffer_size] = {"", "", "", "", "", ""};
    auto string_list = {
        static_cast<const char*>(str_buffer[0]),
        static_cast<const char*>(str_buffer[1]),
        static_cast<const char*>(str_buffer[2]),
        static_cast<const char*>(str_buffer[3]),
        static_cast<const char*>(str_buffer[4]),
        static_cast<const char*>(str_buffer[5])};

    File file;

    const uint32_t b1_size = 1;
    const uint32_t b8_size = 8;
    const uint32_t b32_size = 32;
    const uint32_t b256_size = 256;
    const uint32_t b4096_size = 4096;

    // prepare data for read test
    set_text({"prepare data", "for read speed test", "procedure can be lengthy", "please wait"});
    delay(100);

    if(!fs_api->file.open(&file, "read.test", FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        set_error({"cannot open file ", "in prepare read"});
    }

    for(size_t i = 0; i < benchmark_data_size / b4096_size; i++) {
        bytes_written = fs_api->file.write(&file, benchmark_data, b4096_size);
        if(bytes_written != b4096_size || file.error_id != FSE_OK) {
            set_error({"cannot write to file ", "in prepare read"});
        }
    }

    if(!fs_api->file.close(&file)) {
        set_error({"cannot close file ", "in prepare read"});
    }

    // test start
    set_text({"read speed test", "procedure can be lengthy", "please wait"});
    delay(100);

    // open file
    if(!fs_api->file.open(&file, "read.test", FSAM_READ, FSOM_OPEN_EXISTING)) {
        set_error({"cannot open file ", "in read benchmark"});
    }

    // 1b test
    benchmark_bps = read_benchmark_internal(b1_size, benchmark_data_size / b1_size, &file);
    snprintf(str_buffer[0], str_buffer_size, "1-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 8b test
    benchmark_bps = read_benchmark_internal(b8_size, benchmark_data_size / b8_size, &file);
    snprintf(str_buffer[1], str_buffer_size, "8-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 32b test
    benchmark_bps = read_benchmark_internal(b32_size, benchmark_data_size / b32_size, &file);
    snprintf(str_buffer[2], str_buffer_size, "32-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 256b test
    benchmark_bps = read_benchmark_internal(b256_size, benchmark_data_size / b256_size, &file);
    snprintf(str_buffer[3], str_buffer_size, "256-byte: %lu bps", benchmark_bps);
    set_text(string_list);
    delay(100);

    // 4096b test
    benchmark_bps = read_benchmark_internal(b4096_size, benchmark_data_size / b4096_size, &file);
    snprintf(str_buffer[4], str_buffer_size, "4096-byte: %lu bps", benchmark_bps);
    snprintf(str_buffer[5], str_buffer_size, "press OK to continue");
    set_text(string_list);

    // close file
    if(!fs_api->file.close(&file)) {
        set_error({"cannot close file ", "in read test"});
    }

    blink_green();

    wait_for_button(InputKeyOk);
}

uint32_t SdTest::read_benchmark_internal(
    const uint32_t size,
    const uint32_t count,
    File* file,
    bool silent) {
    uint32_t start_tick, stop_tick, benchmark_bps = 0, benchmark_time, bytes_readed;

    const uint8_t str_buffer_size = 32;
    char str_buffer[str_buffer_size];
    uint8_t* read_buffer;

    read_buffer = static_cast<uint8_t*>(malloc(size));

    if(read_buffer == NULL) {
        if(!silent) {
            snprintf(str_buffer, str_buffer_size, "in %lu-byte read test", size);
            set_error({"cannot allocate memory", static_cast<const char*>(str_buffer)});
        } else {
            benchmark_bps = BENCHMARK_ERROR;
        }
    }

    fs_api->file.seek(file, 0, true);

    start_tick = osKernelGetTickCount();
    for(size_t i = 0; i < count; i++) {
        bytes_readed = fs_api->file.read(file, read_buffer, size);
        if(bytes_readed != size || file->error_id != FSE_OK) {
            if(!silent) {
                snprintf(str_buffer, str_buffer_size, "in %lu-byte read test", size);
                set_error({"cannot read from file ", static_cast<const char*>(str_buffer)});
            } else {
                benchmark_bps = BENCHMARK_ERROR;
                break;
            }
        }
    }
    stop_tick = osKernelGetTickCount();

    free(read_buffer);

    if(benchmark_bps != BENCHMARK_ERROR) {
        benchmark_time = stop_tick - start_tick;
        benchmark_bps = (count * size) * osKernelGetTickFreq() / benchmark_time;
    }

    return benchmark_bps;
}

// hash benchmark, store data to sd with known hash
// then read, calculate hash and compare both hashes
void SdTest::hash_benchmark() {
    uint32_t mcu_data_hash = FNV_1A_INIT;
    uint32_t sdcard_data_hash = FNV_1A_INIT;
    uint8_t* read_buffer;
    uint32_t bytes_readed;

    uint32_t bytes_written;

    const uint8_t str_buffer_size = 32;
    char str_buffer[3][str_buffer_size] = {"", "", ""};

    File file;

    const uint32_t b4096_size = 4096;
    const uint32_t benchmark_count = 20;

    // prepare data for hash test
    set_text({"prepare data", "for hash test"});
    delay(100);

    // write data to test file and calculate hash
    if(!fs_api->file.open(&file, "hash.test", FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        set_error({"cannot open file ", "in prepare hash"});
    }

    for(uint32_t i = 0; i < benchmark_count; i++) {
        mcu_data_hash = fnv1a_buffer_hash(benchmark_data, b4096_size, mcu_data_hash);
        bytes_written = fs_api->file.write(&file, benchmark_data, b4096_size);

        if(bytes_written != b4096_size || file.error_id != FSE_OK) {
            set_error({"cannot write to file ", "in prepare hash"});
        }

        snprintf(str_buffer[0], str_buffer_size, "writing %lu of %lu x 4k", i, benchmark_count);
        set_text({"prepare data", "for hash test", static_cast<const char*>(str_buffer[0])});
        delay(100);
    }

    if(!fs_api->file.close(&file)) {
        set_error({"cannot close file ", "in prepare hash"});
    }

    // show hash of data located in mcu memory
    snprintf(str_buffer[0], str_buffer_size, "hash in mcu 0x%lx", mcu_data_hash);
    set_text({str_buffer[0]});
    delay(100);

    // read data from sd card and calculate hash
    read_buffer = static_cast<uint8_t*>(malloc(b4096_size));

    if(read_buffer == NULL) {
        set_error({"cannot allocate memory", "in hash test"});
    }

    if(!fs_api->file.open(&file, "hash.test", FSAM_READ, FSOM_OPEN_EXISTING)) {
        set_error({"cannot open file ", "in hash test"});
    }

    for(uint32_t i = 0; i < benchmark_count; i++) {
        bytes_readed = fs_api->file.read(&file, read_buffer, b4096_size);
        sdcard_data_hash = fnv1a_buffer_hash(read_buffer, b4096_size, sdcard_data_hash);

        if(bytes_readed != b4096_size || file.error_id != FSE_OK) {
            set_error({"cannot read from file ", "in hash test"});
        }

        snprintf(str_buffer[1], str_buffer_size, "reading %lu of %lu x 4k", i, benchmark_count);
        set_text({str_buffer[0], str_buffer[1]});
        delay(100);
    }

    if(!fs_api->file.close(&file)) {
        set_error({"cannot close file ", "in hash test"});
    }

    free(read_buffer);

    snprintf(str_buffer[1], str_buffer_size, "hash in sdcard 0x%lx", sdcard_data_hash);
    if(mcu_data_hash == sdcard_data_hash) {
        snprintf(str_buffer[2], str_buffer_size, "hashes are equal, press OK");
        set_text(
            {static_cast<const char*>(str_buffer[0]),
             static_cast<const char*>(str_buffer[1]),
             "",
             "",
             "",
             static_cast<const char*>(str_buffer[2])});
    } else {
        snprintf(str_buffer[2], str_buffer_size, "hash error, press BACK to exit");
        set_error(
            {static_cast<const char*>(str_buffer[0]),
             static_cast<const char*>(str_buffer[1]),
             "",
             "",
             "",
             static_cast<const char*>(str_buffer[2])});
    }

    blink_green();

    wait_for_button(InputKeyOk);
}

void SdTest::cli_read_benchmark(Cli* cli, string_t args, void* _ctx) {
    SdTest* _this = static_cast<SdTest*>(_ctx);

    const uint32_t benchmark_data_size = 16384 * 8;
    uint32_t bytes_written;
    uint32_t benchmark_bps = 0;
    File file;

    const uint32_t b1_size = 1;
    const uint32_t b8_size = 8;
    const uint32_t b32_size = 32;
    const uint32_t b256_size = 256;
    const uint32_t b4096_size = 4096;

    const uint8_t str_buffer_size = 64;
    char str_buffer[str_buffer_size];

    printf("preparing benchmark data\r\n");
    bool data_prepared = _this->prepare_benchmark_data();
    if(data_prepared) {
        printf("benchmark data prepared\r\n");
    } else {
        printf("error: cannot allocate buffer for benchmark data\r\n");
    }

    // prepare data for read test
    printf("prepare data for read speed test, procedure can be lengthy, please wait\r\n");

    if(!_this->fs_api->file.open(&file, "read.test", FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        printf("error: cannot open file in prepare read\r\n");
    }

    for(size_t i = 0; i < benchmark_data_size / b4096_size; i++) {
        bytes_written = _this->fs_api->file.write(&file, benchmark_data, b4096_size);
        if(bytes_written != b4096_size || file.error_id != FSE_OK) {
            printf("error: cannot write to file in prepare read\r\n");
        }
    }

    if(!_this->fs_api->file.close(&file)) {
        printf("error: cannot close file in prepare read\r\n");
    }

    // test start
    printf("read speed test, procedure can be lengthy, please wait\r\n");

    // open file
    if(!_this->fs_api->file.open(&file, "read.test", FSAM_READ, FSOM_OPEN_EXISTING)) {
        printf("error: cannot open file in read benchmark\r\n");
    }

    // 1b test
    benchmark_bps =
        _this->read_benchmark_internal(b1_size, benchmark_data_size / b1_size, &file, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 1-byte read test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "1-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 8b test
    benchmark_bps =
        _this->read_benchmark_internal(b8_size, benchmark_data_size / b8_size, &file, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 8-byte read test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "8-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 32b test
    benchmark_bps =
        _this->read_benchmark_internal(b32_size, benchmark_data_size / b32_size, &file, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 32-byte read test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "32-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 256b test
    benchmark_bps =
        _this->read_benchmark_internal(b256_size, benchmark_data_size / b256_size, &file, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 256-byte read test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "256-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 4096b test
    benchmark_bps =
        _this->read_benchmark_internal(b4096_size, benchmark_data_size / b4096_size, &file, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 4096-byte read test\r\n");
    } else {
        snprintf(
            str_buffer, str_buffer_size, "4096-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // close file
    if(!_this->fs_api->file.close(&file)) {
        printf("error: cannot close file\r\n");
    }

    _this->free_benchmark_data();

    printf("test completed\r\n");
}

void SdTest::cli_write_benchmark(Cli* cli, string_t args, void* _ctx) {
    SdTest* _this = static_cast<SdTest*>(_ctx);

    const uint32_t b1_size = 1;
    const uint32_t b8_size = 8;
    const uint32_t b32_size = 32;
    const uint32_t b256_size = 256;
    const uint32_t b4096_size = 4096;

    const uint32_t benchmark_data_size = 16384 * 4;

    uint32_t benchmark_bps = 0;

    const uint8_t str_buffer_size = 64;
    char str_buffer[str_buffer_size];

    printf("preparing benchmark data\r\n");
    bool data_prepared = _this->prepare_benchmark_data();
    if(data_prepared) {
        printf("benchmark data prepared\r\n");
    } else {
        printf("error: cannot allocate buffer for benchmark data\r\n");
    }

    printf("write speed test, procedure can be lengthy, please wait\r\n");

    // 1b test
    benchmark_bps = _this->write_benchmark_internal(b1_size, benchmark_data_size / b1_size, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 1-byte write test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "1-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 8b test
    benchmark_bps = _this->write_benchmark_internal(b8_size, benchmark_data_size / b8_size, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 8-byte write test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "8-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 32b test
    benchmark_bps =
        _this->write_benchmark_internal(b32_size, benchmark_data_size / b32_size, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 32-byte write test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "32-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 256b test
    benchmark_bps =
        _this->write_benchmark_internal(b256_size, benchmark_data_size / b256_size, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 256-byte write test\r\n");
    } else {
        snprintf(str_buffer, str_buffer_size, "256-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    // 4096b test
    benchmark_bps =
        _this->write_benchmark_internal(b4096_size, benchmark_data_size / b4096_size, true);
    if(benchmark_bps == BENCHMARK_ERROR) {
        printf("error: in 4096-byte write test\r\n");
    } else {
        snprintf(
            str_buffer, str_buffer_size, "4096-byte: %lu bytes per second\r\n", benchmark_bps);
        printf(str_buffer);
    }

    _this->free_benchmark_data();

    printf("test completed\r\n");
}

// wait for button press
void SdTest::wait_for_button(InputKey input_button) {
    SdTestEvent event;
    osMessageQueueReset(event_queue);
    while(1) {
        osStatus_t result = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);

        if(result == osOK && event.type == SdTestEvent::EventTypeKey) {
            if(event.value.input.type == InputTypeShort) {
                if(event.value.input.key == InputKeyBack) {
                    exit();
                } else {
                    if(event.value.input.key == input_button) {
                        blink_green();
                        break;
                    } else {
                        blink_red();
                    }
                }
            }
        }
    }
    osMessageQueueReset(event_queue);
}

// ask user to proceed or cancel
bool SdTest::ask(InputKey input_button_cancel, InputKey input_button_ok) {
    bool return_result;
    SdTestEvent event;
    osMessageQueueReset(event_queue);
    while(1) {
        osStatus_t result = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);

        if(result == osOK && event.type == SdTestEvent::EventTypeKey) {
            if(event.value.input.type == InputTypeShort) {
                if(event.value.input.key == InputKeyBack) {
                    exit();
                } else {
                    if(event.value.input.key == input_button_ok) {
                        blink_green();
                        return_result = true;
                        break;
                    } else if(event.value.input.key == input_button_cancel) {
                        blink_green();
                        return_result = false;
                        break;
                    } else {
                        blink_red();
                    }
                }
            }
        }
    }
    osMessageQueueReset(event_queue);
    return return_result;
}

// blink red led
void SdTest::blink_red() {
    notification_message(notification, &sequence_blink_red_100);
}

// blink green led
void SdTest::blink_green() {
    notification_message(notification, &sequence_blink_green_100);
}

// set text, but with infinite loop
template <class T> void SdTest::set_error(std::initializer_list<T> list) {
    set_text(list);
    blink_red();
    wait_for_button(InputKeyBack);
    exit();
}

// set text, sort of variadic function
template <class T> void SdTest::set_text(std::initializer_list<T> list) {
    uint8_t line_position = 0;
    acquire_state();
    printf("------------------------\r\n");

    // set line strings from args
    for(auto element : list) {
        state.line[line_position] = element;
        printf("%s\n", element);
        line_position++;
        if(line_position == state.lines_count) break;
    }

    // set empty lines
    for(; line_position < state.lines_count; line_position++) {
        state.line[line_position] = "";
        printf("\r\n");
    }

    printf("------------------------\r\n");
    release_state();
    update_gui();
}

// render app
void SdTest::render(Canvas* canvas) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < state.lines_count; i++) {
        canvas_draw_str(canvas, 0, (i + 1) * 10, state.line[i]);
    }
}

// app enter function
extern "C" int32_t sd_card_test(void* p) {
    SdTest* app = new SdTest();
    app->run();
    return 0;
}