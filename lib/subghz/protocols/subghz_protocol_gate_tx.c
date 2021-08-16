#include "subghz_protocol_gate_tx.h"


struct SubGhzProtocolGateTX {
    SubGhzProtocolCommon common;
};

SubGhzProtocolGateTX* subghz_protocol_gate_tx_alloc(void) {
    SubGhzProtocolGateTX* instance = furi_alloc(sizeof(SubGhzProtocolGateTX));

    instance->common.name = "GateTX";
    instance->common.code_min_count_bit_for_found = 24;
    instance->common.te_short = 350;
    instance->common.te_long = 700;
    instance->common.te_delta = 100;
    instance->common.type_protocol = TYPE_PROTOCOL_STATIC;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_gate_tx_to_str;
    instance->common.to_save_string =
        (SubGhzProtocolCommonGetStrSave)subghz_protocol_gate_tx_to_save_str;
    instance->common.to_load_protocol=
        (SubGhzProtocolCommonLoad)subghz_protocol_gate_tx_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolEncoderCommonGetUpLoad)subghz_protocol_gate_tx_send_key;
    return instance;
}

void subghz_protocol_gate_tx_free(SubGhzProtocolGateTX* instance) {
    furi_assert(instance);
    free(instance);
}

bool subghz_protocol_gate_tx_send_key(SubGhzProtocolGateTX* instance, SubGhzProtocolEncoderCommon* encoder){
    furi_assert(instance);
    furi_assert(encoder);
    size_t index = 0;
    encoder->size_upload =(instance->common.code_last_count_bit * 2) + 2;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;
    //Send header
    encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short * 49);
    //Send start bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_long);
    //Send key data
    for (uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
        if(bit_read(instance->common.code_last_found, i - 1)){
            //send bit 1
            encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_long);
            encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
        }else{
            //send bit 0
            encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);
            encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_long);
        }
    }
    return true;
}

void subghz_protocol_gate_tx_reset(SubGhzProtocolGateTX* instance) {
    instance->common.parser_step = 0;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolFaacSLH instance
 */
void subghz_protocol_gate_tx_check_remote_controller(SubGhzProtocolGateTX* instance) {
    uint32_t code_found_reverse = subghz_protocol_common_reverse_key(instance->common.code_last_found, instance->common.code_last_count_bit);

    instance->common.serial = (code_found_reverse & 0xFF) << 12 | ((code_found_reverse >>8) & 0xFF) << 4 | ((code_found_reverse >>20) & 0x0F) ;
    instance->common.btn = ((code_found_reverse >> 16) & 0x0F);
}

void subghz_protocol_gate_tx_parse(SubGhzProtocolGateTX* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if ((!level)
                && (DURATION_DIFF(duration,instance->common.te_short * 47)< instance->common.te_delta * 47)) {
            //Found Preambula
            instance->common.parser_step = 1;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        if (level && ((DURATION_DIFF(duration,instance->common.te_long)< instance->common.te_delta*3))){
            //Found start bit
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (!level) {
            if (duration >= (instance->common.te_short * 10 + instance->common.te_delta)) {
                instance->common.parser_step = 1;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {
                    
                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;

                    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                break;
            } else {
                instance->common.te_last = duration;
                instance->common.parser_step = 3;
            }
        }
         break;
    case 3:
        if(level){
            if ((DURATION_DIFF(instance->common.te_last,instance->common.te_short)< instance->common.te_delta)
                    && (DURATION_DIFF(duration,instance->common.te_long)< instance->common.te_delta*3)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long)< instance->common.te_delta*3)
                    && (DURATION_DIFF(duration,instance->common.te_short)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 2;
            } else {
                instance->common.parser_step = 0;
            }
        }else{
            instance->common.parser_step = 0;
        }
        break;
    }
}

void subghz_protocol_gate_tx_to_str(SubGhzProtocolGateTX* instance, string_t output) {
    subghz_protocol_gate_tx_check_remote_controller(instance);
    string_cat_printf(output,
                      "%s, %d Bit\r\n"
                      " KEY:%06lX\r\n"
                      " SN:%05lX  BTN:%lX\r\n",
                      instance->common.name,
                      instance->common.code_last_count_bit,
                      (uint32_t)(instance->common.code_last_found & 0xFFFFFF),
                      instance->common.serial, 
                      instance->common.btn
                      );
}

void subghz_protocol_gate_tx_to_save_str(SubGhzProtocolGateTX* instance, string_t output) {
    string_printf(
        output,
        "Protocol: %s\n"
        "Bit: %d\n"
        "Key: %08lX\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found & 0x00000000ffffffff));
}

bool subghz_protocol_gate_tx_to_load_protocol(FileWorker* file_worker, SubGhzProtocolGateTX* instance){
    bool loaded = false;
    string_t temp_str;
    string_init(temp_str);
    int res = 0;
    int data = 0;

    do {
        // Read and parse bit data from 2nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Bit: %d\n", &data);
        if(res != 1) {
            break;
        }
        instance->common.code_last_count_bit = (uint8_t)data;

        // Read and parse key data from 3nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        uint32_t temp_key = 0;
        res = sscanf(string_get_cstr(temp_str), "Key: %08lX\n", &temp_key);
        if(res != 1) {
            break;
        }
        instance->common.code_last_found = (uint64_t)temp_key;
        subghz_protocol_gate_tx_check_remote_controller(instance);

        loaded = true;
    } while(0);

    string_clear(temp_str);

    return loaded;
}
