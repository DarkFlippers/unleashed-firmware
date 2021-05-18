#include <stdio.h>
#include <furi.h>
#include <api-hal-irda.h>
#include <api-hal.h>

#define IRDA_TIMINGS_SIZE 2000

typedef struct {
    uint32_t timing_cnt;
    struct {
        uint8_t level;
        uint32_t duration;
    } timing[IRDA_TIMINGS_SIZE];
} IrdaDelaysArray;

static void irda_rx_callback(void* ctx, bool level, uint32_t duration) {
    IrdaDelaysArray* delays = ctx;

    if(delays->timing_cnt < IRDA_TIMINGS_SIZE) {
        if(delays->timing_cnt > 1)
            furi_check(level != delays->timing[delays->timing_cnt - 1].level);
        delays->timing[delays->timing_cnt].level = level;
        delays->timing[delays->timing_cnt].duration = duration;
        delays->timing_cnt++; // Read-Modify-Write in ISR only: no need to add synchronization
    }
}

int32_t irda_monitor_app(void* p) {
    (void)p;
    static uint32_t counter = 0;

    IrdaDelaysArray* delays = furi_alloc(sizeof(IrdaDelaysArray));

    api_hal_irda_rx_irq_init();
    api_hal_irda_rx_irq_set_callback(irda_rx_callback, delays);

    while(1) {
        delay(20);

        if(counter != delays->timing_cnt) {
            api_hal_light_set(LightRed, 0x00);
            api_hal_light_set(LightGreen, 0x00);
            api_hal_light_set(LightBlue, 0xFF);
            delay(20);
            api_hal_light_set(LightRed, 0x00);
            api_hal_light_set(LightGreen, 0x00);
            api_hal_light_set(LightBlue, 0x00);
            counter = delays->timing_cnt;
        }

        if(delays->timing_cnt >= IRDA_TIMINGS_SIZE) {
            api_hal_irda_rx_irq_deinit();
            printf("== IRDA MONITOR FOUND (%d) records) ==\r\n", IRDA_TIMINGS_SIZE);
            printf("{");
            for(int i = 0; i < IRDA_TIMINGS_SIZE; ++i) {
                printf(
                    "%s%lu, ",
                    (delays->timing[i].duration > 15000) ? "\r\n" : "",
                    delays->timing[i].duration);
            }
            printf("\r\n};\r\n");
            break;
        }
    }

    free(delays);

    return 0;
}
