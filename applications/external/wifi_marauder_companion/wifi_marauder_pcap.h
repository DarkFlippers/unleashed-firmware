#pragma once

#include "furi_hal.h"

/**
 * Creates a PCAP file to store incoming packets.
 * The file name will have a prefix according to the type of scan being performed by the application (Eg: raw_0.pcap)
 * 
 * @param app Application context
 */
void wifi_marauder_create_pcap_file(WifiMarauderApp* app);