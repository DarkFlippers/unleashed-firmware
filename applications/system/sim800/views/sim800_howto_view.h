// Wiring, what the app does, power and baud - the wiring page is drawn so the TX/RX
// crossover shows, and so it can follow the UART the settings scene selected
#pragma once

#include <gui/view.h>

typedef struct Sim800HowtoView Sim800HowtoView;

Sim800HowtoView* sim800_howto_view_alloc(void);
void sim800_howto_view_free(Sim800HowtoView* instance);
View* sim800_howto_view_get_view(Sim800HowtoView* instance);

/** @param tx_pin,rx_pin header pin numbers of the selected UART. */
void sim800_howto_view_set_pins(Sim800HowtoView* instance, uint8_t tx_pin, uint8_t rx_pin);
void sim800_howto_view_reset(Sim800HowtoView* instance);
