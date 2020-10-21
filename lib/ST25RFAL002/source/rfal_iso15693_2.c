
/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*
 *      PROJECT:   ST25R391x firmware
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file rfal_iso15693_2.c
 *
 *  \author Ulrich Herrmann
 *
 *  \brief Implementation of ISO-15693-2
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "rfal_iso15693_2.h"
#include "rfal_crc.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_NFCV
    #define RFAL_FEATURE_NFCV   false    /* NFC-V module configuration missing. Disabled by default */
#endif

#if RFAL_FEATURE_NFCV

/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/

#define ISO_15693_DEBUG(...)   /*!< Macro for the log method  */

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/
#define ISO15693_DAT_SOF_1_4     0x21 /* LSB constants */
#define ISO15693_DAT_EOF_1_4     0x04
#define ISO15693_DAT_00_1_4      0x02
#define ISO15693_DAT_01_1_4      0x08
#define ISO15693_DAT_10_1_4      0x20
#define ISO15693_DAT_11_1_4      0x80

#define ISO15693_DAT_SOF_1_256   0x81
#define ISO15693_DAT_EOF_1_256   0x04
#define ISO15693_DAT_SLOT0_1_256 0x02
#define ISO15693_DAT_SLOT1_1_256 0x08
#define ISO15693_DAT_SLOT2_1_256 0x20
#define ISO15693_DAT_SLOT3_1_256 0x80

#define ISO15693_PHY_DAT_MANCHESTER_1 0xaaaa

#define ISO15693_PHY_BIT_BUFFER_SIZE 1000 /*!< size of the receiving buffer. Might be adjusted if longer datastreams are expected. */


/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static iso15693PhyConfig_t iso15693PhyConfig; /*!< current phy configuration */

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static ReturnCode iso15693PhyVCDCode1Of4(const uint8_t data, uint8_t* outbuffer, uint16_t maxOutBufLen, uint16_t* outBufLen);
static ReturnCode iso15693PhyVCDCode1Of256(const uint8_t data, uint8_t* outbuffer, uint16_t maxOutBufLen, uint16_t* outBufLen);



/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
ReturnCode iso15693PhyConfigure(const iso15693PhyConfig_t* config, const struct iso15693StreamConfig ** needed_stream_config  )
{
    static struct iso15693StreamConfig stream_config = {                                       /* MISRA 8.9 */
        .useBPSK = 0,              /* 0: subcarrier, 1:BPSK */
        .din = 5,                  /* 2^5*fc = 423750 Hz: divider for the in subcarrier frequency */
        .dout = 7,                 /*!< 2^7*fc = 105937 : divider for the in subcarrier frequency */
        .report_period_length = 3, /*!< 8=2^3 the length of the reporting period */
    };
    
    
    /* make a copy of the configuration */
    ST_MEMCPY( (uint8_t*)&iso15693PhyConfig, (const uint8_t*)config, sizeof(iso15693PhyConfig_t));
    
    if ( config->speedMode <= 3U)
    { /* If valid speed mode adjust report period accordingly */
        stream_config.report_period_length = (3U - (uint8_t)config->speedMode);
    }
    else
    { /* If invalid default to normal (high) speed */
        stream_config.report_period_length = 3;
    }

    *needed_stream_config = &stream_config;

    return ERR_NONE;
}

ReturnCode iso15693PhyGetConfiguration(iso15693PhyConfig_t* config)
{
    ST_MEMCPY(config, &iso15693PhyConfig, sizeof(iso15693PhyConfig_t));

    return ERR_NONE;
}

ReturnCode iso15693VCDCode(uint8_t* buffer, uint16_t length, bool sendCrc, bool sendFlags, bool picopassMode,
                   uint16_t *subbit_total_length, uint16_t *offset,
                   uint8_t* outbuf, uint16_t outBufSize, uint16_t* actOutBufSize)
{
    ReturnCode err = ERR_NONE;
    uint8_t eof, sof;
    uint8_t transbuf[2];
    uint16_t crc = 0;
    ReturnCode (*txFunc)(const uint8_t data, uint8_t* outbuffer, uint16_t maxOutBufLen, uint16_t* outBufLen);
    uint8_t crc_len;
    uint8_t* outputBuf;
    uint16_t outputBufSize;

    crc_len = (uint8_t)((sendCrc)?2:0);

    *actOutBufSize = 0;

    if (ISO15693_VCD_CODING_1_4 == iso15693PhyConfig.coding)
    {
        sof = ISO15693_DAT_SOF_1_4;
        eof = ISO15693_DAT_EOF_1_4;
        txFunc = iso15693PhyVCDCode1Of4;
        *subbit_total_length = (
                ( 1U  /* SOF */
                  + ((length + (uint16_t)crc_len) * 4U)
                  + 1U) /* EOF */
                );
        if (outBufSize < 5U) { /* 5 should be safe: enough for sof + 1byte data in 1of4 */
            return ERR_NOMEM;
        }
    }
    else
    {
        sof = ISO15693_DAT_SOF_1_256;
        eof = ISO15693_DAT_EOF_1_256;
        txFunc = iso15693PhyVCDCode1Of256;
        *subbit_total_length = (
                ( 1U  /* SOF */
                  + ((length + (uint16_t)crc_len) * 64U) 
                  + 1U) /* EOF */
                );

        if (*offset != 0U)
        {
            if (outBufSize < 64U) { /* 64 should be safe: enough a single byte data in 1of256 */
                return ERR_NOMEM;
            }
        }
        else
        {
            if (outBufSize < 65U) { /* At beginning of a frame we need at least 65 bytes to start: enough for sof + 1byte data in 1of256 */
                return ERR_NOMEM;
            }
        }
    }

    if (length == 0U)
    {
        *subbit_total_length = 1;
    }

    if ((length != 0U) && (0U == *offset) && sendFlags && !picopassMode)
    {
        /* set high datarate flag */
        buffer[0] |= (uint8_t)ISO15693_REQ_FLAG_HIGH_DATARATE;
        /* clear sub-carrier flag - we only support single sub-carrier */
        buffer[0] = (uint8_t)(buffer[0] & ~ISO15693_REQ_FLAG_TWO_SUBCARRIERS);  /* MISRA 10.3 */
    }

    outputBuf = outbuf;             /* MISRA 17.8: Use intermediate variable */
    outputBufSize = outBufSize;     /* MISRA 17.8: Use intermediate variable */

    /* Send SOF if at 0 offset */
    if ((length != 0U) && (0U == *offset))
    {
        *outputBuf = sof; 
        (*actOutBufSize)++;
        outputBufSize--;
        outputBuf++;
    }

    while ((*offset < length) && (err == ERR_NONE))
    {
        uint16_t filled_size;
        /* send data */
        err = txFunc(buffer[*offset], outputBuf, outputBufSize, &filled_size);
        (*actOutBufSize) += filled_size;
        outputBuf = &outputBuf[filled_size];	/* MISRA 18.4: Avoid pointer arithmetic */
        outputBufSize -= filled_size;
        if (err == ERR_NONE) {
            (*offset)++;
        }
    }
    if (err != ERR_NONE) {
        return ERR_AGAIN;
    }

    while ((err == ERR_NONE) && sendCrc && (*offset < (length + 2U)))
    {
        uint16_t filled_size;
        if (0U==crc)
        {
            crc = rfalCrcCalculateCcitt( (uint16_t) ((picopassMode) ? 0xE012U : 0xFFFFU),        /* In PicoPass Mode a different Preset Value is used   */
                                                    ((picopassMode) ? (buffer + 1U) : buffer),   /* CMD byte is not taken into account in PicoPass mode */
                                                    ((picopassMode) ? (length - 1U) : length));  /* CMD byte is not taken into account in PicoPass mode */
            
            crc = (uint16_t)((picopassMode) ? crc : ~crc);
        }
        /* send crc */
        transbuf[0] = (uint8_t)(crc & 0xffU);
        transbuf[1] = (uint8_t)((crc >> 8) & 0xffU);
        err = txFunc(transbuf[*offset - length], outputBuf, outputBufSize, &filled_size);
        (*actOutBufSize) += filled_size;
        outputBuf = &outputBuf[filled_size];	/* MISRA 18.4: Avoid pointer arithmetic */
        outputBufSize -= filled_size;
        if (err == ERR_NONE) {
            (*offset)++;
        }
    }
    if (err != ERR_NONE) {
        return ERR_AGAIN;
    }

    if ((!sendCrc && (*offset == length))
            || (sendCrc && (*offset == (length + 2U))))
    {
        *outputBuf = eof; 
        (*actOutBufSize)++;
        outputBufSize--;
        outputBuf++;
    }
    else
    {
        return ERR_AGAIN;
    }

    return err;
}

ReturnCode iso15693VICCDecode(const uint8_t *inBuf,
                      uint16_t inBufLen,
                      uint8_t* outBuf,
                      uint16_t outBufLen,
                      uint16_t* outBufPos,
                      uint16_t* bitsBeforeCol,
                      uint16_t ignoreBits,
                      bool picopassMode )
{
    ReturnCode err = ERR_NONE;
    uint16_t crc;
    uint16_t mp; /* Current bit position in manchester bit inBuf*/
    uint16_t bp; /* Current bit position in outBuf */

    *bitsBeforeCol = 0;
    *outBufPos = 0;

    /* first check for valid SOF. Since it starts with 3 unmodulated pulses it is 0x17. */
    if ((inBuf[0] & 0x1fU) != 0x17U)
    {
		ISO_15693_DEBUG("0x%x\n", iso15693PhyBitBuffer[0]);
		return ERR_FRAMING;
    }
    ISO_15693_DEBUG("SOF\n");

    if (outBufLen == 0U)
    {
        return ERR_NONE;
    }

    mp = 5; /* 5 bits were SOF, now manchester starts: 2 bits per payload bit */
    bp = 0;

    ST_MEMSET(outBuf,0,outBufLen);

    if (inBufLen == 0U)
    {
        return ERR_CRC;
    }

    for ( ; mp < ((inBufLen * 8U) - 2U); mp+=2U )
    {
        bool isEOF = false;
        
        uint8_t man;
        man  = (inBuf[mp/8U] >> (mp%8U)) & 0x1U;
        man |= ((inBuf[(mp+1U)/8U] >> ((mp+1U)%8U)) & 0x1U) << 1;
        if (1U == man)
        {
            bp++;
        }
        if (2U == man)
        {
            outBuf[bp/8U] = (uint8_t)(outBuf[bp/8U] | (1U <<(bp%8U)));  /* MISRA 10.3 */
            bp++;
        }
        if ((bp%8U) == 0U)
        { /* Check for EOF */
            ISO_15693_DEBUG("ceof %hhx %hhx\n", inBuf[mp/8U], inBuf[mp/8+1]);
            if ( ((inBuf[mp/8U]   & 0xe0U) == 0xa0U)
               &&(inBuf[(mp/8U)+1U] == 0x03U))
            { /* Now we know that it was 10111000 = EOF */
                ISO_15693_DEBUG("EOF\n");
                isEOF = true;
            }
        }
        if ( ((0U == man) || (3U == man)) && !isEOF )
        {  
            if (bp >= ignoreBits)
            {
                err = ERR_RF_COLLISION;
            }
            else
            {
                /* ignored collision: leave as 0 */
                bp++;
            }
        }
        if ( (bp >= (outBufLen * 8U)) || (err == ERR_RF_COLLISION) || isEOF )        
        { /* Don't write beyond the end */
            break;
        }
    }

    *outBufPos = (bp / 8U);
    *bitsBeforeCol = bp;

    if (err != ERR_NONE) 
    {
        return err;
    }

    if ((bp%8U) != 0U)
    {
        return ERR_CRC;
    }

    if (*outBufPos > 2U)
    {
        /* finally, check crc */
        ISO_15693_DEBUG("Calculate CRC, val: 0x%x, outBufLen: ", *outBuf);
        ISO_15693_DEBUG("0x%x ", *outBufPos - 2);
        
        crc = rfalCrcCalculateCcitt(((picopassMode) ? 0xE012U : 0xFFFFU), outBuf, *outBufPos - 2U);
        crc = (uint16_t)((picopassMode) ? crc : ~crc);
        
        if (((crc & 0xffU) == outBuf[*outBufPos-2U]) &&
                (((crc >> 8U) & 0xffU) == outBuf[*outBufPos-1U]))
        {
            err = ERR_NONE;
            ISO_15693_DEBUG("OK\n");
        }
        else
        {
            ISO_15693_DEBUG("error! Expected: 0x%x, got ", crc);
            ISO_15693_DEBUG("0x%hhx 0x%hhx\n", outBuf[*outBufPos-2], outBuf[*outBufPos-1]);
            err = ERR_CRC;
        }
    }
    else
    {
        err = ERR_CRC;
    }

    return err;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
/*! 
 *****************************************************************************
 *  \brief  Perform 1 of 4 coding and send coded data
 *
 *  This function takes \a length bytes from \a buffer, perform 1 of 4 coding
 *  (see ISO15693-2 specification) and sends the data using stream mode.
 *
 *  \param[in] sendSof : send SOF prior to data.
 *  \param[in] buffer : data to send.
 *  \param[in] length : number of bytes to send.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
static ReturnCode iso15693PhyVCDCode1Of4(const uint8_t data, uint8_t* outbuffer, uint16_t maxOutBufLen, uint16_t* outBufLen)
{
    uint8_t tmp;
    ReturnCode err = ERR_NONE;
    uint16_t a;
    uint8_t* outbuf = outbuffer;

    *outBufLen = 0;

    if (maxOutBufLen < 4U) {
        return ERR_NOMEM;
    }

    tmp = data;
    for (a = 0; a < 4U; a++)
    {
        switch (tmp & 0x3U)
        {
            case 0:
                *outbuf = ISO15693_DAT_00_1_4;
                break;
            case 1:
                *outbuf = ISO15693_DAT_01_1_4;
                break;
            case 2:
                *outbuf = ISO15693_DAT_10_1_4;
                break;
            case 3:
                *outbuf = ISO15693_DAT_11_1_4;
                break;
            default:
                /* MISRA 16.4: mandatory default statement */
                break;
        }
        outbuf++;
        (*outBufLen)++;
        tmp >>= 2;
    }
    return err;
}

/*! 
 *****************************************************************************
 *  \brief  Perform 1 of 256 coding and send coded data
 *
 *  This function takes \a length bytes from \a buffer, perform 1 of 256 coding
 *  (see ISO15693-2 specification) and sends the data using stream mode.
 *  \note This function sends SOF prior to the data.
 *
 *  \param[in] sendSof : send SOF prior to data.
 *  \param[in] buffer : data to send.
 *  \param[in] length : number of bytes to send.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
static ReturnCode iso15693PhyVCDCode1Of256(const uint8_t data, uint8_t* outbuffer, uint16_t maxOutBufLen, uint16_t* outBufLen)
{
    uint8_t tmp;
    ReturnCode err = ERR_NONE;
    uint16_t a;
    uint8_t* outbuf = outbuffer;

    *outBufLen = 0;

    if (maxOutBufLen < 64U) {
        return ERR_NOMEM;
    }

    tmp = data;
    for (a = 0; a < 64U; a++)
    {
        switch (tmp)
        {
            case 0:
                *outbuf = ISO15693_DAT_SLOT0_1_256;
                break;
            case 1:
                *outbuf = ISO15693_DAT_SLOT1_1_256;
                break;
            case 2:
                *outbuf = ISO15693_DAT_SLOT2_1_256;
                break;
            case 3:
                *outbuf = ISO15693_DAT_SLOT3_1_256;
                break;
            default:
                *outbuf = 0;
                break;               
        }
        outbuf++;
        (*outBufLen)++;
        tmp -= 4U;
    }

    return err;
}

#endif /* RFAL_FEATURE_NFCV */
