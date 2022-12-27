#include "rpc_i.h"

static size_t rpc_debug_print_file_msg(
    FuriString* str,
    const char* prefix,
    const PB_Storage_File* msg_file,
    size_t msg_files_size) {
    size_t cnt = 0;

    for(size_t i = 0; i < msg_files_size; ++i, ++msg_file) {
        furi_string_cat_printf(
            str,
            "%s[%c] size: %5lu",
            prefix,
            msg_file->type == PB_Storage_File_FileType_DIR ? 'd' : 'f',
            msg_file->size);

        if(msg_file->name) {
            furi_string_cat_printf(str, " \'%s\'", msg_file->name);
        }

        if(msg_file->data && msg_file->data->size) {
            furi_string_cat_printf(
                str,
                " (%d):\'%.*s%s\'",
                msg_file->data->size,
                MIN(msg_file->data->size, 30),
                msg_file->data->bytes,
                msg_file->data->size > 30 ? "..." : "");
        }

        furi_string_cat_printf(str, "\r\n");
    }

    return cnt;
}

void rpc_debug_print_data(const char* prefix, uint8_t* buffer, size_t size) {
    FuriString* str;
    str = furi_string_alloc();
    furi_string_reserve(str, 100 + size * 5);

    furi_string_cat_printf(str, "\r\n%s DEC(%zu): {", prefix, size);
    for(size_t i = 0; i < size; ++i) {
        furi_string_cat_printf(str, "%d, ", buffer[i]);
    }
    furi_string_cat_printf(str, "}\r\n");

    printf("%s", furi_string_get_cstr(str));
    furi_string_reset(str);
    furi_string_reserve(str, 100 + size * 3);

    furi_string_cat_printf(str, "%s HEX(%zu): {", prefix, size);
    for(size_t i = 0; i < size; ++i) {
        furi_string_cat_printf(str, "%02X", buffer[i]);
    }
    furi_string_cat_printf(str, "}\r\n\r\n");

    printf("%s", furi_string_get_cstr(str));
    furi_string_free(str);
}

void rpc_debug_print_message(const PB_Main* message) {
    FuriString* str;
    str = furi_string_alloc();

    furi_string_cat_printf(
        str,
        "PB_Main: {\r\n\tresult: %d cmd_id: %lu (%s)\r\n",
        message->command_status,
        message->command_id,
        message->has_next ? "has_next" : "last");
    switch(message->which_content) {
    default:
        /* not implemented yet */
        furi_string_cat_printf(str, "\tNOT_IMPLEMENTED (%d) {\r\n", message->which_content);
        break;
    case PB_Main_stop_session_tag:
        furi_string_cat_printf(str, "\tstop_session {\r\n");
        break;
    case PB_Main_app_start_request_tag: {
        furi_string_cat_printf(str, "\tapp_start {\r\n");
        const char* name = message->content.app_start_request.name;
        const char* args = message->content.app_start_request.args;
        if(name) {
            furi_string_cat_printf(str, "\t\tname: %s\r\n", name);
        }
        if(args) {
            furi_string_cat_printf(str, "\t\targs: %s\r\n", args);
        }
        break;
    }
    case PB_Main_app_lock_status_request_tag: {
        furi_string_cat_printf(str, "\tapp_lock_status_request {\r\n");
        break;
    }
    case PB_Main_app_lock_status_response_tag: {
        furi_string_cat_printf(str, "\tapp_lock_status_response {\r\n");
        bool lock_status = message->content.app_lock_status_response.locked;
        furi_string_cat_printf(str, "\t\tlocked: %s\r\n", lock_status ? "true" : "false");
        break;
    }
    case PB_Main_storage_md5sum_request_tag: {
        furi_string_cat_printf(str, "\tmd5sum_request {\r\n");
        const char* path = message->content.storage_md5sum_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_md5sum_response_tag: {
        furi_string_cat_printf(str, "\tmd5sum_response {\r\n");
        const char* md5sum = message->content.storage_md5sum_response.md5sum;
        if(md5sum) { //-V547
            furi_string_cat_printf(str, "\t\tmd5sum: %s\r\n", md5sum);
        }
        break;
    }
    case PB_Main_system_ping_request_tag:
        furi_string_cat_printf(str, "\tping_request {\r\n");
        break;
    case PB_Main_system_ping_response_tag:
        furi_string_cat_printf(str, "\tping_response {\r\n");
        break;
    case PB_Main_system_device_info_request_tag:
        furi_string_cat_printf(str, "\tdevice_info_request {\r\n");
        break;
    case PB_Main_system_device_info_response_tag:
        furi_string_cat_printf(str, "\tdevice_info_response {\r\n");
        furi_string_cat_printf(
            str,
            "\t\t%s: %s\r\n",
            message->content.system_device_info_response.key,
            message->content.system_device_info_response.value);
        break;
    case PB_Main_storage_mkdir_request_tag:
        furi_string_cat_printf(str, "\tmkdir {\r\n");
        break;
    case PB_Main_storage_delete_request_tag: {
        furi_string_cat_printf(str, "\tdelete {\r\n");
        const char* path = message->content.storage_delete_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_empty_tag:
        furi_string_cat_printf(str, "\tempty {\r\n");
        break;
    case PB_Main_storage_info_request_tag: {
        furi_string_cat_printf(str, "\tinfo_request {\r\n");
        const char* path = message->content.storage_info_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_info_response_tag: {
        furi_string_cat_printf(str, "\tinfo_response {\r\n");
        furi_string_cat_printf(
            str, "\t\ttotal_space: %llu\r\n", message->content.storage_info_response.total_space);
        furi_string_cat_printf(
            str, "\t\tfree_space: %llu\r\n", message->content.storage_info_response.free_space);
        break;
    }
    case PB_Main_storage_stat_request_tag: {
        furi_string_cat_printf(str, "\tstat_request {\r\n");
        const char* path = message->content.storage_stat_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_stat_response_tag: {
        furi_string_cat_printf(str, "\tstat_response {\r\n");
        if(message->content.storage_stat_response.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_stat_response.file;
            rpc_debug_print_file_msg(str, "\t\t\t", msg_file, 1);
        }
        break;
    }
    case PB_Main_storage_list_request_tag: {
        furi_string_cat_printf(str, "\tlist_request {\r\n");
        const char* path = message->content.storage_list_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_read_request_tag: {
        furi_string_cat_printf(str, "\tread_request {\r\n");
        const char* path = message->content.storage_read_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_write_request_tag: {
        furi_string_cat_printf(str, "\twrite_request {\r\n");
        const char* path = message->content.storage_write_request.path;
        if(path) {
            furi_string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        if(message->content.storage_write_request.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_write_request.file;
            rpc_debug_print_file_msg(str, "\t\t\t", msg_file, 1);
        }
        break;
    }
    case PB_Main_storage_read_response_tag:
        furi_string_cat_printf(str, "\tread_response {\r\n");
        if(message->content.storage_read_response.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_read_response.file;
            rpc_debug_print_file_msg(str, "\t\t\t", msg_file, 1);
        }
        break;
    case PB_Main_storage_list_response_tag: {
        const PB_Storage_File* msg_file = message->content.storage_list_response.file;
        size_t msg_file_count = message->content.storage_list_response.file_count;
        furi_string_cat_printf(str, "\tlist_response {\r\n");
        rpc_debug_print_file_msg(str, "\t\t", msg_file, msg_file_count);
        break;
    }
    case PB_Main_storage_rename_request_tag: {
        furi_string_cat_printf(str, "\trename_request {\r\n");
        furi_string_cat_printf(
            str, "\t\told_path: %s\r\n", message->content.storage_rename_request.old_path);
        furi_string_cat_printf(
            str, "\t\tnew_path: %s\r\n", message->content.storage_rename_request.new_path);
        break;
    }
    case PB_Main_gui_start_screen_stream_request_tag:
        furi_string_cat_printf(str, "\tstart_screen_stream {\r\n");
        break;
    case PB_Main_gui_stop_screen_stream_request_tag:
        furi_string_cat_printf(str, "\tstop_screen_stream {\r\n");
        break;
    case PB_Main_gui_screen_frame_tag:
        furi_string_cat_printf(str, "\tscreen_frame {\r\n");
        break;
    case PB_Main_gui_send_input_event_request_tag:
        furi_string_cat_printf(str, "\tsend_input_event {\r\n");
        furi_string_cat_printf(
            str, "\t\tkey: %d\r\n", message->content.gui_send_input_event_request.key);
        furi_string_cat_printf(
            str, "\t\type: %d\r\n", message->content.gui_send_input_event_request.type);
        break;
    case PB_Main_gui_start_virtual_display_request_tag:
        furi_string_cat_printf(str, "\tstart_virtual_display {\r\n");
        break;
    case PB_Main_gui_stop_virtual_display_request_tag:
        furi_string_cat_printf(str, "\tstop_virtual_display {\r\n");
        break;
    }
    furi_string_cat_printf(str, "\t}\r\n}\r\n");
    printf("%s", furi_string_get_cstr(str));

    furi_string_free(str);
}
