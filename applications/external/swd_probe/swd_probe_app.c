

#include "swd_probe_app.h"
#include "swd_probe_icons.h"
#include "jep106.h"
#include "adi.h"

static void render_callback(Canvas* const canvas, void* cb_ctx);
static bool swd_message_process(AppFSM* ctx);
static uint8_t swd_transfer(AppFSM* const ctx, bool ap, bool write, uint8_t a23, uint32_t* data);
static bool swd_execute_script(AppFSM* const ctx, const char* filename);

static const GpioPin* gpios[] = {
    &gpio_ext_pc0,
    &gpio_ext_pc1,
    &gpio_ext_pc3,
    &gpio_ext_pb2,
    &gpio_ext_pb3,
    &gpio_ext_pa4,
    &gpio_ext_pa6,
    &gpio_ext_pa7};

static const char* gpio_names[] = {"PC0", "PC1", "PC3", "PB2", "PB3", "PA4", "PA6", "PA7"};

/* bit set: clock, else data */
static const uint8_t gpio_direction_mask[6] =
    {0b10101010, 0b01010101, 0b11001100, 0b00110011, 0b11110000, 0b00001111};

const NotificationSequence seq_c_minor = {
    &message_note_c4,
    &message_delay_100,
    &message_sound_off,
    &message_delay_10,

    &message_note_ds4,
    &message_delay_100,
    &message_sound_off,
    &message_delay_10,

    &message_note_g4,
    &message_delay_100,
    &message_sound_off,
    &message_delay_10,

    &message_vibro_on,
    &message_delay_50,
    &message_vibro_off,
    NULL,
};

const NotificationSequence seq_error = {

    &message_vibro_on,
    &message_delay_50,
    &message_vibro_off,

    &message_note_g4,
    &message_delay_100,
    &message_sound_off,
    &message_delay_10,

    &message_note_c4,
    &message_delay_500,
    &message_sound_off,
    &message_delay_10,
    NULL,
};

const NotificationSequence* seq_sounds[] = {&seq_c_minor, &seq_error};

static bool has_multiple_bits(uint8_t x) {
    return (x & (x - 1)) != 0;
}

static uint8_t get_bit_num(uint8_t x) {
    return (uint8_t)__builtin_ctz(x);
}

static const char* gpio_name(uint8_t mask) {
    if(has_multiple_bits(mask)) {
        return "Pxx";
    }
    uint8_t io = get_bit_num(mask);
    if(io >= COUNT(gpio_names)) {
        return "Pxx";
    }

    return gpio_names[io];
}

static void swd_configure_pins(AppFSM* const ctx, bool output) {
    if(ctx->mode_page > ModePageFound && ctx->io_num_swc < 8 && ctx->io_num_swd < 8) {
        furi_hal_gpio_init(
            gpios[ctx->io_num_swc], GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        if(!output) {
            furi_hal_gpio_init(
                gpios[ctx->io_num_swd], GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
        } else {
            furi_hal_gpio_init(
                gpios[ctx->io_num_swd], GpioModeOutputOpenDrain, GpioPullUp, GpioSpeedVeryHigh);
        }
        return;
    }

    for(int io = 0; io < 8; io++) {
        uint8_t bitmask = 1 << io;

        /* if neither candidate for SWC nor SWD then skip */
        if(!(ctx->io_swc & bitmask) && !(ctx->io_swd & bitmask)) {
            furi_hal_gpio_init(gpios[io], GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
            continue;
        }

        if(ctx->current_mask & bitmask) {
            /* set for clock */
            furi_hal_gpio_init(gpios[io], GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        } else {
            /* set for data */
            if(!output) {
                furi_hal_gpio_init(gpios[io], GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
            } else {
                furi_hal_gpio_init(
                    gpios[io], GpioModeOutputOpenDrain, GpioPullUp, GpioSpeedVeryHigh);
            }
        }
    }
}

static void swd_set_clock(AppFSM* const ctx, const uint8_t level) {
    if(ctx->mode_page > ModePageFound && ctx->io_num_swc < 8) {
        furi_hal_gpio_write(gpios[ctx->io_num_swc], level);
        return;
    }

    for(int io = 0; io < 8; io++) {
        uint8_t bitmask = 1 << io;

        /* if no candidate for SWC then skip */
        if(!(ctx->io_swc & bitmask)) {
            continue;
        }

        if(ctx->current_mask & bitmask) {
            furi_hal_gpio_write(gpios[io], level);
        }
    }
}

static void swd_set_data(AppFSM* const ctx, const uint8_t level) {
    if(ctx->mode_page > ModePageFound && ctx->io_num_swd < 8) {
        furi_hal_gpio_write(gpios[ctx->io_num_swd], level);
        return;
    }

    for(int io = 0; io < 8; io++) {
        uint8_t bitmask = 1 << io;

        /* if no candidate for SWD then skip */
        if(!(ctx->io_swd & bitmask)) {
            continue;
        }

        if(!(ctx->current_mask & bitmask)) {
            furi_hal_gpio_write(gpios[io], level);
        }
    }
}

static uint8_t swd_get_data(AppFSM* const ctx) {
    if(ctx->mode_page > ModePageFound && ctx->io_num_swd < 8) {
        return furi_hal_gpio_read(gpios[ctx->io_num_swd]);
    }

    uint8_t bits = 0;
    for(int io = 0; io < 8; io++) {
        uint8_t bitmask = 1 << io;

        /* if no candidate for SWD then skip */
        if(!(ctx->io_swd & bitmask)) {
            continue;
        }
        bits |= furi_hal_gpio_read(gpios[io]) ? bitmask : 0;
    }
    return bits;
}

static void swd_clock_delay(AppFSM* const ctx) {
    if(ctx->swd_clock_delay) {
        furi_delay_us(ctx->swd_clock_delay);
    }
}

static void swd_write_bit(AppFSM* const ctx, bool level) {
    swd_set_clock(ctx, 0);
    swd_set_data(ctx, level);
    swd_clock_delay(ctx);
    swd_set_clock(ctx, 1);
    swd_clock_delay(ctx);
    swd_set_clock(ctx, 0);
}

static uint8_t swd_read_bit(AppFSM* const ctx) {
    swd_set_clock(ctx, 1);
    swd_clock_delay(ctx);
    swd_set_clock(ctx, 0);
    uint8_t bits = swd_get_data(ctx);
    swd_clock_delay(ctx);
    swd_set_clock(ctx, 1);

    return bits;
}

/* send a byte or less LSB-first */
static void swd_write_byte(AppFSM* const ctx, const uint8_t data, size_t bits) {
    for(size_t pos = 0; pos < bits; pos++) {
        swd_write_bit(ctx, data & (1 << pos));
    }
}

/* send a sequence of bytes LSB-first */
static void swd_write(AppFSM* const ctx, const uint8_t* data, size_t bits) {
    size_t byte_pos = 0;
    while(bits > 0) {
        size_t remain = (bits > 8) ? 8 : bits;
        swd_write_byte(ctx, data[byte_pos++], remain);
        bits -= remain;
    }
}

static uint8_t swd_transfer(AppFSM* const ctx, bool ap, bool write, uint8_t a23, uint32_t* data) {
    //notification_message(ctx->notification, &sequence_set_blue_255);
    //notification_message(ctx->notification, &sequence_reset_red);

    swd_set_data(ctx, false);
    swd_configure_pins(ctx, true);

    uint32_t idle = 0;
    swd_write(ctx, (uint8_t*)&idle, ctx->swd_idle_bits);

    uint8_t request[] = {0};

    request[0] |= 0x01; /* start bit*/
    request[0] |= ap ? 0x02 : 0; /* APnDP */
    request[0] |= write ? 0 : 0x04; /* operation */
    request[0] |= (a23 & 0x01) ? 0x08 : 0; /* A[2:3] */
    request[0] |= (a23 & 0x02) ? 0x10 : 0; /* A[2:3] */
    request[0] |= 0x80; /* park bit */
    request[0] |= __builtin_parity(request[0]) ? 0x20 : 0; /* parity */

    swd_write(ctx, request, sizeof(request) * 8);

    /* turnaround cycle */
    swd_configure_pins(ctx, false);

    uint8_t ack = 0;

    /* receive 3 ACK bits */
    for(int pos = 0; pos < 3; pos++) {
        ack >>= 1;
        ack |= swd_read_bit(ctx) ? 0x04 : 0;
    }

    /* force ABORT/CTRL to always work */
    if(!ap && a23 == 0) {
        ack = 1;
    }

    if(ack != 0x01) {
        //notification_message(ctx->notification, &sequence_reset_blue);
        //notification_message(ctx->notification, &sequence_set_red_255);
        return ack;
    }

    if(write) {
        swd_write_bit(ctx, 0);
        swd_configure_pins(ctx, true);

        /* send 32 WDATA bits */
        for(int pos = 0; pos < 32; pos++) {
            swd_write_bit(ctx, *data & (1 << pos));
        }

        /* send parity bit */
        swd_write_bit(ctx, __builtin_parity(*data));
    } else {
        *data = 0;
        /* receive 32 RDATA bits */
        for(int pos = 0; pos < 32; pos++) {
            *data >>= 1;
            *data |= swd_read_bit(ctx) ? 0x80000000 : 0;
        }

        /* receive parity bit */
        bool parity = swd_read_bit(ctx);

        if(parity != __builtin_parity(*data)) {
            //notification_message(ctx->notification, &sequence_reset_blue);
            //notification_message(ctx->notification, &sequence_set_red_255);
            return 8;
        }
    }
    swd_set_data(ctx, false);
    swd_configure_pins(ctx, true);
    //notification_message(ctx->notification, &sequence_reset_blue);

    return ack;
}

/* A line reset is achieved by holding the data signal HIGH for at least 50 clock cycles, followed by at least two idle cycles. */
static void swd_line_reset(AppFSM* const ctx) {
    //notification_message(ctx->notification, &sequence_set_red_255);
    for(int bitcount = 0; bitcount < 50; bitcount += 8) {
        swd_write_byte(ctx, 0xFF, 8);
    }
    swd_write_byte(ctx, 0, 8);
    ctx->dp_regs.select_ok = false;
    //notification_message(ctx->notification, &sequence_reset_red);
}

static void swd_abort(AppFSM* const ctx) {
    uint32_t dpidr;

    /* first reset the line */
    swd_line_reset(ctx);
    swd_transfer(ctx, false, false, 0, &dpidr);
    uint32_t abort = 0x0E;
    swd_transfer(ctx, false, true, 0, &abort);
}

static void swd_abort_simple(AppFSM* const ctx) {
    uint32_t abort = 0x0E;
    swd_transfer(ctx, false, true, 0, &abort);

    uint32_t dpidr;
    if(swd_transfer(ctx, false, false, 0, &dpidr) != 1) {
        swd_abort(ctx);
    }
}

static uint8_t swd_select(AppFSM* const ctx, uint8_t ap_sel, uint8_t ap_bank, uint8_t dp_bank) {
    uint32_t bank_reg = (ap_sel << 24) | ((ap_bank & 0x0F) << 4) | (dp_bank & 0x0F);

    if(ctx->dp_regs.select_ok && bank_reg == ctx->dp_regs.select) {
        return 1;
    }

    uint8_t ret = swd_transfer(ctx, false, true, REG_SELECT, &bank_reg);
    if(ret != 1) {
        ctx->dp_regs.select_ok = false;
        DBG("failed: %d", ret);
        return ret;
    }

    ctx->dp_regs.select = bank_reg;
    ctx->dp_regs.select_ok = true;
    return ret;
}

static uint8_t
    swd_read_dpbank(AppFSM* const ctx, uint8_t dp_off, uint8_t dp_bank, uint32_t* data) {
    uint8_t ret = 0;

    /* select target bank */
    if(dp_bank < 0x10) {
        uint8_t ret = swd_select(ctx, 0, 0, dp_bank);
        if(ret != 1) {
            DBGS("swd_select failed");
            return ret;
        }
    }

    /* read data from it */
    *data = 0;
    ret = swd_transfer(ctx, false, false, dp_off, data);
    if(ret != 1) {
        DBG("failed: %d", ret);
        return ret;
    }
    return ret;
}

static uint8_t
    swd_write_dpbank(AppFSM* const ctx, uint8_t dp_off, uint8_t dp_bank, uint32_t* data) {
    uint8_t ret = 0;

    /* select target bank */
    if(dp_bank < 0x10) {
        ret = swd_select(ctx, 0, 0, dp_bank);
        if(ret != 1) {
            DBGS("swd_select failed");
            return ret;
        }
    }

    /* write it */
    ret = swd_transfer(ctx, false, true, dp_off, data);
    if(ret != 1) {
        DBG("failed: %d", ret);
        return ret;
    }
    return ret;
}

static uint8_t swd_read_ap(AppFSM* const ctx, uint8_t ap, uint8_t ap_off, uint32_t* data) {
    /* select target bank */
    uint8_t ret = swd_select(ctx, ap, (ap_off >> 4) & 0x0F, 0);
    if(ret != 1) {
        DBGS("swd_select failed");
        return ret;
    }
    ret = swd_transfer(ctx, true, false, (ap_off >> 2) & 3, data);
    *data = 0;
    ret = swd_transfer(ctx, true, false, (ap_off >> 2) & 3, data);
    if(ret != 1) {
        DBG("failed: %d", ret);
        return ret;
    }
    return ret;
}

static uint8_t swd_read_ap_single(AppFSM* const ctx, uint8_t ap, uint8_t ap_off, uint32_t* data) {
    uint8_t ret = swd_select(ctx, ap, (ap_off >> 4) & 0x0F, 0);
    if(ret != 1) {
        DBGS("swd_select failed");
        return ret;
    }
    *data = 0;
    ret = swd_transfer(ctx, true, false, (ap_off >> 2) & 3, data);
    if(ret != 1) {
        DBG("failed: %d", ret);
        return ret;
    }
    return ret;
}

static uint8_t swd_write_ap(AppFSM* const ctx, uint8_t ap, uint8_t ap_off, uint32_t data) {
    uint8_t ret = swd_select(ctx, ap, (ap_off >> 4) & 0x0F, 0);
    if(ret != 1) {
        DBGS("swd_select failed");
        return ret;
    }
    ret = swd_transfer(ctx, true, true, (ap_off >> 2) & 3, &data);
    if(ret != 1) {
        DBG("failed: %d", ret);
        return ret;
    }
    return ret;
}

static uint8_t swd_write_memory(AppFSM* const ctx, uint8_t ap, uint32_t address, uint32_t data) {
    uint8_t ret = 0;
    uint32_t csw = 0x23000002;

    ret |= swd_write_ap(ctx, ap, MEMAP_CSW, csw);
    ret |= swd_write_ap(ctx, ap, MEMAP_TAR, address);
    ret |= swd_write_ap(ctx, ap, MEMAP_DRW, data);
    DBG("write 0x%08lX to 0x%08lX", data, address);

    if(ret != 1) {
        swd_abort(ctx);
    }
    return ret;
}

uint8_t swd_read_memory(AppFSM* const ctx, uint8_t ap, uint32_t address, uint32_t* data) {
    uint8_t ret = 0;
    uint32_t csw = 0x23000002;

    ret |= swd_write_ap(ctx, ap, MEMAP_CSW, csw);
    ret |= swd_write_ap(ctx, ap, MEMAP_TAR, address);
    ret |= swd_read_ap(ctx, ap, MEMAP_DRW, data);

    if(ret != 1) {
        DBG("read from 0x%08lX failed", address);
        swd_abort(ctx);
    } else {
        DBG("read 0x%08lX from 0x%08lX", *data, address);
    }
    return ret;
}

static uint8_t swd_read_memory_block(
    AppFSM* const ctx,
    uint8_t ap,
    uint32_t address,
    uint8_t* buf,
    uint32_t len) {
    uint8_t ret = 0;
    uint32_t data = 0;
    uint32_t csw = 0x23000012;

    ret |= swd_write_ap(ctx, ap, MEMAP_CSW, csw);
    ret |= swd_write_ap(ctx, ap, MEMAP_TAR, address);
    ret |= swd_read_ap_single(ctx, ap, MEMAP_DRW, &data);

    for(size_t pos = 0; pos < len; pos += 4) {
        data = 0xDEADBEEF;
        ret |= swd_read_ap_single(ctx, ap, MEMAP_DRW, &data);
        DBG("read %lX", data);

        memcpy(&buf[pos], &data, 4);

        if(ret != 1) {
            swd_abort(ctx);
            return ret;
        }
    }
    return ret;
}

static uint32_t swd_detect(AppFSM* const ctx) {
    swd_set_data(ctx, false);
    swd_configure_pins(ctx, true);

    uint8_t data[] = {0xA5};
    swd_write(ctx, data, sizeof(data) * 8);

    /* turnaround cycle */
    swd_configure_pins(ctx, false);

    uint8_t ack_bits[3];
    uint8_t rdata[32];

    /* receive 3 ACK bits */
    for(int pos = 0; pos < 3; pos++) {
        ack_bits[pos] = swd_read_bit(ctx);
    }

    /* receive 32 RDATA bits */
    for(int pos = 0; pos < 32; pos++) {
        rdata[pos] = swd_read_bit(ctx);
    }

    /* receive parity bit */
    uint8_t parity = swd_read_bit(ctx);

    for(int io = 0; io < 8; io++) {
        uint8_t bitmask = 1 << io;

        /* skip if it's a clock */
        if(ctx->current_mask & bitmask) {
            continue;
        }

        uint8_t ack = 0;
        for(int pos = 0; pos < 3; pos++) {
            ack >>= 1;
            ack |= (ack_bits[pos] & bitmask) ? 4 : 0;
        }

        uint32_t dpidr = 0;
        for(int pos = 0; pos < 32; pos++) {
            dpidr >>= 1;
            dpidr |= (rdata[pos] & bitmask) ? 0x80000000 : 0;
        }

        if(ack == 1 && dpidr != 0 && dpidr != 0xFFFFFFFF) {
            bool received_parity = (parity & bitmask);
            if(__builtin_parity(dpidr) == received_parity) {
                ctx->dp_regs.dpidr = dpidr;
                ctx->dp_regs.dpidr_ok = true;
                ctx->detected = true;
                ctx->io_swd = bitmask;
                ctx->io_swc &= ctx->current_mask;
                LOG("swd_detect: data: %08lX, io_swd %02X, io_swc %02X",
                    dpidr,
                    ctx->io_swd,
                    ctx->io_swc);

                if(!has_multiple_bits(ctx->io_swc)) {
                    ctx->io_num_swd = get_bit_num(ctx->io_swd);
                    ctx->io_num_swc = get_bit_num(ctx->io_swc);
                }
            }
        }
    }
    swd_set_data(ctx, false);
    swd_configure_pins(ctx, true);

    return 0;
}

static void swd_scan(AppFSM* const ctx) {
    /* To switch SWJ-DP from JTAG to SWD operation:
        1. Send at least 50 SWCLKTCK cycles with SWDIOTMS HIGH. This ensures that the current interface is in its reset state. The JTAG interface only detects the 16-bit JTAG-to-SWD sequence starting from the Test-Logic-Reset state.
        2. Send the 16-bit JTAG-to-SWD select sequence 0x79e7 on SWDIOTMS.
        3. Send at least 50 SWCLKTCK cycles with SWDIOTMS HIGH. This ensures that if SWJ-DP was already in SWD operation before sending the select sequence, the SWD interface enters line reset state.
    */
    swd_configure_pins(ctx, true);

    /* reset JTAG interface */
    for(int bitcount = 0; bitcount < 50; bitcount += 8) {
        swd_write_byte(ctx, 0xFF, 8);
    }

    /* Send the 16-bit JTAG-to-SWD select sequence */
    swd_write_byte(ctx, 0x9E, 8);
    swd_write_byte(ctx, 0xE7, 8);

    /* resynchronize SWD */
    swd_line_reset(ctx);

    swd_detect(ctx);
}

static bool swd_ensure_powerup(AppFSM* const ctx) {
    bool ret = true;

    if(!(ctx->dp_regs.ctrlstat & (CSYSPWRUPREQ | CDBGPWRUPREQ))) {
        DBGS("no (CSYSPWRUPREQ | CDBGPWRUPREQ)");

        /* fetch current CTRL/STAT */
        DBGS(" - Fetch CTRL/STAT");
        ctx->dp_regs.ctrlstat_ok =
            swd_read_dpbank(ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat) == 1;
        DBG("     %08lX %s", ctx->dp_regs.ctrlstat, ctx->dp_regs.ctrlstat_ok ? "OK" : "FAIL");
        /* enable requests */
        ctx->dp_regs.ctrlstat |= (CSYSPWRUPREQ | CDBGPWRUPREQ);

        swd_write_dpbank(ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat);

        ret = false;
    }
    if(!(ctx->dp_regs.ctrlstat & CDBGPWRUPACK)) {
        DBGS("no CDBGPWRUPACK");
        /* fetch current CTRL/STAT */
        swd_read_dpbank(ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat);
        ret = false;
    }

    if(!ret) {
        DBGS(" - Fetch CTRL/STAT");
        ctx->dp_regs.ctrlstat_ok =
            swd_read_dpbank(ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat) == 1;
        DBG("     %08lX %s", ctx->dp_regs.ctrlstat, ctx->dp_regs.ctrlstat_ok ? "OK" : "FAIL");
    }

    return ret;
}

static void swd_apscan_reset(AppFSM* const ctx) {
    for(size_t reset_ap = 0; reset_ap < COUNT(ctx->apidr_info); reset_ap++) {
        ctx->apidr_info[reset_ap].tested = false;
    }
}

static bool swd_apscan_test(AppFSM* const ctx, uint32_t ap) {
    furi_assert(ctx);
    furi_assert(ap < sizeof(ctx->apidr_info));
    bool ret = true;

    ctx->apidr_info[ap].tested = true;

    uint32_t data = 0;
    if(swd_read_ap(ctx, ap, AP_IDR, &data) != 1) {
        swd_abort(ctx);
        return false;
    }
    if(data == 0) {
        return false;
    }
    DBG("AP%lu detected", ap);
    ctx->apidr_info[ap].ok = true;
    ctx->apidr_info[ap].revision = (data >> 24) & 0x0F;
    ctx->apidr_info[ap].designer = (data >> 17) & 0x3FF;
    ctx->apidr_info[ap].class = (data >> 13) & 0x0F;
    ctx->apidr_info[ap].variant = (data >> 4) & 0x0F;
    ctx->apidr_info[ap].type = (data >> 0) & 0x0F;

    if(swd_read_ap(ctx, ap, AP_BASE, &ctx->apidr_info[ap].base) != 1) {
        swd_abort(ctx);
        ret = false;
    }
    return ret;
}

/**************************  script helpers  **************************/

static void swd_script_log(ScriptContext* ctx, FuriLogLevel level, const char* format, ...) {
    bool commandline = false;
    ScriptContext* cur = ctx;
    char buffer[256];
    va_list argp;
    va_start(argp, format);

    do {
        if(cur == ctx->app->commandline) {
            commandline = true;
        }
        cur = cur->parent;
    } while(cur);

    if(commandline) {
        const char* prefix = "";

        switch(level) {
        case FuriLogLevelWarn:
            prefix = "Warning: ";
            break;
        case FuriLogLevelError:
            prefix = "ERROR: ";
            break;
        default:
            break;
        }

        strcpy(buffer, prefix);
        size_t pos = strlen(buffer);
        vsnprintf(&buffer[pos], sizeof(buffer) - pos - 2, format, argp);
        strcat(buffer, "\n");
        if(!usb_uart_tx_data(ctx->app->uart, (uint8_t*)buffer, strlen(buffer))) {
            DBGS("Sending via USB failed");
        }
    } else {
        LOG(buffer);
    }
    va_end(argp);
}

/* read characters until newline was read */
static bool swd_script_seek_newline(ScriptContext* ctx) {
    while(true) {
        uint8_t ch = 0;

        if(ctx->script_file) {
            if(storage_file_read(ctx->script_file, &ch, 1) != 1) {
                return false;
            }
        } else {
            ch = ctx->line_data[ctx->line_pos];
            if(ch == 0) {
                return false;
            }
            ctx->line_pos++;
        }
        if(ch == '\n') {
            return true;
        }
    }
}

/* read whitespaces until the next character is read. 
   returns false if EOF or newline was read */
static bool swd_script_skip_whitespace(ScriptContext* ctx) {
    while(true) {
        uint8_t ch = 0;
        uint64_t start_pos = 0;

        if(ctx->script_file) {
            start_pos = storage_file_tell(ctx->script_file);

            if(storage_file_read(ctx->script_file, &ch, 1) != 1) {
                return false;
            }
        } else {
            start_pos = ctx->line_pos;
            ch = ctx->line_data[ctx->line_pos];

            if(ch == 0) {
                return false;
            }
            ctx->line_pos++;
        }
        if(ch == '\n') {
            return false;
        }
        if(ch != ' ') {
            if(ctx->script_file) {
                storage_file_seek(ctx->script_file, start_pos, true);
            } else {
                ctx->line_pos = start_pos;
            }
            return true;
        }
    }
}

static bool swd_script_get_string(ScriptContext* ctx, char* str, size_t max_length) {
    bool quot = false;
    size_t pos = 0;

    str[pos] = '\000';

    while(true) {
        char ch = 0;
        uint64_t start_pos = 0;

        if(ctx->script_file) {
            start_pos = storage_file_tell(ctx->script_file);

            if(storage_file_read(ctx->script_file, &ch, 1) != 1) {
                DBGS("end reached");
                return false;
            }
        } else {
            start_pos = ctx->line_pos;
            ch = ctx->line_data[ctx->line_pos];

            if(ch == 0) {
                DBGS("end reached");
                return false;
            }
            ctx->line_pos++;
        }

        if(ch == '"') {
            quot = !quot;
            continue;
        }
        if(!quot) {
            if(ch == ' ') {
                break;
            }
            if(ch == '\r' || ch == '\n') {
                if(ctx->script_file) {
                    storage_file_seek(ctx->script_file, start_pos, true);
                } else {
                    ctx->line_pos = start_pos;
                }
                break;
            }
        }
        if(pos + 2 > max_length) {
            DBGS("too long");
            return false;
        }
        str[pos++] = ch;
        str[pos] = '\000';
    }
    DBG("got '%s'", str);

    return true;
}

static bool swd_script_get_number(ScriptContext* ctx, uint32_t* number) {
    char str[16];

    if(!swd_script_get_string(ctx, str, sizeof(str))) {
        DBGS("could not get string");
        return false;
    }
    DBG("got '%s'", str);

    size_t pos = 0;
    *number = 0;

    /* hex number? */
    if(!strncmp(str, "0x", 2)) {
        pos += 2;
        while(str[pos]) {
            uint8_t ch = str[pos++];
            uint8_t ch_num = ch - '0';
            uint8_t ch_hex = (ch & ~0x20) - 'A';

            *number <<= 4;

            if(ch_num <= 10) {
                *number += ch_num;
            } else if(ch_hex <= 5) {
                *number += 10 + ch_hex;
            } else {
                return false;
            }
        }
    } else {
        while(str[pos]) {
            uint8_t ch = str[pos++];
            uint8_t ch_num = ch - '0';

            *number *= 10;

            if(ch_num < 10) {
                *number += ch_num;
            } else {
                return false;
            }
        }
    }

    return true;
}

static void swd_script_gui_refresh(ScriptContext* ctx) {
    if(furi_message_queue_get_count(ctx->app->event_queue) > 0) {
        swd_message_process(ctx->app);
    }
    if(!ctx->status_ignore) {
        DBG("Status: %s", ctx->app->state_string);
        view_port_update(ctx->app->view_port);
    }
}

/************************** script functions **************************/

static bool swd_scriptfunc_comment(ScriptContext* ctx) {
    DBGS("comment");

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_label(ScriptContext* ctx) {
    char label[256];
    DBGS("label");

    swd_script_skip_whitespace(ctx);
    if(!swd_script_get_string(ctx, label, sizeof(label))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse label");
        return false;
    }

    if(!strcmp(label, ctx->goto_label)) {
        ctx->goto_active = false;
        DBG("matches '%s'", ctx->goto_label);
    }

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_goto(ScriptContext* ctx) {
    DBGS("goto");

    swd_script_skip_whitespace(ctx);

    if(!swd_script_get_string(ctx, ctx->goto_label, sizeof(ctx->goto_label))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse target label");
        return false;
    }

    /* start from beginning and rerun starting from label */
    ctx->goto_active = true;
    ctx->restart = true;

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_call(ScriptContext* ctx) {
    DBGS("call");

    swd_script_skip_whitespace(ctx);

    /* fetch previous file directory */
    char filename[MAX_FILE_LENGTH];
    strncpy(filename, ctx->filename, sizeof(filename));
    char* path = strrchr(filename, '/');
    path[1] = '\000';

    /* append filename */
    if(!swd_script_get_string(ctx, &path[1], sizeof(filename) - strlen(path))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse filename");
        return false;
    }

    swd_script_seek_newline(ctx);

    /* append extension */
    if(strlen(filename) + 5 >= sizeof(filename)) {
        swd_script_log(ctx, FuriLogLevelError, "name too long");
        return false;
    }

    strcat(filename, ".swd");

    bool ret = swd_execute_script(ctx->app, filename);

    if(!ret) {
        swd_script_log(ctx, FuriLogLevelError, "failed to exec '%s'", filename);
        return false;
    }

    return true;
}

static bool swd_scriptfunc_status(ScriptContext* ctx) {
    uint32_t status = 1;
    DBGS("status");

    swd_script_skip_whitespace(ctx);
    swd_script_get_number(ctx, &status);

    ctx->status_ignore = (status == 0);

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_errors(ScriptContext* ctx) {
    char type[32];
    DBGS("errors");

    swd_script_skip_whitespace(ctx);

    if(!swd_script_get_string(ctx, type, sizeof(type))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse");
        return false;
    }

    if(!strcmp(type, "ignore")) {
        ctx->errors_ignore = true;
    }
    if(!strcmp(type, "fail")) {
        ctx->errors_ignore = false;
    }
    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_beep(ScriptContext* ctx) {
    uint32_t sound = 0;
    DBGS("beep");

    swd_script_skip_whitespace(ctx);
    swd_script_get_number(ctx, &sound);

    notification_message_block(ctx->app->notification, seq_sounds[sound]);

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_message(ScriptContext* ctx) {
    uint32_t wait_time = 0;
    char message[256];
    char type[256];
    bool success = true;
    bool show_dialog = false;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &wait_time)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse wait_time");
        return false;
    }

    if(!swd_script_get_string(ctx, message, sizeof(message))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse message");
        return false;
    }

    if(swd_script_get_string(ctx, type, sizeof(type))) {
        if(!strcmp(type, "dialog")) {
            show_dialog = true;
        }
    }

    if(wait_time <= 60 * 1000) {
        strncpy(ctx->app->state_string, message, sizeof(ctx->app->state_string));
        swd_script_gui_refresh(ctx);
        furi_delay_ms(wait_time);
        if(show_dialog) {
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_header(message, "SWD Probe", 16, 2, AlignLeft, AlignTop);
            dialog_message_set_icon(message, &I_app, 3, 2);
            dialog_message_set_text(message, ctx->app->state_string, 3, 16, AlignLeft, AlignTop);
            dialog_message_set_buttons(message, "Abort", "Ok", NULL);
            success = dialog_message_show(ctx->app->dialogs, message) == DialogMessageButtonCenter;
            dialog_message_free(message);
        }
    }

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_swd_idle_bits(ScriptContext* ctx) {
    uint32_t swd_idle_bits = 0;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &swd_idle_bits)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse");
        return false;
    }

    if(swd_idle_bits <= 32) {
        ctx->app->swd_idle_bits = swd_idle_bits;
    } else {
        swd_script_log(ctx, FuriLogLevelError, "value must be between 1 and 32");
    }

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_swd_clock_delay(ScriptContext* ctx) {
    uint32_t swd_clock_delay = 0;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &swd_clock_delay)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse");
        return false;
    }

    if(swd_clock_delay <= 1000000) {
        ctx->app->swd_clock_delay = swd_clock_delay;
    } else {
        swd_script_log(ctx, FuriLogLevelError, "value must be between 1 and 1000000");
    }

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_maxtries(ScriptContext* ctx) {
    uint32_t max_tries = 0;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &max_tries)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse");
        return false;
    }

    if(max_tries >= 1 && max_tries <= 1024) {
        ctx->max_tries = max_tries;
    } else {
        DBGS("value must be between 1 and 1024");
    }

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_blocksize(ScriptContext* ctx) {
    uint32_t block_size = 0;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &block_size)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse");
        return false;
    }

    if(block_size >= 4 && block_size <= 0x1000) {
        ctx->block_size = block_size;
    } else {
        swd_script_log(ctx, FuriLogLevelError, "value must be between 4 and 4096");
    }

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_apselect(ScriptContext* ctx) {
    uint32_t ap = 0;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &ap)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse AP");
        return false;
    }

    if(!swd_apscan_test(ctx->app, ap)) {
        swd_script_log(ctx, FuriLogLevelError, "no selected AP");
        return false;
    }

    ctx->selected_ap = ap;

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_apscan(ScriptContext* ctx) {
    DBGS("Scanning APs");
    for(uint32_t ap = 0; ap < 255; ap++) {
        snprintf(ctx->app->state_string, sizeof(ctx->app->state_string), "Scan AP %lu", ap);
        swd_script_gui_refresh(ctx);
        if(swd_apscan_test(ctx->app, ap)) {
            DBG("  AP%lu detected", ap);
        }
    }
    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_abort(ScriptContext* ctx) {
    DBGS("Aborting");
    swd_abort(ctx->app);
    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_mem_dump(ScriptContext* ctx) {
    char filename[MAX_FILE_LENGTH];
    uint32_t address = 0;
    uint32_t length = 0;
    uint32_t flags = 0;
    bool success = true;

    /* get file */
    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_string(ctx, filename, sizeof(filename))) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse filename");
        return false;
    }
    /* get address */
    if(!swd_script_get_number(ctx, &address)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse address");
        return false;
    }

    /* get length */
    if(!swd_script_get_number(ctx, &length)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse length");
        return false;
    }

    /* get flags */
    if(swd_script_get_number(ctx, &flags)) {
        DBGS("found extra flags");
    }

    LOG("would dump %08lX, len %08lX into %s", address, length, filename);

    File* dump = storage_file_alloc(ctx->app->storage);

    if(!storage_file_open(dump, filename, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(dump);
        snprintf(ctx->app->state_string, sizeof(ctx->app->state_string), "Failed to create file");
        swd_script_gui_refresh(ctx);
        notification_message_block(ctx->app->notification, &seq_error);
        return false;
    }

    if(ctx->block_size == 0) {
        ctx->block_size = 0x100;
    }
    if(ctx->block_size > 0x1000) {
        ctx->block_size = 0x1000;
    }

    uint8_t* buffer = malloc(ctx->block_size);

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    for(uint32_t pos = 0; pos < length; pos += ctx->block_size) {
        if((pos & 0xFF) == 0) {
            int pct = pos * 100 / length;
            snprintf(
                ctx->app->state_string,
                sizeof(ctx->app->state_string),
                "Dump %08lX (%d%%)",
                pos,
                pct);
            swd_script_gui_refresh(ctx);
        }

        bool read_ok = false;

        for(uint32_t tries = 0; tries < ctx->max_tries; tries++) {
            if(ctx->abort) {
                DBGS("aborting read");
                break;
            }
            uint32_t ret = 0;

            if(ctx->block_size > 4) {
                ret = swd_read_memory_block(
                    ctx->app, ctx->selected_ap, address + pos, buffer, ctx->block_size);
            } else {
                ret =
                    swd_read_memory(ctx->app, ctx->selected_ap, address + pos, (uint32_t*)buffer);
            }
            read_ok = (ret == 1);

            if(!read_ok) {
                snprintf(
                    ctx->app->state_string,
                    sizeof(ctx->app->state_string),
                    "Failed at 0x%08lX",
                    address + pos);
                swd_script_gui_refresh(ctx);
                furi_delay_ms(100);
            } else {
                break;
            }
        }
        if(ctx->abort) {
            DBGS("aborting");
            break;
        }

        if(!read_ok) {
            /* flags == 1: "continue reading even if it fails" */
            /* flags == 2: "its okay if cannot dump fully" */
            if(flags & 1) {
                /* set all content to a known value as indication */
                for(size_t fill_pos = 0; fill_pos < ctx->block_size; fill_pos += 4) {
                    *((uint32_t*)&buffer[fill_pos]) = 0xDEADFACE;
                }
            } else if(flags & 2) {
                success = (pos > 0);
                break;
            } else {
                notification_message_block(ctx->app->notification, &seq_error);
                success = false;
                break;
            }
        }
        storage_file_write(dump, buffer, ctx->block_size);
    }

    furi_mutex_release(ctx->app->swd_mutex);

    storage_file_close(dump);
    swd_script_seek_newline(ctx);
    free(buffer);

    return success;
}

static bool swd_scriptfunc_mem_write(ScriptContext* ctx) {
    uint32_t address = 0;
    uint32_t data = 0;
    bool success = true;

    /* get file */
    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get address */
    if(!swd_script_get_number(ctx, &address)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse address");
        return false;
    }

    /* get data */
    if(!swd_script_get_number(ctx, &data)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse data");
        return false;
    }

    DBG("write %08lX to %08lX", data, address);

    bool access_ok = false;
    for(uint32_t tries = 0; tries < ctx->max_tries; tries++) {
        if(ctx->abort) {
            DBGS("aborting");
            break;
        }

        furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
        access_ok = swd_write_memory(ctx->app, ctx->selected_ap, address, data) == 1;
        access_ok |= ctx->errors_ignore;
        swd_read_memory(ctx->app, ctx->selected_ap, address, &data);
        furi_mutex_release(ctx->app->swd_mutex);

        DBG("read %08lX from %08lX", data, address);

        if(!access_ok) {
            snprintf(
                ctx->app->state_string,
                sizeof(ctx->app->state_string),
                "Failed write 0x%08lX",
                address);
            swd_script_gui_refresh(ctx);
        } else {
            break;
        }
    }

    if(!access_ok) {
        notification_message_block(ctx->app->notification, &seq_error);
        success = false;
    }

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_mem_read(ScriptContext* ctx) {
    uint32_t address = 0;
    bool success = true;

    /* get file */
    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get address */
    if(!swd_script_get_number(ctx, &address)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse address");
        return false;
    }

    DBG("read from %08lX", address);

    uint32_t data = 0;
    bool access_ok = false;
    for(uint32_t tries = 0; tries < ctx->max_tries; tries++) {
        if(ctx->abort) {
            DBGS("aborting");
            break;
        }

        furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
        access_ok = swd_read_memory(ctx->app, ctx->selected_ap, address, &data) == 1;
        furi_mutex_release(ctx->app->swd_mutex);

        if(!access_ok) {
            swd_script_log(ctx, FuriLogLevelError, "Failed to read from %08lX", address);
            snprintf(
                ctx->app->state_string,
                sizeof(ctx->app->state_string),
                "Failed read 0x%08lX",
                address);
            swd_script_gui_refresh(ctx);
        } else {
            swd_script_log(ctx, FuriLogLevelDefault, "%08lX", data);
            break;
        }
    }

    if(!access_ok) {
        notification_message_block(ctx->app->notification, &seq_error);
        success = false;
    }

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_mem_ldmst(ScriptContext* ctx) {
    uint32_t address = 0;
    uint32_t data = 0;
    uint32_t mask = 0;
    bool success = true;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get address */
    if(!swd_script_get_number(ctx, &address)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse address");
        return false;
    }

    /* get data */
    if(!swd_script_get_number(ctx, &data)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse data");
        return false;
    }

    /* get mask */
    if(!swd_script_get_number(ctx, &mask)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse mask");
        return false;
    }

    LOG("write %08lX to %08lX, mask %08lX", data, address, mask);

    bool access_ok = false;
    uint32_t modified = 0;
    for(uint32_t tries = 0; tries < ctx->max_tries; tries++) {
        if(ctx->abort) {
            DBGS("aborting");
            break;
        }
        furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

        access_ok = swd_read_memory(ctx->app, ctx->selected_ap, address, &modified) == 1;
        modified = (modified & mask) | data;
        access_ok &= swd_write_memory(ctx->app, ctx->selected_ap, address, modified) == 1;

        furi_mutex_release(ctx->app->swd_mutex);
        access_ok |= ctx->errors_ignore;

        if(!access_ok) {
            snprintf(
                ctx->app->state_string,
                sizeof(ctx->app->state_string),
                "Failed access 0x%08lX",
                address);
            swd_script_gui_refresh(ctx);
        } else {
            break;
        }
    }

    if(!access_ok) {
        notification_message_block(ctx->app->notification, &seq_error);
        success = false;
    }

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_dp_write(ScriptContext* ctx) {
    uint32_t dp_bank = 0;
    uint32_t dp_off = 0;
    uint32_t data = 0;
    bool success = true;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get data */
    if(!swd_script_get_number(ctx, &data)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse data");
        return false;
    }

    /* get dp_off */
    if(!swd_script_get_number(ctx, &dp_off)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse DP offset");
        return false;
    }

    /* get dp_bank */
    if(!swd_script_get_number(ctx, &dp_bank)) {
        dp_bank = 0xFF;
    }

    swd_script_log(
        ctx, FuriLogLevelDefault, "write %08lX to reg %08lX / bank %08lX", data, dp_off, dp_bank);

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    uint8_t ret = swd_write_dpbank(ctx->app, dp_off, dp_bank, &data);
    if(ret != 1) {
        swd_script_log(ctx, FuriLogLevelError, "swd_write_dpbank failed");
        success = false;
    }

    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_dp_read(ScriptContext* ctx) {
    uint32_t dp_bank = 0;
    uint32_t dp_off = 0;
    uint32_t data = 0;
    bool success = true;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get dp_off */
    if(!swd_script_get_number(ctx, &dp_off)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse DP offset");
        return false;
    }

    /* get dp_bank */
    if(!swd_script_get_number(ctx, &dp_bank)) {
        dp_bank = 0xFF;
    }

    swd_script_log(ctx, FuriLogLevelDefault, "read reg %02lX / bank %02lX", dp_off, dp_bank);

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    uint8_t ret = swd_read_dpbank(ctx->app, dp_off, dp_bank, &data);
    if(ret != 1) {
        swd_script_log(ctx, FuriLogLevelError, "swd_read_dpbank failed");
        success = false;
    } else {
        swd_script_log(ctx, FuriLogLevelDefault, "result: 0x%08lX", data);
    }
    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_ap_write(ScriptContext* ctx) {
    uint32_t ap_reg = 0;
    uint32_t data = 0;
    bool success = true;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get data */
    if(!swd_script_get_number(ctx, &data)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse data");
        return false;
    }

    /* get ap_reg */
    if(!swd_script_get_number(ctx, &ap_reg)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse AP register");
        return false;
    }

    swd_script_log(
        ctx, FuriLogLevelDefault, "AP%d %08lX -> %02lX", ctx->selected_ap, data, ap_reg);

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    uint8_t ret = swd_write_ap(ctx->app, ctx->selected_ap, ap_reg, data);
    if(ret != 1) {
        swd_script_log(ctx, FuriLogLevelError, "swd_write_ap failed");
        success = false;
    }
    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_ap_read(ScriptContext* ctx) {
    uint32_t ap_reg = 0;
    uint32_t data = 0;
    bool success = true;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    /* get ap_reg */
    if(!swd_script_get_number(ctx, &ap_reg)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse AP register");
        return false;
    }

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    uint8_t ret = swd_read_ap(ctx->app, ctx->selected_ap, ap_reg, &data);
    if(ret != 1) {
        swd_script_log(ctx, FuriLogLevelError, "swd_read_ap failed");
        success = false;
    } else {
        swd_script_log(
            ctx, FuriLogLevelDefault, "AP%d %02lX: %08lX", ctx->selected_ap, ap_reg, data);
    }
    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return success;
}

static bool swd_scriptfunc_core_halt(ScriptContext* ctx) {
    bool succ = false;

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    uint32_t reg_dhcsr = SCS_DHCSR_KEY | SCS_DHCSR_C_HALT | SCS_DHCSR_C_DEBUGEN;

    succ = swd_write_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, reg_dhcsr) == 1;

    if(!succ) {
        swd_script_log(ctx, FuriLogLevelError, "swd_write_memory failed");
    } else {
        swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &reg_dhcsr);

        if(!(reg_dhcsr & SCS_DHCSR_S_HALT)) {
            swd_script_log(ctx, FuriLogLevelError, "Core did not halt");
            succ = false;
        } else {
            swd_script_log(ctx, FuriLogLevelDefault, "Core halted");
        }
    }

    furi_mutex_release(ctx->app->swd_mutex);
    swd_script_seek_newline(ctx);

    return succ;
}

static bool swd_scriptfunc_core_continue(ScriptContext* ctx) {
    bool succ = false;
    uint32_t data = 0;

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    succ = swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &data) == 1;

    if(!(data & SCS_DHCSR_S_HALT)) {
        swd_script_log(ctx, FuriLogLevelError, "Core is not in debug state");
        succ = false;
    } else {
        succ = swd_write_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, SCS_DHCSR_KEY) == 1;
        furi_mutex_release(ctx->app->swd_mutex);
    }

    if(!succ) {
        swd_script_log(ctx, FuriLogLevelError, "swd_write_memory failed");
    } else {
        swd_script_log(ctx, FuriLogLevelDefault, "Core continued");
    }

    swd_script_seek_newline(ctx);

    return succ;
}

static bool swd_scriptfunc_core_step(ScriptContext* ctx) {
    bool succ = false;
    uint32_t data = 0;

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    succ = swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &data) == 1;

    if(!(data & SCS_DHCSR_S_HALT)) {
        swd_script_log(ctx, FuriLogLevelError, "Core is not in debug state");
        succ = false;
    } else {
        succ = swd_write_memory(
                   ctx->app,
                   ctx->selected_ap,
                   SCS_DHCSR,
                   SCS_DHCSR_KEY | SCS_DHCSR_C_STEP | SCS_DHCSR_C_MASKINTS |
                       SCS_DHCSR_C_DEBUGEN) == 1;
    }
    furi_mutex_release(ctx->app->swd_mutex);

    if(!succ) {
        swd_script_log(ctx, FuriLogLevelError, "swd_write_memory failed");
    } else {
        swd_script_log(ctx, FuriLogLevelDefault, "Core stepped");
    }

    swd_script_seek_newline(ctx);

    return succ;
}

static struct cpu_regs_type {
    uint8_t regsel;
    const char* desc;
} cpu_regs[] = {
    {0x00, "R00"},    {0x01, "R01"},    {0x02, "R02"},    {0x03, "R03"},    {0x04, "R04"},
    {0x05, "R05"},    {0x06, "R06"},    {0x07, "R07"},    {0x08, "R08"},    {0x09, "R09"},
    {0x0A, "R10"},    {0x0B, "R11"},    {0x0C, "R12"},    {0x0D, "SP/R13"}, {0x0E, "LR/R14"},
    {0x0F, "PC/R15"}, {0x10, "xPSR"},   {0x11, "MSP"},    {0x12, "PSP"},    {0x14, "Flags"},
    {0x21, "FPCSR"},  {0x40, "FP S00"}, {0x41, "FP S01"}, {0x42, "FP S02"}, {0x43, "FP S03"},
    {0x44, "FP S04"}, {0x45, "FP S05"}, {0x46, "FP S06"}, {0x47, "FP S07"}, {0x48, "FP S08"},
    {0x49, "FP S09"}, {0x4A, "FP S10"}, {0x4B, "FP S11"}, {0x4C, "FP S12"}, {0x4D, "FP S13"},
    {0x4E, "FP S14"}, {0x4F, "FP S15"}, {0x50, "FP S16"}, {0x51, "FP S17"}, {0x52, "FP S18"},
    {0x53, "FP S19"}, {0x54, "FP S20"}, {0x55, "FP S21"}, {0x56, "FP S22"}, {0x57, "FP S23"},
    {0x58, "FP S24"}, {0x59, "FP S25"}, {0x5A, "FP S26"}, {0x5B, "FP S27"}, {0x5C, "FP S28"},
    {0x5D, "FP S29"}, {0x5E, "FP S30"}, {0x5F, "FP S31"}};

static bool swd_scriptfunc_core_regs(ScriptContext* ctx) {
    bool succ = false;

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);

    uint32_t reg_dhcsr = 0;
    uint32_t reg_cpacr = 0;
    swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &reg_dhcsr);
    swd_read_memory(ctx->app, ctx->selected_ap, SCS_CPACR, &reg_cpacr);

    /* when FPU is enabled/available, CP10 and CP11 are implemented */
    bool has_fpu = ((reg_cpacr >> 20) & 0x0F) != 0;

    if(!(reg_dhcsr & SCS_DHCSR_S_HALT)) {
        swd_script_log(ctx, FuriLogLevelError, "Core is not in debug state");
        succ = false;
    } else {
        for(size_t pos = 0; pos < COUNT(cpu_regs); pos++) {
            if(!has_fpu && (cpu_regs[pos].regsel >= 0x20)) {
                continue;
            }
            uint32_t core_data = 0;
            succ =
                swd_write_memory(
                    ctx->app, ctx->selected_ap, SCS_DCRSR, SCS_DCRSR_RD | cpu_regs[pos].regsel) ==
                1;
            succ &= swd_read_memory(ctx->app, ctx->selected_ap, SCS_DCRDR, &core_data) == 1;

            if(!succ) {
                swd_script_log(ctx, FuriLogLevelDefault, "%08s ----------", cpu_regs[pos].desc);
            } else {
                swd_script_log(
                    ctx, FuriLogLevelDefault, "%06s 0x%08X", cpu_regs[pos].desc, core_data);
            }
        }
    }
    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return true;
}

static bool swd_scriptfunc_core_reg_get(ScriptContext* ctx) {
    uint32_t core_reg = 0;
    uint32_t core_data = 0;
    bool succ = false;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &core_reg)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse register");
        return false;
    }

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    uint32_t reg_dhcsr = 0;
    uint32_t reg_cpacr = 0;
    swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &reg_dhcsr);
    swd_read_memory(ctx->app, ctx->selected_ap, SCS_CPACR, &reg_cpacr);

    /* when FPU is enabled/available, CP10 and CP11 are implemented */
    bool has_fpu = ((reg_cpacr >> 20) & 0x0F) != 0;

    if(!(reg_dhcsr & SCS_DHCSR_S_HALT)) {
        swd_script_log(ctx, FuriLogLevelError, "Core is not in debug state");
        succ = false;
    } else {
        if(!has_fpu && (core_reg >= 0x20)) {
            swd_script_log(ctx, FuriLogLevelError, "Core has no FP extensions");
            succ = false;
        } else {
            succ = swd_write_memory(
                       ctx->app, ctx->selected_ap, SCS_DCRSR, SCS_DCRSR_RD | core_reg) == 1;
            succ &= swd_read_memory(ctx->app, ctx->selected_ap, SCS_DCRDR, &core_data) == 1;
            if(!succ) {
                swd_script_log(ctx, FuriLogLevelError, "swd_write_memory failed");
            }
        }
    }
    furi_mutex_release(ctx->app->swd_mutex);

    if(succ) {
        swd_script_log(ctx, FuriLogLevelDefault, "0x%08X", core_data);
    }

    swd_script_seek_newline(ctx);

    return succ;
}

static bool swd_scriptfunc_core_reg_set(ScriptContext* ctx) {
    uint32_t core_reg = 0;
    uint32_t core_data = 0;
    bool succ = false;

    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }

    if(!swd_script_get_number(ctx, &core_reg)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse register");
        return false;
    }
    if(!swd_script_skip_whitespace(ctx)) {
        swd_script_log(ctx, FuriLogLevelError, "missing whitespace");
        return false;
    }
    if(!swd_script_get_number(ctx, &core_data)) {
        swd_script_log(ctx, FuriLogLevelError, "failed to parse data");
        return false;
    }

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    uint32_t reg_dhcsr = 0;
    uint32_t reg_cpacr = 0;

    swd_read_memory(ctx->app, ctx->selected_ap, SCS_DHCSR, &reg_dhcsr);
    swd_read_memory(ctx->app, ctx->selected_ap, SCS_CPACR, &reg_cpacr);

    /* when FPU is enabled/available, CP10 and CP11 are implemented */
    bool has_fpu = ((reg_cpacr >> 20) & 0x0F) != 0;

    if(!(reg_dhcsr & SCS_DHCSR_S_HALT)) {
        swd_script_log(ctx, FuriLogLevelError, "Core is not in debug state");
        succ = false;
    } else {
        if(!has_fpu && (core_reg >= 0x20)) {
            swd_script_log(ctx, FuriLogLevelError, "Core has no FP extensions");
            succ = false;
        } else {
            succ = swd_write_memory(ctx->app, ctx->selected_ap, SCS_DCRDR, core_data) == 1;
            succ &= swd_write_memory(
                        ctx->app, ctx->selected_ap, SCS_DCRSR, SCS_DCRSR_WR | core_reg) == 1;
            if(!succ) {
                swd_script_log(ctx, FuriLogLevelError, "swd_write_memory failed");
            }
        }
    }
    furi_mutex_release(ctx->app->swd_mutex);

    swd_script_seek_newline(ctx);

    return succ;
}

static bool swd_scriptfunc_core_cpuid(ScriptContext* ctx) {
    bool succ = false;
    uint32_t reg_cpuid = 0;

    furi_mutex_acquire(ctx->app->swd_mutex, FuriWaitForever);
    succ = swd_read_memory(ctx->app, ctx->selected_ap, SCS_CPUID, &reg_cpuid) == 1;
    furi_mutex_release(ctx->app->swd_mutex);

    if(!succ) {
        swd_script_log(ctx, FuriLogLevelError, "swd_read_memory failed");
    } else {
        swd_script_log(ctx, FuriLogLevelDefault, "0x%08X", reg_cpuid);
    }

    swd_script_seek_newline(ctx);

    return succ;
}

static const ScriptFunctionInfo script_funcs[] = {
    {"#", &swd_scriptfunc_comment},
    {".label", &swd_scriptfunc_label},
    {"goto", &swd_scriptfunc_goto},
    {"call", &swd_scriptfunc_call},
    {"status", &swd_scriptfunc_status},
    {"errors", &swd_scriptfunc_errors},
    {"message", &swd_scriptfunc_message},
    {"beep", &swd_scriptfunc_beep},
    {"max_tries", &swd_scriptfunc_maxtries},
    {"swd_clock_delay", &swd_scriptfunc_swd_clock_delay},
    {"swd_idle_bits", &swd_scriptfunc_swd_idle_bits},
    {"block_size", &swd_scriptfunc_blocksize},
    {"abort", &swd_scriptfunc_abort},
    {"mem_dump", &swd_scriptfunc_mem_dump},
    {"mem_ldmst", &swd_scriptfunc_mem_ldmst},
    {"mem_write", &swd_scriptfunc_mem_write},
    {"mem_read", &swd_scriptfunc_mem_read},
    {"dp_write", &swd_scriptfunc_dp_write},
    {"dp_read", &swd_scriptfunc_dp_read},
    {"ap_scan", &swd_scriptfunc_apscan},
    {"ap_select", &swd_scriptfunc_apselect},
    {"ap_read", &swd_scriptfunc_ap_read},
    {"ap_write", &swd_scriptfunc_ap_write},
    {"core_halt", &swd_scriptfunc_core_halt},
    {"core_step", &swd_scriptfunc_core_step},
    {"core_continue", &swd_scriptfunc_core_continue},
    {"core_regs", &swd_scriptfunc_core_regs},
    {"core_reg_get", &swd_scriptfunc_core_reg_get},
    {"core_reg_set", &swd_scriptfunc_core_reg_set},
    {"core_cpuid", &swd_scriptfunc_core_cpuid}};

/************************** script main code **************************/

static bool swd_execute_script_line(ScriptContext* const ctx) {
    char buffer[64];
    uint64_t start_pos = 0;

    if(ctx->script_file) {
        start_pos = storage_file_tell(ctx->script_file);
        uint16_t ret = storage_file_read(ctx->script_file, buffer, 2);
        storage_file_seek(ctx->script_file, start_pos, true);

        if(ret < 2) {
            return true;
        }
    } else {
        start_pos = ctx->line_pos;
        strncpy(buffer, ctx->line_data, 2);

        if(buffer[0] == 0 || buffer[1] == 0) {
            return true;
        }
    }

    if(buffer[0] == '\n' || (buffer[0] == '\r' && buffer[1] == '\n')) {
        swd_script_seek_newline(ctx);
        return true;
    }

    for(size_t entry = 0; entry < COUNT(script_funcs); entry++) {
        if(ctx->abort) {
            DBGS("aborting");
            break;
        }
        size_t expected = strlen(script_funcs[entry].prefix);

        if(ctx->script_file) {
            storage_file_seek(ctx->script_file, start_pos, true);

            if(storage_file_read(ctx->script_file, buffer, expected) != expected) {
                continue;
            }
        } else {
            ctx->line_pos = start_pos;

            if(strlen(ctx->line_data) < expected) {
                continue;
            }
            strncpy(buffer, ctx->line_data, expected);
            ctx->line_pos += expected;
        }

        buffer[expected] = '\000';
        if(strncmp(buffer, script_funcs[entry].prefix, expected)) {
            continue;
        }
        bool success = true;

        if(ctx->goto_active) {
            DBG("ignore: '%s'", script_funcs[entry].prefix);

            /* only execute label handlers */
            if(buffer[0] == '.') {
                success = script_funcs[entry].func(ctx);
            } else {
                swd_script_seek_newline(ctx);
            }
        } else {
            DBG("command: '%s'", script_funcs[entry].prefix);

            if(!ctx->status_ignore) {
                snprintf(
                    ctx->app->state_string,
                    sizeof(ctx->app->state_string),
                    "CMD: %s",
                    script_funcs[entry].prefix);
            }
            swd_script_gui_refresh(ctx);

            /* function, execute */
            success = script_funcs[entry].func(ctx);

            if(!success && !ctx->errors_ignore) {
                swd_script_log(
                    ctx, FuriLogLevelError, "Command failed: %s", script_funcs[entry].prefix);
                snprintf(
                    ctx->app->state_string,
                    sizeof(ctx->app->state_string),
                    "Command failed: %s",
                    script_funcs[entry].prefix);
                return false;
            }
        }

        return true;
    }
    swd_script_log(ctx, FuriLogLevelError, "unknown command '%s'", buffer);

    return false;
}

static bool swd_execute_script(AppFSM* const ctx, const char* filename) {
    bool success = true;

    /* fetch current script and set as parent */
    ScriptContext* parent = ctx->script;

    ctx->script = malloc(sizeof(ScriptContext));
    ctx->script->app = ctx;
    ctx->script->max_tries = 1;
    ctx->script->parent = parent;
    strcpy(ctx->script->filename, filename);

    if(!storage_file_exists(ctx->storage, filename)) {
        DBG("Does not exist '%s'", filename);
        parent = ctx->script->parent;
        free(ctx->script);
        ctx->script = parent;
        return false;
    }

    /* first allocate a file object */
    ctx->script->script_file = storage_file_alloc(ctx->storage);

    /* then get our script opened */
    if(!storage_file_open(ctx->script->script_file, filename, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "open, %s", storage_file_get_error_desc(ctx->script->script_file));
        DBG("Failed to open '%s'", filename);
        storage_file_free(ctx->script->script_file);
        parent = ctx->script->parent;
        free(ctx->script);
        ctx->script = parent;
        return false;
    }

    do {
        success = true;
        ctx->script->restart = false;

        storage_file_seek(ctx->script->script_file, 0, true);

        uint32_t line = 1;
        while(line < SCRIPT_MAX_LINES) {
            if(ctx->script->abort) {
                DBGS("Abort requested");
                break;
            }
            if(storage_file_eof(ctx->script->script_file)) {
                break;
            }
            DBG("line %lu", line);
            if(!swd_execute_script_line(ctx->script)) {
                success = false;
                break;
            }
            if(ctx->script->restart) {
                break;
            }
            line++;
        }

        if(ctx->script->restart) {
            DBGS("Restarting");
        } else {
            DBGS("Finished");
        }

        if(line >= SCRIPT_MAX_LINES) {
            success = true;
            char text_buf[128];

            snprintf(text_buf, sizeof(text_buf), "aborting after %d lines", SCRIPT_MAX_LINES);
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_header(message, "SWD Probe", 16, 2, AlignLeft, AlignTop);
            dialog_message_set_icon(message, &I_app, 3, 2);
            dialog_message_set_text(message, text_buf, 3, 16, AlignLeft, AlignTop);
            dialog_message_set_buttons(message, "Back", NULL, NULL);
            dialog_message_free(message);

            ctx->script->restart = false;
        }

        if(!success) {
            char text_buf[128];

            snprintf(text_buf, sizeof(text_buf), "Line %lu failed:\n%s", line, ctx->state_string);
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_header(message, "SWD Probe", 16, 2, AlignLeft, AlignTop);
            dialog_message_set_icon(message, &I_app, 3, 2);
            dialog_message_set_text(message, text_buf, 3, 16, AlignLeft, AlignTop);
            dialog_message_set_buttons(message, "Back", "Retry", NULL);
            if(dialog_message_show(ctx->dialogs, message) == DialogMessageButtonCenter) {
                ctx->script->restart = true;
            }
            dialog_message_free(message);
        }
    } while(ctx->script->restart);

    storage_file_close(ctx->script->script_file);
    storage_file_free(ctx->script->script_file);

    parent = ctx->script->parent;
    free(ctx->script);
    ctx->script = parent;

    return success;
}

/************************** UI functions **************************/

#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64

#define COERCE(d, min, max) \
    do {                    \
        if(d < (min)) {     \
            d = (min);      \
        }                   \
        if(d > (max)) {     \
            d = (max);      \
        }                   \
    } while(0)

#define COERCE_COORDS(x1, y1, x2, y2) \
    do {                              \
        COERCE(x1, 0, CANVAS_WIDTH);  \
        COERCE(x2, 0, CANVAS_WIDTH);  \
        COERCE(y1, 0, CANVAS_HEIGHT); \
        COERCE(y1, 0, CANVAS_HEIGHT); \
    } while(0)

#include "model/model_chip.h"

static int rotatedVertexCoords[NUM_VERTICES][3];

static void draw_model(Canvas* const canvas) {
    static float xAngle = 0;
    static float yAngle = 0;
    static float zAngle = 0;
    static float zoom = 0;
    static float speed = 0.6f;

    float cosXAngle = cosf(xAngle);
    float sinXAngle = sinf(xAngle);
    float cosYAngle = cosf(yAngle);
    float sinYAngle = sinf(yAngle);
    float cosZAngle = cosf(zAngle);
    float sinZAngle = sinf(zAngle);
    float sinZoom = 1.2f + sinf(zoom) * 0.25f;

    int centerX = CANVAS_WIDTH / 2;
    int centerY = CANVAS_HEIGHT / 2 + 5;

    for(int i = 0; i < NUM_VERTICES; i++) {
        int x = vertexCoords[i][0] * sinZoom * 16;
        int y = vertexCoords[i][1] * sinZoom * 16;
        int z = vertexCoords[i][2] * sinZoom * 16;

        int y1 = y * cosXAngle - z * sinXAngle;
        int z1 = y * sinXAngle + z * cosXAngle;

        int x2 = x * cosYAngle + z1 * sinYAngle;
        int z2 = -x * sinYAngle + z1 * cosYAngle;

        int x3 = x2 * cosZAngle - y1 * sinZAngle;
        int y3 = x2 * sinZAngle + y1 * cosZAngle;

        rotatedVertexCoords[i][0] = x3 + centerX;
        rotatedVertexCoords[i][1] = y3 + centerY;
        rotatedVertexCoords[i][2] = z2;
    }

    for(size_t i = 0; i < COUNT(edgeIndices); i++) {
        int v1Index = edgeIndices[i][0];
        int v2Index = edgeIndices[i][1];
        int x1 = rotatedVertexCoords[v1Index][0];
        int y1 = rotatedVertexCoords[v1Index][1];
        int x2 = rotatedVertexCoords[v2Index][0];
        int y2 = rotatedVertexCoords[v2Index][1];

        COERCE_COORDS(x1, y1, x2, y2);
        canvas_draw_line(canvas, x1, y1, x2, y2);
    }

    xAngle += speed * 0.02 / sinZoom;
    yAngle += speed * 0.023 / sinZoom;
    zAngle += speed * 0.029 * sinZoom;
    zoom += speed * 0.005;
}

static void render_callback(Canvas* const canvas, void* ctx_in) {
    furi_assert(canvas);
    furi_assert(ctx_in);

    AppFSM* ctx = ctx_in;
    furi_mutex_acquire(ctx->gui_mutex, FuriWaitForever);

    char buffer[64];
    int y = 10;

    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_set_font(canvas, FontPrimary);

    if(!ctx->detected_device) {
        ctx->mode_page = ModePageScan;
    } else if(ctx->mode_page == ModePageScan) {
        ctx->mode_page = ModePageFound;
    }

    /* if seen less than a quarter second ago */
    switch(ctx->mode_page) {
    case ModePageScan: {
        draw_model(canvas);

        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "Searching");
        y += 14;

        canvas_set_font(canvas, FontSecondary);

        bool info_page = (ctx->loop_count % 500) >= 250;
        if(info_page) {
            canvas_draw_str(canvas, 2, y, "Connect GND with target GND");
            y += 10;
            canvas_draw_str(canvas, 2, y, "and any two GPIOs with pads");
            y += 10;
            canvas_draw_str(canvas, 2, y, "you want to check for SWD");

            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 111, 62, "2/2");
        } else {
            const char* filename = "<none>";
            if(strlen(ctx->script_detected) > 0) {
                const char* slash = strrchr(ctx->script_detected, '/');
                if(slash) {
                    filename = &slash[1];
                } else {
                    filename = ctx->script_detected;
                }
            }

            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "Autoexec Script");
            y += 10;
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, filename);
            y += 16;

            canvas_set_font(canvas, FontSecondary);
            canvas_draw_icon(canvas, 14, y - 5, &I_ButtonUp_7x4);
            canvas_draw_icon(canvas, 78, y - 5, &I_ButtonDown_7x4);
            canvas_draw_str(canvas, 23, y, "Clear");
            canvas_draw_str(canvas, 87, y, "Choose");

            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 111, 62, "1/2");
        }
        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "Script");
        break;
    }
    case ModePageFound: {
        if((ctx->detected_timeout + TIMER_HZ / 4) >= TIMER_HZ * TIMEOUT) {
            snprintf(buffer, sizeof(buffer), "FOUND!");
        } else {
            /* if it was seen more than a quarter second ago, show countdown */
            snprintf(
                buffer, sizeof(buffer), "FOUND! (%lus)", (ctx->detected_timeout / TIMER_HZ) + 1);
        }
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, buffer);
        y += 10;
        canvas_set_font(canvas, FontKeyboard);

        snprintf(
            buffer,
            sizeof(buffer),
            "SWC/SWD: %s/%s",
            gpio_name(ctx->io_swc),
            gpio_name(ctx->io_swd));
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;
        snprintf(buffer, sizeof(buffer), "DPIDR 0x%08lX", ctx->dp_regs.dpidr);
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;

        snprintf(
            buffer,
            sizeof(buffer),
            "Part %02X Rev %X DAPv%d",
            ctx->dpidr_info.partno,
            ctx->dpidr_info.revision,
            ctx->dpidr_info.version);
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;

        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, sizeof(buffer), "%s", jep106_manufacturer(ctx->dpidr_info.designer));
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;

        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "Script");
        elements_button_right(canvas, "DP Regs");

        break;
    }
    case ModePageDPRegs: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "DP Registers");
        y += 10;
        canvas_set_font(canvas, FontKeyboard);
        if(ctx->dp_regs.dpidr_ok) {
            snprintf(buffer, sizeof(buffer), "DPIDR %08lX", ctx->dp_regs.dpidr);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        }
        y += 10;

        if(ctx->dp_regs.ctrlstat_ok) {
            snprintf(buffer, sizeof(buffer), "CTRL  %08lX", ctx->dp_regs.ctrlstat);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        }
        y += 10;

        if(ctx->dp_regs.targetid_ok) {
            snprintf(buffer, sizeof(buffer), "TGTID %08lX", ctx->dp_regs.targetid);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        }
        y += 10;

        if(ctx->dp_regs.eventstat_ok) {
            snprintf(buffer, sizeof(buffer), "EVTST %08lX", ctx->dp_regs.eventstat);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        }
        y += 10;
        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "Scan");
        elements_button_right(canvas, "DPID");

        break;
    }

    case ModePageDPID: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "DP ID Register");
        y += 10;
        canvas_set_font(canvas, FontKeyboard);
        if(ctx->dpidr_info.version != 2) {
            snprintf(buffer, sizeof(buffer), "TARGETID not supported");
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;
        } else {
            if(ctx->dp_regs.targetid_ok) {
                snprintf(buffer, sizeof(buffer), "TGTID %08lX", ctx->dp_regs.targetid);
                canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
                y += 10;

                snprintf(buffer, sizeof(buffer), "Part No. %04X", ctx->targetid_info.partno);
                canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
                y += 10;
                snprintf(
                    buffer, sizeof(buffer), "%s", jep106_manufacturer(ctx->targetid_info.designer));
                canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
                y += 10;
            }
        }
        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "DP Regs");
        elements_button_right(canvas, "APs");
        break;
    }

    case ModePageAPID: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "AP Menu");
        y += 10;
        canvas_set_font(canvas, FontKeyboard);

        char state = ' ';
        if(ctx->ap_pos >= ctx->ap_scanned && ctx->ap_pos <= ctx->ap_scanned + 10) {
            state = '*';
        }

        if(!ctx->apidr_info[ctx->ap_pos].ok) {
            snprintf(buffer, sizeof(buffer), "[%d]%c<none>", ctx->ap_pos, state);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;

            if(ctx->ap_pos == 0) {
                for(size_t pos = 0; pos < COUNT(ctx->apidr_info); pos++) {
                    if(ctx->apidr_info[pos].ok) {
                        ctx->ap_pos = pos;
                    }
                }
            }
        } else {
            const char* class = "";

            switch(ctx->apidr_info[ctx->ap_pos].class) {
            case 0:
                class = "und";
                break;
            case 1:
                class = "COM";
                break;
            case 8:
                class = "MEM";
                break;
            default:
                class = "unk";
                break;
            }

            const char* types[] = {
                "COM-AP",
                "AHB3",
                "APB2 or APB3",
                "Type unknown",
                "AXI3 or AXI4",
                "AHB5",
                "APB4 and APB5",
                "AXI5",
                "AHB5 enh.",
            };
            const char* type = "Type unk";

            if(ctx->apidr_info[ctx->ap_pos].type < COUNT(types)) {
                type = types[ctx->apidr_info[ctx->ap_pos].type];
            }

            snprintf(buffer, sizeof(buffer), "[%d]%c%s, %s", ctx->ap_pos, state, class, type);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;

            snprintf(buffer, sizeof(buffer), "Base 0x%08lX", ctx->apidr_info[ctx->ap_pos].base);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;

            snprintf(
                buffer,
                sizeof(buffer),
                "Rev %d Var %d",
                ctx->apidr_info[ctx->ap_pos].revision,
                ctx->apidr_info[ctx->ap_pos].variant);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;

            snprintf(
                buffer,
                sizeof(buffer),
                "%s",
                jep106_manufacturer(ctx->apidr_info[ctx->ap_pos].designer));
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            y += 10;

            elements_button_center(canvas, "Show");
        }
        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "DPID");
        elements_button_right(canvas, "CoreS.");
        elements_scrollbar_pos(canvas, 4, 10, 40, ctx->ap_pos / 32, COUNT(ctx->apidr_info) / 32);
        break;
    }

    /* hex dump view */
    case ModePageHexDump: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "Hex dump");
        y += 10;
        canvas_set_font(canvas, FontKeyboard);

        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, "Addr:");

        snprintf(buffer, sizeof(buffer), "%08lX", ctx->hex_addr);
        canvas_draw_str_aligned(canvas, 38, y, AlignLeft, AlignBottom, buffer);
        uint32_t font_width = canvas_glyph_width(canvas, '0');
        uint32_t x = 37 + (7 - ctx->hex_select) * font_width;

        /* draw selection */
        canvas_draw_line(canvas, x, y + 1, x + font_width, y + 1);
        y += 10;

        uint32_t byte_num = 0;
        for(int line = 0; line < 4; line++) {
            uint32_t x_pos = 5;

            for(int byte_pos = 0; byte_pos < 8; byte_pos++) {
                if(ctx->hex_buffer_valid[byte_num / 4]) {
                    snprintf(buffer, sizeof(buffer), "%02X", ctx->hex_buffer[byte_num]);
                } else {
                    snprintf(buffer, sizeof(buffer), "--");
                }
                byte_num++;
                canvas_draw_str_aligned(canvas, x_pos, y, AlignLeft, AlignBottom, buffer);
                x_pos += font_width * 2 + font_width / 2;
            }
            y += 10;
        }
        break;
    }

    case ModePageCoresight: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "Coresight");
        y += 10;
        canvas_set_font(canvas, FontSecondary);

        uint32_t base = ctx->coresight_bases[ctx->coresight_level];
        uint32_t base_next = adi_romtable_get(ctx, base, ctx->coresight_pos[ctx->coresight_level]);

        snprintf(buffer, sizeof(buffer), "Base: %08lX", base);
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;
        snprintf(buffer, sizeof(buffer), "Type: %s", adi_romtable_type(ctx, base));
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;
        snprintf(buffer, sizeof(buffer), "Full: %s", adi_romtable_full(ctx, base));
        canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
        y += 10;

        if(adi_is_romtable(ctx, base)) {
            snprintf(
                buffer,
                sizeof(buffer),
                "[%lu/%lu] -> %08lX",
                ctx->coresight_pos[ctx->coresight_level] + 1,
                ctx->coresight_count[ctx->coresight_level],
                base_next);
            canvas_draw_str_aligned(canvas, 5, y, AlignLeft, AlignBottom, buffer);
            canvas_set_font(canvas, FontSecondary);
            elements_button_center(canvas, "Enter");
        }
        y += 10;

        canvas_set_font(canvas, FontSecondary);

        if(ctx->coresight_level) {
            elements_button_left(canvas, "Prev");
        } else {
            elements_button_left(canvas, "APs");
        }
        elements_scrollbar_pos(
            canvas,
            4,
            10,
            40,
            ctx->coresight_pos[ctx->coresight_level],
            ctx->coresight_count[ctx->coresight_level]);

        break;
    }

    /* hex dump view */
    case ModePageScript: {
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, "Script");
        y += 10;
        y += 10;
        canvas_draw_str_aligned(canvas, 10, y, AlignLeft, AlignBottom, "Status:");
        y += 10;
        canvas_set_font(canvas, FontKeyboard);
        canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignBottom, ctx->state_string);
        y += 10;
        break;
    }
    }

    furi_mutex_release(ctx->gui_mutex);
}

static void input_callback(InputEvent* input_event, void* ctx_in) {
    furi_assert(input_event);
    furi_assert(ctx_in);
    AppFSM* ctx = ctx_in;

    int entries = furi_message_queue_get_count(ctx->event_queue);

    /* better skip than sorry */
    if(entries < QUEUE_SIZE) {
        AppEvent event = {.type = EventKeyPress, .input = *input_event};
        furi_message_queue_put(ctx->event_queue, &event, 0);
    }
}

static void app_init(AppFSM* const app) {
    furi_assert(app);

    app->loop_count = 0;
    app->current_mask_id = 0;
    app->current_mask = gpio_direction_mask[app->current_mask_id];
    app->io_swd = 0xFF;
    app->io_swc = 0xFF;
    app->hex_addr = 0x40002800;
    app->hex_addr = 0xE000EDF0;
    app->swd_clock_delay = CLOCK_DELAY;
    app->swd_idle_bits = IDLE_BITS;

    strcpy(app->state_string, "none");
    strcpy(app->script_detected, "");
}

static void app_deinit(AppFSM* const app) {
    furi_assert(app);

    strcpy(app->state_string, "exiting");
}

static void swd_main_loop(AppFSM* ctx) {
    furi_assert(ctx);

    ctx->loop_count++;

    switch(ctx->mode_page) {
    case ModePageScan:
    case ModePageFound: {
        /* reset after timeout */
        if(ctx->detected_timeout > 0) {
            ctx->detected_timeout--;
        } else {
            DBGS("Reset detected flag");
            ctx->detected_device = false;
            ctx->io_swd = 0xFF;
            ctx->io_swc = 0xFF;
            ctx->io_num_swd = 0xFF;
            ctx->io_num_swc = 0xFF;
            ctx->ap_scanned = 0;
            memset(&ctx->dp_regs, 0x00, sizeof(ctx->dp_regs));
            memset(&ctx->targetid_info, 0x00, sizeof(ctx->targetid_info));
            memset(&ctx->apidr_info, 0x00, sizeof(ctx->apidr_info));
            ctx->script_detected_executed = false;
        }

        ctx->detected = false;
        ctx->current_mask = gpio_direction_mask[ctx->current_mask_id];

        /* when SWD was already detected, set it to data pin regardless of the mask */
        if(ctx->detected_device) {
            ctx->current_mask &= ~ctx->io_swd;
        }

        /* do the scan */
        furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
        swd_scan(ctx);
        furi_mutex_release(ctx->swd_mutex);

        /* now when detected a device, set the timeout */
        if(ctx->detected) {
            DBGS("Set detected flag");
            ctx->detected_device = true;
            ctx->detected_timeout = TIMER_HZ * TIMEOUT;

            /* update DPIDR fields */
            ctx->dpidr_info.revision = (ctx->dp_regs.dpidr >> 28) & 0x0F;
            ctx->dpidr_info.partno = (ctx->dp_regs.dpidr >> 20) & 0xFF;
            ctx->dpidr_info.version = (ctx->dp_regs.dpidr >> 12) & 0x0F;
            ctx->dpidr_info.designer = (ctx->dp_regs.dpidr >> 1) & 0x3FF;

            if(!has_multiple_bits(ctx->io_swc)) {
                DBGS(" - Detected pins");
                DBGS(" - Resetting error");

                furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
                /* reset error */
                /* first make sure we have the correct bank by invalidating the current select cache */
                ctx->dp_regs.select_ok = false;
                uint8_t ack =
                    swd_read_dpbank(ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat);

                if(ack != 1 || (ctx->dp_regs.ctrlstat & STAT_ERROR_FLAGS)) {
                    DBGS(" - send ABORT");
                    swd_abort(ctx);
                }
                DBGS(" - Fetch CTRL/STAT");
                ctx->dp_regs.ctrlstat_ok =
                    swd_read_dpbank(
                        ctx, REG_CTRLSTAT, REG_CTRLSTAT_BANK, &ctx->dp_regs.ctrlstat) == 1;
                DBG("     %08lX %s",
                    ctx->dp_regs.ctrlstat,
                    ctx->dp_regs.ctrlstat_ok ? "OK" : "FAIL");

                if(ctx->dpidr_info.version >= 1) {
                    DBGS(" - DAPv1, read DLCR");
                    ctx->dp_regs.dlcr_ok =
                        swd_read_dpbank(ctx, REG_DLCR, REG_DLCR_BANK, &ctx->dp_regs.dlcr) == 1;
                    DBG("     %08lX %s", ctx->dp_regs.dlcr, ctx->dp_regs.dlcr_ok ? "OK" : "FAIL");
                }

                if(ctx->dpidr_info.version >= 2) {
                    DBGS(" - DAPv2, read TARGETID");
                    ctx->dp_regs.targetid_ok =
                        swd_read_dpbank(
                            ctx, REG_TARGETID, REG_TARGETID_BANK, &ctx->dp_regs.targetid) == 1;
                    DBG("     %08lX %s",
                        ctx->dp_regs.targetid,
                        ctx->dp_regs.targetid_ok ? "OK" : "FAIL");
                    DBGS(" - DAPv2, read EVENTSTAT");
                    ctx->dp_regs.eventstat_ok =
                        swd_read_dpbank(
                            ctx, REG_EVENTSTAT, REG_EVENTSTAT_BANK, &ctx->dp_regs.eventstat) == 1;
                    DBG("     %08lX %s",
                        ctx->dp_regs.eventstat,
                        ctx->dp_regs.eventstat_ok ? "OK" : "FAIL");
                    DBGS(" - DAPv2, read DLPIDR");
                    ctx->dp_regs.dlpidr_ok =
                        swd_read_dpbank(ctx, REG_DLPIDR, REG_DLPIDR_BANK, &ctx->dp_regs.dlpidr) ==
                        1;
                    DBG("     %08lX %s",
                        ctx->dp_regs.dlpidr,
                        ctx->dp_regs.dlpidr_ok ? "OK" : "FAIL");
                }

                if(ctx->dp_regs.targetid_ok) {
                    ctx->targetid_info.revision = (ctx->dp_regs.targetid >> 28) & 0x0F;
                    ctx->targetid_info.partno = (ctx->dp_regs.targetid >> 12) & 0xFFFF;
                    ctx->targetid_info.designer = (ctx->dp_regs.targetid >> 1) & 0x3FF;
                }

                if(!ctx->script_detected_executed && strlen(ctx->script_detected) > 0) {
                    DBG(" - Run script '%s'", ctx->script_detected);

                    ctx->script_detected_executed = true;

                    ctx->mode_page = ModePageScript;
                    swd_execute_script(ctx, ctx->script_detected);
                    ctx->mode_page = ModePageFound;
                }
                furi_mutex_release(ctx->swd_mutex);
            }
        } else {
            if(!has_multiple_bits(ctx->io_swc)) {
                DBGS(" - Lost device");
            }
        }

        ctx->current_mask_id = (ctx->current_mask_id + 1) % COUNT(gpio_direction_mask);
        break;
    }

    case ModePageDPRegs:
    case ModePageAPID: {
        furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);
        /* set debug enable request */
        if(!swd_ensure_powerup(ctx)) {
            furi_mutex_release(ctx->swd_mutex);
            break;
        }

        /* only scan a few APs at once to stay responsive */
        for(int pos = 0; pos < 8; pos++) {
            if(ctx->ap_scanned == 0) {
                swd_apscan_reset(ctx);
            }

            uint8_t ap = ctx->ap_scanned++;

            if(ctx->apidr_info[ap].tested) {
                continue;
            }
            if(swd_apscan_test(ctx, ap)) {
                break;
            }
        }
        furi_mutex_release(ctx->swd_mutex);
        break;
    }

    case ModePageHexDump: {
        furi_mutex_acquire(ctx->swd_mutex, FuriWaitForever);

        for(size_t byte_pos = 0; byte_pos < sizeof(ctx->hex_buffer); byte_pos += 4) {
            uint32_t* data = (uint32_t*)&ctx->hex_buffer[byte_pos];
            bool ret = swd_read_memory(ctx, ctx->ap_pos, ctx->hex_addr + byte_pos, data) == 1;

            ctx->hex_buffer_valid[byte_pos / 4] = ret;

            if(!ret) {
                swd_abort_simple(ctx);
            }
        }
        furi_mutex_release(ctx->swd_mutex);
        break;
    }

    case ModePageDPID:
    case ModePageCoresight:
        furi_delay_ms(50);
        break;
    }
}

static bool swd_message_process(AppFSM* ctx) {
    bool processing = true;
    AppEvent event;

    /* wait to make sure the OS can do its stuff */
    FuriStatus event_status = furi_message_queue_get(ctx->event_queue, &event, 1000 / TIMER_HZ);

    if(event_status != FuriStatusOk) {
        return processing;
    }

    if(event.type == EventKeyPress) {
        if(event.input.type == InputTypePress) {
            switch(event.input.key) {
            case InputKeyUp:
                switch(ctx->mode_page) {
                default:
                    break;

                case ModePageScan:
                case ModePageFound: {
                    strcpy(ctx->script_detected, "");
                    break;
                }

                case ModePageAPID:
                    if(ctx->ap_pos > 0) {
                        ctx->ap_pos--;
                    }
                    break;

                case ModePageHexDump: {
                    ctx->hex_addr += ((ctx->hex_select) ? 1 : 8) * (1 << (4 * ctx->hex_select));
                    break;
                }

                case ModePageCoresight: {
                    if(ctx->coresight_pos[ctx->coresight_level] > 0) {
                        ctx->coresight_pos[ctx->coresight_level]--;
                    }
                    break;
                }
                }
                break;

            case InputKeyDown: {
                switch(ctx->mode_page) {
                default:
                    break;

                case ModePageScan: {
                    FuriString* result_path = furi_string_alloc_printf(ANY_PATH("swd_scripts"));
                    FuriString* preselected = furi_string_alloc_printf(
                        (strlen(ctx->script_detected) > 0) ? ctx->script_detected :
                                                             ANY_PATH("swd_scripts"));
                    DialogsFileBrowserOptions options;

                    dialog_file_browser_set_basic_options(&options, "swd", &I_swd);

                    if(dialog_file_browser_show(ctx->dialogs, result_path, preselected, &options)) {
                        const char* path = furi_string_get_cstr(result_path);
                        strcpy(ctx->script_detected, path);
                    }

                    furi_string_free(result_path);
                    furi_string_free(preselected);
                    break;
                }

                case ModePageAPID:
                    if(ctx->ap_pos + 1U < COUNT(ctx->apidr_info)) {
                        ctx->ap_pos++;
                    }
                    break;

                case ModePageHexDump: {
                    ctx->hex_addr -= ((ctx->hex_select) ? 1 : 8) * (1 << (4 * ctx->hex_select));
                    break;
                }

                case ModePageCoresight: {
                    if(ctx->coresight_pos[ctx->coresight_level] + 1 <
                       ctx->coresight_count[ctx->coresight_level]) {
                        ctx->coresight_pos[ctx->coresight_level]++;
                    }
                    break;
                }
                }
                break;
            }

            case InputKeyRight:
                if(ctx->mode_page == ModePageHexDump) {
                    if(ctx->hex_select > 0) {
                        ctx->hex_select--;
                    }
                } else if(ctx->mode_page == ModePageAPID && ctx->apidr_info[ctx->ap_pos].ok) {
                    ctx->mode_page = ModePageCoresight;
                    uint32_t base = ctx->apidr_info[ctx->ap_pos].base & 0xFFFFF000;
                    ctx->coresight_level = 0;
                    ctx->coresight_bases[ctx->coresight_level] = base;
                    ctx->coresight_pos[ctx->coresight_level] = 0;
                    ctx->coresight_count[ctx->coresight_level] =
                        adi_romtable_entry_count(ctx, base);
                } else if(ctx->detected) {
                    if(ctx->mode_page + 1 < ModePageCount) {
                        ctx->mode_page++;
                    }
                }
                break;

            case InputKeyLeft:
                if(ctx->mode_page == ModePageHexDump) {
                    if(ctx->hex_select < 7) {
                        ctx->hex_select++;
                    }
                } else if(ctx->mode_page == ModePageCoresight) {
                    if(ctx->coresight_level > 0) {
                        ctx->coresight_level--;
                    } else {
                        ctx->mode_page = ModePageAPID;
                    }
                } else if((ctx->mode_page == ModePageScan) || (ctx->mode_page == ModePageFound)) {
                    uint32_t mode_page = ctx->mode_page;
                    FuriString* result_path = furi_string_alloc_printf(ANY_PATH("swd_scripts"));
                    FuriString* preselected = furi_string_alloc_printf(
                        (strlen(ctx->script_detected) > 0) ? ctx->script_detected :
                                                             ANY_PATH("swd_scripts"));
                    DialogsFileBrowserOptions options;

                    dialog_file_browser_set_basic_options(&options, "swd", &I_swd);

                    if(dialog_file_browser_show(ctx->dialogs, result_path, preselected, &options)) {
                        const char* path = furi_string_get_cstr(result_path);
                        ctx->mode_page = ModePageScript;
                        swd_execute_script(ctx, path);
                        ctx->mode_page = mode_page;
                    }

                    furi_string_free(result_path);
                    furi_string_free(preselected);
                    break;
                } else {
                    if(ctx->mode_page > 0) {
                        ctx->mode_page--;
                    }
                }
                break;

            case InputKeyOk:
                if(ctx->mode_page == ModePageAPID && ctx->apidr_info[ctx->ap_pos].ok) {
                    ctx->mode_page = ModePageHexDump;
                } else if(ctx->mode_page == ModePageCoresight) {
                    uint32_t base = ctx->coresight_bases[ctx->coresight_level];

                    if(!adi_is_romtable(ctx, base)) {
                        break;
                    }

                    uint32_t cur_pos = ctx->coresight_pos[ctx->coresight_level];
                    uint32_t base_next = adi_romtable_get(ctx, base, cur_pos);
                    uint32_t new_count = adi_romtable_entry_count(ctx, base_next);

                    ctx->coresight_level++;
                    ctx->coresight_pos[ctx->coresight_level] = 0;
                    ctx->coresight_count[ctx->coresight_level] = new_count;
                    ctx->coresight_bases[ctx->coresight_level] = base_next;
                }
                break;

            case InputKeyBack:
                if(ctx->mode_page == ModePageHexDump) {
                    ctx->mode_page = ModePageAPID;
                } else if(ctx->mode_page == ModePageScript) {
                    ctx->script->abort = true;
                } else if(ctx->mode_page > ModePageFound) {
                    ctx->mode_page = ModePageScan;
                } else if(ctx->mode_page == ModePageScan) {
                    processing = false;
                } else if(ctx->mode_page == ModePageFound) {
                    processing = false;
                }
                break;

            default:
                break;
            }
        }
    }
    return processing;
}

size_t data_received(void* ctx, uint8_t* data, size_t length) {
    AppFSM* app = (AppFSM*)ctx;

    strncpy(app->commandline->line_data, (const char*)data, length);
    app->commandline->line_pos = 0;

    for(size_t pos = 0; pos < length; pos++) {
        uint8_t ch = app->commandline->line_data[pos];

        if((ch == '\r') || (ch == '\n')) {
            app->commandline->line_data[pos++] = '\n';
            app->commandline->line_data[pos] = 0;
            LOG("direct command '%s'", app->commandline->line_data);
            swd_execute_script_line(app->commandline);
            return pos;
        }
    }

    return 0;
}

int32_t swd_probe_app_main(void* p) {
    UNUSED(p);

    AppFSM* app = malloc(sizeof(AppFSM));

    DBGS("App init");
    app_init(app);

    DBGS("furi_record_open");
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->storage = furi_record_open(RECORD_STORAGE);

    DBGS("furi_mutex_alloc");
    app->swd_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->gui_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->event_queue = furi_message_queue_alloc(QUEUE_SIZE, sizeof(AppEvent));

    DBGS("usb_uart_enable");
    UsbUartConfig uart_config;
    uart_config.vcp_ch = 1;
    uart_config.rx_data = &data_received;
    uart_config.rx_data_ctx = app;
    app->uart = usb_uart_enable(&uart_config);

    app->commandline = malloc(sizeof(ScriptContext));
    app->commandline->max_tries = 1;
    app->commandline->app = app;

    DBGS("view_port_alloc");
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    DBGS("notification_message_block");
    notification_message(app->notification, &sequence_display_backlight_enforce_on);

    DBGS("swd_execute_script");
    swd_execute_script(app, ANY_PATH("swd_scripts/startup.swd"));

    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    DBGS("processing");
    for(bool processing = true; processing;) {
        swd_main_loop(app);
        view_port_update(app->view_port);

        processing = swd_message_process(app);

        bool beep = false;

        if(app->detected_device && !app->detected_notified) {
            app->detected_notified = true;
            beep = true;
        }
        if(!app->detected_device && app->detected_notified) {
            app->detected_notified = false;
        }
        if(beep) {
            notification_message_block(app->notification, &seq_c_minor);
        }
    }

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

    app_deinit(app);

    notification_message(app->notification, &sequence_display_backlight_enforce_auto);

    usb_uart_disable(app->uart);

    furi_message_queue_free(app->event_queue);
    furi_mutex_free(app->gui_mutex);
    furi_mutex_free(app->swd_mutex);

    // Reset GPIO pins to default state
    for(int io = 0; io < 8; io++) {
        furi_hal_gpio_init(gpios[io], GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    }
    free(app);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);

    return 0;
}
