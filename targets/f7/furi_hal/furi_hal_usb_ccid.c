#include <furi_hal_version.h>
#include <furi_hal_usb_i.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_ccid.h>
#include <furi.h>

#include <usb_ccid.h>

#define USB_DEVICE_NO_CLASS    (0x0)
#define USB_DEVICE_NO_SUBCLASS (0x0)
#define USB_DEVICE_NO_PROTOCOL (0x0)

#define FIXED_CONTROL_ENDPOINT_SIZE (8)
#define IF_NUM_MAX                  (1)

#define CCID_VID_DEFAULT (0x1234)
#define CCID_PID_DEFAULT (0xABCD)
#define CCID_TOTAL_SLOTS (1)
#define CCID_SLOT_INDEX  (0)

#define CCID_DATABLOCK_SIZE \
    (4 + 1 + CCID_SHORT_APDU_SIZE + 1) //APDU Header + Lc + Short APDU size + Le

#define ENDPOINT_DIR_IN  (0x80)
#define ENDPOINT_DIR_OUT (0x00)

#define INTERFACE_ID_CCID (0)

#define CCID_IN_EPADDR (ENDPOINT_DIR_IN | 2)

/** Endpoint address of the CCID data OUT endpoint, for host-to-device data transfers. */
#define CCID_OUT_EPADDR (ENDPOINT_DIR_OUT | 1)

/** Endpoint size in bytes of the CCID data being sent between IN and OUT endpoints. */
#define CCID_EPSIZE 64

struct CcidIntfDescriptor {
    struct usb_interface_descriptor ccid;
    struct usb_ccid_descriptor ccid_desc;
    struct usb_endpoint_descriptor ccid_bulk_in;
    struct usb_endpoint_descriptor ccid_bulk_out;
} FURI_PACKED;

struct CcidConfigDescriptor {
    struct usb_config_descriptor config;
    struct CcidIntfDescriptor intf_0;
} FURI_PACKED;

enum CCID_Features_Auto_t {
    CCID_Features_Auto_None = 0x0,
    CCID_Features_Auto_ParameterConfiguration = 0x2,
    CCID_Features_Auto_ICCActivation = 0x4,
    CCID_Features_Auto_VoltageSelection = 0x8,

    CCID_Features_Auto_ICCClockFrequencyChange = 0x10,
    CCID_Features_Auto_ICCBaudRateChange = 0x20,
    CCID_Features_Auto_ParameterNegotiation = 0x40,
    CCID_Features_Auto_PPS = 0x80,
};

enum CCID_Features_ExchangeLevel_t {
    CCID_Features_ExchangeLevel_TPDU = 0x00010000,
    CCID_Features_ExchangeLevel_ShortAPDU = 0x00020000,
    CCID_Features_ExchangeLevel_ShortExtendedAPDU = 0x00040000
};

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRequest = (1 << 1),
} WorkerEvtFlags;

typedef struct ccid_bulk_message_header {
    uint8_t bMessageType;
    uint32_t dwLength;
    uint8_t bSlot;
    uint8_t bSeq;
} FURI_PACKED ccid_bulk_message_header_t;

/* Device descriptor */
static struct usb_device_descriptor ccid_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_DEVICE_NO_CLASS,
    .bDeviceSubClass = USB_DEVICE_NO_SUBCLASS,
    .bDeviceProtocol = USB_DEVICE_NO_PROTOCOL,
    .bMaxPacketSize0 = FIXED_CONTROL_ENDPOINT_SIZE,
    .idVendor = CCID_VID_DEFAULT,
    .idProduct = CCID_PID_DEFAULT,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = UsbDevManuf,
    .iProduct = UsbDevProduct,
    .iSerialNumber = UsbDevSerial,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor*/
static const struct CcidConfigDescriptor ccid_cfg_desc = {
    .config =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct CcidConfigDescriptor),
            .bNumInterfaces = 1,

            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
            .bMaxPower = USB_CFG_POWER_MA(500),
        },
    .intf_0 =
        {
            .ccid =
                {.bLength = sizeof(struct usb_interface_descriptor),
                 .bDescriptorType = USB_DTYPE_INTERFACE,

                 .bInterfaceNumber = INTERFACE_ID_CCID,
                 .bAlternateSetting = 0x00,
                 .bNumEndpoints = 2,

                 .bInterfaceClass = USB_CLASS_CCID,
                 .bInterfaceSubClass = 0,
                 .bInterfaceProtocol = 0,

                 .iInterface = NO_DESCRIPTOR

                },
            .ccid_desc =
                {.bLength = sizeof(struct usb_ccid_descriptor),
                 .bDescriptorType = USB_DTYPE_CCID_FUNCTIONAL,
                 .bcdCCID = CCID_CURRENT_SPEC_RELEASE_NUMBER,
                 .bMaxSlotIndex = 0x00,
                 .bVoltageSupport = CCID_VOLTAGESUPPORT_5V,
                 .dwProtocols = 0x01, //T0
                 .dwDefaultClock = 16000, //16MHz
                 .dwMaximumClock = 16000, //16MHz
                 .bNumClockSupported = 0,
                 .dwDataRate = 307200,
                 .dwMaxDataRate = 307200,
                 .bNumDataRatesSupported = 0,
                 .dwMaxIFSD = 2038,
                 .dwSynchProtocols = 0,
                 .dwMechanical = 0,
                 .dwFeatures = CCID_Features_ExchangeLevel_ShortAPDU |
                               CCID_Features_Auto_ParameterConfiguration |
                               CCID_Features_Auto_ICCActivation |
                               CCID_Features_Auto_VoltageSelection,
                 .dwMaxCCIDMessageLength = 0x0c00,
                 .bClassGetResponse = 0xff,
                 .bClassEnvelope = 0xff,
                 .wLcdLayout = 0,
                 .bPINSupport = 0,
                 .bMaxCCIDBusySlots = 1},
            .ccid_bulk_in =
                {.bLength = sizeof(struct usb_endpoint_descriptor),
                 .bDescriptorType = USB_DTYPE_ENDPOINT,
                 .bEndpointAddress = CCID_IN_EPADDR,
                 .bmAttributes = USB_EPTYPE_BULK,
                 .wMaxPacketSize = CCID_EPSIZE,
                 .bInterval = 0x05

                },
            .ccid_bulk_out =
                {.bLength = sizeof(struct usb_endpoint_descriptor),
                 .bDescriptorType = USB_DTYPE_ENDPOINT,
                 .bEndpointAddress = CCID_OUT_EPADDR,
                 .bmAttributes = USB_EPTYPE_BULK,
                 .wMaxPacketSize = CCID_EPSIZE,
                 .bInterval = 0x05},
        },
};

static void ccid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void ccid_deinit(usbd_device* dev);
static void ccid_on_wakeup(usbd_device* dev);
static void ccid_on_suspend(usbd_device* dev);
static int32_t ccid_worker(void* context);

FuriHalUsbInterface usb_ccid = {
    .init = ccid_init,
    .deinit = ccid_deinit,
    .wakeup = ccid_on_wakeup,
    .suspend = ccid_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&ccid_device_desc,

    .str_manuf_descr = NULL,
    .str_prod_descr = NULL,
    .str_serial_descr = NULL,

    .cfg_descr = (void*)&ccid_cfg_desc,
};

static usbd_respond ccid_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond ccid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);

typedef struct {
    bool connected;
    bool smartcard_inserted;
    CcidCallbacks* callbacks[CCID_TOTAL_SLOTS];
    void* cb_ctx[CCID_TOTAL_SLOTS];
    FuriThread* ccid_thread;
    FuriSemaphore* ccid_semaphore;
    usbd_device* usb_dev;

    uint16_t receive_buffer_data_index;
    uint8_t send_buffer[sizeof(ccid_bulk_message_header_t) + CCID_DATABLOCK_SIZE];
    uint8_t receive_buffer[sizeof(ccid_bulk_message_header_t) + CCID_DATABLOCK_SIZE];
} FuriHalUsbCcid;

static FuriHalUsbCcid* furi_hal_usb_ccid = NULL;

static void* ccid_set_string_descr(char* str) {
    furi_check(str);

    size_t len = strlen(str);
    struct usb_string_descriptor* dev_str_desc = malloc(len * 2 + 2);
    dev_str_desc->bLength = len * 2 + 2;
    dev_str_desc->bDescriptorType = USB_DTYPE_STRING;
    for(size_t i = 0; i < len; i++)
        dev_str_desc->wString[i] = str[i];

    return dev_str_desc;
}

static void ccid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);

    FuriHalUsbCcidConfig* cfg = (FuriHalUsbCcidConfig*)ctx;

    furi_check(furi_hal_usb_ccid == NULL);
    furi_hal_usb_ccid = malloc(sizeof(FuriHalUsbCcid));

    furi_hal_usb_ccid->usb_dev = dev;
    furi_hal_usb_ccid->ccid_semaphore = furi_semaphore_alloc(1, 1);

    usb_ccid.dev_descr->iManufacturer = 0;
    usb_ccid.dev_descr->iProduct = 0;
    usb_ccid.str_manuf_descr = NULL;
    usb_ccid.str_prod_descr = NULL;
    usb_ccid.dev_descr->idVendor = CCID_VID_DEFAULT;
    usb_ccid.dev_descr->idProduct = CCID_PID_DEFAULT;

    if(cfg != NULL) {
        usb_ccid.dev_descr->idVendor = cfg->vid;
        usb_ccid.dev_descr->idProduct = cfg->pid;

        if(cfg->manuf[0] != '\0') {
            usb_ccid.str_manuf_descr = ccid_set_string_descr(cfg->manuf);
            usb_ccid.dev_descr->iManufacturer = UsbDevManuf;
        }

        if(cfg->product[0] != '\0') {
            usb_ccid.str_prod_descr = ccid_set_string_descr(cfg->product);
            usb_ccid.dev_descr->iProduct = UsbDevProduct;
        }
    }

    usbd_reg_config(dev, ccid_ep_config);
    usbd_reg_control(dev, ccid_control);

    usbd_connect(dev, true);

    furi_hal_usb_ccid->receive_buffer_data_index = 0;
    furi_hal_usb_ccid->ccid_thread = furi_thread_alloc_ex("CcidWorker", 2048, ccid_worker, ctx);
    furi_thread_start(furi_hal_usb_ccid->ccid_thread);
}

static void ccid_deinit(usbd_device* dev) {
    furi_thread_flags_set(furi_thread_get_id(furi_hal_usb_ccid->ccid_thread), WorkerEvtStop);
    furi_thread_join(furi_hal_usb_ccid->ccid_thread);
    furi_thread_free(furi_hal_usb_ccid->ccid_thread);

    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);

    free(usb_ccid.str_prod_descr);
    free(usb_ccid.str_serial_descr);

    free(furi_hal_usb_ccid);

    furi_hal_usb_ccid = NULL;
}

static void ccid_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    furi_check(furi_hal_usb_ccid);

    furi_hal_usb_ccid->connected = true;
}

static void ccid_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    furi_check(furi_hal_usb_ccid);

    furi_hal_usb_ccid->connected = false;
}

void CALLBACK_CCID_GetSlotStatus(
    uint8_t slot,
    uint8_t seq,
    struct rdr_to_pc_slot_status* responseSlotStatus) {
    furi_check(responseSlotStatus);

    furi_check(furi_hal_usb_ccid);

    responseSlotStatus->bMessageType = RDR_TO_PC_SLOTSTATUS;

    responseSlotStatus->bSlot = slot;
    responseSlotStatus->bSeq = seq;
    responseSlotStatus->bClockStatus = 0;

    responseSlotStatus->dwLength = 0;

    if(responseSlotStatus->bSlot == CCID_SLOT_INDEX) {
        responseSlotStatus->bError = CCID_ERROR_NOERROR;
        if(furi_hal_usb_ccid->smartcard_inserted) {
            responseSlotStatus->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                          CCID_ICCSTATUS_PRESENTANDACTIVE;
        } else {
            responseSlotStatus->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                          CCID_ICCSTATUS_NOICCPRESENT;
        }
    } else {
        responseSlotStatus->bError = CCID_ERROR_SLOTNOTFOUND;
        responseSlotStatus->bStatus = CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
    }
}

void CALLBACK_CCID_SetParametersT0(

    struct pc_to_rdr_set_parameters_t0* requestSetParametersT0,
    struct rdr_to_pc_parameters_t0* responseSetParametersT0) {
    furi_check(requestSetParametersT0);
    furi_check(responseSetParametersT0);

    furi_check(furi_hal_usb_ccid);

    furi_check(requestSetParametersT0->bProtocolNum == 0x00); //T0

    responseSetParametersT0->bMessageType = RDR_TO_PC_PARAMETERS;
    responseSetParametersT0->bSlot = requestSetParametersT0->bSlot;
    responseSetParametersT0->bSeq = requestSetParametersT0->bSeq;

    responseSetParametersT0->dwLength =
        sizeof(struct pc_to_rdr_set_parameters_t0) - sizeof(ccid_bulk_message_header_t);

    if(responseSetParametersT0->bSlot == CCID_SLOT_INDEX) {
        responseSetParametersT0->bError = CCID_ERROR_NOERROR;
        if(furi_hal_usb_ccid->smartcard_inserted) {
            responseSetParametersT0->bProtocolNum = requestSetParametersT0->bProtocolNum;
            responseSetParametersT0->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                               CCID_ICCSTATUS_PRESENTANDACTIVE;
        } else {
            responseSetParametersT0->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                               CCID_ICCSTATUS_NOICCPRESENT;
        }
    } else {
        responseSetParametersT0->bError = CCID_ERROR_SLOTNOTFOUND;
        responseSetParametersT0->bStatus = CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
    }
}

void CALLBACK_CCID_IccPowerOn(
    uint8_t slot,
    uint8_t seq,
    struct rdr_to_pc_data_block* responseDataBlock) {
    furi_check(responseDataBlock);

    furi_check(furi_hal_usb_ccid);

    responseDataBlock->bMessageType = RDR_TO_PC_DATABLOCK;
    responseDataBlock->dwLength = 0;
    responseDataBlock->bSlot = slot;
    responseDataBlock->bSeq = seq;

    if(responseDataBlock->bSlot == CCID_SLOT_INDEX) {
        responseDataBlock->bError = CCID_ERROR_NOERROR;
        if(furi_hal_usb_ccid->smartcard_inserted) {
            if(furi_hal_usb_ccid->callbacks[CCID_SLOT_INDEX] != NULL) {
                furi_hal_usb_ccid->callbacks[CCID_SLOT_INDEX]->icc_power_on_callback(
                    responseDataBlock->abData,
                    &responseDataBlock->dwLength,
                    furi_hal_usb_ccid->cb_ctx[CCID_SLOT_INDEX]);
                responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                             CCID_ICCSTATUS_PRESENTANDACTIVE;
            } else {
                responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                             CCID_ICCSTATUS_PRESENTANDINACTIVE;
            }
        } else {
            responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                         CCID_ICCSTATUS_NOICCPRESENT;
        }
    } else {
        responseDataBlock->bError = CCID_ERROR_SLOTNOTFOUND;
        responseDataBlock->bStatus = CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
    }
}

void CALLBACK_CCID_XfrBlock(
    struct pc_to_rdr_xfr_block* receivedXfrBlock,
    struct rdr_to_pc_data_block* responseDataBlock) {
    furi_check(receivedXfrBlock);
    furi_check(responseDataBlock);

    furi_check(furi_hal_usb_ccid);

    responseDataBlock->bMessageType = RDR_TO_PC_DATABLOCK;
    responseDataBlock->bSlot = receivedXfrBlock->bSlot;
    responseDataBlock->bSeq = receivedXfrBlock->bSeq;
    responseDataBlock->bChainParameter = 0;

    if(responseDataBlock->bSlot == CCID_SLOT_INDEX) {
        responseDataBlock->bError = CCID_ERROR_NOERROR;
        if(furi_hal_usb_ccid->smartcard_inserted) {
            if(furi_hal_usb_ccid->callbacks[CCID_SLOT_INDEX] != NULL) {
                furi_hal_usb_ccid->callbacks[CCID_SLOT_INDEX]->xfr_datablock_callback(
                    (const uint8_t*)receivedXfrBlock->abData,
                    receivedXfrBlock->dwLength,
                    responseDataBlock->abData,
                    &responseDataBlock->dwLength,
                    furi_hal_usb_ccid->cb_ctx[CCID_SLOT_INDEX]);
                responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                             CCID_ICCSTATUS_PRESENTANDACTIVE;
            } else {
                responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                             CCID_ICCSTATUS_PRESENTANDINACTIVE;
            }
        } else {
            responseDataBlock->bStatus = CCID_COMMANDSTATUS_PROCESSEDWITHOUTERROR |
                                         CCID_ICCSTATUS_NOICCPRESENT;
        }
    } else {
        responseDataBlock->bError = CCID_ERROR_SLOTNOTFOUND;
        responseDataBlock->bStatus = CCID_COMMANDSTATUS_FAILED | CCID_ICCSTATUS_NOICCPRESENT;
    }
}

void furi_hal_usb_ccid_insert_smartcard(void) {
    furi_check(furi_hal_usb_ccid);

    furi_hal_usb_ccid->smartcard_inserted = true;
}

void furi_hal_usb_ccid_remove_smartcard(void) {
    furi_check(furi_hal_usb_ccid);

    furi_hal_usb_ccid->smartcard_inserted = false;
}

void furi_hal_usb_ccid_set_callbacks(CcidCallbacks* cb, void* context) {
    furi_check(furi_hal_usb_ccid);

    furi_hal_usb_ccid->callbacks[CCID_SLOT_INDEX] = cb;
    furi_hal_usb_ccid->cb_ctx[CCID_SLOT_INDEX] = context;
}

void ccid_send_packet(uint8_t* data, uint8_t len) {
    furi_check(furi_hal_usb_ccid);

    if(furi_hal_usb_ccid->ccid_semaphore == NULL || furi_hal_usb_ccid->connected == false) return;
    furi_check(
        furi_semaphore_acquire(furi_hal_usb_ccid->ccid_semaphore, FuriWaitForever) ==
        FuriStatusOk);
    if(furi_hal_usb_ccid->connected == true) {
        usbd_ep_write(furi_hal_usb_ccid->usb_dev, CCID_IN_EPADDR, data, len);
    }
}

void ccid_send_response(uint8_t* data, uint32_t len) {
    uint32_t data_to_send = len;
    uint32_t data_index = 0;
    while(data_to_send >= CCID_EPSIZE) {
        ccid_send_packet(&data[data_index], CCID_EPSIZE);
        data_to_send = data_to_send - CCID_EPSIZE;
        data_index = data_index + CCID_EPSIZE;
    }

    ccid_send_packet(&data[data_index], data_to_send);
}

static void ccid_rx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    furi_check(furi_hal_usb_ccid);

    furi_semaphore_release(furi_hal_usb_ccid->ccid_semaphore);
}

static void ccid_tx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(ep);
    if(event == usbd_evt_eprx) {
        furi_check(furi_hal_usb_ccid);
        int32_t bytes_read = usbd_ep_read(
            furi_hal_usb_ccid->usb_dev,
            CCID_OUT_EPADDR,
            &furi_hal_usb_ccid->receive_buffer[furi_hal_usb_ccid->receive_buffer_data_index],
            CCID_EPSIZE);

        if(bytes_read > 0) {
            furi_hal_usb_ccid->receive_buffer_data_index =
                furi_hal_usb_ccid->receive_buffer_data_index + bytes_read;

            furi_thread_flags_set(
                furi_thread_get_id(furi_hal_usb_ccid->ccid_thread), WorkerEvtRequest);
        }
    }
}

static int32_t ccid_worker(void* context) {
    UNUSED(context);

    while(1) {
        furi_check(furi_hal_usb_ccid);
        uint32_t flags = furi_thread_flags_wait(
            WorkerEvtStop | WorkerEvtRequest, FuriFlagWaitAny, FuriWaitForever);

        if(flags & WorkerEvtRequest) {
            //read initial CCID message header

            ccid_bulk_message_header_t* message =
                (ccid_bulk_message_header_t*)&furi_hal_usb_ccid->receive_buffer; //-V641

            furi_check(message);

            if(message->bMessageType == PC_TO_RDR_ICCPOWERON) {
                struct pc_to_rdr_icc_power_on* requestDataBlock =
                    (struct pc_to_rdr_icc_power_on*)message; //-V641
                struct rdr_to_pc_data_block* responseDataBlock =
                    (struct rdr_to_pc_data_block*)&furi_hal_usb_ccid->send_buffer;

                CALLBACK_CCID_IccPowerOn(
                    requestDataBlock->bSlot, requestDataBlock->bSeq, responseDataBlock);

                ccid_send_response(
                    furi_hal_usb_ccid->send_buffer,
                    sizeof(struct rdr_to_pc_data_block) +
                        (sizeof(uint8_t) * responseDataBlock->dwLength));

                furi_hal_usb_ccid->receive_buffer_data_index = 0;

            } else if(message->bMessageType == PC_TO_RDR_ICCPOWEROFF) {
                struct pc_to_rdr_icc_power_off* requestIccPowerOff =
                    (struct pc_to_rdr_icc_power_off*)message; //-V641
                struct rdr_to_pc_slot_status* responseSlotStatus =
                    (struct rdr_to_pc_slot_status*)&furi_hal_usb_ccid->send_buffer; //-V641

                CALLBACK_CCID_GetSlotStatus(
                    requestIccPowerOff->bSlot, requestIccPowerOff->bSeq, responseSlotStatus);

                ccid_send_response(
                    furi_hal_usb_ccid->send_buffer, sizeof(struct rdr_to_pc_slot_status));

                furi_hal_usb_ccid->receive_buffer_data_index = 0;

            } else if(message->bMessageType == PC_TO_RDR_GETSLOTSTATUS) {
                struct pc_to_rdr_get_slot_status* requestSlotStatus =
                    (struct pc_to_rdr_get_slot_status*)message; //-V641
                struct rdr_to_pc_slot_status* responseSlotStatus =
                    (struct rdr_to_pc_slot_status*)&furi_hal_usb_ccid->send_buffer; //-V641

                CALLBACK_CCID_GetSlotStatus(
                    requestSlotStatus->bSlot, requestSlotStatus->bSeq, responseSlotStatus);

                ccid_send_response(
                    furi_hal_usb_ccid->send_buffer, sizeof(struct rdr_to_pc_slot_status));

                furi_hal_usb_ccid->receive_buffer_data_index = 0;

            } else if(message->bMessageType == PC_TO_RDR_XFRBLOCK) {
                struct pc_to_rdr_xfr_block* receivedXfrBlock =
                    (struct pc_to_rdr_xfr_block*)message;
                struct rdr_to_pc_data_block* responseDataBlock =
                    (struct rdr_to_pc_data_block*)&furi_hal_usb_ccid->send_buffer;

                if(furi_hal_usb_ccid->receive_buffer_data_index >=
                   sizeof(struct pc_to_rdr_xfr_block) + receivedXfrBlock->dwLength) {
                    CALLBACK_CCID_XfrBlock(receivedXfrBlock, responseDataBlock);

                    ccid_send_response(
                        furi_hal_usb_ccid->send_buffer,
                        sizeof(struct rdr_to_pc_data_block) +
                            (sizeof(uint8_t) * responseDataBlock->dwLength));

                    furi_hal_usb_ccid->receive_buffer_data_index = 0;
                }

            } else if(message->bMessageType == PC_TO_RDR_SETPARAMETERS) {
                struct pc_to_rdr_set_parameters_t0* requestSetParametersT0 =
                    (struct pc_to_rdr_set_parameters_t0*)message; //-V641
                struct rdr_to_pc_parameters_t0* responseSetParametersT0 =
                    (struct rdr_to_pc_parameters_t0*)&furi_hal_usb_ccid->send_buffer; //-V641

                CALLBACK_CCID_SetParametersT0(requestSetParametersT0, responseSetParametersT0);

                ccid_send_response(
                    furi_hal_usb_ccid->send_buffer, sizeof(struct rdr_to_pc_parameters_t0));

                furi_hal_usb_ccid->receive_buffer_data_index = 0;
            }
        } else if(flags & WorkerEvtStop) {
            break;
        }
    }
    return 0;
}

/* Configure endpoints */
static usbd_respond ccid_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, CCID_IN_EPADDR);
        usbd_ep_deconfig(dev, CCID_OUT_EPADDR);
        usbd_reg_endpoint(dev, CCID_IN_EPADDR, 0);
        usbd_reg_endpoint(dev, CCID_OUT_EPADDR, 0);
        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, CCID_IN_EPADDR, USB_EPTYPE_BULK, CCID_EPSIZE);
        usbd_ep_config(dev, CCID_OUT_EPADDR, USB_EPTYPE_BULK, CCID_EPSIZE);
        usbd_reg_endpoint(dev, CCID_IN_EPADDR, ccid_rx_ep_callback);
        usbd_reg_endpoint(dev, CCID_OUT_EPADDR, ccid_tx_ep_callback);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond ccid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);
    /* CDC control requests */
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       (req->wIndex == 0 || req->wIndex == 2)) {
        switch(req->bRequest) {
        case CCID_ABORT:
            return usbd_fail;
        case CCID_GET_CLOCK_FREQUENCIES:
            dev->status.data_ptr = (void*)&(ccid_cfg_desc.intf_0.ccid_desc.dwDefaultClock);
            dev->status.data_count = sizeof(ccid_cfg_desc.intf_0.ccid_desc.dwDefaultClock);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    return usbd_fail;
}
