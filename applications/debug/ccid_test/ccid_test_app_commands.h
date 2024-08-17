#include "iso7816/iso7816_t0_apdu.h"

void iso7816_answer_to_reset(Iso7816Atr* atr);

void iso7816_process_command(
    const ISO7816_Command_APDU* command_apdu,
    ISO7816_Response_APDU* response_apdu);
