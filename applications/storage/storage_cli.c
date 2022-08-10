#include <furi.h>
#include <furi_hal.h>

#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/md5.h>
#include <lib/toolbox/dir_walk.h>
#include <storage/storage.h>
#include <storage/storage_sd_api.h>
#include <power/power_service/power.h>

#define MAX_NAME_LENGTH 255

static void storage_cli_print_usage() {
    printf("Usage:\r\n");
    printf("storage <cmd> <path> <args>\r\n");
    printf("The path must start with /int or /ext\r\n");
    printf("Cmd list:\r\n");
    printf("\tinfo\t - get FS info\r\n");
    printf("\tformat\t - format filesystem\r\n");
    printf("\tlist\t - list files and dirs\r\n");
    printf("\ttree\t - list files and dirs, recursive\r\n");
    printf("\tremove\t - delete the file or directory\r\n");
    printf("\tread\t - read text from file and print file size and content to cli\r\n");
    printf(
        "\tread_chunks\t - read data from file and print file size and content to cli, <args> should contain how many bytes you want to read in block\r\n");
    printf("\twrite\t - read text from cli and append it to file, stops by ctrl+c\r\n");
    printf(
        "\twrite_chunk\t - read data from cli and append it to file, <args> should contain how many bytes you want to write\r\n");
    printf("\tcopy\t - copy file to new file, <args> must contain new path\r\n");
    printf("\trename\t - move file to new file, <args> must contain new path\r\n");
    printf("\tmkdir\t - creates a new directory\r\n");
    printf("\tmd5\t - md5 hash of the file\r\n");
    printf("\tstat\t - info about file or dir\r\n");
};

static void storage_cli_print_error(FS_Error error) {
    printf("Storage error: %s\r\n", storage_error_get_desc(error));
}

static void storage_cli_info(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);

    if(string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0) {
        uint64_t total_space;
        uint64_t free_space;
        FS_Error error =
            storage_common_fs_info(api, STORAGE_INT_PATH_PREFIX, &total_space, &free_space);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Label: %s\r\nType: LittleFS\r\n%luKB total\r\n%luKB free\r\n",
                furi_hal_version_get_name_ptr() ? furi_hal_version_get_name_ptr() : "Unknown",
                (uint32_t)(total_space / 1024),
                (uint32_t)(free_space / 1024));
        }
    } else if(string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0) {
        SDInfo sd_info;
        FS_Error error = storage_sd_info(api, &sd_info);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Label: %s\r\nType: %s\r\n%luKB total\r\n%luKB free\r\n",
                sd_info.label,
                sd_api_get_fs_type_text(sd_info.fs_type),
                sd_info.kb_total,
                sd_info.kb_free);
        }
    } else {
        storage_cli_print_usage();
    }

    furi_record_close(RECORD_STORAGE);
};

static void storage_cli_format(Cli* cli, string_t path) {
    if(string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0) {
        storage_cli_print_error(FSE_NOT_IMPLEMENTED);
    } else if(string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0) {
        printf("Formatting SD card, All data will be lost! Are you sure (y/n)?\r\n");
        char answer = cli_getc(cli);
        if(answer == 'y' || answer == 'Y') {
            Storage* api = furi_record_open(RECORD_STORAGE);
            printf("Formatting, please wait...\r\n");

            FS_Error error = storage_sd_format(api);

            if(error != FSE_OK) {
                storage_cli_print_error(error);
            } else {
                printf("SD card was successfully formatted.\r\n");
            }
            furi_record_close(RECORD_STORAGE);
        } else {
            printf("Cancelled.\r\n");
        }
    } else {
        storage_cli_print_usage();
    }
};

static void storage_cli_list(Cli* cli, string_t path) {
    UNUSED(cli);
    if(string_cmp_str(path, "/") == 0) {
        printf("\t[D] int\r\n");
        printf("\t[D] ext\r\n");
        printf("\t[D] any\r\n");
    } else {
        Storage* api = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(api);

        if(storage_dir_open(file, string_get_cstr(path))) {
            FileInfo fileinfo;
            char name[MAX_NAME_LENGTH];
            bool read_done = false;

            while(storage_dir_read(file, &fileinfo, name, MAX_NAME_LENGTH)) {
                read_done = true;
                if(fileinfo.flags & FSF_DIRECTORY) {
                    printf("\t[D] %s\r\n", name);
                } else {
                    printf("\t[F] %s %lub\r\n", name, (uint32_t)(fileinfo.size));
                }
            }

            if(!read_done) {
                printf("\tEmpty\r\n");
            }
        } else {
            storage_cli_print_error(storage_file_get_error(file));
        }

        storage_dir_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }
}

static void storage_cli_tree(Cli* cli, string_t path) {
    if(string_cmp_str(path, "/") == 0) {
        string_set(path, STORAGE_INT_PATH_PREFIX);
        storage_cli_tree(cli, path);
        string_set(path, STORAGE_EXT_PATH_PREFIX);
        storage_cli_tree(cli, path);
    } else {
        Storage* api = furi_record_open(RECORD_STORAGE);
        DirWalk* dir_walk = dir_walk_alloc(api);
        string_t name;
        string_init(name);

        if(dir_walk_open(dir_walk, string_get_cstr(path))) {
            FileInfo fileinfo;
            bool read_done = false;

            while(dir_walk_read(dir_walk, name, &fileinfo) == DirWalkOK) {
                read_done = true;
                if(fileinfo.flags & FSF_DIRECTORY) {
                    printf("\t[D] %s\r\n", string_get_cstr(name));
                } else {
                    printf("\t[F] %s %lub\r\n", string_get_cstr(name), (uint32_t)(fileinfo.size));
                }
            }

            if(!read_done) {
                printf("\tEmpty\r\n");
            }
        } else {
            storage_cli_print_error(dir_walk_get_error(dir_walk));
        }

        string_clear(name);
        dir_walk_free(dir_walk);
        furi_record_close(RECORD_STORAGE);
    }
}

static void storage_cli_read(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    if(storage_file_open(file, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        const uint16_t buffer_size = 128;
        uint16_t read_size = 0;
        uint8_t* data = malloc(buffer_size);

        printf("Size: %lu\r\n", (uint32_t)storage_file_size(file));

        do {
            read_size = storage_file_read(file, data, buffer_size);
            for(uint16_t i = 0; i < read_size; i++) {
                printf("%c", data[i]);
            }
        } while(read_size > 0);
        printf("\r\n");

        free(data);
    } else {
        storage_cli_print_error(storage_file_get_error(file));
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_write(Cli* cli, string_t path) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    const uint16_t buffer_size = 512;
    uint8_t* buffer = malloc(buffer_size);

    if(storage_file_open(file, string_get_cstr(path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
        printf("Just write your text data. New line by Ctrl+Enter, exit by Ctrl+C.\r\n");

        uint32_t read_index = 0;

        while(true) {
            uint8_t symbol = cli_getc(cli);

            if(symbol == CliSymbolAsciiETX) {
                uint16_t write_size = read_index % buffer_size;

                if(write_size > 0) {
                    uint16_t written_size = storage_file_write(file, buffer, write_size);

                    if(written_size != write_size) {
                        storage_cli_print_error(storage_file_get_error(file));
                    }
                    break;
                }
            }

            buffer[read_index % buffer_size] = symbol;
            printf("%c", buffer[read_index % buffer_size]);
            fflush(stdout);
            read_index++;

            if(((read_index % buffer_size) == 0)) {
                uint16_t written_size = storage_file_write(file, buffer, buffer_size);

                if(written_size != buffer_size) {
                    storage_cli_print_error(storage_file_get_error(file));
                    break;
                }
            }
        }
        printf("\r\n");

    } else {
        storage_cli_print_error(storage_file_get_error(file));
    }
    storage_file_close(file);

    free(buffer);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_read_chunks(Cli* cli, string_t path, string_t args) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    uint32_t buffer_size;
    int parsed_count = sscanf(string_get_cstr(args), "%lu", &buffer_size);

    if(parsed_count == EOF || parsed_count != 1) {
        storage_cli_print_usage();
    } else if(storage_file_open(file, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint8_t* data = malloc(buffer_size);
        uint64_t file_size = storage_file_size(file);

        printf("Size: %lu\r\n", (uint32_t)file_size);

        while(file_size > 0) {
            printf("\r\nReady?\r\n");
            cli_getc(cli);

            uint16_t read_size = storage_file_read(file, data, buffer_size);
            for(uint16_t i = 0; i < read_size; i++) {
                putchar(data[i]);
            }
            file_size -= read_size;
        }
        printf("\r\n");

        free(data);
    } else {
        storage_cli_print_error(storage_file_get_error(file));
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_write_chunk(Cli* cli, string_t path, string_t args) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    uint32_t buffer_size;
    int parsed_count = sscanf(string_get_cstr(args), "%lu", &buffer_size);

    if(parsed_count == EOF || parsed_count != 1) {
        storage_cli_print_usage();
    } else {
        if(storage_file_open(file, string_get_cstr(path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
            printf("Ready\r\n");

            uint8_t* buffer = malloc(buffer_size);

            for(uint32_t i = 0; i < buffer_size; i++) {
                buffer[i] = cli_getc(cli);
            }

            uint16_t written_size = storage_file_write(file, buffer, buffer_size);

            if(written_size != buffer_size) {
                storage_cli_print_error(storage_file_get_error(file));
            }

            free(buffer);
        } else {
            storage_cli_print_error(storage_file_get_error(file));
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_stat(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);

    if(string_cmp_str(path, "/") == 0) {
        printf("Storage\r\n");
    } else if(
        string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0 ||
        string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0 ||
        string_cmp_str(path, STORAGE_ANY_PATH_PREFIX) == 0) {
        uint64_t total_space;
        uint64_t free_space;
        FS_Error error =
            storage_common_fs_info(api, string_get_cstr(path), &total_space, &free_space);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Storage, %luKB total, %luKB free\r\n",
                (uint32_t)(total_space / 1024),
                (uint32_t)(free_space / 1024));
        }
    } else {
        FileInfo fileinfo;
        FS_Error error = storage_common_stat(api, string_get_cstr(path), &fileinfo);

        if(error == FSE_OK) {
            if(fileinfo.flags & FSF_DIRECTORY) {
                printf("Directory\r\n");
            } else {
                printf("File, size: %lub\r\n", (uint32_t)(fileinfo.size));
            }
        } else {
            storage_cli_print_error(error);
        }
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_copy(Cli* cli, string_t old_path, string_t args) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
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
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_remove(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_remove(api, string_get_cstr(path));

    if(error != FSE_OK) {
        storage_cli_print_error(error);
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_rename(Cli* cli, string_t old_path, string_t args) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
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
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_mkdir(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_mkdir(api, string_get_cstr(path));

    if(error != FSE_OK) {
        storage_cli_print_error(error);
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_md5(Cli* cli, string_t path) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    if(storage_file_open(file, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        const uint16_t buffer_size = 512;
        const uint8_t hash_size = 16;
        uint8_t* data = malloc(buffer_size);
        uint8_t* hash = malloc(sizeof(uint8_t) * hash_size);
        md5_context* md5_ctx = malloc(sizeof(md5_context));

        md5_starts(md5_ctx);
        while(true) {
            uint16_t read_size = storage_file_read(file, data, buffer_size);
            if(read_size == 0) break;
            md5_update(md5_ctx, data, read_size);
        }
        md5_finish(md5_ctx, hash);
        free(md5_ctx);

        for(uint8_t i = 0; i < hash_size; i++) {
            printf("%02x", hash[i]);
        }
        printf("\r\n");

        free(hash);
        free(data);
    } else {
        storage_cli_print_error(storage_file_get_error(file));
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

void storage_cli(Cli* cli, string_t args, void* context) {
    UNUSED(context);
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

        if(string_cmp_str(cmd, "tree") == 0) {
            storage_cli_tree(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "read") == 0) {
            storage_cli_read(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "read_chunks") == 0) {
            storage_cli_read_chunks(cli, path, args);
            break;
        }

        if(string_cmp_str(cmd, "write") == 0) {
            storage_cli_write(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "write_chunk") == 0) {
            storage_cli_write_chunk(cli, path, args);
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

        if(string_cmp_str(cmd, "mkdir") == 0) {
            storage_cli_mkdir(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "md5") == 0) {
            storage_cli_md5(cli, path);
            break;
        }

        if(string_cmp_str(cmd, "stat") == 0) {
            storage_cli_stat(cli, path);
            break;
        }

        storage_cli_print_usage();
    } while(false);

    string_clear(path);
    string_clear(cmd);
}

static void storage_cli_factory_reset(Cli* cli, string_t args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("All data will be lost! Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Data will be wiped after reboot.\r\n");
        furi_hal_rtc_set_flag(FuriHalRtcFlagFactoryReset);
        power_reboot(PowerBootModeNormal);
    } else {
        printf("Safe choice.\r\n");
    }
}

void storage_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, RECORD_STORAGE, CliCommandFlagParallelSafe, storage_cli, NULL);
    cli_add_command(
        cli, "factory_reset", CliCommandFlagParallelSafe, storage_cli_factory_reset, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(storage_cli_factory_reset);
#endif
}
