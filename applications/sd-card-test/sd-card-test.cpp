#include "app-template.h"
#include "fatfs/ff.h"
#include "stm32_adafruit_sd.h"
#include "fnv1a-hash.h"

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
    GpioPin* red_led_record;
    GpioPin* green_led_record;
    FATFS sd_fat_fs;
    char sd_path[6];
    const uint32_t benchmark_data_size = 4096;
    uint8_t* benchmark_data;

    // funcs
    void run();
    void render(CanvasApi* canvas);
    template <class T> void set_text(std::initializer_list<T> list);
    template <class T> void set_error(std::initializer_list<T> list);
    const char* fatfs_error_desc(FRESULT res);
    void wait_for_button(Input input_button);
    bool ask(Input input_button_cancel, Input input_button_ok);
    void blink_red();
    void set_red();
    void blink_green();

    // "tests"
    void detect_sd_card();
    void show_warning();
    void init_sd_card();
    bool is_sd_card_formatted();
    void ask_and_format_sd_card();
    void mount_sd_card();
    void format_sd_card();
    void get_sd_card_info();

    void prepare_benchmark_data();
    void free_benchmark_data();
    void write_benchmark();
    uint32_t write_benchmark_internal(const uint32_t size, const uint32_t tcount);

    void read_benchmark();
    uint32_t read_benchmark_internal(const uint32_t size, const uint32_t count, FIL* file);

    void hash_benchmark();
};

// start app
void SdTest::run() {
    // create pin
    GpioPin red_led = led_gpio[0];
    GpioPin green_led = led_gpio[1];

    // TODO open record
    red_led_record = &red_led;
    green_led_record = &green_led;

    // configure pin
    gpio_init(red_led_record, GpioModeOutputOpenDrain);
    gpio_init(green_led_record, GpioModeOutputOpenDrain);

    detect_sd_card();
    show_warning();
    init_sd_card();
    if(!is_sd_card_formatted()) {
        format_sd_card();
    } else {
        ask_and_format_sd_card();
    }
    mount_sd_card();
    get_sd_card_info();
    prepare_benchmark_data();
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
    wait_for_button(InputBack);
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
    while(!hal_gpio_read_sd_detect()) {
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
         "card may be formatted",
         "or data on card may be lost",
         "",
         "press UP DOWN OK to continue"});

    wait_for_button(InputUp);
    wait_for_button(InputDown);
    wait_for_button(InputOk);
}

// init low level driver
void SdTest::init_sd_card() {
    uint8_t bsp_result = BSP_SD_Init();

    // BSP_SD_OK = 0
    if(bsp_result) {
        set_error({"SD card init error", "BSP error"});
    }
    blink_green();
}

// test, if sd card need to be formatted
bool SdTest::is_sd_card_formatted() {
    FRESULT result;
    set_text({"checking if card needs to be formatted"});

    result = f_mount(&sd_fat_fs, sd_path, 1);
    if(result == FR_NO_FILESYSTEM) {
        return false;
    } else {
        return true;
    }
}

void SdTest::ask_and_format_sd_card() {
    set_text({"Want to format sd card?", "", "", "", "", "LEFT to CANCEL | RIGHT to OK"});
    if(ask(InputLeft, InputRight)) {
        format_sd_card();
    }
}

// mount sd card
void SdTest::mount_sd_card() {
    FRESULT result;
    set_text({"mounting sdcard"});

    result = f_mount(&sd_fat_fs, sd_path, 1);
    if(result) {
        set_error({"SD card mount error", fatfs_error_desc(result)});
    }
    blink_green();
}

// format sd card
void SdTest::format_sd_card() {
    FRESULT result;
    BYTE* work_area;

    set_text({"formatting sdcard", "procedure can be lengthy", "please wait"});
    delay(100);

    work_area = static_cast<BYTE*>(malloc(_MAX_SS));
    if(work_area == NULL) {
        set_error({"SD card format error", "cannot allocate memory"});
    }

    result = f_mkfs(sd_path, (FM_FAT | FM_FAT32 | FM_EXFAT), 0, work_area, _MAX_SS);
    free(work_area);

    if(result) {
        set_error({"SD card format error", fatfs_error_desc(result)});
    }

    result = f_setlabel("Flipper SD");
    if(result) {
        set_error({"SD card set label error", fatfs_error_desc(result)});
    }
    blink_green();
}

// get info about sd card, label, sn
// sector, cluster, total and free size
void SdTest::get_sd_card_info() {
    const uint8_t str_buffer_size = 26;
    char str_buffer[4][str_buffer_size];
    char volume_label[128];
    DWORD serial_num;
    FRESULT result;
    FATFS* fs;
    DWORD free_clusters, free_sectors, total_sectors;

    // suppress "'%s' directive output may be truncated" warning about snprintf
    int __attribute__((unused)) snprintf_count = 0;

    // get label and s/n
    result = f_getlabel(sd_path, volume_label, &serial_num);
    if(result) set_error({"f_getlabel error", fatfs_error_desc(result)});

    snprintf_count = snprintf(str_buffer[0], str_buffer_size, "Label: %s", volume_label);
    snprintf(str_buffer[1], str_buffer_size, "S/N: %lu", serial_num);

    set_text(
        {static_cast<const char*>(str_buffer[0]),
         static_cast<const char*>(str_buffer[1]),
         "",
         "",
         "",
         "press OK to continue"});

    blink_green();

    wait_for_button(InputOk);

    // get total and free space
    result = f_getfree(sd_path, &free_clusters, &fs);
    if(result) set_error({"f_getfree error", fatfs_error_desc(result)});

    total_sectors = (fs->n_fatent - 2) * fs->csize;
    free_sectors = free_clusters * fs->csize;

    snprintf(str_buffer[0], str_buffer_size, "Cluster: %d sectors", fs->csize);
    snprintf(str_buffer[1], str_buffer_size, "Sector: %d bytes", fs->ssize);
    snprintf(str_buffer[2], str_buffer_size, "%lu KB total", total_sectors / 1024 * fs->ssize);
    snprintf(str_buffer[3], str_buffer_size, "%lu KB free", free_sectors / 1024 * fs->ssize);

    set_text(
        {static_cast<const char*>(str_buffer[0]),
         static_cast<const char*>(str_buffer[1]),
         static_cast<const char*>(str_buffer[2]),
         static_cast<const char*>(str_buffer[3]),
         "",
         "press OK to continue"});

    blink_green();

    wait_for_button(InputOk);
}

// prepare benchmark data (allocate data in ram)
void SdTest::prepare_benchmark_data() {
    set_text({"preparing benchmark data"});
    benchmark_data = static_cast<uint8_t*>(malloc(benchmark_data_size));

    if(benchmark_data == NULL) {
        set_error({"cannot allocate buffer", "for benchmark data"});
    }

    for(size_t i = 0; i < benchmark_data_size; i++) {
        benchmark_data[i] = static_cast<uint8_t>(i);
    }

    set_text({"benchmark data prepared"});
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

    wait_for_button(InputOk);
}

uint32_t SdTest::write_benchmark_internal(const uint32_t size, const uint32_t count) {
    uint32_t start_tick, stop_tick, benchmark_bps, benchmark_time, bytes_written;
    FRESULT result;
    FIL file;

    const uint8_t str_buffer_size = 32;
    char str_buffer[str_buffer_size];

    result = f_open(&file, "write.test", FA_WRITE | FA_OPEN_ALWAYS);
    if(result) {
        snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
        set_error({"cannot open file ", static_cast<const char*>(str_buffer)});
    }

    start_tick = osKernelGetTickCount();
    for(size_t i = 0; i < count; i++) {
        result = f_write(&file, benchmark_data, size, reinterpret_cast<UINT*>(&bytes_written));
        if(bytes_written != size || result) {
            snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
            set_error({"cannot write to file ", static_cast<const char*>(str_buffer)});
        }
    }
    stop_tick = osKernelGetTickCount();

    result = f_close(&file);
    if(result) {
        snprintf(str_buffer, str_buffer_size, "in %lu-byte write test", size);
        set_error({"cannot close file ", static_cast<const char*>(str_buffer)});
    }

    benchmark_time = stop_tick - start_tick;
    benchmark_bps = (count * size) * osKernelGetTickFreq() / benchmark_time;

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

    FRESULT result;
    FIL file;

    const uint32_t b1_size = 1;
    const uint32_t b8_size = 8;
    const uint32_t b32_size = 32;
    const uint32_t b256_size = 256;
    const uint32_t b4096_size = 4096;

    // prepare data for read test
    set_text({"prepare data", "for read speed test", "procedure can be lengthy", "please wait"});
    delay(100);

    result = f_open(&file, "read.test", FA_WRITE | FA_OPEN_ALWAYS);
    if(result) {
        set_error({"cannot open file ", "in prepare read"});
    }

    for(size_t i = 0; i < benchmark_data_size / b4096_size; i++) {
        result =
            f_write(&file, benchmark_data, b4096_size, reinterpret_cast<UINT*>(&bytes_written));
        if(bytes_written != b4096_size || result) {
            set_error({"cannot write to file ", "in prepare read"});
        }
    }

    result = f_close(&file);
    if(result) {
        set_error({"cannot close file ", "in prepare read"});
    }

    // test start
    set_text({"read speed test", "procedure can be lengthy", "please wait"});
    delay(100);

    // open file
    result = f_open(&file, "read.test", FA_READ | FA_OPEN_EXISTING);
    if(result) {
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
    result = f_close(&file);
    if(result) {
        set_error({"cannot close file ", "in read test"});
    }

    blink_green();

    wait_for_button(InputOk);
}

uint32_t SdTest::read_benchmark_internal(const uint32_t size, const uint32_t count, FIL* file) {
    uint32_t start_tick, stop_tick, benchmark_bps, benchmark_time, bytes_readed;
    FRESULT result;

    const uint8_t str_buffer_size = 32;
    char str_buffer[str_buffer_size];
    uint8_t* read_buffer;

    read_buffer = static_cast<uint8_t*>(malloc(size));

    if(read_buffer == NULL) {
        snprintf(str_buffer, str_buffer_size, "in %lu-byte read test", size);
        set_error({"cannot allocate memory", static_cast<const char*>(str_buffer)});
    }

    f_rewind(file);

    start_tick = osKernelGetTickCount();
    for(size_t i = 0; i < count; i++) {
        result = f_read(file, read_buffer, size, reinterpret_cast<UINT*>(&bytes_readed));
        if(bytes_readed != size || result) {
            snprintf(str_buffer, str_buffer_size, "in %lu-byte read test", size);
            set_error({"cannot read from file ", static_cast<const char*>(str_buffer)});
        }
    }
    stop_tick = osKernelGetTickCount();

    free(read_buffer);

    benchmark_time = stop_tick - start_tick;
    benchmark_bps = (count * size) * osKernelGetTickFreq() / benchmark_time;

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

    FRESULT result;
    FIL file;

    const uint32_t b4096_size = 4096;
    const uint32_t benchmark_count = 20;

    // prepare data for hash test
    set_text({"prepare data", "for hash test"});
    delay(100);

    // write data to test file and calculate hash
    result = f_open(&file, "hash.test", FA_WRITE | FA_OPEN_ALWAYS);
    if(result) {
        set_error({"cannot open file ", "in prepare hash"});
    }

    for(uint32_t i = 0; i < benchmark_count; i++) {
        mcu_data_hash = fnv1a_buffer_hash(benchmark_data, b4096_size, mcu_data_hash);
        result =
            f_write(&file, benchmark_data, b4096_size, reinterpret_cast<UINT*>(&bytes_written));

        if(bytes_written != b4096_size || result) {
            set_error({"cannot write to file ", "in prepare hash"});
        }

        snprintf(str_buffer[0], str_buffer_size, "writing %lu of %lu x 4k", i, benchmark_count);
        set_text({"prepare data", "for hash test", static_cast<const char*>(str_buffer[0])});
        delay(100);
    }

    result = f_close(&file);
    if(result) {
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

    result = f_open(&file, "hash.test", FA_READ | FA_OPEN_EXISTING);
    if(result) {
        set_error({"cannot open file ", "in hash test"});
    }

    for(uint32_t i = 0; i < benchmark_count; i++) {
        result = f_read(&file, read_buffer, b4096_size, reinterpret_cast<UINT*>(&bytes_readed));
        sdcard_data_hash = fnv1a_buffer_hash(read_buffer, b4096_size, sdcard_data_hash);

        if(bytes_readed != b4096_size || result) {
            set_error({"cannot read from file ", "in hash test"});
        }

        snprintf(str_buffer[1], str_buffer_size, "reading %lu of %lu x 4k", i, benchmark_count);
        set_text({str_buffer[0], str_buffer[1]});
        delay(100);
    }

    result = f_close(&file);

    if(result) {
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

    wait_for_button(InputOk);
}

// wait for button press
void SdTest::wait_for_button(Input input_button) {
    SdTestEvent event;
    osMessageQueueReset(event_queue);
    while(1) {
        osStatus_t result = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);

        if(result == osOK && event.type == SdTestEvent::EventTypeKey) {
            if(event.value.input.state == true) {
                if(event.value.input.input == InputBack) {
                    exit();
                } else {
                    if(event.value.input.input == input_button) {
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
bool SdTest::ask(Input input_button_cancel, Input input_button_ok) {
    bool return_result;
    SdTestEvent event;
    osMessageQueueReset(event_queue);
    while(1) {
        osStatus_t result = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);

        if(result == osOK && event.type == SdTestEvent::EventTypeKey) {
            if(event.value.input.state == true) {
                if(event.value.input.input == InputBack) {
                    exit();
                } else {
                    if(event.value.input.input == input_button_ok) {
                        blink_green();
                        return_result = true;
                        break;
                    } else if(event.value.input.input == input_button_cancel) {
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
    gpio_write(red_led_record, 0);
    delay(50);
    gpio_write(red_led_record, 1);
}

// light up red led
void SdTest::set_red() {
    gpio_write(red_led_record, 0);
}

// blink green led
void SdTest::blink_green() {
    gpio_write(green_led_record, 0);
    delay(50);
    gpio_write(green_led_record, 1);
}

// FatFs errors descriptions
const char* SdTest::fatfs_error_desc(FRESULT res) {
    switch(res) {
    case FR_OK:
        return "ok";
        break;
    case FR_DISK_ERR:
        return "low level error";
        break;
    case FR_INT_ERR:
        return "internal error";
        break;
    case FR_NOT_READY:
        return "not ready";
        break;
    case FR_NO_FILE:
        return "no file";
        break;
    case FR_NO_PATH:
        return "no path";
        break;
    case FR_INVALID_NAME:
        return "invalid name";
        break;
    case FR_DENIED:
        return "denied";
        break;
    case FR_EXIST:
        return "already exist";
        break;
    case FR_INVALID_OBJECT:
        return "invalid file/dir obj";
        break;
    case FR_WRITE_PROTECTED:
        return "write protected";
        break;
    case FR_INVALID_DRIVE:
        return "invalid drive";
        break;
    case FR_NOT_ENABLED:
        return "no work area in volume";
        break;
    case FR_NO_FILESYSTEM:
        return "no valid FS volume";
        break;
    case FR_MKFS_ABORTED:
        return "aborted, any problem";
        break;
    case FR_TIMEOUT:
        return "timeout";
        break;
    case FR_LOCKED:
        return "file locked";
        break;
    case FR_NOT_ENOUGH_CORE:
        return "not enough core memory";
        break;
    case FR_TOO_MANY_OPEN_FILES:
        return "too many open files";
        break;
    case FR_INVALID_PARAMETER:
        return "invalid parameter";
        break;

    default:
        return "unknown error";
        break;
    }
}

// set text, but with infinite loop
template <class T> void SdTest::set_error(std::initializer_list<T> list) {
    set_text(list);
    set_red();
    wait_for_button(InputBack);
    exit();
}

// set text, sort of variadic function
template <class T> void SdTest::set_text(std::initializer_list<T> list) {
    uint8_t line_position = 0;
    acquire_state();
    printf("------------------------\n");

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
        printf("\n");
    }

    printf("------------------------\n");
    release_state();
}

// render app
void SdTest::render(CanvasApi* canvas) {
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < state.lines_count; i++) {
        canvas->draw_str(canvas, 0, (i + 1) * 10, state.line[i]);
    }
}

// app enter function
extern "C" void sd_card_test(void* p) {
    SdTest* app = new SdTest();
    app->run();
}