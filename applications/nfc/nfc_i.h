#pragma once

#include <flipper_v2.h>

#include <rfal_analogConfig.h>
#include <rfal_rf.h>
#include <rfal_nfc.h>
#include <rfal_nfca.h>
#include <st25r3916.h>
#include <st25r3916_irq.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/canvas.h>
#include <assets_icons.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

#include "dispatcher.h"

typedef enum {
    MessageTypeBase,
} NfcMessageType;

typedef struct {
    Message base;
    void* data;
} NfcMessage;

struct Nfc {
    Dispatcher* dispatcher;
    Icon* icon;
    Widget* widget;
    ValueMutex* menu_vm;
    MenuItem* menu;
    rfalNfcDiscoverParam* disParams;

    osThreadAttr_t worker_attr;
    osThreadId_t worker;

    uint8_t screen;
    uint8_t ret;
    uint8_t devCnt;
    rfalNfcaSensRes first_atqa;
    rfalNfcaSelRes first_sak;

    char* current;
};
