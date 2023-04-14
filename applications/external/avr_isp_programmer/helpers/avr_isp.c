#include "avr_isp.h"
#include "../lib/driver/avr_isp_prog_cmd.h"
#include "../lib/driver/avr_isp_spi_sw.h"

#include <furi.h>

#define AVR_ISP_PROG_TX_RX_BUF_SIZE 320
#define TAG "AvrIsp"

struct AvrIsp {
    AvrIspSpiSw* spi;
    bool pmode;
    AvrIspCallback callback;
    void* context;
};

AvrIsp* avr_isp_alloc(void) {
    AvrIsp* instance = malloc(sizeof(AvrIsp));
    return instance;
}

void avr_isp_free(AvrIsp* instance) {
    furi_assert(instance);

    if(instance->spi) avr_isp_end_pmode(instance);
    free(instance);
}

void avr_isp_set_tx_callback(AvrIsp* instance, AvrIspCallback callback, void* context) {
    furi_assert(instance);
    furi_assert(context);

    instance->callback = callback;
    instance->context = context;
}

uint8_t avr_isp_spi_transaction(
    AvrIsp* instance,
    uint8_t cmd,
    uint8_t addr_hi,
    uint8_t addr_lo,
    uint8_t data) {
    furi_assert(instance);

    avr_isp_spi_sw_txrx(instance->spi, cmd);
    avr_isp_spi_sw_txrx(instance->spi, addr_hi);
    avr_isp_spi_sw_txrx(instance->spi, addr_lo);
    return avr_isp_spi_sw_txrx(instance->spi, data);
}

static bool avr_isp_set_pmode(AvrIsp* instance, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    furi_assert(instance);

    uint8_t res = 0;
    avr_isp_spi_sw_txrx(instance->spi, a);
    avr_isp_spi_sw_txrx(instance->spi, b);
    res = avr_isp_spi_sw_txrx(instance->spi, c);
    avr_isp_spi_sw_txrx(instance->spi, d);
    return res == 0x53;
}

void avr_isp_end_pmode(AvrIsp* instance) {
    furi_assert(instance);

    if(instance->pmode) {
        avr_isp_spi_sw_res_set(instance->spi, true);
        // We're about to take the target out of reset
        // so configure SPI pins as input
        if(instance->spi) avr_isp_spi_sw_free(instance->spi);
        instance->spi = NULL;
    }

    instance->pmode = false;
}

static bool avr_isp_start_pmode(AvrIsp* instance, AvrIspSpiSwSpeed spi_speed) {
    furi_assert(instance);

    // Reset target before driving PIN_SCK or PIN_MOSI

    // SPI.begin() will configure SS as output,
    // so SPI master mode is selected.
    // We have defined RESET as pin 10,
    // which for many arduino's is not the SS pin.
    // So we have to configure RESET as output here,
    // (reset_target() first sets the correct level)
    if(instance->spi) avr_isp_spi_sw_free(instance->spi);
    instance->spi = avr_isp_spi_sw_init(spi_speed);

    avr_isp_spi_sw_res_set(instance->spi, false);
    // See avr datasheets, chapter "SERIAL_PRG Programming Algorithm":

    // Pulse RESET after PIN_SCK is low:
    avr_isp_spi_sw_sck_set(instance->spi, false);

    // discharge PIN_SCK, value arbitrally chosen
    furi_delay_ms(20);
    avr_isp_spi_sw_res_set(instance->spi, true);

    // Pulse must be minimum 2 target CPU speed cycles
    // so 100 usec is ok for CPU speeds above 20KHz
    furi_delay_ms(1);

    avr_isp_spi_sw_res_set(instance->spi, false);

    // Send the enable programming command:
    // datasheet: must be > 20 msec
    furi_delay_ms(50);
    if(avr_isp_set_pmode(instance, AVR_ISP_SET_PMODE)) {
        instance->pmode = true;
        return true;
    }
    return false;
}

bool avr_isp_auto_set_spi_speed_start_pmode(AvrIsp* instance) {
    furi_assert(instance);

    AvrIspSpiSwSpeed spi_speed[] = {
        AvrIspSpiSwSpeed1Mhz,
        AvrIspSpiSwSpeed400Khz,
        AvrIspSpiSwSpeed250Khz,
        AvrIspSpiSwSpeed125Khz,
        AvrIspSpiSwSpeed60Khz,
        AvrIspSpiSwSpeed40Khz,
        AvrIspSpiSwSpeed20Khz,
        AvrIspSpiSwSpeed10Khz,
        AvrIspSpiSwSpeed5Khz,
        AvrIspSpiSwSpeed1Khz,
    };
    for(uint8_t i = 0; i < COUNT_OF(spi_speed); i++) {
        if(avr_isp_start_pmode(instance, spi_speed[i])) {
            AvrIspSignature sig = avr_isp_read_signature(instance);
            AvrIspSignature sig_examination = avr_isp_read_signature(instance); //-V656
            uint8_t y = 0;
            while(y < 8) {
                if(memcmp((uint8_t*)&sig, (uint8_t*)&sig_examination, sizeof(AvrIspSignature)) !=
                   0)
                    break;
                sig_examination = avr_isp_read_signature(instance);
                y++;
            }
            if(y == 8) {
                if(spi_speed[i] > AvrIspSpiSwSpeed1Mhz) {
                    if(i < (COUNT_OF(spi_speed) - 1)) {
                        avr_isp_end_pmode(instance);
                        i++;
                        return avr_isp_start_pmode(instance, spi_speed[i]);
                    }
                }
                return true;
            }
        }
    }

    if(instance->spi) {
        avr_isp_spi_sw_free(instance->spi);
        instance->spi = NULL;
    }

    return false;
}

static void avr_isp_commit(AvrIsp* instance, uint16_t addr, uint8_t data) {
    furi_assert(instance);

    avr_isp_spi_transaction(instance, AVR_ISP_COMMIT(addr));
    /* polling flash */
    if(data == 0xFF) {
        furi_delay_ms(5);
    } else {
        /* polling flash */
        uint32_t starttime = furi_get_tick();
        while((furi_get_tick() - starttime) < 30) {
            if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FLASH_HI(addr)) != 0xFF) {
                break;
            };
        }
    }
}

static uint16_t avr_isp_current_page(AvrIsp* instance, uint32_t addr, uint16_t page_size) {
    furi_assert(instance);

    uint16_t page = 0;
    switch(page_size) {
    case 32:
        page = addr & 0xFFFFFFF0;
        break;
    case 64:
        page = addr & 0xFFFFFFE0;
        break;
    case 128:
        page = addr & 0xFFFFFFC0;
        break;
    case 256:
        page = addr & 0xFFFFFF80;
        break;

    default:
        page = addr;
        break;
    }

    return page;
}

static bool avr_isp_flash_write_pages(
    AvrIsp* instance,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);

    size_t x = 0;
    uint16_t page = avr_isp_current_page(instance, addr, page_size);

    while(x < data_size) {
        if(page != avr_isp_current_page(instance, addr, page_size)) {
            avr_isp_commit(instance, page, data[x - 1]);
            page = avr_isp_current_page(instance, addr, page_size);
        }
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_FLASH_LO(addr, data[x++]));
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_FLASH_HI(addr, data[x++]));
        addr++;
    }
    avr_isp_commit(instance, page, data[x - 1]);
    return true;
}

bool avr_isp_erase_chip(AvrIsp* instance) {
    furi_assert(instance);

    bool ret = false;
    if(!instance->pmode) avr_isp_auto_set_spi_speed_start_pmode(instance);
    if(instance->pmode) {
        avr_isp_spi_transaction(instance, AVR_ISP_ERASE_CHIP);
        furi_delay_ms(100);
        avr_isp_end_pmode(instance);
        ret = true;
    }
    return ret;
}

static bool
    avr_isp_eeprom_write(AvrIsp* instance, uint16_t addr, uint8_t* data, uint32_t data_size) {
    furi_assert(instance);

    for(uint16_t i = 0; i < data_size; i++) {
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_EEPROM(addr, data[i]));
        furi_delay_ms(10);
        addr++;
    }
    return true;
}

bool avr_isp_write_page(
    AvrIsp* instance,
    uint32_t mem_type,
    uint32_t mem_size,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);

    bool ret = false;
    switch(mem_type) {
    case STK_SET_FLASH_TYPE:
        if((addr + data_size / 2) <= mem_size) {
            ret = avr_isp_flash_write_pages(instance, addr, page_size, data, data_size);
        }
        break;

    case STK_SET_EEPROM_TYPE:
        if((addr + data_size) <= mem_size) {
            ret = avr_isp_eeprom_write(instance, addr, data, data_size);
        }
        break;

    default:
        furi_crash(TAG " Incorrect mem type.");
        break;
    }

    return ret;
}

static bool avr_isp_flash_read_page(
    AvrIsp* instance,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);

    if(page_size > data_size) return false;
    for(uint16_t i = 0; i < page_size; i += 2) {
        data[i] = avr_isp_spi_transaction(instance, AVR_ISP_READ_FLASH_LO(addr));
        data[i + 1] = avr_isp_spi_transaction(instance, AVR_ISP_READ_FLASH_HI(addr));
        addr++;
    }
    return true;
}

static bool avr_isp_eeprom_read_page(
    AvrIsp* instance,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);

    if(page_size > data_size) return false;
    for(uint16_t i = 0; i < page_size; i++) {
        data[i] = avr_isp_spi_transaction(instance, AVR_ISP_READ_EEPROM(addr));
        addr++;
    }
    return true;
}

bool avr_isp_read_page(
    AvrIsp* instance,
    uint32_t mem_type,
    uint16_t addr,
    uint16_t page_size,
    uint8_t* data,
    uint32_t data_size) {
    furi_assert(instance);

    bool res = false;
    if(mem_type == STK_SET_FLASH_TYPE)
        res = avr_isp_flash_read_page(instance, addr, page_size, data, data_size);
    if(mem_type == STK_SET_EEPROM_TYPE)
        res = avr_isp_eeprom_read_page(instance, addr, page_size, data, data_size);

    return res;
}

AvrIspSignature avr_isp_read_signature(AvrIsp* instance) {
    furi_assert(instance);

    AvrIspSignature signature;
    signature.vendor = avr_isp_spi_transaction(instance, AVR_ISP_READ_VENDOR);
    signature.part_family = avr_isp_spi_transaction(instance, AVR_ISP_READ_PART_FAMILY);
    signature.part_number = avr_isp_spi_transaction(instance, AVR_ISP_READ_PART_NUMBER);
    return signature;
}

uint8_t avr_isp_read_lock_byte(AvrIsp* instance) {
    furi_assert(instance);

    uint8_t data = 0;
    uint32_t starttime = furi_get_tick();
    while((furi_get_tick() - starttime) < 300) {
        data = avr_isp_spi_transaction(instance, AVR_ISP_READ_LOCK_BYTE);
        if(avr_isp_spi_transaction(instance, AVR_ISP_READ_LOCK_BYTE) == data) {
            break;
        };
        data = 0x00;
    }
    return data;
}

bool avr_isp_write_lock_byte(AvrIsp* instance, uint8_t lock) {
    furi_assert(instance);

    bool ret = false;
    if(avr_isp_read_lock_byte(instance) == lock) {
        ret = true;
    } else {
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_LOCK_BYTE(lock));
        /* polling lock byte */
        uint32_t starttime = furi_get_tick();
        while((furi_get_tick() - starttime) < 30) {
            if(avr_isp_spi_transaction(instance, AVR_ISP_READ_LOCK_BYTE) == lock) {
                ret = true;
                break;
            };
        }
    }
    return ret;
}

uint8_t avr_isp_read_fuse_low(AvrIsp* instance) {
    furi_assert(instance);

    uint8_t data = 0;
    uint32_t starttime = furi_get_tick();
    while((furi_get_tick() - starttime) < 300) {
        data = avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_LOW);
        if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_LOW) == data) {
            break;
        };
        data = 0x00;
    }
    return data;
}

bool avr_isp_write_fuse_low(AvrIsp* instance, uint8_t lfuse) {
    furi_assert(instance);

    bool ret = false;
    if(avr_isp_read_fuse_low(instance) == lfuse) {
        ret = true;
    } else {
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_FUSE_LOW(lfuse));
        /* polling fuse */
        uint32_t starttime = furi_get_tick();
        while((furi_get_tick() - starttime) < 30) {
            if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_LOW) == lfuse) {
                ret = true;
                break;
            };
        }
    }
    return ret;
}

uint8_t avr_isp_read_fuse_high(AvrIsp* instance) {
    furi_assert(instance);

    uint8_t data = 0;
    uint32_t starttime = furi_get_tick();
    while((furi_get_tick() - starttime) < 300) {
        data = avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_HIGH);
        if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_HIGH) == data) {
            break;
        };
        data = 0x00;
    }
    return data;
}

bool avr_isp_write_fuse_high(AvrIsp* instance, uint8_t hfuse) {
    furi_assert(instance);

    bool ret = false;
    if(avr_isp_read_fuse_high(instance) == hfuse) {
        ret = true;
    } else {
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_FUSE_HIGH(hfuse));
        /* polling fuse */
        uint32_t starttime = furi_get_tick();
        while((furi_get_tick() - starttime) < 30) {
            if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_HIGH) == hfuse) {
                ret = true;
                break;
            };
        }
    }
    return ret;
}

uint8_t avr_isp_read_fuse_extended(AvrIsp* instance) {
    furi_assert(instance);

    uint8_t data = 0;
    uint32_t starttime = furi_get_tick();
    while((furi_get_tick() - starttime) < 300) {
        data = avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_EXTENDED);
        if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_EXTENDED) == data) {
            break;
        };
        data = 0x00;
    }
    return data;
}

bool avr_isp_write_fuse_extended(AvrIsp* instance, uint8_t efuse) {
    furi_assert(instance);

    bool ret = false;
    if(avr_isp_read_fuse_extended(instance) == efuse) {
        ret = true;
    } else {
        avr_isp_spi_transaction(instance, AVR_ISP_WRITE_FUSE_EXTENDED(efuse));
        /* polling fuse */
        uint32_t starttime = furi_get_tick();
        while((furi_get_tick() - starttime) < 30) {
            if(avr_isp_spi_transaction(instance, AVR_ISP_READ_FUSE_EXTENDED) == efuse) {
                ret = true;
                break;
            };
        }
    }
    return ret;
}

void avr_isp_write_extended_addr(AvrIsp* instance, uint8_t extended_addr) {
    furi_assert(instance);

    avr_isp_spi_transaction(instance, AVR_ISP_EXTENDED_ADDR(extended_addr));
    furi_delay_ms(10);
}