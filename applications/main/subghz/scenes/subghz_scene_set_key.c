#include "../subghz_i.h"
#include "../helpers/subghz_txrx_create_protocol_key.h"

#include <machine/endian.h>

#define TAG "SubGhzSetKey"

void subghz_scene_set_key_byte_input_callback(void* context) {
    SubGhz* subghz = context;

    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventByteInputDone);
}

void subghz_scene_set_key_on_enter(void* context) {
    SubGhz* subghz = context;

    uint8_t* byte_ptr = NULL;
    uint8_t byte_count = 0;
    const char* header_text = "Enter KEY in hex";

    if(subghz->gen_info->type == GenData) {
        byte_ptr = (uint8_t*)&subghz->gen_info->data.key;
        byte_count = sizeof(subghz->gen_info->data.key);
    } else if(subghz->gen_info->type == GenCardinS508) {
        if(subghz->gen_info->cardin_s508.rolling) {
            byte_ptr = (uint8_t*)&subghz->gen_info->cardin_s508.key_hi;
            byte_count = sizeof(subghz->gen_info->cardin_s508.key_hi) +
                         sizeof(subghz->gen_info->cardin_s508.key_lo);
            subghz->gen_info->cardin_s508.key_hi = __bswap64(subghz->gen_info->cardin_s508.key_hi);
            subghz->gen_info->cardin_s508.key_lo = __bswap64(subghz->gen_info->cardin_s508.key_lo);
            header_text = "Enter 128-bit CARDIN KEY";
        } else {
            byte_ptr = (uint8_t*)&subghz->gen_info->cardin_s508.payload_hi;
            byte_count = sizeof(subghz->gen_info->cardin_s508.payload_hi) +
                         sizeof(subghz->gen_info->cardin_s508.payload_lo);
            subghz->gen_info->cardin_s508.payload_hi =
                __bswap64(subghz->gen_info->cardin_s508.payload_hi);
            subghz->gen_info->cardin_s508.payload_lo =
                __bswap64(subghz->gen_info->cardin_s508.payload_lo);
            header_text = "Enter 128-bit PAYLOAD";
        }
    } else {
        furi_crash("Not implemented");
    }

    furi_assert(byte_ptr);
    furi_assert(byte_count > 0);

    if(subghz->gen_info->type == GenData) {
        subghz->gen_info->data.key = __bswap64(subghz->gen_info->data.key);
    }

    // Setup view
    ByteInput* byte_input = subghz->byte_input;
    byte_input_set_header_text(byte_input, header_text);
    byte_input_set_result_callback(
        byte_input, subghz_scene_set_key_byte_input_callback, NULL, subghz, byte_ptr, byte_count);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
}

bool subghz_scene_set_key_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    bool generated_protocol = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventByteInputDone) {
            if(subghz->gen_info->type == GenData) {
                subghz->gen_info->data.key = __bswap64(subghz->gen_info->data.key);

                if(subghz->gen_info->data.te) {
                    generated_protocol = subghz_txrx_gen_data_protocol_and_te(
                        subghz->txrx,
                        subghz->gen_info->mod,
                        subghz->gen_info->freq,
                        subghz->gen_info->data.name,
                        subghz->gen_info->data.key,
                        subghz->gen_info->data.bits,
                        subghz->gen_info->data.te);
                } else {
                    generated_protocol = subghz_txrx_gen_data_protocol(
                        subghz->txrx,
                        subghz->gen_info->mod,
                        subghz->gen_info->freq,
                        subghz->gen_info->data.name,
                        subghz->gen_info->data.key,
                        subghz->gen_info->data.bits);
                }
            } else if(subghz->gen_info->type == GenCardinS508) {
                if(subghz->gen_info->cardin_s508.rolling) {
                    subghz->gen_info->cardin_s508.key_hi =
                        __bswap64(subghz->gen_info->cardin_s508.key_hi);
                    subghz->gen_info->cardin_s508.key_lo =
                        __bswap64(subghz->gen_info->cardin_s508.key_lo);
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetCounter);
                    consumed = true;
                    return consumed;
                } else {
                    subghz->gen_info->cardin_s508.payload_hi =
                        __bswap64(subghz->gen_info->cardin_s508.payload_hi);
                    subghz->gen_info->cardin_s508.payload_lo =
                        __bswap64(subghz->gen_info->cardin_s508.payload_lo);
                    generated_protocol = subghz_txrx_gen_cardin_s508_protocol(
                        subghz->txrx,
                        subghz->gen_info->mod,
                        subghz->gen_info->freq,
                        subghz->gen_info->cardin_s508.payload_hi,
                        subghz->gen_info->cardin_s508.payload_lo);
                }
            }

            consumed = true;

            if(!generated_protocol) {
                furi_string_set(
                    subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            } else {
                subghz_file_name_clear(subghz);

                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneSetType, SubGhzCustomEventManagerSet);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            }
        }
    }
    return consumed;
}

void subghz_scene_set_key_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    byte_input_set_result_callback(subghz->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(subghz->byte_input, "");
}
