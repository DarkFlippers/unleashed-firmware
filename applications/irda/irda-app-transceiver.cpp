#include "irda-app.hpp"
#include "irda.h"
#include <api-hal-irda.h>

void IrdaAppSignalTransceiver::irda_rx_callback(void* ctx, bool level, uint32_t duration) {
    IrdaAppEvent event;
    const IrdaMessage* irda_message;
    IrdaAppSignalTransceiver* this_ = static_cast<IrdaAppSignalTransceiver*>(ctx);

    irda_message = irda_decode(this_->decoder, level, duration);
    if(irda_message) {
        this_->capture_stop();
        this_->message = *irda_message;
        event.type = IrdaAppEvent::Type::IrdaMessageReceived;
        osStatus_t result = osMessageQueuePut(this_->event_queue, &event, 0, 0);
        furi_check(result == osOK);
    }
}

IrdaAppSignalTransceiver::IrdaAppSignalTransceiver(void)
    : decoder(irda_alloc_decoder()) {
}

IrdaAppSignalTransceiver::~IrdaAppSignalTransceiver() {
    api_hal_irda_rx_irq_deinit();
    irda_free_decoder(decoder);
}

void IrdaAppSignalTransceiver::capture_once_start(osMessageQueueId_t queue) {
    event_queue = queue;
    irda_reset_decoder(decoder);
    api_hal_irda_rx_irq_init();
    api_hal_irda_rx_irq_set_callback(IrdaAppSignalTransceiver::irda_rx_callback, this);
}

void IrdaAppSignalTransceiver::capture_stop(void) {
    api_hal_irda_rx_irq_deinit();
}

IrdaMessage* IrdaAppSignalTransceiver::get_last_message(void) {
    return &message;
}

void IrdaAppSignalTransceiver::send_message(const IrdaMessage* message) const {
    irda_send(message, 1);
}
