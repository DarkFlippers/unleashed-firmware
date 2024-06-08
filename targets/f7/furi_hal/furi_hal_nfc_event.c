#include <furi_hal_nfc_i.h>

FuriHalNfcEventInternal* furi_hal_nfc_event = NULL;

void furi_hal_nfc_event_init(void) {
    furi_hal_nfc_event = malloc(sizeof(FuriHalNfcEventInternal));
}

FuriHalNfcError furi_hal_nfc_event_start(void) {
    furi_check(furi_hal_nfc_event);

    furi_hal_nfc_event->thread = furi_thread_get_current_id();
    furi_thread_flags_clear(FURI_HAL_NFC_EVENT_INTERNAL_ALL);

    return FuriHalNfcErrorNone;
}

FuriHalNfcError furi_hal_nfc_event_stop(void) {
    furi_check(furi_hal_nfc_event);

    furi_hal_nfc_event->thread = NULL;

    return FuriHalNfcErrorNone;
}

void furi_hal_nfc_event_set(FuriHalNfcEventInternalType event) {
    furi_check(furi_hal_nfc_event);

    if(furi_hal_nfc_event->thread) {
        furi_thread_flags_set(furi_hal_nfc_event->thread, event);
    }
}

FuriHalNfcError furi_hal_nfc_abort(void) {
    furi_hal_nfc_event_set(FuriHalNfcEventInternalTypeAbort);
    return FuriHalNfcErrorNone;
}

FuriHalNfcEvent furi_hal_nfc_wait_event_common(uint32_t timeout_ms) {
    furi_check(furi_hal_nfc_event);
    furi_check(furi_hal_nfc_event->thread);

    FuriHalNfcEvent event = 0;
    uint32_t event_timeout = timeout_ms == FURI_HAL_NFC_EVENT_WAIT_FOREVER ? FuriWaitForever :
                                                                             timeout_ms;
    uint32_t event_flag =
        furi_thread_flags_wait(FURI_HAL_NFC_EVENT_INTERNAL_ALL, FuriFlagWaitAny, event_timeout);
    if(event_flag != (unsigned)FuriFlagErrorTimeout) {
        if(event_flag & FuriHalNfcEventInternalTypeIrq) {
            furi_thread_flags_clear(FuriHalNfcEventInternalTypeIrq);
            FuriHalSpiBusHandle* handle = &furi_hal_spi_bus_handle_nfc;
            uint32_t irq = furi_hal_nfc_get_irq(handle);
            if(irq & ST25R3916_IRQ_MASK_OSC) {
                event |= FuriHalNfcEventOscOn;
            }
            if(irq & ST25R3916_IRQ_MASK_TXE) {
                event |= FuriHalNfcEventTxEnd;
            }
            if(irq & ST25R3916_IRQ_MASK_RXS) {
                event |= FuriHalNfcEventRxStart;
            }
            if(irq & ST25R3916_IRQ_MASK_RXE) {
                event |= FuriHalNfcEventRxEnd;
            }
            if(irq & ST25R3916_IRQ_MASK_COL) {
                event |= FuriHalNfcEventCollision;
            }
            if(irq & ST25R3916_IRQ_MASK_EON) {
                event |= FuriHalNfcEventFieldOn;
            }
            if(irq & ST25R3916_IRQ_MASK_EOF) {
                event |= FuriHalNfcEventFieldOff;
            }
            if(irq & ST25R3916_IRQ_MASK_WU_A) {
                event |= FuriHalNfcEventListenerActive;
            }
            if(irq & ST25R3916_IRQ_MASK_WU_A_X) {
                event |= FuriHalNfcEventListenerActive;
            }
            if(irq & ST25R3916_IRQ_MASK_WU_F) {
                event |= FuriHalNfcEventListenerActive;
            }
        }
        if(event_flag & FuriHalNfcEventInternalTypeTimerFwtExpired) {
            event |= FuriHalNfcEventTimerFwtExpired;
            furi_thread_flags_clear(FuriHalNfcEventInternalTypeTimerFwtExpired);
        }
        if(event_flag & FuriHalNfcEventInternalTypeTimerBlockTxExpired) {
            event |= FuriHalNfcEventTimerBlockTxExpired;
            furi_thread_flags_clear(FuriHalNfcEventInternalTypeTimerBlockTxExpired);
        }
        if(event_flag & FuriHalNfcEventInternalTypeAbort) {
            event |= FuriHalNfcEventAbortRequest;
            furi_thread_flags_clear(FuriHalNfcEventInternalTypeAbort);
        }
    } else {
        event = FuriHalNfcEventTimeout;
    }

    return event;
}

bool furi_hal_nfc_event_wait_for_specific_irq(
    FuriHalSpiBusHandle* handle,
    uint32_t mask,
    uint32_t timeout_ms) {
    furi_check(furi_hal_nfc_event);
    furi_check(furi_hal_nfc_event->thread);

    bool irq_received = false;
    uint32_t event_flag =
        furi_thread_flags_wait(FuriHalNfcEventInternalTypeIrq, FuriFlagWaitAny, timeout_ms);
    if(event_flag == FuriHalNfcEventInternalTypeIrq) {
        uint32_t irq = furi_hal_nfc_get_irq(handle);
        irq_received = ((irq & mask) == mask);
        furi_thread_flags_clear(FuriHalNfcEventInternalTypeIrq);
    }

    return irq_received;
}
