#pragma once

#include "furi_hal.h"

/**
 * Creates a PCAP file to store incoming packets.
 * The file name will have a prefix according to the type of scan being performed by the application (Eg: raw_0.pcap)
 * 
 * @param app Application context
 */
void wifi_marauder_create_pcap_file(WifiMarauderApp* app);

/**
 * Creates a log file to store text from console output.
 * The file name will have a prefix according to the command being performed by the application (Eg: scanap_0.log)
 *
 * @param app Application context
 */
// same as wifi_marauder_create_pcap_file, but for log files (to save console text output)
void wifi_marauder_create_log_file(WifiMarauderApp* app);
