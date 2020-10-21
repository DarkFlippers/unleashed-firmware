
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

/*! \file rfal_iso15693_2.h
 *
 *  \author Ulrich Herrmann
 *
 *  \brief Implementation of ISO-15693-2
 *
 */
/*!
 * 
 */

#ifndef RFAL_ISO_15693_2_H
#define RFAL_ISO_15693_2_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"

/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/
/*! Enum holding possible VCD codings  */
typedef enum
{
    ISO15693_VCD_CODING_1_4,
    ISO15693_VCD_CODING_1_256
}iso15693VcdCoding_t;

/*! Enum holding possible VICC datarates */

/*! Configuration parameter used by #iso15693PhyConfigure  */
typedef struct
{
    iso15693VcdCoding_t coding;           /*!< desired VCD coding                                       */
    uint32_t                speedMode;    /*!< 0: normal mode, 1: 2^1 = x2 Fast mode, 2 : 2^2 = x4 mode, 3 : 2^3 = x8 mode - all rx pulse numbers and times are divided by 1,2,4,8 */
}iso15693PhyConfig_t;

/*! Parameters how the stream mode should work */
struct iso15693StreamConfig {
    uint8_t useBPSK;              /*!< 0: subcarrier, 1:BPSK */
    uint8_t din;                  /*!< the divider for the in subcarrier frequency: fc/2^din  */
    uint8_t dout;                 /*!< the divider for the in subcarrier frequency fc/2^dout */
    uint8_t report_period_length; /*!< the length of the reporting period 2^report_period_length*/
};
/*
******************************************************************************
* GLOBAL CONSTANTS
******************************************************************************
*/

#define ISO15693_REQ_FLAG_TWO_SUBCARRIERS 0x01U   /*!< Flag indication that communication uses two subcarriers */
#define ISO15693_REQ_FLAG_HIGH_DATARATE   0x02U   /*!< Flag indication that communication uses high bitrate    */
#define ISO15693_MASK_FDT_LISTEN         (65)     /*!< t1min = 308,2us = 4192/fc = 65.5 * 64/fc                */

/*! t1max = 323,3us = 4384/fc = 68.5 * 64/fc
 *         12 = 768/fc unmodulated time of single subcarrior SoF */
#define ISO15693_FWT (69 + 12)




/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*! 
 *****************************************************************************
 *  \brief  Initialize the ISO15693 phy
 *
 *  \param[in] config : ISO15693 phy related configuration (See #iso15693PhyConfig_t)
 *  \param[out] needed_stream_config : return a pointer to the stream config 
 *              needed for this iso15693 config. To be used for configure RF chip.
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern ReturnCode iso15693PhyConfigure(const iso15693PhyConfig_t* config,
                                       const struct iso15693StreamConfig ** needed_stream_config  );

/*! 
 *****************************************************************************
 *  \brief  Return current phy configuration
 *
 *  This function returns current Phy configuration previously
 *  set by #iso15693PhyConfigure
 *
 *  \param[out] config : ISO15693 phy configuration.
 *
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern ReturnCode iso15693PhyGetConfiguration(iso15693PhyConfig_t* config);

/*! 
 *****************************************************************************
 *  \brief  Code an ISO15693 compatible frame
 *
 *  This function takes \a length bytes from \a buffer, perform proper
 *  encoding and sends out the frame to the ST25R391x.
 *
 *  \param[in] buffer : data to send, modified to adapt flags.
 *  \param[in] length : number of bytes to send.
 *  \param[in] sendCrc : If set to true, CRC is appended to the frame
 *  \param[in] sendFlags: If set to true, flag field is sent according to
 *                        ISO15693.
 *  \param[in] picopassMode :  If set to true, the coding will be according to Picopass
 *  \param[out] subbit_total_length : Return the complete bytes which need to 
 *                                   be send for the current coding
 *  \param[in,out] offset : Set to 0 for first transfer, function will update it to
                            point to next byte to be coded
 *  \param[out] outbuf : buffer where the function will store the coded subbit stream
 *  \param[out] outBufSize : the size of the output buffer
 *  \param[out] actOutBufSize : the amount of data stored into the buffer at this call
 *
 *  \return ERR_IO : Error during communication.
 *  \return ERR_AGAIN : Data was not coded all the way. Call function again with a new/emptied buffer
 *  \return ERR_NO_MEM : In case outBuf is not big enough. Needs to have at 
                         least 5 bytes for 1of4 coding and 65 bytes for 1of256 coding
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern ReturnCode iso15693VCDCode(uint8_t* buffer, uint16_t length, bool sendCrc, bool sendFlags, bool picopassMode,
                   uint16_t *subbit_total_length, uint16_t *offset,
                   uint8_t* outbuf, uint16_t outBufSize, uint16_t* actOutBufSize);


/*! 
 *****************************************************************************
 *  \brief  Receive an ISO15693 compatible frame
 *
 *  This function receives an ISO15693 frame from the ST25R391x, decodes the frame
 *  and writes the raw data to \a buffer.
 *  \note Buffer needs to be big enough to hold CRC also (+2 bytes)
 *
 *  \param[in] inBuf : buffer with the hamming coded stream to be decoded
 *  \param[in] inBufLen : number of bytes to decode (=length of buffer).
 *  \param[out] outBuf : buffer where received data shall be written to.
 *  \param[in] outBufLen : Length of output buffer, should be approx twice the size of inBuf
 *  \param[out] outBufPos : The number of decoded bytes. Could be used in 
 *                          extended implementation to allow multiple calls
 *  \param[out] bitsBeforeCol : in case of ERR_COLLISION this value holds the
 *   number of bits in the current byte where the collision happened.
 *  \param[in] ignoreBits : number of bits in the beginning where collisions will be ignored
 *  \param[in] picopassMode :  if set to true, the decoding will be according to Picopass
 *
 *  \return ERR_COLLISION : collision occured, data uncorrect
 *  \return ERR_CRC : CRC error, data uncorrect
 *  \return ERR_TIMEOUT : timeout waiting for data.
 *  \return ERR_NONE : No error.
 *
 *****************************************************************************
 */
extern ReturnCode iso15693VICCDecode(const uint8_t *inBuf,
                      uint16_t inBufLen,
                      uint8_t* outBuf,
                      uint16_t outBufLen,
                      uint16_t* outBufPos,
                      uint16_t* bitsBeforeCol,
                      uint16_t ignoreBits,
                      bool picopassMode );

#endif /* RFAL_ISO_15693_2_H */

