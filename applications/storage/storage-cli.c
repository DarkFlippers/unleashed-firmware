#include <furi.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <storage/storage.h>
#include <storage/storage-sd-api.h>
#include <api-hal-version.h>

#define MAX_NAME_LENGTH 255

void storage_cli(Cli* cli, string_t args, void* context);

// app cli function
void storage_cli_init() {
    Cli* cli = furi_record_open("cli");
    cli_add_command(cli, "storage", CliCommandFlagDefault, storage_cli, NULL);
    furi_record_close("cli");
}

void storage_cli_print_usage() {
    printf("Usage:\r\n");
    printf("storage <cmd> <path> <args>\r\n");
    printf("The path must start with /int or /ext\r\n");
    printf("Cmd list:\r\n");
    printf("\tinfo\t - get FS info\r\n");
    printf("\tformat\t - format filesystem\r\n");
    printf("\tlist\t - list files and dirs\r\n");
    printf("\tremove\t - delete the file or directory\r\n");
    printf("\tread\t - read data from file and print file size and content to cli\r\n");
    printf(
        "\twrite\t - read data from cli and append it to file, <args> should contain how many bytes you want to write\r\n");
    printf("\tcopy\t - copy file to new file, <args> must contain new path\r\n");
    printf("\trename\t - move file to new file, <args> must contain new path\r\n");
};

void storage_cli_print_error(FS_Error error) {
    printf("Storage error: %s\r\n", storage_error_get_desc(error));
}

void storage_cli_print_path_error(string_t path, FS_Error error) {
    printf(
        "Storage error for path \"%s\": %s\r\n",
        string_get_cstr(path),
        storage_error_get_desc(error));
}

void storage_cli_print_file_error(string_t path, File* file) {
    printf(
        "Storage error for path \"%s\": %s\r\n",
        string_get_cstr(path),
        storage_file_get_error_desc(file));
}

void storage_cli_info(Cli* cli, string_t path) {
    Storage* api = furi_record_open("storage");

    if(string_cmp_str(path, "/int") == 0) {
        uint64_t total_space;
        uint64_t free_space;
        FS_Error error = storage_common_fs_info(api, "/int", &total_space, &free_space);

        if(error != FSE_OK) {
            storage_cli_print_path_error(path, error);
        } else {
            printf(
                "Label: %s\r\nType: LittleFS\r\n%lu KB total\r\n%lu KB free\r\n",
                api_hal_version_get_name_ptr(),
                (uint32_t)(total_space / 1024),
                (uint32_t)(free_space / 1024));
        }
    } else if(string_cmp_str(path, "/ext") == 0) {
        SDInfo sd_info;
        FS_Error error = storage_sd_info(api, &sd_info);

        if(error != FSE_OK) {
            storage_cli_print_path_error(path, error);
        } else {
            printf(
                "Label: %s\r\nType: %s\r\n%lu KB total\r\n%lu KB free\r\n",
                sd_info.label,
                sd_api_get_fs_type_text(sd_info.fs_type),
                sd_info.kb_total,
                sd_info.kb_free);
        }
    } else {
        storage_cli_print_usage();
    }

    furi_record_close("storage");
};

void storage_cli_format(Cli* cli, string_t path) {
    if(string_cmp_str(path, "/int") == 0) {
        storage_cli_print_path_error(path, FSE_NOT_IMPLEMENTED);
    } else if(string_cmp_str(path, "/ext") == 0) {
        printf("Formatting SD card, all data will be lost. Are you sure (y/n)?\r\n");
        char answer = cli_getc(cli);
        if(answer == 'y' || answer == 'Y') {
            Storage* api = furi_record_open("storage");
            printf("Formatting, please wait...\r\n");

            FS_Error error = storage_sd_format(api);

            if(error != FSE_OK) {
                storage_cli_print_path_error(path, error);
            } else {
                printf("SD card was successfully formatted.\r\n");
            }
            furi_record_close("storage");
        } else {
            printf("Cancelled.\r\n");
        }
    } else {
        storage_cli_print_usage();
    }
};

void storage_cli_list(Cli* cli, string_t path) {
    if(string_cmp_str(path, "/") == 0) {
        printf("\t[D] int\r\n");
        printf("\t[D] ext\r\n");
        printf("\t[D] any\r\n");
    } else {
        Storage* api = furi_record_open("storage");
        File* file = storage_file_alloc(api);

        if(storage_dir_open(file, string_get_cstr(path))) {
            FileInfo fileinfo;
            char name[MAX_NAME_LENGTH];
            bool readed = false;

            while(storage_dir_read(file, &fileinfo, name, MAX_NAME_LENGTH)) {
                readed = true;
                if(fileinfo.flags & FSF_DIRECTORY) {
                    printf("\t[D] %s\r\n", name);
                } else {
                    printf("\t[F] %s %lub\r\n", name, (uint32_t)(fileinfo.size));
                }
            }

            if(!readed) {
                printf("\tEmpty\r\n");
            }
        } else {
            storage_cli_print_file_error(path, file);
        }

        storage_dir_close(file);
        storage_file_free(file);
        furi_record_close("storage");
    }
}

void storage_cli_read(Cli* cli, string_t path) {
    Storage* api = furi_record_open("storage");
    File* file = storage_file_alloc(api);

    if(storage_file_open(file, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        const uint16_t read_size = 128;
        uint16_t readed_size = 0;
        uint8_t* data = furi_alloc(read_size);

        printf("Size: %lu\r\n", (uint32_t)storage_file_size(file));

        do {
            readed_size = storage_file_read(file, data, read_size);
            for(uint16_t i = 0; i < readed_size; i++) {
                printf("%c", data[i]);
            }
        } while(readed_size > 0);
        printf("\r\n");

        free(data);
    } else {
        storage_cli_print_file_error(path, file);
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close("storage");
}

void storage_cli_write(Cli* cli, string_t path, string_t args) {
    Storage* api = furi_record_open("storage");
    File* file = storage_file_alloc(api);

    uint32_t size;
    int parsed_count = sscanf(string_get_cstr(args), "%lu", &size);

    if(parsed_count == EOF || parsed_count != 1) {
        storage_cli_print_usage();
    } else {
        if(storage_file_open(file, string_get_cstr(path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
            const uint16_t write_size = 8;
            uint32_t readed_index = 0;
            uint8_t* data = furi_alloc(write_size);

            while(true) {
                data[readed_index % write_size] = cli_getc(cli);
                printf("%c", data[readed_index % write_size]);
                fflush(stdout);
                readed_index++;

                if(((readed_index % write_size) == 0)) {
                    uint16_t writed_size = storage_file_write(file, data, write_size);

                    if(writed_size != write_size) {
                        storage_cli_print_file_error(path, file);
                        break;
                    }
                } else if(readed_index == size) {
                    uint16_t writed_size = storage_file_write(file, data, size % write_size);

                    if(writed_size != (size % write_size)) {
                        storage_cli_print_file_error(path, file);
                        break;
                    }
                }

                if(readed_index == size) {
                    break;
                }
            }
            printf("\r\n");

            free(data);
        } else {
            storage_cli_print_file_error(path, file);
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close("storage");
}

void storage_cli_copy(Cli* cli, string_t old_path, string_t args) {
    Storage* api = furi_record_open("storage");
    string_t new_path;
    string_init(new_path);

    if(!args_read_probably_quoted_string_and_trim(args, new_path)) {
        storage_cli_print_usage();
    } else {
        FS_Error error =
            storage_common_copy(api, string_get_cstr(old_path), string_get_cstr(new_path));

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        }
    }

    string_clear(new_path);
    furi_record_close("storage");
}

void storage_cli_remove(Cli* cli, string_t path) {
    Storage* api = furi_record_open("storage");
    FS_Error error = storage_common_remove(api, string_get_cstr(path));

    if(error != FSE_OK) {
        storage_cli_print_error(error);
    }

    furi_record_close("storage");
}

void storage_cli_rename(Cli* cli, string_t old_path, string_t args) {
    Storage* api = furi_record_open("storage");
    string_t new_path;
    string_init(new_path);

    if(!args_read_probably_quoted_string_and_trim(args, new_path)) {
        storage_cli_print_usage();
    } else {
        FS_Error error =
            storage_common_rename(api, string_get_cstr(old_path), string_get_cstr(new_path));

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        }
    }

    string_clear(new_path);
    furi_record_close("storage");
}

void storage_cli(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_t path;
    string_init(cmd);
    string_init(path);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            storage_cli_print_usage();
            break;
        }

        if(!args_read_probably_quoted_string_and_trim(args, path)) {
            storage_cli_print_usage();
            break;
        }

        if(string_cmp_str(cmd, "info") == 0) {
            storage_cli_info(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "format") == 0) {
            storage_cli_format(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "list") == 0) {
            storage_cli_list(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "read") == 0) {
            storage_cli_read(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "write") == 0) {
            storage_cli_write(cli, path, args);
            break;
        }

        if(string_cmp_str(cmd, "copy") == 0) {
            storage_cli_copy(cli, path, args);
            break;
        }

        if(string_cmp_str(cmd, "remove") == 0) {
            storage_cli_remove(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "rename") == 0) {
            storage_cli_rename(cli, path, args);
            break;
        }

        storage_cli_print_usage();
    } while(false);

    string_clear(path);
    string_clear(cmd);
}