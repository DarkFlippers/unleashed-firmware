// Saved in apps_data/sim800/sim800.conf
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define SIM800_BAUDRATE_COUNT   9 // down to 1200: plenty of SIM800C ship fixed at 2400
#define SIM800_PWRKEY_COUNT     10 // "Off" plus every pin the app can offer
#define SIM800_DEFAULT_BAUDRATE 2400UL // what SIM800C boards are shipping fixed at

// Matches FuriHalSerialId, without pulling in the HAL header
typedef enum {
    Sim800UartUsart, //!< pin 13 TX, pin 14 RX
    Sim800UartLpuart, //!< pin 15 TX, pin 16 RX

    Sim800UartCount,
} Sim800UartPort;

typedef struct {
    uint32_t uart_port; //!< ::Sim800UartPort
    uint32_t baudrate;
    uint32_t pwrkey_pin; //!< index into ::sim800_settings_pwrkey_names, 0 = not wired
    bool notify; //!< feedback on an incoming call, a sent SMS or a save
    bool echo; //!< leave the module's command echo on, for the terminal
} Sim800Settings;

extern const uint32_t sim800_settings_baudrates[SIM800_BAUDRATE_COUNT];
extern const char* const sim800_settings_baudrate_names[SIM800_BAUDRATE_COUNT];
extern const char* const sim800_settings_port_names[Sim800UartCount];
extern const char* const sim800_settings_port_pins[Sim800UartCount];
extern const char* const sim800_settings_pwrkey_names[SIM800_PWRKEY_COUNT];

/** Defaults first, then whatever the file has. A bad file leaves the defaults. */
void sim800_settings_load(Sim800Settings* instance);

bool sim800_settings_save(const Sim800Settings* instance);
