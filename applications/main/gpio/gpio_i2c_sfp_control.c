#include "gpio_i2c_sfp_control.h"
#include <furi_hal.h>
#include <string.h>

// This is map mapping the connector type to the appropriate name. (see SFF-8024 Rev. 4.9, Table 4-3)
const char* sfp_connector_map[256] = {
    "Unknown or unspecified",
    "SC",
    "Fibre Channel Style 1",
    "Fibre Channel Style 2",
    "BNC/TNC",
    "Fibre Channel Coax",
    "Fiber Jack",
    "LC",
    "MT-RJ",
    "MU",
    "SG",
    "Optical Pigtail",
    "MP0 1x12",
    "MP 2x16",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "HSSDC II",
    "Copper pigtail",
    "RJ45",
    "No seperable connector",
    "MXC 2x16",
    "CS optical connector",
    "SN (prev. Mini CS)",
    "MPO 2x12",
    "MPO 1x16",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"};

void str_part(uint8_t* data, char* buffer, int start, int len) {
    int i;
    for(i = start; i < start + len; i++) {
        buffer[i - start] = data[i];
    }
    buffer[i - start] = '\0';
}

/*
void print_part(uint8_t *data, char *ptype, char *prefix, int start, int len){
    printf("%s: ", prefix);
    for (int i=start; i<start+len; i++){
        printf(ptype, data[i]);
    }
    printf("\n");
}
*/

void gpio_i2c_sfp_run_once(I2CSfpState* i2c_sfp_state) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    // Allocate buffer for SFP Data
    uint8_t sfp_data[255];
    uint32_t response_timeout_ticks = furi_ms_to_ticks(5.f);

    // Check for SFP Module on I2C Bus
    if(furi_hal_i2c_is_device_ready(
           &furi_hal_i2c_handle_external, SFP_I2C_ADDRESS << 1, response_timeout_ticks)) {
        for(uint8_t i = 0; i <= 254; i++) {
            uint8_t data = 0;
            // Read data from register
            furi_hal_i2c_read_reg_8(
                &furi_hal_i2c_handle_external,
                SFP_I2C_ADDRESS << 1,
                i,
                &data,
                response_timeout_ticks);
            // Save data
            sfp_data[i] = data;
        }

        /*
        print_part(sfp_data, "%c",   "Vendor", 20, 16);
        print_part(sfp_data, "%02X", "OUI", 37, 3);
        print_part(sfp_data, "%c",   "Rev", 56, 4);
        print_part(sfp_data, "%c",   "PN", 40, 16);
        print_part(sfp_data, "%c",   "SN", 68, 16);
        print_part(sfp_data, "%c",   "DC", 84,  6);

        printf("Typ: 0x%02X\r\n", sfp_data[0]);
        printf("Connector: 0x%02X\r\n", sfp_data[2]);
        printf("Bitrate: %u MBd\r\n", sfp_data[12]*100);
        printf("Wavelength: %u nm\r\n", sfp_data[60] * 256 + sfp_data[61]);
        printf("             %-6s %-6s %-6s %-6s %-6s\r\n", "SM", "OM1", "OM2", "OM3", "OM4");
        printf("Max length: %3u km %4u m %4u m %4u m %4u m\r\n", sfp_data[14], sfp_data[17]*10, sfp_data[16]*10, sfp_data[19]*10, sfp_data[18]*10);
        */

        str_part(sfp_data, i2c_sfp_state->vendor, 20, 16);
        str_part(sfp_data, i2c_sfp_state->rev, 56, 4);
        str_part(sfp_data, i2c_sfp_state->pn, 40, 16);
        str_part(sfp_data, i2c_sfp_state->sn, 68, 16);
        str_part(sfp_data, i2c_sfp_state->dc, 84, 6);

        //Look up connector in table and copy to struct.
        strcpy(i2c_sfp_state->connector, sfp_connector_map[sfp_data[2]]);
        i2c_sfp_state->bitrate = sfp_data[12] * 100;
        i2c_sfp_state->wavelength = sfp_data[60] * 256 + sfp_data[61];
        i2c_sfp_state->sm_reach = sfp_data[14];
        i2c_sfp_state->mm_reach_om3 = sfp_data[19] * 10;
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}
