#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Frequency Synthesizer constants */
#define CC1101_QUARTZ 26000000
#define CC1101_FMASK 0xFFFFFF
#define CC1101_FDIV 0x10000
#define CC1101_IFDIV 0x400

/* IO Bus constants */
#define CC1101_TIMEOUT 250

/* Bits and pieces */
#define CC1101_READ (1 << 7) /** Read Bit */
#define CC1101_BURST (1 << 6) /** Burst Bit */

/* Common registers, CC1101_BURST and CC1101_WRITE behaves as expected  */
#define CC1101_IOCFG2 0x00 /** GDO2 output pin configuration */
#define CC1101_IOCFG1 0x01 /** GDO1 output pin configuration */
#define CC1101_IOCFG0 0x02 /** GDO0 output pin configuration */
#define CC1101_FIFOTHR 0x03 /** RX FIFO and TX FIFO thresholds */
#define CC1101_SYNC1 0x04 /** Sync word, high byte */
#define CC1101_SYNC0 0x05 /** Sync word, low byte */
#define CC1101_PKTLEN 0x06 /** Packet length */
#define CC1101_PKTCTRL1 0x07 /** Packet automation control */
#define CC1101_PKTCTRL0 0x08 /** Packet automation control */
#define CC1101_ADDR 0x09 /** Device address */
#define CC1101_CHANNR 0x0A /** Channel number */
#define CC1101_FSCTRL1 0x0B /** Frequency synthesizer control */
#define CC1101_FSCTRL0 0x0C /** Frequency synthesizer control */
#define CC1101_FREQ2 0x0D /** Frequency control word, high byte */
#define CC1101_FREQ1 0x0E /** Frequency control word, middle byte */
#define CC1101_FREQ0 0x0F /** Frequency control word, low byte */
#define CC1101_MDMCFG4 0x10 /** Modem configuration */
#define CC1101_MDMCFG3 0x11 /** Modem configuration */
#define CC1101_MDMCFG2 0x12 /** Modem configuration */
#define CC1101_MDMCFG1 0x13 /** Modem configuration */
#define CC1101_MDMCFG0 0x14 /** Modem configuration */
#define CC1101_DEVIATN 0x15 /** Modem deviation setting */
#define CC1101_MCSM2 0x16 /** Main Radio Control State Machine configuration */
#define CC1101_MCSM1 0x17 /** Main Radio Control State Machine configuration */
#define CC1101_MCSM0 0x18 /** Main Radio Control State Machine configuration */
#define CC1101_FOCCFG 0x19 /** Frequency Offset Compensation configuration */
#define CC1101_BSCFG 0x1A /** Bit Synchronization configuration */
#define CC1101_AGCCTRL2 0x1B /** AGC control */
#define CC1101_AGCCTRL1 0x1C /** AGC control */
#define CC1101_AGCCTRL0 0x1D /** AGC control */
#define CC1101_WOREVT1 0x1E /** High byte Event 0 timeout */
#define CC1101_WOREVT0 0x1F /** Low byte Event 0 timeout */
#define CC1101_WORCTRL 0x20 /** Wake On Radio control */
#define CC1101_FREND1 0x21 /** Front end RX configuration */
#define CC1101_FREND0 0x22 /** Front end TX configuration */
#define CC1101_FSCAL3 0x23 /** Frequency synthesizer calibration */
#define CC1101_FSCAL2 0x24 /** Frequency synthesizer calibration */
#define CC1101_FSCAL1 0x25 /** Frequency synthesizer calibration */
#define CC1101_FSCAL0 0x26 /** Frequency synthesizer calibration */
#define CC1101_RCCTRL1 0x27 /** RC oscillator configuration */
#define CC1101_RCCTRL0 0x28 /** RC oscillator configuration */
#define CC1101_FSTEST 0x29 /** Frequency synthesizer calibration control */
#define CC1101_PTEST 0x2A /** Production test */
#define CC1101_AGCTEST 0x2B /** AGC test */
#define CC1101_TEST2 0x2C /** Various test settings */
#define CC1101_TEST1 0x2D /** Various test settings */
#define CC1101_TEST0 0x2E /** Various test settings */

/* Strobe registers, CC1101_BURST is not available, CC1101_WRITE ignored */
#define CC1101_STROBE_SRES 0x30 /** Reset chip. */
#define CC1101_STROBE_SFSTXON \
    0x31 /** Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround). */
#define CC1101_STROBE_SXOFF 0x32 /** Turn off crystal oscillator. */
#define CC1101_STROBE_SCAL \
    0x33 /** Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0) */
#define CC1101_STROBE_SRX \
    0x34 /** Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1. */
#define CC1101_STROBE_STX \
    0x35 /** In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear. */
#define CC1101_STROBE_SIDLE \
    0x36 /** Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable. */
#define CC1101_STROBE_SWOR \
    0x38 /** Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0. */
/* 0x37 is unused */
#define CC1101_STROBE_SPWD 0x39 /** Enter power down mode when CSn goes high. */
#define CC1101_STROBE_SFRX \
    0x3A /** Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states. */
#define CC1101_STROBE_SFTX \
    0x3B /** Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states. */
#define CC1101_STROBE_SWORRST 0x3C /** Reset real time clock to Event1 value. */
#define CC1101_STROBE_SNOP \
    0x3D /** No operation. May be used to get access to the chip status byte.*/

/* Status registers, must be accessed with CC1101_BURST, but one by one */
#define CC1101_STATUS_PARTNUM 0x30 /** Chip ID Part Number */
#define CC1101_STATUS_VERSION 0x31 /** Chip ID Version */
#define CC1101_STATUS_FREQEST 0x32 /** Frequency Offset Estimate from Demodulator */
#define CC1101_STATUS_LQI 0x33 /** Demodulator Estimate for Link Quality, 7bit-CRC, 6..0-LQI*/
#define CC1101_STATUS_RSSI 0x34 /** Received Signal Strength Indication */
#define CC1101_STATUS_MARCSTATE 0x35 /** Main Radio Control State Machine State */
#define CC1101_STATUS_WORTIME1 0x36 /** High Byte of WOR Time */
#define CC1101_STATUS_WORTIME0 0x37 /** Low Byte of WOR Time */
#define CC1101_STATUS_PKTSTATUS 0x38 /** Current GDOx Status and Packet Status */
#define CC1101_STATUS_VCO_VC_DAC 0x39 /** Current Setting from PLL Calibration Module */
#define CC1101_STATUS_TXBYTES \
    0x3A /** Underflow and Number of Bytes, 7bit-Underflow, 6..0-Number of Bytes*/
#define CC1101_STATUS_RXBYTES \
    0x3B /** Overflow and Number of Bytes, 7bit-Overflow*, 6..0-Number of Bytes*/
#define CC1101_STATUS_RCCTRL1_STATUS 0x3C /** Last RC Oscillator Calibration Result */
#define CC1101_STATUS_RCCTRL0_STATUS 0x3D /** Last RC Oscillator Calibration Result */

/* Some special registers, use CC1101_BURST to read/write data */
#define CC1101_PATABLE \
    0x3E /** PATABLE register number, an 8-byte table that defines the PA control settings */
#define CC1101_FIFO \
    0x3F /** FIFO register nunmber, can be combined with CC1101_WRITE and/or CC1101_BURST */
#define CC1101_IOCFG_INV (1 << 6) /** IOCFG inversion */

typedef enum {
    CC1101IocfgRxFifoThreshold = 0x00,
    CC1101IocfgRxFifoThresholdOrPacket = 0x01,
    CC1101IocfgTxFifoThreshold = 0x02,
    CC1101IocfgTxFifoFull = 0x03,
    CC1101IocfgRxOverflow = 0x04,
    CC1101IocfgTxUnderflow = 0x05,
    CC1101IocfgSyncWord = 0x06,
    CC1101IocfgPacket = 0x07,
    CC1101IocfgPreamble = 0x08,
    CC1101IocfgClearChannel = 0x09,
    CC1101IocfgLockDetector = 0x0A,
    CC1101IocfgSerialClock = 0x0B,
    CC1101IocfgSerialSynchronousDataOutput = 0x0C,
    CC1101IocfgSerialDataOutput = 0x0D,
    CC1101IocfgCarrierSense = 0x0E,
    CC1101IocfgCrcOk = 0x0F,
    /* Reserved range: 0x10 - 0x15 */
    CC1101IocfgRxHardData1 = 0x16,
    CC1101IocfgRxHardData0 = 0x17,
    /* Reserved range: 0x18 - 0x1A */
    CC1101IocfgPaPd = 0x1B,
    CC1101IocfgLnaPd = 0x1C,
    CC1101IocfgRxSymbolTick = 0x1D,
    /* Reserved range: 0x1E - 0x23 */
    CC1101IocfgWorEvnt0 = 0x24,
    CC1101IocfgWorEvnt1 = 0x25,
    CC1101IocfgClk256 = 0x26,
    CC1101IocfgClk32k = 0x27,
    /* Reserved: 0x28 */
    CC1101IocfgChpRdyN = 0x29,
    /* Reserved: 0x2A */
    CC1101IocfgXoscStable = 0x2B,
    /* Reserved range: 0x2C - 0x2D */
    CC1101IocfgHighImpedance = 0x2E,
    CC1101IocfgHW = 0x2F,
    /* Only one CC1101IocfgClkXoscN can be selected as an output at any time */
    CC1101IocfgClkXosc1 = 0x30,
    CC1101IocfgClkXosc1_5 = 0x31,
    CC1101IocfgClkXosc2 = 0x32,
    CC1101IocfgClkXosc3 = 0x33,
    CC1101IocfgClkXosc4 = 0x34,
    CC1101IocfgClkXosc6 = 0x35,
    CC1101IocfgClkXosc8 = 0x36,
    CC1101IocfgClkXosc12 = 0x37,
    CC1101IocfgClkXosc16 = 0x38,
    CC1101IocfgClkXosc24 = 0x39,
    CC1101IocfgClkXosc32 = 0x3A,
    CC1101IocfgClkXosc48 = 0x3B,
    CC1101IocfgClkXosc64 = 0x3C,
    CC1101IocfgClkXosc96 = 0x3D,
    CC1101IocfgClkXosc128 = 0x3E,
    CC1101IocfgClkXosc192 = 0x3F,
} CC1101Iocfg;

typedef enum {
    CC1101StateIDLE = 0b000, /** IDLE state */
    CC1101StateRX = 0b001, /** Receive mode */
    CC1101StateTX = 0b010, /** Transmit mode */
    CC1101StateFSTXON = 0b011, /** Fast TX ready */
    CC1101StateCALIBRATE = 0b100, /** Frequency synthesizer calibration is running */
    CC1101StateSETTLING = 0b101, /** PLL is settling */
    CC1101StateRXFIFO_OVERFLOW =
        0b110, /** RX FIFO has overflowed. Read out any useful data, then flush the FIFO with SFRX */
    CC1101StateTXFIFO_UNDERFLOW = 0b111, /** TX FIFO has underflowed. Acknowledge with SFTX */
} CC1101State;

typedef struct {
    uint8_t FIFO_BYTES_AVAILABLE : 4;
    CC1101State STATE : 3;
    bool CHIP_RDYn : 1;
} CC1101Status;

typedef union {
    CC1101Status status;
    uint8_t status_raw;
} CC1101StatusRaw;

typedef struct {
    uint8_t NUM_TXBYTES : 7;
    bool TXFIFO_UNDERFLOW : 1;
} CC1101TxBytes;

typedef struct {
    uint8_t NUM_RXBYTES : 7;
    bool RXFIFO_OVERFLOW : 1;
} CC1101RxBytes;

#ifdef __cplusplus
}
#endif
