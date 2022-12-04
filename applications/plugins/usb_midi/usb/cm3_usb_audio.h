/** @defgroup usb_audio_defines USB Audio Type Definitions

@brief <b>Defined Constants and Types for the USB Audio Type Definitions</b>

@ingroup USB_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2014
Daniel Thompson <daniel@redfelineninja.org.uk>
Seb Holzapfel <schnommus@gmail.com>

@date 19 April 2014

LGPL License Terms @ref lgpl_license
*/

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 * Copyright (C) 2018 Seb Holzapfel <schnommus@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@{*/

#ifndef LIBOPENCM3_USB_AUDIO_H
#define LIBOPENCM3_USB_AUDIO_H

#include <stdint.h>

/*
 * Definitions from the USB_AUDIO_ or usb_audio_ namespace come from:
 * "Universal Serial Bus Class Definitions for Audio Devices, Revision 1.0"
 */

/* Table A-1: Audio Interface Class Code */
#define USB_CLASS_AUDIO 0x01

/* Table A-2: Audio Interface Subclass Codes */
#define USB_AUDIO_SUBCLASS_UNDEFINED 0x00
#define USB_AUDIO_SUBCLASS_CONTROL 0x01
#define USB_AUDIO_SUBCLASS_AUDIOSTREAMING 0x02
#define USB_AUDIO_SUBCLASS_MIDISTREAMING 0x03

/* Table A-4: Audio Class-specific Descriptor Types */
#define USB_AUDIO_DT_CS_UNDEFINED 0x20
#define USB_AUDIO_DT_CS_DEVICE 0x21
#define USB_AUDIO_DT_CS_CONFIGURATION 0x22
#define USB_AUDIO_DT_CS_STRING 0x23
#define USB_AUDIO_DT_CS_INTERFACE 0x24
#define USB_AUDIO_DT_CS_ENDPOINT 0x25

/* Table A-5: Audio Class-Specific AC Interface Descriptor Subtypes */
#define USB_AUDIO_TYPE_AC_DESCRIPTOR_UNDEFINED 0x00
#define USB_AUDIO_TYPE_HEADER 0x01
#define USB_AUDIO_TYPE_INPUT_TERMINAL 0x02
#define USB_AUDIO_TYPE_OUTPUT_TERMINAL 0x03
#define USB_AUDIO_TYPE_MIXER_UNIT 0x04
#define USB_AUDIO_TYPE_SELECTOR_UNIT 0x05
#define USB_AUDIO_TYPE_FEATURE_UNIT 0x06
#define USB_AUDIO_TYPE_PROCESSING_UNIT 0x07
#define USB_AUDIO_TYPE_EXTENSION_UNIT 0x08

/* Table 4-2: Class-Specific AC Interface Header Descriptor (head) */
struct usb_audio_header_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdADC;
    uint16_t wTotalLength;
    uint8_t bInCollection;
    /* ... */
} __attribute__((packed));

/* Table 4-2: Class-Specific AC Interface Header Descriptor (body) */
struct usb_audio_header_descriptor_body {
    /* ... */
    uint8_t baInterfaceNr;
} __attribute__((packed));

/* Table 4-3: Input Terminal Descriptor */
struct usb_audio_input_terminal_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bNrChannels;
    uint16_t wChannelConfig;
    uint8_t iChannelNames;
    uint8_t iTerminal;
} __attribute__((packed));

/* Table 4-3: Output Terminal Descriptor */
struct usb_audio_output_terminal_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bSourceID;
    uint8_t iTerminal;
} __attribute__((packed));

/* Table 4-7: Feature Unit Descriptor (head) */
struct usb_audio_feature_unit_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t bControlSize;
    uint16_t bmaControlMaster; /* device can assume 16-bit, given highest
				    * defined bit in spec is bit #9.
				    * (it is thus required bControlSize=2) */
    /* ... */
} __attribute__((packed));

/* Table 4-7: Feature Unit Descriptor (body) */
struct usb_audio_feature_unit_descriptor_body {
    /* ... */
    uint16_t bmaControl;
    /* ... */
} __attribute__((packed));

/* Table 4-7: Feature Unit Descriptor (tail) */
struct usb_audio_feature_unit_descriptor_tail {
    /* ... */
    uint8_t iFeature;
} __attribute__((packed));

/* Table 4-7: Feature Unit Descriptor (2-channel)
 *
 * This structure is a convenience covering the (common) case where
 * there are 2 channels associated with the feature unit
 */
struct usb_audio_feature_unit_descriptor_2ch {
    struct usb_audio_feature_unit_descriptor_head head;
    struct usb_audio_feature_unit_descriptor_body channel_control[2];
    struct usb_audio_feature_unit_descriptor_tail tail;
} __attribute__((packed));

/* Table 4-19: Class-Specific AS Interface Descriptor */
struct usb_audio_stream_interface_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalLink;
    uint8_t bDelay;
    uint16_t wFormatTag;
} __attribute__((packed));

/* Table 4-20: Standard AS Isochronous Audio Data Endpoint Descriptor */
struct usb_audio_stream_endpoint_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
    uint8_t bRefresh;
    uint8_t bSynchAddress;
} __attribute__((packed));

/* Table 4-21: Class-Specific AS Isochronous Audio Data Endpoint Descriptor */
struct usb_audio_stream_audio_endpoint_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmAttributes;
    uint8_t bLockDelayUnits;
    uint16_t wLockDelay;
} __attribute__((packed));

/*
 * Definitions from the USB_AUDIO_FORMAT_ or usb_audio_format_ namespace come from:
 * "Universal Serial Bus Device Class Definition for Audio Data Formats, Revision 1.0"
 */

/* Table 2-1: Type I Format Type Descriptor (head) */
struct usb_audio_format_type1_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatType;
    uint8_t bNrChannels;
    uint8_t bSubFrameSize;
    uint8_t bBitResolution;
    uint8_t bSamFreqType;
    /* ... */
} __attribute__((packed));

/* Table 2-2: Continuous Sampling Frequency */
struct usb_audio_format_continuous_sampling_frequency {
    /* ... */
    uint32_t tLowerSamFreq : 24;
    uint32_t tUpperSamFreq : 24;
} __attribute__((packed));

/* Table 2-3: Discrete Number of Sampling Frequencies */
struct usb_audio_format_discrete_sampling_frequency {
    /* ... */
    uint32_t tSamFreq : 24;
} __attribute__((packed));

/* Table 2-1: Type I Format Type Descriptor (1 sampling frequency)
 *
 * This structure is a convenience covering the (common) case where
 * only 1 discrete sampling frequency is used
 */
struct usb_audio_format_type1_descriptor_1freq {
    struct usb_audio_format_type1_descriptor_head head;
    struct usb_audio_format_discrete_sampling_frequency freqs[1];
} __attribute__((packed));

#endif

/**@}*/
