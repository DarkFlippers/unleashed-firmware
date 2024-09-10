#include <furi.h>
#include <furi_hal.h>

#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/dir_walk.h>
#include <lib/toolbox/md5_calc.h>
#include <lib/toolbox/strint.h>
#include <lib/toolbox/tar/tar_archive.h>
#include <storage/storage.h>
#include <storage/storage_sd_api.h>
#include <power/power_service/power.h>

#define MAX_NAME_LENGTH 255

static void storage_cli_print_usage(void);

static void storage_cli_print_error(FS_Error error) {
    printf("Storage error: %s\r\n", storage_error_get_desc(error));
}

static void storage_cli_info(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);

    if(furi_string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0) {
        uint64_t total_space;
        uint64_t free_space;
        FS_Error error =
            storage_common_fs_info(api, STORAGE_INT_PATH_PREFIX, &total_space, &free_space);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Label: %s\r\nType: Virtual\r\n%luKiB total\r\n%luKiB free\r\n",
                furi_hal_version_get_name_ptr() ? furi_hal_version_get_name_ptr() : "Unknown",
                (uint32_t)(total_space / 1024),
                (uint32_t)(free_space / 1024));
        }
    } else if(furi_string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0) {
        SDInfo sd_info;
        FS_Error error = storage_sd_info(api, &sd_info);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Label: %s\r\nType: %s\r\n%luKiB total\r\n%luKiB free\r\n"
                "%02x%s %s v%i.%i\r\nSN:%04lx %02i/%i\r\n",
                sd_info.label,
                sd_api_get_fs_type_text(sd_info.fs_type),
                sd_info.kb_total,
                sd_info.kb_free,
                sd_info.manufacturer_id,
                sd_info.oem_id,
                sd_info.product_name,
                sd_info.product_revision_major,
                sd_info.product_revision_minor,
                sd_info.product_serial_number,
                sd_info.manufacturing_month,
                sd_info.manufacturing_year);
        }
    } else {
        storage_cli_print_usage();
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_format(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(args);
    if(furi_string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0) {
        storage_cli_print_error(FSE_NOT_IMPLEMENTED);
    } else if(furi_string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0) {
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
}

static void storage_cli_list(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    if(furi_string_cmp_str(path, "/") == 0) {
        printf("\t[D] int\r\n");
        printf("\t[D] ext\r\n");
        printf("\t[D] any\r\n");
    } else {
        Storage* api = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(api);

        if(storage_dir_open(file, furi_string_get_cstr(path))) {
            FileInfo fileinfo;
            char name[MAX_NAME_LENGTH];
            bool read_done = false;

            while(storage_dir_read(file, &fileinfo, name, MAX_NAME_LENGTH)) {
                read_done = true;
                if(file_info_is_dir(&fileinfo)) {
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

static void storage_cli_tree(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(args);
    if(furi_string_cmp_str(path, "/") == 0) {
        furi_string_set(path, STORAGE_INT_PATH_PREFIX);
        storage_cli_tree(cli, path, NULL);
        furi_string_set(path, STORAGE_EXT_PATH_PREFIX);
        storage_cli_tree(cli, path, NULL);
    } else {
        Storage* api = furi_record_open(RECORD_STORAGE);
        DirWalk* dir_walk = dir_walk_alloc(api);
        FuriString* name;
        name = furi_string_alloc();

        if(dir_walk_open(dir_walk, furi_string_get_cstr(path))) {
            FileInfo fileinfo;
            bool read_done = false;

            while(dir_walk_read(dir_walk, name, &fileinfo) == DirWalkOK) {
                read_done = true;
                if(file_info_is_dir(&fileinfo)) {
                    printf("\t[D] %s\r\n", furi_string_get_cstr(name));
                } else {
                    printf(
                        "\t[F] %s %lub\r\n",
                        furi_string_get_cstr(name),
                        (uint32_t)(fileinfo.size));
                }
            }

            if(!read_done) {
                printf("\tEmpty\r\n");
            }
        } else {
            storage_cli_print_error(dir_walk_get_error(dir_walk));
        }

        furi_string_free(name);
        dir_walk_free(dir_walk);
        furi_record_close(RECORD_STORAGE);
    }
}

static void storage_cli_read(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    if(storage_file_open(file, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        const size_t buffer_size = 128;
        size_t read_size = 0;
        uint8_t* data = malloc(buffer_size);

        printf("Size: %lu\r\n", (uint32_t)storage_file_size(file));

        do {
            read_size = storage_file_read(file, data, buffer_size);
            for(size_t i = 0; i < read_size; i++) {
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

static void storage_cli_write(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    const size_t buffer_size = 512;
    uint8_t* buffer = malloc(buffer_size);

    if(storage_file_open(file, furi_string_get_cstr(path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
        printf("Just write your text data. New line by Ctrl+Enter, exit by Ctrl+C.\r\n");

        uint32_t read_index = 0;

        while(true) {
            uint8_t symbol = cli_getc(cli);

            if(symbol == CliSymbolAsciiETX) {
                size_t write_size = read_index % buffer_size;

                if(write_size > 0) {
                    size_t written_size = storage_file_write(file, buffer, write_size);

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

            if((read_index % buffer_size) == 0) {
                size_t written_size = storage_file_write(file, buffer, buffer_size);

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

static void storage_cli_read_chunks(Cli* cli, FuriString* path, FuriString* args) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    uint32_t buffer_size;
    if(strint_to_uint32(furi_string_get_cstr(args), NULL, &buffer_size, 10) !=
       StrintParseNoError) {
        storage_cli_print_usage();
    } else if(storage_file_open(file, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint64_t file_size = storage_file_size(file);

        printf("Size: %llu\r\n", file_size);

        if(buffer_size) {
            uint8_t* data = malloc(buffer_size);
            while(file_size > 0) {
                printf("\r\nReady?\r\n");
                cli_getc(cli);

                size_t read_size = storage_file_read(file, data, buffer_size);
                for(size_t i = 0; i < read_size; i++) {
                    putchar(data[i]);
                }
                file_size -= read_size;
            }
            free(data);
        }
        printf("\r\n");

    } else {
        storage_cli_print_error(storage_file_get_error(file));
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_write_chunk(Cli* cli, FuriString* path, FuriString* args) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    uint32_t buffer_size;
    if(strint_to_uint32(furi_string_get_cstr(args), NULL, &buffer_size, 10) !=
       StrintParseNoError) {
        storage_cli_print_usage();
    } else {
        if(storage_file_open(file, furi_string_get_cstr(path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
            printf("Ready\r\n");

            if(buffer_size) {
                uint8_t* buffer = malloc(buffer_size);

                size_t read_bytes = cli_read(cli, buffer, buffer_size);

                size_t written_size = storage_file_write(file, buffer, read_bytes);

                if(written_size != buffer_size) {
                    storage_cli_print_error(storage_file_get_error(file));
                }

                free(buffer);
            }
        } else {
            storage_cli_print_error(storage_file_get_error(file));
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_stat(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);

    if(furi_string_cmp_str(path, "/") == 0) {
        printf("Storage\r\n");
    } else if(
        furi_string_cmp_str(path, STORAGE_EXT_PATH_PREFIX) == 0 ||
        furi_string_cmp_str(path, STORAGE_INT_PATH_PREFIX) == 0 ||
        furi_string_cmp_str(path, STORAGE_ANY_PATH_PREFIX) == 0) {
        uint64_t total_space;
        uint64_t free_space;
        FS_Error error =
            storage_common_fs_info(api, furi_string_get_cstr(path), &total_space, &free_space);

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        } else {
            printf(
                "Storage, %luKiB total, %luKiB free\r\n",
                (uint32_t)(total_space / 1024),
                (uint32_t)(free_space / 1024));
        }
    } else {
        FileInfo fileinfo;
        FS_Error error = storage_common_stat(api, furi_string_get_cstr(path), &fileinfo);

        if(error == FSE_OK) {
            if(file_info_is_dir(&fileinfo)) {
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

static void storage_cli_timestamp(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);

    uint32_t timestamp = 0;
    FS_Error error = storage_common_timestamp(api, furi_string_get_cstr(path), &timestamp);

    if(error != FSE_OK) {
        printf("Invalid arguments\r\n");
    } else {
        printf("Timestamp %lu\r\n", timestamp);
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_copy(Cli* cli, FuriString* old_path, FuriString* args) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FuriString* new_path;
    new_path = furi_string_alloc();

    if(!args_read_probably_quoted_string_and_trim(args, new_path)) {
        storage_cli_print_usage();
    } else {
        FS_Error error = storage_common_copy(
            api, furi_string_get_cstr(old_path), furi_string_get_cstr(new_path));

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        }
    }

    furi_string_free(new_path);
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_remove(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_remove(api, furi_string_get_cstr(path));

    if(error != FSE_OK) {
        storage_cli_print_error(error);
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_rename(Cli* cli, FuriString* old_path, FuriString* args) {
    UNUSED(cli);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FuriString* new_path;
    new_path = furi_string_alloc();

    if(!args_read_probably_quoted_string_and_trim(args, new_path)) {
        storage_cli_print_usage();
    } else {
        FS_Error error = storage_common_rename(
            api, furi_string_get_cstr(old_path), furi_string_get_cstr(new_path));

        if(error != FSE_OK) {
            storage_cli_print_error(error);
        }
    }

    furi_string_free(new_path);
    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_mkdir(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_mkdir(api, furi_string_get_cstr(path));

    if(error != FSE_OK) {
        storage_cli_print_error(error);
    }

    furi_record_close(RECORD_STORAGE);
}

static void storage_cli_md5(Cli* cli, FuriString* path, FuriString* args) {
    UNUSED(cli);
    UNUSED(args);
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);
    FuriString* md5 = furi_string_alloc();
    FS_Error file_error;

    if(md5_string_calc_file(file, furi_string_get_cstr(path), md5, &file_error)) {
        printf("%s\r\n", furi_string_get_cstr(md5));
    } else {
        storage_cli_print_error(file_error);
    }

    furi_string_free(md5);
    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static bool tar_extract_file_callback(const char* name, bool is_directory, void* context) {
    UNUSED(context);
    printf("\t%s %s\r\n", is_directory ? "D" : "F", name);
    return true;
}

static void storage_cli_extract(Cli* cli, FuriString* old_path, FuriString* args) {
    UNUSED(cli);
    FuriString* new_path = furi_string_alloc();

    if(!args_read_probably_quoted_string_and_trim(args, new_path)) {
        storage_cli_print_usage();
        furi_string_free(new_path);
        return;
    }

    Storage* api = furi_record_open(RECORD_STORAGE);

    TarArchive* archive = tar_archive_alloc(api);
    TarOpenMode tar_mode = tar_archive_get_mode_for_path(furi_string_get_cstr(old_path));
    do {
        if(!tar_archive_open(archive, furi_string_get_cstr(old_path), tar_mode)) {
            printf("Failed to open archive\r\n");
            break;
        }
        uint32_t start_tick = furi_get_tick();
        tar_archive_set_file_callback(archive, tar_extract_file_callback, NULL);
        printf("Unpacking to %s\r\n", furi_string_get_cstr(new_path));
        bool success = tar_archive_unpack_to(archive, furi_string_get_cstr(new_path), NULL);
        uint32_t end_tick = furi_get_tick();
        printf(
            "Decompression %s in %lu ticks \r\n",
            success ? "success" : "failed",
            end_tick - start_tick);
    } while(false);

    tar_archive_free(archive);
    furi_string_free(new_path);
    furi_record_close(RECORD_STORAGE);
}

typedef void (*StorageCliCommandCallback)(Cli* cli, FuriString* path, FuriString* args);

typedef struct {
    const char* command;
    const char* help;
    const StorageCliCommandCallback impl;
} StorageCliCommand;

static const StorageCliCommand storage_cli_commands[] = {
    {
        "write_chunk",
        "read data from cli and append it to file, <args> should contain how many bytes you want to write",
        &storage_cli_write_chunk,
    },
    {
        "read_chunks",
        "read data from file and print file size and content to cli, <args> should contain how many bytes you want to read in block",
        &storage_cli_read_chunks,
    },
    {
        "list",
        "list files and dirs",
        &storage_cli_list,
    },
    {
        "md5",
        "md5 hash of the file",
        &storage_cli_md5,
    },
    {
        "stat",
        "info about file or dir",
        &storage_cli_stat,
    },
    {
        "info",
        "get FS info",
        &storage_cli_info,
    },
    {
        "tree",
        "list files and dirs, recursive",
        &storage_cli_tree,
    },
    {
        "read",
        "read text from file and print file size and content to cli",
        &storage_cli_read,
    },
    {
        "write",
        "read text from cli and append it to file, stops by ctrl+c",
        &storage_cli_write,
    },
    {
        "copy",
        "copy file to new file, <args> must contain new path",
        &storage_cli_copy,
    },
    {
        "remove",
        "delete the file or directory",
        &storage_cli_remove,
    },
    {
        "rename",
        "move file to new file, <args> must contain new path",
        &storage_cli_rename,
    },
    {
        "mkdir",
        "creates a new directory",
        &storage_cli_mkdir,
    },
    {
        "timestamp",
        "last modification timestamp",
        &storage_cli_timestamp,
    },
    {
        "extract",
        "extract tar archive to destination",
        &storage_cli_extract,
    },
    {
        "format",
        "format filesystem",
        &storage_cli_format,
    },
};

static void storage_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("storage <cmd> <path> <args>\r\n");
    printf("The path must start with /int or /ext\r\n");
    printf("Cmd list:\r\n");

    for(size_t i = 0; i < COUNT_OF(storage_cli_commands); ++i) {
        const StorageCliCommand* command_descr = &storage_cli_commands[i];
        const char* cli_cmd = command_descr->command;
        printf(
            "\t%s%s - %s\r\n", cli_cmd, strlen(cli_cmd) > 8 ? "\t" : "\t\t", command_descr->help);
    }
}

void storage_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    FuriString* path;
    cmd = furi_string_alloc();
    path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            storage_cli_print_usage();
            break;
        }

        if(!args_read_probably_quoted_string_and_trim(args, path)) {
            storage_cli_print_usage();
            break;
        }

        size_t i = 0;
        for(; i < COUNT_OF(storage_cli_commands); ++i) {
            const StorageCliCommand* command_descr = &storage_cli_commands[i];
            if(furi_string_cmp_str(cmd, command_descr->command) == 0) {
                command_descr->impl(cli, path, args);
                break;
            }
        }

        if(i == COUNT_OF(storage_cli_commands)) {
            storage_cli_print_usage();
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}

static void storage_cli_factory_reset(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("All data will be lost! Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Data will be wiped after reboot.\r\n");

        furi_hal_rtc_reset_registers();
        furi_hal_rtc_set_flag(FuriHalRtcFlagStorageFormatInternal);

        Power* power = furi_record_open(RECORD_POWER);
        power_reboot(power, PowerBootModeNormal);
    } else {
        printf("Safe choice.\r\n");
    }
}

void storage_on_system_start(void) {
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
