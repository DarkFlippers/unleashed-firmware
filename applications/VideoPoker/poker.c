#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <gui/elements.h>
#include "assets_icons.h"
#include <gui/icon_i.h>

/* Core game logic from
https://github.com/Yaoir/VideoPoker-C
*/

/* KNOWN BUGS
This has been converted from a standalone PC console app to flipper
All of the original input/output handing code has been ripped out
Original code also used TONS of defines and everything was a global.
As is, it does what I wanted and doesn't seem to have major issues, so that's pretty good.
Game logic is handled during input and this is a bit of a mess of nested ifs.
Sometimes duplicate cards will show up. there is a function to test this. I should use it better.

*/

#define TAG "Video Poker"

static void Shake(void) {
    NotificationApp* notification = furi_record_open("notification");
    notification_message(notification, &sequence_single_vibro);
    furi_record_close("notification");
}

typedef struct {
    int index; /* cards value, minus 1 */
    char* sym; /* text appearance */
    int suit; /* card's suit (see just below) */
    int gone; /* true if it's been dealt */
    int held; /* for hand */
} PokerPlayer_card;

typedef struct {
    FuriMutex** model_mutex;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Gui* gui;
    PokerPlayer_card hand[5];
    PokerPlayer_card shand[5];
    PokerPlayer_card deck[52];
    int GameType; /* What rules are we using */
    int held[5];
    int score;
    int highscore;
    int pot;
    int GameState;
    int selected;
    int bet;
    int minbet;
} PokerPlayer;

/* GameState 
0=Splash/help, OK button (later on up/down for rules or settings)
1=cards down, betting enabled, left/right to change bet, OK to confirm
2=first hand, holding enabled, left/right to pick card, OK to hold/unhold card, down to confirm
3=second hand, only confirm to claim rewards
4=game over/won 
5=WIP saving gamestate
*/

/*
#define AllAmerican 0
#define TensOrBetter 1
#define BonusPoker 2
#define DoubleBonus 3
#define DoubleBonusBonus 4
#define JacksOrBetter 5 
#define JacksOrBetter95 6
#define JacksOrBetter86 7
#define JacksOrBetter85 8
#define JacksOrBetter75 9
#define JacksOrBetter65 10
#define NUMGAMES 11
*/
/*
    The game in play. Default is Jacks or Better,
    which is coded into initialization of static data
*/
const char* gamenames[11] = {
    "All American",
    "Tens or Better",
    "Bonus Poker",
    "Double Bonus",
    "Double Bonus Bonus",
    "Jacks or Better",
    "9/5 Jacks or Better",
    "8/6 Jacks or Better",
    "8/5 Jacks or Better",
    "7/5 Jacks or Better",
    "6/5 Jacks or Better"};

PokerPlayer_card deck[52] = {
    /*	index, card name, suit, gone */
    /* Clubs:0  Diamonds:1  Hearts: 2   Spades: 3 */
    {1, "2", 0, 0, 0},  {2, "3", 0, 0, 0},  {3, "4", 0, 0, 0},  {4, "5", 0, 0, 0},  {5, "6", 0, 0, 0},
    {6, "7", 0, 0, 0},  {7, "8", 0, 0, 0},  {8, "9", 0, 0, 0},  {9, "10", 0, 0, 0}, {10, "J", 0, 0, 0},
    {11, "Q", 0, 0, 0}, {12, "K", 0, 0, 0}, {13, "A", 0, 0, 0},

    {1, "2", 1, 0, 0},  {2, "3", 1, 0, 0},  {3, "4", 1, 0, 0},  {4, "5", 1, 0, 0},  {5, "6", 1, 0, 0},
    {6, "7", 1, 0, 0},  {7, "8", 1, 0, 0},  {8, "9", 1, 0, 0},  {9, "10", 1, 0, 0}, {10, "J", 1, 0, 0},
    {11, "Q", 1, 0, 0}, {12, "K", 1, 0, 0}, {13, "A", 1, 0, 0},

    {1, "2", 2, 0, 0},  {2, "3", 2, 0, 0},  {3, "4", 2, 0, 0},  {4, "5", 2, 0, 0},  {5, "6", 2, 0, 0},
    {6, "7", 2, 0, 0},  {7, "8", 2, 0, 0},  {8, "9", 2, 0, 0},  {9, "10", 2, 0, 0}, {10, "J", 2, 0, 0},
    {11, "Q", 2, 0, 0}, {12, "K", 2, 0, 0}, {13, "A", 2, 0, 0},

    {1, "2", 3, 0, 0},  {2, "3", 3, 0, 0},  {3, "4", 3, 0, 0},  {4, "5", 3, 0, 0},  {5, "6", 3, 0, 0},
    {6, "7", 3, 0, 0},  {7, "8", 3, 0, 0},  {8, "9", 3, 0, 0},  {9, "10", 3, 0, 0}, {10, "J", 3, 0, 0},
    {11, "Q", 3, 0, 0}, {12, "K", 3, 0, 0}, {13, "A", 3, 0, 0},
};

/* 
Image Format
0x01 = Compressed
0x00 = Reserved Section
0xa4,0x01 = 0x1a4, or, 420 - the size of the compressed array, minus this header.
Rest of the data is char array output from heatshrink of the original XBM char array.
Calculated Header: 0x01,0x00,0xa4,0x01
from furi_hal_compress.c:
typedef struct {
    uint8_t is_compressed;
    uint8_t reserved;
    uint16_t compressed_buff_size;
} FuriHalCompressHeader;
*/

const uint8_t _I_Splash_128x64_0[] = {
    0x01, 0x00, 0x8a, 0x02, 0x00, 0x78, 0x02, 0x60, 0xe0, 0x54, 0xc0, 0x03, 0x9f, 0xc0, 0x0f, 0x5a,
    0x04, 0x04, 0x1e, 0xdf, 0x08, 0x78, 0x0c, 0x60, 0xc0, 0x21, 0x90, 0x40, 0xa3, 0x00, 0xf5, 0xfe,
    0x61, 0xc1, 0xe9, 0x1e, 0x8e, 0x59, 0xf0, 0x02, 0x24, 0x9f, 0x70, 0xc0, 0x63, 0x03, 0x01, 0x0c,
    0x0b, 0xc1, 0x80, 0xbc, 0x83, 0xd3, 0x3f, 0x63, 0x98, 0x03, 0xcf, 0x88, 0x02, 0x1c, 0x31, 0x5d,
    0x38, 0xf6, 0x19, 0xc0, 0xa0, 0xfc, 0x93, 0x13, 0x12, 0xf0, 0x38, 0x76, 0x08, 0xc7, 0x00, 0x1e,
    0x5e, 0x8b, 0xcc, 0x32, 0x86, 0x0f, 0x4f, 0x0c, 0x80, 0x06, 0x20, 0x72, 0xe4, 0x5e, 0x33, 0xd4,
    0x73, 0xf2, 0x5d, 0xe2, 0x10, 0xef, 0xe6, 0x02, 0x0f, 0x07, 0x84, 0x4c, 0x33, 0xd2, 0x70, 0x79,
    0xd8, 0x2e, 0x11, 0x88, 0x3d, 0xff, 0xc1, 0xc7, 0x83, 0xc4, 0x20, 0x10, 0xc9, 0x18, 0x3d, 0x27,
    0x18, 0x8c, 0x3c, 0xde, 0xe1, 0xe6, 0x87, 0x7e, 0x0c, 0x62, 0x12, 0x10, 0x01, 0xce, 0x31, 0x9c,
    0x39, 0x9c, 0x62, 0x67, 0x0f, 0x83, 0x7f, 0x27, 0xe0, 0xf5, 0x8c, 0x71, 0xbc, 0x31, 0x8c, 0xc4,
    0xe2, 0x1e, 0x62, 0x1e, 0x02, 0xe0, 0x80, 0x05, 0x1c, 0xe1, 0xdc, 0x23, 0x97, 0xc8, 0xe4, 0x5c,
    0x12, 0x50, 0x40, 0x7a, 0x43, 0x38, 0x77, 0x88, 0xf4, 0x36, 0x3d, 0x1f, 0x04, 0x94, 0x20, 0x1e,
    0x98, 0xce, 0x0d, 0xbe, 0x37, 0x0d, 0xcd, 0xbd, 0x0c, 0x7e, 0xbe, 0xce, 0x07, 0x1f, 0xf3, 0xfc,
    0xf8, 0xb2, 0x8d, 0x30, 0x20, 0x53, 0xbe, 0x60, 0x06, 0x03, 0x78, 0xf0, 0x06, 0x4c, 0x1e, 0x34,
    0x10, 0x29, 0x5e, 0x05, 0x0f, 0x00, 0xa0, 0x40, 0x24, 0x20, 0x52, 0x76, 0x88, 0x01, 0xc1, 0xe3,
    0x11, 0x05, 0xc3, 0xe9, 0x20, 0x10, 0x97, 0x01, 0xcf, 0xc1, 0xf2, 0x81, 0x3f, 0xe7, 0xfc, 0x66,
    0xf4, 0x02, 0xf1, 0xc0, 0x3f, 0xdf, 0xf0, 0x30, 0xc6, 0x1e, 0xe5, 0xff, 0x81, 0xf0, 0x3f, 0xe5,
    0xb2, 0x80, 0x7f, 0xc1, 0xe5, 0x1c, 0x03, 0x0f, 0xe3, 0xff, 0x1f, 0xf8, 0x02, 0x48, 0x00, 0x31,
    0xfe, 0x0b, 0xa4, 0x61, 0xcc, 0x62, 0xfc, 0x4f, 0xe3, 0x0f, 0x31, 0x41, 0x0e, 0x02, 0x07, 0x01,
    0x07, 0x8a, 0xb4, 0xa3, 0x84, 0x71, 0x8f, 0xff, 0x20, 0x77, 0x00, 0x78, 0x95, 0x46, 0x06, 0x13,
    0x10, 0x78, 0xef, 0x3f, 0x5f, 0xfc, 0xff, 0xea, 0x07, 0xf0, 0x37, 0x90, 0x3c, 0x78, 0x00, 0xf2,
    0xae, 0x7f, 0x77, 0xf7, 0xaf, 0xec, 0x0f, 0x88, 0x41, 0x1b, 0x06, 0x02, 0x03, 0xc0, 0x02, 0x8c,
    0x08, 0x5c, 0x37, 0xff, 0xa9, 0x3c, 0x7b, 0xcc, 0x52, 0xe0, 0x70, 0x7c, 0x31, 0x89, 0xe4, 0xff,
    0xfb, 0xff, 0xdf, 0x8c, 0x46, 0x03, 0x1f, 0x34, 0x17, 0x83, 0xe1, 0x71, 0x8f, 0x6f, 0xe7, 0xe0,
    0xc1, 0x8f, 0xfd, 0x20, 0x18, 0x65, 0x59, 0x47, 0xaf, 0x9b, 0x8b, 0x9e, 0x6f, 0xe7, 0x1f, 0x16,
    0x0c, 0x3e, 0x3d, 0x00, 0xe4, 0x43, 0xd1, 0xe5, 0x3f, 0xe6, 0x6e, 0xfb, 0x39, 0x88, 0x67, 0xea,
    0xff, 0xc5, 0x22, 0x8f, 0xc0, 0xf0, 0x41, 0x71, 0xe7, 0x76, 0xf9, 0x98, 0x48, 0x64, 0x17, 0x59,
    0x38, 0x05, 0x8f, 0xc0, 0xd0, 0x5f, 0xe8, 0x0f, 0x1a, 0xdb, 0xe6, 0xb1, 0xd1, 0xa0, 0x50, 0x85,
    0x59, 0x7e, 0x16, 0x05, 0x06, 0x80, 0x71, 0xbf, 0xf7, 0x19, 0x85, 0x99, 0x74, 0x6d, 0x31, 0x02,
    0x10, 0x88, 0x7c, 0xdd, 0xdb, 0x84, 0x62, 0x7c, 0x0f, 0x38, 0xe5, 0xf0, 0x1e, 0x97, 0xce, 0x67,
    0xbc, 0xb6, 0x40, 0xa3, 0x98, 0x00, 0xc5, 0x76, 0x53, 0x8c, 0x67, 0x1e, 0x07, 0x0e, 0x63, 0x0a,
    0xe4, 0x9c, 0x62, 0x0f, 0x11, 0x41, 0x95, 0x88, 0x1e, 0x41, 0xd1, 0x8c, 0x49, 0x80, 0xe6, 0x00,
    0x50, 0xb8, 0xa3, 0x07, 0xf1, 0x7f, 0x06, 0xb8, 0x00, 0x61, 0xce, 0xb2, 0x9c, 0x53, 0x01, 0xf3,
    0xf0, 0x55, 0x97, 0xd0, 0x3f, 0x40, 0x03, 0xfd, 0x33, 0xc8, 0x01, 0x71, 0x92, 0x78, 0x80, 0x2f,
    0x80, 0x6f, 0x20, 0x03, 0xff, 0x23, 0xe7, 0x02, 0x02, 0x18, 0x01, 0xa3, 0x91, 0x00, 0x18, 0xc3,
    0x20, 0x91, 0xc0, 0x7c, 0x7f, 0x83, 0x42, 0xaa, 0x1f, 0xe0, 0xbe, 0x60, 0x46, 0xa2, 0x81, 0xe2,
    0x24, 0x21, 0xf9, 0x54, 0x14, 0x18, 0x9e, 0x3f, 0xe4, 0x29, 0x00, 0x12, 0x0e, 0xb0, 0x28, 0x50,
    0x3c, 0x60, 0x50, 0x85, 0xf4, 0x7f, 0xb8, 0x3f, 0xf3, 0xf8, 0x83, 0xe0, 0x00, 0x38, 0x6e, 0x0c,
    0xc3, 0xf2, 0x2f, 0x94, 0x09, 0x07, 0xc7, 0xf7, 0x3f, 0xfe, 0x0d, 0xc4, 0x00, 0xfc, 0x4c, 0x05,
    0x86, 0x15, 0x23, 0x92, 0x03, 0xe7, 0xf9, 0x80, 0x0f, 0x97, 0x52, 0x0c, 0x2f, 0xb1, 0xf8, 0xe3,
    0x01, 0xf3, 0x82, 0x27, 0x8d, 0xe6, 0x41, 0x1c, 0x17, 0xcf, 0xfc, 0x3e, 0x64, 0xf8,
};
const uint8_t* _I_Splash_128x64[] = {_I_Splash_128x64_0};
const Icon I_Splash_128x64 =
    {.width = 128, .height = 64, .frame_count = 1, .frame_rate = 0, .frames = _I_Splash_128x64};

/*
const uint8_t _I_BadEnd_128x64_0[] = {
    0x01, 0x00, 0xDF, 0x01, 0x00, 0x2c, 0x12, 0x01, 0x02, 0x80, 0x40, 0x70, 0x10, 0x0a, 0x04, 0x02,
    0x41, 0x3e, 0xcf, 0x63, 0xfb, 0xfe, 0xc8, 0x18, 0x3e, 0x6f, 0xdb, 0xfc, 0xf8, 0x3c, 0x60, 0xe0,
    0xf9, 0xb3, 0x6c, 0xf3, 0x3c, 0x1b, 0x6c, 0x18, 0x5f, 0x40, 0xf1, 0xe7, 0xdb, 0xc1, 0xf4, 0x2f,
    0x10, 0x78, 0xdb, 0xbc, 0xdf, 0xf0, 0x04, 0x59, 0x81, 0xe3, 0xc1, 0xb6, 0x41, 0x83, 0xd1, 0x00,
    0xbf, 0x6c, 0xc9, 0xe6, 0x0f, 0x91, 0xf8, 0x9b, 0xcc, 0x1f, 0x20, 0x06, 0x07, 0xf8, 0x3e, 0x0b,
    0x32, 0x00, 0x50, 0x88, 0xc4, 0x20, 0x10, 0x85, 0xfd, 0x03, 0xfc, 0x1f, 0xe0, 0xff, 0x07, 0xf9,
    0x7f, 0xc3, 0xdc, 0x89, 0x10, 0x7d, 0x00, 0x04, 0x1f, 0xe0, 0xfd, 0xfc, 0x40, 0xc1, 0xfb, 0x07,
    0x8e, 0x2f, 0xf3, 0x9f, 0x00, 0xb0, 0x7f, 0x97, 0xf6, 0x0a, 0x11, 0x10, 0xa3, 0xec, 0x10, 0x21,
    0x32, 0x07, 0xd0, 0x18, 0x40, 0xa2, 0x0f, 0xb0, 0x20, 0x81, 0xc4, 0x1f, 0xeb, 0xfa, 0xbf, 0x84,
    0x86, 0x01, 0xc8, 0x5f, 0xd0, 0x0c, 0x81, 0xe2, 0x05, 0x10, 0x7e, 0xdc, 0xc1, 0xf5, 0x01, 0xe0,
    0x41, 0xf2, 0x17, 0xf0, 0x7d, 0xaf, 0x0a, 0x7e, 0x0f, 0xbf, 0x84, 0x7f, 0x21, 0x1f, 0x2b, 0x8e,
    0x3c, 0xbe, 0xd3, 0xf0, 0x78, 0xc4, 0xfa, 0x0b, 0xf2, 0x00, 0x08, 0x81, 0xa1, 0xf3, 0x08, 0x9f,
    0xc0, 0x1e, 0x57, 0x00, 0x7b, 0x60, 0x60, 0x3e, 0x08, 0x4f, 0x80, 0x1e, 0x59, 0x05, 0xc1, 0x03,
    0xce, 0xc3, 0x00, 0x2f, 0x88, 0x3c, 0xe2, 0x10, 0x20, 0x78, 0xbd, 0xc6, 0xff, 0x7c, 0x8c, 0x0e,
    0x48, 0x1e, 0x90, 0x48, 0x47, 0xe2, 0x06, 0x1b, 0x1e, 0x3c, 0x1c, 0x1e, 0x80, 0x01, 0x93, 0xad,
    0x06, 0x1e, 0x0a, 0x28, 0x04, 0x18, 0x1e, 0x81, 0xe1, 0x90, 0x20, 0x46, 0x49, 0xa9, 0x91, 0x3e,
    0x46, 0xf8, 0x0f, 0xac, 0x48, 0x3c, 0xb0, 0x82, 0x52, 0x07, 0xa1, 0x08, 0x43, 0xe5, 0x72, 0x93,
    0x41, 0x7e, 0x01, 0x01, 0x07, 0xc7, 0x8a, 0x97, 0xa9, 0x39, 0x88, 0xa0, 0x7f, 0x00, 0xf2, 0x08,
    0x0c, 0x03, 0x25, 0x54, 0x88, 0xe9, 0x66, 0x11, 0xc2, 0x99, 0x9e, 0x07, 0xff, 0x13, 0x90, 0x7f,
    0xb2, 0x60, 0xf2, 0xaa, 0x79, 0x1b, 0xe5, 0x01, 0xfe, 0x1f, 0xca, 0x41, 0x08, 0xb0, 0xd4, 0xe2,
    0x33, 0x9c, 0x9f, 0x13, 0xff, 0x07, 0xc0, 0x0c, 0x04, 0x1e, 0x54, 0x08, 0x40, 0x64, 0x80, 0x03,
    0x84, 0xff, 0xc0, 0x68, 0x10, 0x0f, 0x80, 0x3d, 0x13, 0xc2, 0x00, 0x28, 0x25, 0xfa, 0x00, 0x0f,
    0x76, 0x60, 0x83, 0xcc, 0x04, 0x20, 0xc1, 0x07, 0xaf, 0xc8, 0x52, 0x52, 0x00, 0x7a, 0x2f, 0xcc,
    0x16, 0x31, 0x30, 0x49, 0x48, 0x17, 0xe5, 0x20, 0xc0, 0x23, 0xce, 0x81, 0x80, 0x88, 0xe6, 0x24,
    0x7c, 0x69, 0xc0, 0xd0, 0xa2, 0x1c, 0x00, 0x79, 0x85, 0x07, 0xe3, 0xa4, 0xb0, 0x4a, 0x64, 0xa0,
    0xf3, 0x57, 0x9d, 0x82, 0x01, 0x80, 0x84, 0x54, 0xb2, 0x19, 0x48, 0x91, 0x90, 0xa2, 0x1f, 0x00,
    0x79, 0x0f, 0x87, 0x80, 0x0f, 0x44, 0x21, 0x03, 0xd0, 0x3e, 0x26, 0x01, 0xa6, 0x44, 0x2c, 0x79,
    0xc0, 0x79, 0xb3, 0xc4, 0xbe, 0x5e, 0x01, 0x08, 0x80, 0x09, 0x56, 0x20, 0x01, 0x98, 0x03, 0xc4,
    0xfe, 0x51, 0x0b, 0xf8, 0x3c, 0xf8, 0x00, 0x32, 0x9c, 0x7f, 0x01, 0xe8, 0x1f, 0x40, 0x21, 0xd7,
    0x81, 0xfb, 0x80, 0xcf, 0x8f, 0x44, 0x1e, 0x7c, 0x88, 0x38, 0x28, 0x70, 0xe4, 0x92, 0xff, 0xc7,
    0xef, 0x1f, 0x80,
};
const uint8_t* _I_BadEnd_128x64[] = {_I_BadEnd_128x64_0};
const Icon I_BadEnd_128x64 =
    {.width = 128, .height = 64, .frame_count = 1, .frame_rate = 0, .frames = _I_BadEnd_128x64};
*/ /* space savings until external apps are possible */
const uint8_t _I_Hand_12x10_0[] = {
    0x01, 0x00, 0x11, 0x00, 0x8c, 0x40, 0x25, 0x00, 0x16, 0xb4, 0x40,
    0x35, 0x10, 0x1d, 0x5c, 0x1b, 0x5b, 0x0a, 0x84, 0xc2, 0x80,
};
const uint8_t* _I_Hand_12x10[] = {_I_Hand_12x10_0};
const Icon I_Hand_12x10 =
    {.width = 12, .height = 10, .frame_count = 1, .frame_rate = 0, .frames = _I_Hand_12x10};

const uint8_t _I_CardBack_22x35_0[] = {
    0x01, 0x00, 0x23, 0x00, 0xfe, 0x7f, 0xe1, 0xf0, 0x28, 0x04, 0x43, 0xe3, 0xff,
    0x91, 0xea, 0x75, 0x52, 0x6a, 0xad, 0x56, 0x5b, 0xad, 0xd5, 0x4a, 0x80, 0xbe,
    0x05, 0xf0, 0x2f, 0x81, 0x7c, 0x0b, 0x45, 0x32, 0x2c, 0x91, 0x7c, 0x8c, 0xa4,
};
const uint8_t* _I_CardBack_22x35[] = {_I_CardBack_22x35_0};
const Icon I_CardBack_22x35 =
    {.width = 22, .height = 35, .frame_count = 1, .frame_rate = 0, .frames = _I_CardBack_22x35};

//uncompressed but lol
const uint8_t _I_club_7x8_0[] = {0x00, 0x08, 0x1c, 0x1c, 0x6b, 0x7f, 0x36, 0x08, 0x1c};
const uint8_t* _I_club_7x8[] = {_I_club_7x8_0};
const Icon I_club_7x8 =
    {.width = 7, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_club_7x8};

//uncompressed but lol
const uint8_t _I_diamond_7x8_0[] = {0x00, 0x00, 0x08, 0x1c, 0x3e, 0x7f, 0x3e, 0x1c, 0x08};
const uint8_t* _I_diamond_7x8[] = {_I_diamond_7x8_0};
const Icon I_diamond_7x8 =
    {.width = 7, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_diamond_7x8};

//uncompressed
const uint8_t _I_hearts_7x8_0[] = {0x00, 0x00, 0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08};
const uint8_t* _I_hearts_7x8[] = {_I_hearts_7x8_0};
const Icon I_hearts_7x8 =
    {.width = 7, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_hearts_7x8};

//uncompressed
const uint8_t _I_spade_7x8_0[] = {0x00, 0x08, 0x1c, 0x3e, 0x7f, 0x7f, 0x36, 0x08, 0x1c};
const uint8_t* _I_spade_7x8[] = {_I_spade_7x8_0};
const Icon I_spade_7x8 =
    {.width = 7, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_spade_7x8};

// They only included Numeric Profont22 glyphs and I don't want to fuck up the font embeds right now sooo..

const uint8_t _I_King_7x8_0[] = {
    0x01, 0x00, 0x1a, 0x00, 0xc1, 0xc0, 0xf8, 0x70, 0x1f, 0x1c, 0x02, 0xe7, 0x00, 0x9d, 0xc0,
    0x23, 0xf0, 0x08, 0x78, 0x0c, 0x80, 0xe2, 0x0b, 0x10, 0x78, 0x84, 0xc4, 0x2e, 0x20, 0x01,
};
const uint8_t* _I_King_7x8[] = {_I_King_7x8_0};
const Icon I_King_7x8 =
    {.width = 10, .height = 14, .frame_count = 1, .frame_rate = 0, .frames = _I_King_7x8};

const uint8_t _I_Queen_7x8_0[] = {
    0x01, 0x00, 0x13, 0x00, 0xfe, 0x40, 0x3f, 0xd0, 0x1c, 0x3c, 0x0c, 0x01,
    0x76, 0x38, 0x1f, 0x8e, 0x07, 0xc7, 0x81, 0x85, 0x47, 0xf9, 0x01,
};
const uint8_t* _I_Queen_7x8[] = {_I_Queen_7x8_0};
const Icon I_Queen_7x8 =
    {.width = 10, .height = 14, .frame_count = 1, .frame_rate = 0, .frames = _I_Queen_7x8};

const uint8_t _I_Jack_7x8_0[] = {
    0x01,
    0x00,
    0x0D,
    0x00,
    0x80,
    0x40,
    0xc0,
    0x3a,
    0x00,
    0x5c,
    0x3c,
    0x0f,
    0xfd,
    0x01,
    0xfe,
    0x40,
    0x00,
};
const uint8_t* _I_Jack_7x8[] = {_I_Jack_7x8_0};
const Icon I_Jack_7x8 =
    {.width = 10, .height = 14, .frame_count = 1, .frame_rate = 0, .frames = _I_Jack_7x8};

const uint8_t _I_Ace_7x8_0[] = {
    0x01, 0x00, 0x13, 0x00, 0x98, 0x40, 0x2f, 0x00, 0x12, 0xe6, 0x00, 0x4b,
    0x0d, 0x01, 0x00, 0x8c, 0x0e, 0x07, 0xff, 0x00, 0x90, 0x01, 0xc0,
};
const uint8_t* _I_Ace_7x8[] = {_I_Ace_7x8_0};
const Icon I_Ace_7x8 =
    {.width = 10, .height = 14, .frame_count = 1, .frame_rate = 0, .frames = _I_Ace_7x8};

const uint8_t _I_Ten_7x8_0[] = {
    0x01, 0x00, 0x29, 0x00, 0x86, 0x7f, 0x00, 0x43, 0xfe, 0x80, 0xc3, 0xf0, 0xf0, 0x38, 0x7e,
    0x0e, 0x07, 0x0c, 0xe1, 0x80, 0x87, 0xc6, 0x02, 0x1b, 0x98, 0x08, 0x67, 0x60, 0x21, 0x8f,
    0x80, 0x86, 0x1e, 0x02, 0x18, 0x38, 0x08, 0x43, 0x43, 0x7f, 0x10, 0x0d, 0xfc, 0x4c, 0x20,
};
const uint8_t* _I_Ten_7x8[] = {_I_Ten_7x8_0};
const Icon I_Ten_7x8 =
    {.width = 18, .height = 14, .frame_count = 1, .frame_rate = 0, .frames = _I_Ten_7x8};

const Icon card_suit[4] = {I_diamond_7x8, I_club_7x8, I_hearts_7x8, I_spade_7x8};

const Icon card_face[5] = {I_Ten_7x8, I_Jack_7x8, I_Queen_7x8, I_King_7x8, I_Ace_7x8};

/* Sanity check: check that there are no duplicate cards in hand */

static void playcard(PokerPlayer* app) {
    int i, c, crd;

    int hold[5];
    hold[5] = 2;
    // int digit;
    c = 1;
    c++;
    c = hold[5]; /* FIX for unused-but-set-variable */
    /* initialize deck */
    for(i = 0; i < 52; i++) deck[i].gone = 0;

    /* initialize hold[] */
    for(i = 0; i < 5; i++) hold[i] = 1;

    /* app->score -= bet; */
    if (app->score>app->highscore){app->highscore=app->score;} /* record high water mark */

    for(i = 0; i < 5; i++) {
        /* find a card not already dealt */
        do crd = random() % 52;
        while(deck[crd].gone);
        hold[i] = 1;
        deck[crd].gone = 1;
        if(!app->held[i]) {
            app->hand[i] = deck[crd];
        }
    }
}

static int check_for_dupes(PokerPlayer* app) {
    int i, j;

    for(i = 0; i < 5; i++) {
        for(j = i + 1; j < 5; j++) {
            if(app->hand[i].index == app->hand[j].index && app->hand[i].suit == app->hand[j].suit)
                return 0;
        }
    }

    return 1;
}

/* Functions that recognize winning hands */

/*
    Flush:
    returns 1 if the sorted hand is a flush
*/

static int flush(PokerPlayer* app) {
    if(app->shand[0].suit == app->shand[1].suit && app->shand[1].suit == app->shand[2].suit &&
       app->shand[2].suit == app->shand[3].suit && app->shand[3].suit == app->shand[4].suit)
        return 1;

    return 0;
}

/*
    Straight:
    returns 1 if the sorted hand is a straight
*/

static int straight(PokerPlayer* app) {
    if(app->shand[1].index == app->shand[0].index + 1 &&
       app->shand[2].index == app->shand[1].index + 1 &&
       app->shand[3].index == app->shand[2].index + 1 &&
       app->shand[4].index == app->shand[3].index + 1)
        return 1;

    /* Ace low straight: Ace, 2, 3, 4, 5 */

    if(app->shand[4].index == 13 && app->shand[0].index == 1 && app->shand[1].index == 2 &&
       app->shand[2].index == 3 && app->shand[3].index == 4)
        return 1;

    return 0;
}

/*
    Four of a kind:
    the middle 3 all match, and the first or last matches those
*/

static int four(PokerPlayer* app) {
    if((app->shand[1].index == app->shand[2].index &&
        app->shand[2].index == app->shand[3].index) &&
       (app->shand[0].index == app->shand[2].index || app->shand[4].index == app->shand[2].index))
        return 1;

    return 0;
}

/*
    Full house:
    3 of a kind and a pair
*/

static int full(PokerPlayer* app) {
    if(app->shand[0].index == app->shand[1].index &&
       (app->shand[2].index == app->shand[3].index && app->shand[3].index == app->shand[4].index))
        return 1;

    if(app->shand[3].index == app->shand[4].index &&
       (app->shand[0].index == app->shand[1].index && app->shand[1].index == app->shand[2].index))
        return 1;

    return 0;
}

/*
    Three of a kind:
    it can appear 3 ways
*/

static int three(PokerPlayer* app) {
    if(app->shand[0].index == app->shand[1].index && app->shand[1].index == app->shand[2].index)
        return 1;

    if(app->shand[1].index == app->shand[2].index && app->shand[2].index == app->shand[3].index)
        return 1;

    if(app->shand[2].index == app->shand[3].index && app->shand[3].index == app->shand[4].index)
        return 1;

    return 0;
}

/*
    Two pair:
    it can appear in 3 ways
*/

static int twopair(PokerPlayer* app) {
    if(((app->shand[0].index == app->shand[1].index) &&
        (app->shand[2].index == app->shand[3].index)) ||
       ((app->shand[0].index == app->shand[1].index) &&
        (app->shand[3].index == app->shand[4].index)) ||
       ((app->shand[1].index == app->shand[2].index) &&
        (app->shand[3].index == app->shand[4].index)))
        return 1;

    return 0;
}

/*
    Two of a kind (pair), jacks or better
    or if the game is Tens or Better, 10s or better.
*/

static int two(PokerPlayer* app) {
    int min = 10;

    if(app->GameType == 1) min = 9;

    if(app->shand[0].index == app->shand[1].index && app->shand[1].index >= min) return 1;
    if(app->shand[1].index == app->shand[2].index && app->shand[2].index >= min) return 1;
    if(app->shand[2].index == app->shand[3].index && app->shand[3].index >= min) return 1;
    if(app->shand[3].index == app->shand[4].index && app->shand[4].index >= min) return 1;

    return 0;
}

static int paytable[10] = {
    800, /* royal flush: 800 */
    50, /* straight flush: 50 */
    25, /* 4 of a kind: 25 */
    9, /* full house: 9 */
    6, /* flush: 6 */
    4, /* straight: 4 */
    3, /* 3 of a kind: 3 */
    2, /* two pair: 2 */
    1, /* jacks or better: 1 */
    0 /* nothing */
};

static const char* poker_handname[10] = {
    "Royal Flush",
    "Straight Flush",
    "Four of a Kind",
    "Full House",
    "Flush",
    "Straight",
    "Three of a Kind",
    "Two Pair",
    "Pair",
    "Nothing",
};

static int recognize(PokerPlayer* app) {
    int i, j, f = 0;
    int min = 100;
    PokerPlayer_card tmp[5];
    int st = 0, fl = 0;

    /* Sort hand into sorted hand (app->shand) */

    /* make copy of hand */
    for(i = 0; i < 5; i++) tmp[i] = app->hand[i];

    for(i = 0; i < 5; i++) {
        /* put lowest card in hand into next place in app->shand */

        for(j = 0; j < 5; j++)
            if(tmp[j].index <= min) {
                min = tmp[j].index;
                f = j;
            }

        app->shand[i] = tmp[f];
        tmp[f].index = 100; /* larger than any card */
        min = 100;
    }

    /* royal and straight flushes, strait, and flush */

    fl = flush(app);
    st = straight(app);

    if(st && fl && app->shand[0].index == 9) return 0;
    if(st && fl) return 1;
    if(four(app)) return 2;
    if(full(app)) return 3;
    if(fl) return 4;
    if(st) return 5;
    if(three(app)) return 6;
    if(twopair(app)) return 7;
    if(two(app)) return 8;

    /* Nothing */

    return 9;
}

void poker_draw_callback(Canvas* canvas, void* ctx) {
    PokerPlayer* poker_player = ctx;
    furi_check(furi_mutex_acquire(poker_player->model_mutex, FuriWaitForever) == FuriStatusOk);
    canvas_clear(canvas);
    char buffer[30];
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    /* Magic Begins */

    /* Status Info */
    if(poker_player->GameState != 0 && poker_player->GameState != 4) {
        snprintf(buffer, sizeof(buffer), "%d", poker_player->score);
        canvas_draw_str_aligned(canvas, 127, 0, AlignRight, AlignTop, buffer);
    }

    /* Start of game. Cards are face down, bet can be changed */
    if(poker_player->GameState == 1) {
        snprintf(buffer, sizeof(buffer), "Bet:%d", poker_player->bet);
        canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, buffer);
        snprintf(buffer, sizeof(buffer), "<*> Place Bet");
        canvas_draw_str_aligned(canvas, 0, 9, AlignLeft, AlignTop, buffer);

        for(int i = 0; i < 5; ++i) {
            canvas_draw_icon(canvas, 5 + (i * 24), 18, &I_CardBack_22x35); /* 5, 29, 53, 77, 101 */
        }
    }
    /* Cards are turned face up. Bet is deducted and put in th pot. Show the selector hand */
    else if(poker_player->GameState == 2 || poker_player->GameState == 3) {
        snprintf(buffer, sizeof(buffer), "Pot:%d", poker_player->bet);
        canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, buffer);
        snprintf(buffer, sizeof(buffer), "<*> Select Hold");
        canvas_draw_str_aligned(canvas, 0, 9, AlignLeft, AlignTop, buffer);

        /* Normal or inverse to indicate selection - cards*/
        for(int i = 0; i < 5; ++i) {
            poker_player->held[i] ? canvas_draw_rbox(canvas, 5 + (i * 24), 18, 22, 35, 3) :
                                    canvas_draw_rframe(canvas, 5 + (i * 24), 18, 22, 35, 3);
        }

        /* Normal or inverse to indicate selection - card suit and value */

        for(int i = 0; i < 5; ++i) {
            poker_player->held[i] ? canvas_set_color(canvas, ColorWhite) :
                                    canvas_set_color(canvas, ColorBlack);

            canvas_draw_icon(canvas, 18 + (i * 24), 43, &card_suit[poker_player->hand[i].suit]);
        }

        /* Card Value. Profont_22 does not include letters (AJQK), and "10" is too big. These are bitmaps. */
        canvas_set_font(canvas, FontBigNumbers);

        for(int i = 0; i < 5; ++i) {
            poker_player->held[i] ? canvas_set_color(canvas, ColorWhite) :
                                    canvas_set_color(canvas, ColorBlack);
            if(poker_player->hand[i].index >= 1 && poker_player->hand[i].index <= 8) {
                snprintf(buffer, sizeof(buffer), "%s", poker_player->hand[i].sym);
                canvas_draw_str_aligned(canvas, 8 + (i * 24), 21, AlignLeft, AlignTop, buffer);
            } else {
                if(poker_player->hand[i].index >= 9 && poker_player->hand[i].index <= 13) {
                    canvas_draw_icon(
                        canvas, 7 + (i * 24), 21, &card_face[poker_player->hand[i].index - 9]);
                }
            }
        }

        /* Draw the Select hand */
        if(poker_player->GameState == 2) {
            canvas_set_color(canvas, ColorBlack);

            canvas_draw_icon(canvas, 11 + (poker_player->selected * 24), 54, &I_Hand_12x10);
        }
    } // GameState 2 or 3

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    if(poker_player->GameState == 3) {
        snprintf(
            buffer,
            sizeof(buffer),
            "%s:%ix",
            poker_handname[recognize(poker_player)],
            paytable[recognize(poker_player)]);
        canvas_draw_str_aligned(canvas, 63, 61, AlignCenter, AlignBottom, buffer);
    }
    if(poker_player->GameState == 0) {
        canvas_draw_icon(canvas, 0, 0, &I_Splash_128x64); /* Initial launch */
    }
    if(poker_player->GameState == 4) {
        /* canvas_draw_icon(canvas, 0, 0, &I_BadEnd_128x64);  Just Lost The Game - disabled for now :( */
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, sizeof(buffer), "%s", "You have run out of money!");
    canvas_draw_str_aligned(canvas, 63, 22, AlignCenter, AlignCenter, buffer);
        snprintf(buffer, sizeof(buffer), "%s", "At one point, you had");
    canvas_draw_str_aligned(canvas, 63, 32, AlignCenter, AlignCenter, buffer);
    snprintf(buffer, sizeof(buffer), "%d dollars", poker_player->highscore);
    canvas_draw_str_aligned(canvas, 63, 42, AlignCenter, AlignCenter, buffer);
    }
    
    

    furi_mutex_release(poker_player->model_mutex);
}

void poker_input_callback(InputEvent* input, void* ctx) {
    PokerPlayer* poker_player = ctx;
    furi_message_queue_put(poker_player->event_queue, input, FuriWaitForever);
}

PokerPlayer* poker_player_alloc() {
    PokerPlayer* poker_player = malloc(sizeof(PokerPlayer));

    poker_player->score = 1000;
    poker_player->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    poker_player->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    poker_player->view_port = view_port_alloc();
    poker_player->selected = 0;
    poker_player->GameState = 0;
    poker_player->bet = 10;
    poker_player->minbet = 10;
    poker_player->highscore = 1000;

    playcard(
        poker_player); /* Get things rolling before the player gets into the game. This will preload the hand. */
    view_port_draw_callback_set(poker_player->view_port, poker_draw_callback, poker_player);

    view_port_input_callback_set(poker_player->view_port, poker_input_callback, poker_player);

    poker_player->gui = furi_record_open("gui");
    gui_add_view_port(poker_player->gui, poker_player->view_port, GuiLayerFullscreen);

    return poker_player;
}

void poker_player_free(PokerPlayer* poker_player) {
    view_port_enabled_set(poker_player->view_port, false);
    gui_remove_view_port(poker_player->gui, poker_player->view_port);
    furi_record_close("gui");
    view_port_free(poker_player->view_port);
    furi_message_queue_free(poker_player->event_queue);
    furi_mutex_free(poker_player->model_mutex);

    free(poker_player);
}

int32_t video_poker_app(void* p) {
    UNUSED(p);
    PokerPlayer* poker_player = poker_player_alloc();

    InputEvent event;
    for(bool processing = true; processing;) {
        FuriStatus status = furi_message_queue_get(poker_player->event_queue, &event, 100);
        furi_check(furi_mutex_acquire(poker_player->model_mutex, FuriWaitForever) == FuriStatusOk);
        if(status == FuriStatusOk) {
            if(event.type == InputTypePress) {
                switch(event.key) {
                case InputKeyUp:
                    Shake();
                    break;
                case InputKeyDown:
                    if(poker_player->GameState == 2) {
                        playcard(poker_player);
                        if(check_for_dupes(poker_player) == 0) {
                            playcard(poker_player);
                        }

                        poker_player->GameState = 3;
                    }
                    break;
                case InputKeyLeft:
                    if(poker_player->GameState == 1) {
                        if(poker_player->bet >= poker_player->minbet + 10) {
                            poker_player->bet -= 10;
                        }
                    } else if(poker_player->selected > 0 && poker_player->GameState == 2) {
                        poker_player->selected--;
                    } // Move hand left/right
                    else if(poker_player->selected == 0 && poker_player->GameState == 2) {
                        poker_player->selected = 4; //wraparound
                    }
                    break;
                case InputKeyRight:
                    if(poker_player->GameState == 1) {
                        if(poker_player->bet < poker_player->score + 10) {
                            poker_player->bet += 10;
                        }
                    }
                    if(poker_player->selected < 4 && poker_player->GameState == 2) {
                        poker_player->selected++;
                    } // Move hand left/right
                    else if(poker_player->selected == 4 && poker_player->GameState == 2) {
                        poker_player->selected = 0; //wraparound
                    }
                    break;
                case InputKeyOk:
                /* close splash screen */
                    if(poker_player->GameState == 0) {
                        poker_player->GameState = 1;
                    } else if(poker_player->GameState == 1) {
                        /* Pledge bet. Bet is subtracted here. Original code subtracts it during playcard
                        but playcard is called multiple times which would otherwise subtract bet
                        multiple times */
                        poker_player->score -= poker_player->bet; 
                        poker_player->GameState = 2;
                    } else if(poker_player->GameState == 2) {
                        /* Select or un-select card to be held */
                        poker_player->held[poker_player->selected] =
                            !poker_player
                                 ->held[poker_player->selected]; //cursed and bad pls replace
                    } else if(poker_player->GameState == 3) {
                        /* accept your fate */
                        if(recognize(poker_player) != 9) {
                            poker_player->score +=
                                poker_player->bet * paytable[recognize(poker_player)];
                        }
                        poker_player->GameState = 1;
                        if(poker_player->bet > poker_player->score) {
                            poker_player->bet = poker_player->score;
                        }
                        poker_player->held[0] = 0;
                        poker_player->held[1] = 0;
                        poker_player->held[2] = 0;
                        poker_player->held[3] = 0;
                        poker_player->held[4] = 0;
                        if(poker_player->score <= 0) {
                            /* lost the game */
                            poker_player->GameState = 4;
                        }
                        playcard(poker_player); // shuffle shuffle
                    } else if(poker_player->GameState == 4) {
                        /* escape the summary, return to splash */
                        Shake();
                        poker_player->selected = 0;
                        poker_player->GameState = 0;
                        poker_player->bet = 10;
                        poker_player->minbet = 10;
                        poker_player->highscore = 1000;
                        poker_player->score = 1000;
                        poker_player->GameState = 0;
                    }
                    break;
                case InputKeyBack:
                    /* if game is not over, we should store the game state. */
                    processing = false;
                    break;
                }
            }
        }
        furi_mutex_release(poker_player->model_mutex);
        view_port_update(poker_player->view_port);
    }

    poker_player_free(poker_player);
    return 0;
}
