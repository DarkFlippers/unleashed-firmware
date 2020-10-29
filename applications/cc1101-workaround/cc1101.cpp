#include "flipper_v2.h"
#include "cc1101-workaround/cc1101.h"

// ******************************************************************************
#define WRITE_BURST 0x40
#define READ_SINGLE 0x80
#define READ_BURST 0xC0
#define BYTES_IN_FIFO 0x7F //used to detect FIFO underflow or overflow

/*********************ss_pin as global variable****************************** */
/*                         cc1101                                       */
/******************************************************************************/
GpioPin ss_pin;

CC1101::CC1101(GpioPin* ss_pin) {
    /*
    pinMode(gdo0_pin, OUTPUT); //GDO0 as asynchronous serial mode input
    pinMode(gdo2_pin, INPUT); //GDO2 as asynchronous serial mode output
    */
    gpio_init(ss_pin, GpioModeOutputPushPull);
    this->ss_pin = ss_pin;

    // TODO open record
    this->miso_pin = MISO_PIN;
    this->miso_pin_record = &this->miso_pin;
}
//******************************************************************************
//SpiInit
/******************************************************************************/
extern SPI_HandleTypeDef hspi3;
void CC1101::SpiInit(void) {
    //initialize spi pins

    //Enable spi master, MSB, SPI mode 0, FOSC/4
    SpiMode(0);

    if(HAL_SPI_DeInit(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;
    hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    if(HAL_SPI_Init(&hspi3) != HAL_OK) {
        Error_Handler();
    }
}

void CC1101::SpiEnd(void) {
    /*
    SPCR = ((0<<SPE) |                  // SPI Enable
        (0<<SPIE)|                      // SPI Interupt Enable
        (0<<DORD)|                      // Data Order (0:MSB first / 1:LSB first)
        (1<<MSTR)|                      // Master/Slave select
        (0<<SPR1)|(0<<SPR0)|        // SPI Clock Rate ( 0 0 = osc/4; 0 1 = osc/16; 1 0 = osc/64; 1 1= 0sc/128)
        (0<<CPOL)|                  // Clock Polarity (0:SCK low / 1:SCK hi when idle)
        (0<<CPHA));                     // Clock Phase (0:leading / 1:trailing edge sampling)

    //SPSR =  (0<<SPI2X);                   // Double Clock Rate
    */
}
/******************************************************************************
Function: SpiMode
 *INPUT        :        config               mode
               (0<<CPOL) | (0 << CPHA)       0
               (0<<CPOL) | (1 << CPHA)       1
               (1<<CPOL) | (0 << CPHA)       2
               (1<<CPOL) | (1 << CPHA)       3
*OUTPUT       :none
******************************************************************************/
void CC1101::SpiMode(uint8_t config) {
    /*
  uint8_t tmp;
  // enable SPI master with configuration byte specified
  SPCR = 0;
  SPCR = (config & 0x7F) | (1<<SPE) | (1<<MSTR);
  tmp = SPSR;
  tmp = SPDR;
  */
}
/****************************************************************
*FUNCTION NAME:SpiTransfer
*FUNCTION     :spi transfer
*INPUT        :value: data to send
*OUTPUT       :data to receive
****************************************************************/
uint8_t CC1101::SpiTransfer(uint8_t value) {
    uint8_t buf[1] = {value};
    uint8_t rxbuf[1] = {0};

    HAL_SPI_TransmitReceive(&hspi3, buf, rxbuf, 1, HAL_MAX_DELAY);

    return rxbuf[0];
}

/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC1101 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none
****************************************************************/
void CC1101::SpiWriteReg(uint8_t addr, uint8_t value) {
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(addr);
    SpiTransfer(value);
    gpio_write(ss_pin, true);
}

/****************************************************************
*FUNCTION NAME:SpiWriteBurstReg
*FUNCTION     :CC1101 write burst data to register
*INPUT        :addr: register address; buffer:register value array; num:number to write
*OUTPUT       :none
****************************************************************/
void CC1101::SpiWriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t num) {
    uint8_t i, temp;

    temp = addr | WRITE_BURST;
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(temp);
    for(i = 0; i < num; i++) {
        SpiTransfer(buffer[i]);
    }
    gpio_write(ss_pin, true);
}

/****************************************************************
*FUNCTION NAME:SpiStrobe
*FUNCTION     :CC1101 Strobe
*INPUT        :strobe: command; //refer define in CC1101.h//
*OUTPUT       :none
****************************************************************/
void CC1101::SpiStrobe(uint8_t strobe) {
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(strobe);
    gpio_write(ss_pin, true);
}

/****************************************************************
*FUNCTION NAME:SpiReadReg
*FUNCTION     :CC1101 read data from register
*INPUT        :addr: register address
*OUTPUT       :register value
****************************************************************/
uint8_t CC1101::SpiReadReg(uint8_t addr) {
    uint8_t temp, value;

    temp = addr | READ_SINGLE;
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(temp);
    value = SpiTransfer(0);
    gpio_write(ss_pin, true);

    return value;
}

/****************************************************************
*FUNCTION NAME:SpiReadBurstReg
*FUNCTION     :CC1101 read burst data from register
*INPUT        :addr: register address; buffer:array to store register value; num: number to read
*OUTPUT       :none
****************************************************************/
void CC1101::SpiReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t num) {
    uint8_t i, temp;

    temp = addr | READ_BURST;
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(temp);
    for(i = 0; i < num; i++) {
        buffer[i] = SpiTransfer(0);
    }
    gpio_write(ss_pin, true);
}

/****************************************************************
*FUNCTION NAME:SpiReadStatus
*FUNCTION     :CC1101 read status register
*INPUT        :addr: register address
*OUTPUT       :status value
****************************************************************/
uint8_t CC1101::SpiReadStatus(uint8_t addr) {
    uint8_t value, temp;

    temp = addr | READ_BURST;
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(temp);
    value = SpiTransfer(0);
    gpio_write(ss_pin, true);

    return value;
}

/****************************************************************
*FUNCTION NAME:Reset
*FUNCTION     :CC1101 reset //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC1101::Reset(void) {
    gpio_write(ss_pin, false);
    delay(1);
    gpio_write(ss_pin, true);
    delay(1);
    gpio_write(ss_pin, false);
    while(gpio_read(this->miso_pin_record))
        ;
    SpiTransfer(CC1101_SRES);
    while(gpio_read(this->miso_pin_record))
        ;
    gpio_write(ss_pin, true);
}
/****************************************************************
*FUNCTION NAME:Init
*FUNCTION     :CC1101 initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
uint8_t CC1101::Init(void) {
#ifdef CC1101_DEBUG
    printf("Init SPI...\n");
#endif
    SpiInit(); //spi initialization
    gpio_write(ss_pin, true);
// gpio_write(SCK_PIN, true);
// gpio_write(MOSI_PIN, false);
#ifdef CC1101_DEBUG
    printf("Reset CC1101...\n");
#endif
    Reset(); //CC1101 reset

    uint8_t partnum __attribute__((unused));
    uint8_t version;
    partnum = SpiReadStatus(CC1101_PARTNUM);
    version = SpiReadStatus(CC1101_VERSION);

#ifdef CC1101_DEBUG

    printf("Partnum:0x%02X, Version:0x%02X\n", partnum, version);
#endif

#ifdef CC1101_DEBUG
    printf("Init CC1101...");
#endif
    RegConfigSettings(); //CC1101 register config

#ifdef CC1101_DEBUG
    printf("Done!\n");
#endif

    return version;
}
/****************************************************************
*FUNCTION NAME:SetMod
*FUNCTION     :CC1101 modulation type
*INPUT        :byte mode
*OUTPUT       :none
****************************************************************/
void CC1101::SetMod(uint8_t mode) {
    SpiWriteReg(CC1101_MDMCFG2, mode); //no sync/preamble; ASK/OOK only support up to -1dbm
    if((mode | 0x30) == ASK) {
        SpiWriteReg(CC1101_FREND0, 0x11); //use first up to PATABLE(0)
        uint8_t PaTabel[8] = {0x00, POWER, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        SpiWriteBurstReg(CC1101_PATABLE, PaTabel, 8); //CC1101 PATABLE config
    } else {
        SpiWriteReg(CC1101_FREND0, 0x10); //use first up to PATABLE(0)
        uint8_t PaTabel[8] = {POWER, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        SpiWriteBurstReg(CC1101_PATABLE, PaTabel, 8); //CC1101 PATABLE config
    }

#ifdef CC1101_DEBUG
    switch(mode | 0x30) {
    case GFSK: {
        printf("CC1101 Modulation: GFSK");
        break;
    }
    case MSK: {
        printf("CC1101 Modulation: MSK");
        break;
    }
    case ASK: {
        printf("CC1101 Modulation: ASK/OOK");
        break;
    }
    case FSK2: {
        printf("CC1101 Modulation: 2-FSK");
        break;
    }
    case FSK4: {
        printf("CC1101 Modulation: 4-FSK");
        break;
    }
    default: //default to GFSK
    {
        printf("Modulation mode not supported");
        break;
    }
    }

    printf("\n");
#endif
}
/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC1101 register config //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC1101::RegConfigSettings(void) {
    SpiWriteReg(CC1101_FSCTRL1, 0x06); //IF frequency
    SpiWriteReg(CC1101_FSCTRL0, 0x00); //frequency offset before synthesizer

    SpiWriteReg(CC1101_MDMCFG4, 0xCC); // RX filter bandwidth 100k(0xcc)
    SpiWriteReg(
        CC1101_MDMCFG3, 0x43); //datarate config 512kBaud  for the purpose of fast rssi measurement
    SpiWriteReg(CC1101_MDMCFG1, 0x21); //FEC preamble etc. last 2 bits for channel spacing
    SpiWriteReg(CC1101_MDMCFG0, 0xF8); //100khz channel spacing
    //CC1101_CHANNR moved to SetChannel func

    //SpiWriteReg(CC1101_DEVIATN,  0x47);
    SpiWriteReg(
        CC1101_MCSM0, 0x18); // calibrate when going from IDLE to RX or TX ; 149 - 155 μs timeout
    SpiWriteReg(CC1101_FOCCFG, 0x16); //frequency compensation
    //SpiWriteReg(CC1101_BSCFG,    0x1C);   //bit synchronization config
    SpiWriteReg(CC1101_AGCCTRL2, 0x43);
    SpiWriteReg(CC1101_AGCCTRL1, 0x49);
    SpiWriteReg(CC1101_AGCCTRL0, 0x91);
    //freq synthesizer calibration
    SpiWriteReg(CC1101_FSCAL3, 0xEA);
    SpiWriteReg(CC1101_FSCAL2, 0x2A);
    SpiWriteReg(CC1101_FSCAL1, 0x00);
    SpiWriteReg(CC1101_FSCAL0, 0x1F);
    SpiWriteReg(CC1101_TEST2, 0x81);
    SpiWriteReg(CC1101_TEST1, 0x35);
    SpiWriteReg(CC1101_TEST0, 0x0B); //should be 0x0B for lower than 430.6MHz and 0x09 for higher

    //SpiWriteReg(CC1101_FREND1,   0x56);

    //SpiWriteReg(CC1101_IOCFG2,   0x0B);   //serial clock.synchronous to the data in synchronous serial mode
    //SpiWriteReg(CC1101_IOCFG0,   0x06);   //asserts when sync word has been sent/received, and de-asserts at the end of the packet
    SpiWriteReg(CC1101_IOCFG2, 0x0D); //data output pin for asynchronous mode
    SpiWriteReg(
        CC1101_IOCFG0,
        0x2E); //High impedance (3-state), GDO0 configed as data input for asynchronous mode
    //SpiWriteReg(CC1101_PKTCTRL0, 0x05);       //whitening off;CRC Enable；variable length packets, packet length configured by the first byte after sync word
    SpiWriteReg(
        CC1101_PKTCTRL0, 0x33); //whitening off; asynchronous serial mode; CRC diable；reserved
    //SpiWriteReg(CC1101_PKTLEN,   0x3D);   //61 bytes max length
    SpiWriteReg(
        CC1101_FIFOTHR,
        0x47); //Adc_retention enabled for RX filter bandwidth less than 325KHz; defalut fifo threthold.
}
/****************************************************************
 *FUNCTION NAME:SetFreq
 *FUNCTION     :SetFreq
 *INPUT        :Freq2, Freq1, Freq0
 *OUTPUT       :none
 ****************************************************************/
void CC1101::SetFreq(uint8_t freq2, uint8_t freq1, uint8_t freq0) {
    SpiWriteReg(CC1101_FREQ2, freq2);
    SpiWriteReg(CC1101_FREQ1, freq1);
    SpiWriteReg(CC1101_FREQ0, freq0);
}
/****************************************************************
 *FUNCTION NAME:SetChannel
 *FUNCTION     :SetChannel
 *INPUT        :int channel
 *OUTPUT       :none
 ****************************************************************/
void CC1101::SetChannel(int channel) {
#ifdef CC1101_DEBUG
    printf("Set CC1101 channel to: %d \n", channel);
#endif
    SpiWriteReg(CC1101_CHANNR, (uint8_t)channel); //related to channel numbers
}
/****************************************************************
 *FUNCTION NAME:SetReceive
 *FUNCTION     :SetReceive
 *INPUT        :none
 *OUTPUT       :none
 ****************************************************************/
void CC1101::SetReceive(void) {
    SpiStrobe(CC1101_SRX);
    while(SpiReadStatus(CC1101_MARCSTATE) ^ CC1101_STATUS_RX) {
        // delay(1);
        // printf("wait status\n");
    }
}
/****************************************************************
 *FUNCTION NAME:SetTransmit
 *FUNCTION     :
 *INPUT        :none
 *OUTPUT       :none
 ****************************************************************/
void CC1101::SetTransmit(void) {
    SpiStrobe(CC1101_STX);
    while(SpiReadStatus(CC1101_MARCSTATE) ^ CC1101_STATUS_TX)
        ;
}
//cc1101 cc1101;
