/** @defgroup usb_audio_defines USB MIDI Type Definitions

@brief <b>Defined Constants and Types for the USB MIDI Type Definitions</b>

@ingroup USB_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2014
Daniel Thompson <daniel@redfelineninja.org.uk>

@date 19 April 2014

LGPL License Terms @ref lgpl_license
*/

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
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

#ifndef LIBOPENCM3_USB_MIDI_H
#define LIBOPENCM3_USB_MIDI_H

#include <stdint.h>

/*
 * Definitions from the USB_MIDI_ or usb_midi_ namespace come from:
 * "Universal Serial Bus Class Definitions for MIDI Devices, Revision 1.0"
 */

/* Appendix A.1: MS Class-Specific Interface Descriptor Subtypes */
#define USB_MIDI_SUBTYPE_MS_DESCRIPTOR_UNDEFINED 0x00
#define USB_MIDI_SUBTYPE_MS_HEADER 0x01
#define USB_MIDI_SUBTYPE_MIDI_IN_JACK 0x02
#define USB_MIDI_SUBTYPE_MIDI_OUT_JACK 0x03
#define USB_MIDI_SUBTYPE_MIDI_ELEMENT 0x04

/* Appendix A.2: MS Class-Specific Endpoint Descriptor Subtypes */
#define USB_MIDI_SUBTYPE_DESCRIPTOR_UNDEFINED 0x00
#define USB_MIDI_SUBTYPE_MS_GENERAL 0x01

/* Appendix A.3: MS MIDI IN and OUT Jack types */
#define USB_MIDI_JACK_TYPE_UNDEFINED 0x00
#define USB_MIDI_JACK_TYPE_EMBEDDED 0x01
#define USB_MIDI_JACK_TYPE_EXTERNAL 0x02

/* Appendix A.5.1 Endpoint Control Selectors */
#define USB_MIDI_EP_CONTROL_UNDEFINED 0x00
#define USB_MIDI_ASSOCIATION_CONTROL 0x01

/* Table 6-2: Class-Specific MS Interface Header Descriptor */
struct usb_midi_header_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdMSC;
    uint16_t wTotalLength;
} __attribute__((packed));

/* Table 6-3: MIDI IN Jack Descriptor */
struct usb_midi_in_jack_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bJackType;
    uint8_t bJackID;
    uint8_t iJack;
} __attribute__((packed));

/* Table 6-4: MIDI OUT Jack Descriptor (head) */
struct usb_midi_out_jack_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bJackType;
    uint8_t bJackID;
    uint8_t bNrInputPins;
    /* ... */
} __attribute__((packed));

/* Table 6.4: MIDI OUT Jack Descriptor (body) */
struct usb_midi_out_jack_descriptor_body {
    /* ... */
    uint8_t baSourceID;
    uint8_t baSourcePin;
    /* ... */
} __attribute__((packed));

/* Table 6.4: MIDI OUT Jack Descriptor (tail) */
struct usb_midi_out_jack_descriptor_tail {
    /* ... */
    uint8_t iJack;
} __attribute__((packed));

/* Table 6.4: MIDI OUT Jack Descriptor (single)
 *
 * This structure is a convenience covering the (normal) case where
 * there is only one input pin.
 */
struct usb_midi_out_jack_descriptor {
    struct usb_midi_out_jack_descriptor_head head;
    struct usb_midi_out_jack_descriptor_body source[1];
    struct usb_midi_out_jack_descriptor_tail tail;
} __attribute__((packed));

/* Table 6-5: MIDI Element Descriptor (head) */
struct usb_midi_element_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bElementID;
    uint8_t bNrInputPins;
    /* ... */
} __attribute__((packed));

/* Table 6-5: MIDI Element Descriptor (body) */
struct usb_midi_element_descriptor_body {
    /* ... */
    uint8_t baSourceID;
    uint8_t baSourcePin;
    /* ... */
} __attribute__((packed));

/* Table 6-5: MIDI Element Descriptor (tail) */
struct usb_midi_element_descriptor_tail {
    /* ... */
    uint8_t bNrOutputPins;
    uint8_t bInTerminalLink;
    uint8_t bOutTerminalLink;
    uint8_t bElCapsSize;
    uint16_t bmElementCaps; /* host cannot assume this is 16-bit but device
				   can (since highest defined bitmap value in
				   v1.0 is bit 11) */
    uint8_t iElement;
} __attribute__((packed));

/* Table 6-5: MIDI Element Descriptor (single)
 *
 * This structure is a convenience covering the (common) case where
 * there is only one input pin.
 */
struct usb_midi_element_descriptor {
    struct usb_midi_element_descriptor_head head;
    struct usb_midi_element_descriptor_body source[1];
    struct usb_midi_element_descriptor_tail tail;
} __attribute__((packed));

/* Table 6-7: Class-specific MS Bulk Data Endpoint Descriptor (head) */
struct usb_midi_endpoint_descriptor_head {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubType;
    uint8_t bNumEmbMIDIJack;
} __attribute__((packed));

/* Table 6-7: Class-specific MS Bulk Data Endpoint Descriptor (body) */
struct usb_midi_endpoint_descriptor_body {
    uint8_t baAssocJackID;
} __attribute__((packed));

/* Table 6.7: Class-specific MS Bulk Data Endpoint Descriptor (single)
 *
 * This structure is a convenience covering the (normal) case where
 * there is only one input pin.
 */
struct usb_midi_endpoint_descriptor {
    struct usb_midi_endpoint_descriptor_head head;
    struct usb_midi_endpoint_descriptor_body jack[1];
} __attribute__((packed));

#endif

/**@}*/
