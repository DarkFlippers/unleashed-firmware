#include <lib/drivers/cc1101_regs.h>

#define NUM_CHANNELS 132

/*
 * wide mode (default): 44 MHz on screen, 333 kHz per channel
 * narrow mode: 6.6 MHz on screen, 50 kHz per channel
 */
#define WIDE 0
#define NARROW 1
#define ULTRAWIDE 2

/* vertical scrolling */
#define VERTICAL_SHORT_STEP  16
#define VERTICAL_TALL_STEP   4

#define MAX_VSCROLL 120
#define MIN_VSCROLL 0
#define DEFAULT_VSCROLL 48

/* frequencies in MHz */
#define DEFAULT_FREQ     440
#define WIDE_STEP        5
#define NARROW_STEP      1
#define ULTRAWIDE_STEP   20
#define WIDE_MARGIN      13
#define NARROW_MARGIN    3
#define ULTRAWIDE_MARGIN 42

/* frequency bands supported by device */
#define BAND_300 0
#define BAND_400 1
#define BAND_900 2

/* band limits in MHz */
#define MIN_300  281
#define CEN_300	 315
#define MAX_300  361
#define MIN_400  378
#define CEN_400	 435
#define MAX_400  481
#define MIN_900  749
#define CEN_900	 855
#define MAX_900  962

/* band transition points in MHz */
#define EDGE_400 369
#define EDGE_900 615

/* VCO transition points in Hz */
#define MID_300  318000000
#define MID_400  424000000
#define MID_900  848000000

/* channel spacing in Hz */
#define WIDE_SPACING      199952
#define NARROW_SPACING    49988
#define ULTRAWIDE_SPACING 666504

/* display peaks long enough to be seen (don't set higher than 20) */
// #define PERSIST 8
#define PERSIST 1

/* power button debouncing for wake from sleep */
#define DEBOUNCE_COUNT  4
#define DEBOUNCE_PERIOD 50

#define UPPER(a, b, c)  ((((a) - (b) + ((c) / 2)) / (c)) * (c))
#define LOWER(a, b, c)  ((((a) + (b)) / (c)) * (c))

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} SpectrumAnalyzerEvent;

typedef struct {
    /* frequency setting */
    uint32_t frequency;

    /* signal strength */
    uint8_t ss;
} ChannelInfo;

#define NUM_CHANNELS 132
typedef struct {
    float		max_rssi;
    uint8_t		max_rssi_dec;
    uint8_t		max_rssi_channel;
    ChannelInfo chan_table[NUM_CHANNELS];
} SpectrumAnalyzerModel; 

bool update_values_flag;
uint16_t user_freq;
uint8_t band;
uint8_t width;
char max_hold;
char height;
uint8_t vscroll;
uint8_t min_chan;
uint8_t max_chan;

typedef struct {
    ValueMutex* model_mutex;
    osMessageQueueId_t event_queue;
} SpectrumAnalyzerContext;

static const uint8_t radio_config[][2] = {
    {CC1101_FSCTRL1,0x12},
    {CC1101_FSCTRL0,0x00},

    {CC1101_AGCCTRL2, 0xC0},
    
    {CC1101_MDMCFG4, 0x6C},
    {CC1101_TEST2, 0x88},
    {CC1101_TEST1, 0x31},
    {CC1101_TEST0, 0x09},
    /* End  */
    {0, 0},
};



