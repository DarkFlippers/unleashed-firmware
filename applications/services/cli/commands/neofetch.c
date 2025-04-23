#include "../cli_main_commands.h"
#include <toolbox/cli/cli_ansi.h>
#include <toolbox/version.h>
#include <furi_hal.h>
#include <furi_hal_info.h>
#include <FreeRTOS.h>
#include <FreeRTOS-Kernel/include/task.h>

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    static const char* const neofetch_logo[] = {
        "            _.-------.._                    -,",
        "        .-\"```\"--..,,_/ /`-,               -,  \\ ",
        "     .:\"          /:/  /'\\  \\     ,_...,  `. |  |",
        "    /       ,----/:/  /`\\ _\\~`_-\"`     _;",
        "   '      / /`\"\"\"'\\ \\ \\.~`_-'      ,-\"'/ ",
        "  |      | |  0    | | .-'      ,/`  /",
        " |    ,..\\ \\     ,.-\"`       ,/`    /",
        ";    :    `/`\"\"\\`           ,/--==,/-----,",
        "|    `-...|        -.___-Z:_______J...---;",
        ":         `                           _-'",
    };
#define NEOFETCH_COLOR ANSI_FLIPPER_BRAND_ORANGE

    // Determine logo parameters
    size_t logo_height = COUNT_OF(neofetch_logo), logo_width = 0;
    for(size_t i = 0; i < logo_height; i++)
        logo_width = MAX(logo_width, strlen(neofetch_logo[i]));
    logo_width += 4; // space between logo and info

    // Format hostname delimiter
    const size_t size_of_hostname = 4 + strlen(furi_hal_version_get_name_ptr());
    char delimiter[64];
    memset(delimiter, '-', size_of_hostname);
    delimiter[size_of_hostname] = '\0';

    // Get heap info
    size_t heap_total = memmgr_get_total_heap();
    size_t heap_used = heap_total - memmgr_get_free_heap();
    uint16_t heap_percent = (100 * heap_used) / heap_total;

    // Get storage info
    Storage* storage = furi_record_open(RECORD_STORAGE);
    uint64_t ext_total, ext_free, ext_used, ext_percent;
    storage_common_fs_info(storage, "/ext", &ext_total, &ext_free);
    ext_used = ext_total - ext_free;
    ext_percent = (100 * ext_used) / ext_total;
    ext_used /= 1024 * 1024;
    ext_total /= 1024 * 1024;
    furi_record_close(RECORD_STORAGE);

    // Get battery info
    uint16_t charge_percent = furi_hal_power_get_pct();
    const char* charge_state;
    if(furi_hal_power_is_charging()) {
        if((charge_percent < 100) && (!furi_hal_power_is_charging_done())) {
            charge_state = "charging";
        } else {
            charge_state = "charged";
        }
    } else {
        charge_state = "discharging";
    }

    // Get misc info
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    const Version* version = version_get();
    uint16_t major, minor;
    furi_hal_info_get_api_version(&major, &minor);

    // Print ASCII art with info
    const size_t info_height = 16;
    for(size_t i = 0; i < MAX(logo_height, info_height); i++) {
        printf(NEOFETCH_COLOR "%-*s", logo_width, (i < logo_height) ? neofetch_logo[i] : "");
        switch(i) {
        case 0: // you@<hostname>
            printf("you" ANSI_RESET "@" NEOFETCH_COLOR "%s", furi_hal_version_get_name_ptr());
            break;
        case 1: // delimiter
            printf(ANSI_RESET "%s", delimiter);
            break;
        case 2: // OS: FURI <edition> <branch> <version> <commit> (SDK <maj>.<min>)
            printf(
                "OS" ANSI_RESET ": FURI %s %s %s %s (SDK %hu.%hu)",
                version_get_version(version),
                version_get_gitbranch(version),
                version_get_version(version),
                version_get_githash(version),
                major,
                minor);
            break;
        case 3: // Host: <model> <hostname>
            printf(
                "Host" ANSI_RESET ": %s %s",
                furi_hal_version_get_model_code(),
                furi_hal_version_get_device_name_ptr());
            break;
        case 4: // Kernel: FreeRTOS <maj>.<min>.<build>
            printf(
                "Kernel" ANSI_RESET ": FreeRTOS %d.%d.%d",
                tskKERNEL_VERSION_MAJOR,
                tskKERNEL_VERSION_MINOR,
                tskKERNEL_VERSION_BUILD);
            break;
        case 5: // Uptime: ?h?m?s
            printf(
                "Uptime" ANSI_RESET ": %luh%lum%lus",
                uptime / 60 / 60,
                uptime / 60 % 60,
                uptime % 60);
            break;
        case 6: // ST7567 128x64 @ 1 bpp in 1.4"
            printf("Display" ANSI_RESET ": ST7567 128x64 @ 1 bpp in 1.4\"");
            break;
        case 7: // DE: GuiSrv
            printf("DE" ANSI_RESET ": GuiSrv");
            break;
        case 8: // Shell: CliSrv
            printf("Shell" ANSI_RESET ": CliShell");
            break;
        case 9: // CPU: STM32WB55RG @ 64 MHz
            printf("CPU" ANSI_RESET ": STM32WB55RG @ 64 MHz");
            break;
        case 10: // Memory: <used> / <total> B (??%)
            printf(
                "Memory" ANSI_RESET ": %zu / %zu B (%hu%%)", heap_used, heap_total, heap_percent);
            break;
        case 11: // Disk (/ext): <used> / <total> MiB (??%)
            printf(
                "Disk (/ext)" ANSI_RESET ": %llu / %llu MiB (%llu%%)",
                ext_used,
                ext_total,
                ext_percent);
            break;
        case 12: // Battery: ??% (<state>)
            printf("Battery" ANSI_RESET ": %hu%% (%s)" ANSI_RESET, charge_percent, charge_state);
            break;
        case 13: // empty space
            break;
        case 14: // Colors (line 1)
            for(size_t j = 30; j <= 37; j++)
                printf("\e[%dm███", j);
            break;
        case 15: // Colors (line 2)
            for(size_t j = 90; j <= 97; j++)
                printf("\e[%dm███", j);
            break;
        default:
            break;
        }
        printf("\r\n");
    }
    printf(ANSI_RESET);
#undef NEOFETCH_COLOR
}

CLI_COMMAND_INTERFACE(neofetch, execute, CliCommandFlagParallelSafe, 2048, CLI_APPID);
