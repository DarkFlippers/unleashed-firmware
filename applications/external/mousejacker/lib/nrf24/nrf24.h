#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_spi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define REGISTER_MASK 0x1F
#define ACTIVATE 0x50
#define R_RX_PL_WID 0x60
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define W_TX_PAYLOAD_NOACK 0xB0
#define W_ACK_PAYLOAD 0xA8
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define REUSE_TX_PL 0xE3
#define RF24_NOP 0xFF

#define REG_CONFIG 0x00
#define REG_EN_AA 0x01
#define REG_EN_RXADDR 0x02
#define REG_SETUP_AW 0x03
#define REG_SETUP_RETR 0x04
#define REG_DYNPD 0x1C
#define REG_FEATURE 0x1D
#define REG_RF_SETUP 0x06
#define REG_STATUS 0x07
#define REG_RX_ADDR_P0 0x0A
#define REG_RF_CH 0x05
#define REG_TX_ADDR 0x10

#define RX_PW_P0 0x11
#define TX_DS 0x20
#define MAX_RT 0x10

#define nrf24_TIMEOUT 500
#define nrf24_CE_PIN &gpio_ext_pb2
#define nrf24_HANDLE &furi_hal_spi_bus_handle_external

/* Low level API */

/** Write device register
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      reg     - register
 * @param      data    - data to write
 *
 * @return     device status
 */
uint8_t nrf24_write_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t data);

/** Write buffer to device register
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      reg     - register
 * @param      data    - data to write
 * @param      size    - size of data to write
 *
 * @return     device status
 */
uint8_t nrf24_write_buf_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* data, uint8_t size);

/** Read device register
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      reg     - register
 * @param[out] data    - pointer to data
 *
 * @return     device status
 */
uint8_t nrf24_read_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* data, uint8_t size);

/** Power up the radio for operation
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     device status
 */
uint8_t nrf24_power_up(FuriHalSpiBusHandle* handle);

/** Power down the radio
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     device status
 */
uint8_t nrf24_set_idle(FuriHalSpiBusHandle* handle);

/** Sets the radio to RX mode
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     device status
 */
uint8_t nrf24_set_rx_mode(FuriHalSpiBusHandle* handle);

/** Sets the radio to TX mode
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     device status
 */
uint8_t nrf24_set_tx_mode(FuriHalSpiBusHandle* handle);

/*=============================================================================================================*/

/* High level API */

/** Must call this before using any other nrf24 API
 * 
 */
void nrf24_init();

/** Must call this when we end using nrf24 device
 * 
 */
void nrf24_deinit();

/** Send flush rx command
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 *
 * @return     device status
 */
uint8_t nrf24_flush_rx(FuriHalSpiBusHandle* handle);

/** Send flush tx command
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 *
 * @return     device status
 */
uint8_t nrf24_flush_tx(FuriHalSpiBusHandle* handle);

/** Gets the RX packet length in data pipe 0
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     packet length in data pipe 0
 */
uint8_t nrf24_get_packetlen(FuriHalSpiBusHandle* handle);

/** Sets the RX packet length in data pipe 0
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      len - length to set
 * 
 * @return     device status
 */
uint8_t nrf24_set_packetlen(FuriHalSpiBusHandle* handle, uint8_t len);

/** Gets configured length of MAC address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     MAC address length
 */
uint8_t nrf24_get_maclen(FuriHalSpiBusHandle* handle);

/** Sets configured length of MAC address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      maclen - length to set MAC address to, must be greater than 1 and less than 6
 * 
 * @return     MAC address length
 */
uint8_t nrf24_set_maclen(FuriHalSpiBusHandle* handle, uint8_t maclen);

/** Gets the current status flags from the STATUS register
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     status flags
 */
uint8_t nrf24_status(FuriHalSpiBusHandle* handle);

/** Gets the current transfer rate
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     transfer rate in bps
 */
uint32_t nrf24_get_rate(FuriHalSpiBusHandle* handle);

/** Sets the transfer rate
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      rate - the transfer rate in bps
 * 
 * @return     device status
 */
uint8_t nrf24_set_rate(FuriHalSpiBusHandle* handle, uint32_t rate);

/** Gets the current channel
 * In nrf24, the channel number is multiplied times 1MHz and added to 2400MHz to get the frequency
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     channel
 */
uint8_t nrf24_get_chan(FuriHalSpiBusHandle* handle);

/** Sets the channel
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      frequency - the frequency in hertz
 * 
 * @return     device status
 */
uint8_t nrf24_set_chan(FuriHalSpiBusHandle* handle, uint8_t chan);

/** Gets the source mac address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param[out] mac - the source mac address
 * 
 * @return     device status
 */
uint8_t nrf24_get_src_mac(FuriHalSpiBusHandle* handle, uint8_t* mac);

/** Sets the source mac address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      mac - the mac address to set
 * @param      size - the size of the mac address (2 to 5)
 * 
 * @return     device status
 */
uint8_t nrf24_set_src_mac(FuriHalSpiBusHandle* handle, uint8_t* mac, uint8_t size);

/** Gets the dest mac address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param[out] mac - the source mac address
 * 
 * @return     device status
 */
uint8_t nrf24_get_dst_mac(FuriHalSpiBusHandle* handle, uint8_t* mac);

/** Sets the dest mac address
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      mac - the mac address to set
 * @param      size - the size of the mac address (2 to 5)
 * 
 * @return     device status
 */
uint8_t nrf24_set_dst_mac(FuriHalSpiBusHandle* handle, uint8_t* mac, uint8_t size);

/** Reads RX packet
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param[out] packet - the packet contents
 * @param[out] packetsize - size of the received packet
 * @param      full - boolean set to true, packet length is determined by RX_PW_P0 register, false it is determined by dynamic payload length command
 * 
 * @return     device status
 */
uint8_t
    nrf24_rxpacket(FuriHalSpiBusHandle* handle, uint8_t* packet, uint8_t* packetsize, bool full);

/** Sends TX packet
 *
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      packet - the packet contents
 * @param      size - packet size
 * @param      ack - boolean to determine whether an ACK is required for the packet or not
 * 
 * @return     device status
 */
uint8_t nrf24_txpacket(FuriHalSpiBusHandle* handle, uint8_t* payload, uint8_t size, bool ack);

/** Configure the radio
 * This is not comprehensive, but covers a lot of the common configuration options that may be changed
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      rate - transfer rate in Mbps (1 or 2)
 * @param      srcmac - source mac address
 * @param      dstmac - destination mac address
 * @param      maclen - length of mac address
 * @param      channel - channel to tune to
 * @param      noack - if true, disable auto-acknowledge
 * @param      disable_aa - if true, disable ShockBurst
 * 
 */
void nrf24_configure(
    FuriHalSpiBusHandle* handle,
    uint8_t rate,
    uint8_t* srcmac,
    uint8_t* dstmac,
    uint8_t maclen,
    uint8_t channel,
    bool noack,
    bool disable_aa);

/** Configures the radio for "promiscuous mode" and primes it for rx
 * This is not an actual mode of the nrf24, but this function exploits a few bugs in the chip that allows it to act as if it were.
 * See http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html for details.
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      channel - channel to tune to
 * @param      rate - transfer rate in Mbps (1 or 2) 
 */
void nrf24_init_promisc_mode(FuriHalSpiBusHandle* handle, uint8_t channel, uint8_t rate);

/** Listens for a packet and returns first possible address sniffed
 * Call this only after calling nrf24_init_promisc_mode
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      maclen - length of target mac address
 * @param[out] addresses - sniffed address
 * 
 * @return     success
 */
bool nrf24_sniff_address(FuriHalSpiBusHandle* handle, uint8_t maclen, uint8_t* address);

/** Sends ping packet on each channel for designated tx mac looking for ack
 * 
 * @param      handle  - pointer to FuriHalSpiHandle
 * @param      srcmac - source address
 * @param      dstmac - destination address
 * @param      maclen - length of address
 * @param      rate - transfer rate in Mbps (1 or 2) 
 * @param      min_channel - channel to start with
 * @param      max_channel - channel to end at
 * @param      autoinit - if true, automatically configure radio for this channel
 * 
 * @return     channel that the address is listening on, if this value is above the max_channel param, it failed
 */
uint8_t nrf24_find_channel(
    FuriHalSpiBusHandle* handle,
    uint8_t* srcmac,
    uint8_t* dstmac,
    uint8_t maclen,
    uint8_t rate,
    uint8_t min_channel,
    uint8_t max_channel,
    bool autoinit);

/** Converts 64 bit value into uint8_t array
 * @param      val  - 64-bit integer
 * @param[out] out - bytes out
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 */
void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian);

/** Converts 32 bit value into uint8_t array
 * @param      val  - 32-bit integer
 * @param[out] out - bytes out
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 */
void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian);

/** Converts uint8_t array into 32 bit value
 * @param      bytes  - uint8_t array
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 * 
 * @return     32-bit value
 */
uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian);

/** Check if the nrf24 is connected
 * @param      handle  - pointer to FuriHalSpiHandle
 * 
 * @return     true if connected, otherwise false
*/
bool nrf24_check_connected(FuriHalSpiBusHandle* handle);

#ifdef __cplusplus
}
#endif