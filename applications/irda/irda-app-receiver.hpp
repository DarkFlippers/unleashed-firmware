#include <furi.h>
#include <irda.h>

class IrdaAppSignalReceiver {
public:
    IrdaAppSignalReceiver(void);
    ~IrdaAppSignalReceiver(void);
    void capture_once_start(osMessageQueueId_t event_queue);
    void capture_stop(void);
    IrdaMessage* get_last_message(void);
    void send_message(const IrdaMessage* message);

private:
    osMessageQueueId_t event_queue;
    static void irda_rx_callback(void* ctx, bool level, uint32_t duration);
    IrdaHandler* decoder;
    IrdaMessage message;
};

