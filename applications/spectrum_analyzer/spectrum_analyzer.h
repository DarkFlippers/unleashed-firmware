#define NUM_CHANNELS 132

// Screen coordinates
#define FREQ_BOTTOM_Y 50
#define FREQ_START_X 14
// How many channels displayed on the scale (On screen still 218)
#define FREQ_LENGTH_X 102
// dBm threshold to show peak value
#define PEAK_THRESHOLD -85

/*
 * ultrawide mode: 80 MHz on screen, 784 kHz per channel
 * wide mode (default): 20 MHz on screen, 196 kHz per channel
 * narrow mode: 4 MHz on screen, 39 kHz per channel
 */
#define WIDE 0
#define NARROW 1
#define ULTRAWIDE 2

/* channel spacing in Hz */
#define WIDE_SPACING 196078
#define NARROW_SPACING 39215
#define ULTRAWIDE_SPACING 784313

/* vertical scrolling */
#define VERTICAL_SHORT_STEP 16
#define MAX_VSCROLL 120
#define MIN_VSCROLL 0
#define DEFAULT_VSCROLL 48

/* frequencies in MHz */
#define DEFAULT_FREQ 440
#define WIDE_STEP 5
#define NARROW_STEP 1
#define ULTRAWIDE_STEP 20
#define WIDE_MARGIN 13
#define NARROW_MARGIN 3
#define ULTRAWIDE_MARGIN 42

/* frequency bands supported by device */
#define BAND_300 0
#define BAND_400 1
#define BAND_900 2

/* band limits in MHz */
#define MIN_300 281
#define CEN_300 315
#define MAX_300 361
#define MIN_400 378
#define CEN_400 435
#define MAX_400 481
#define MIN_900 749
#define CEN_900 855
#define MAX_900 962

/* band transition points in MHz */
#define EDGE_400 369
#define EDGE_900 615

/* VCO transition points in Hz */
#define MID_300 318000000
#define MID_400 424000000
#define MID_900 848000000

#define UPPER(a, b, c) ((((a) - (b) + ((c) / 2)) / (c)) * (c))
#define LOWER(a, b, c) ((((a) + (b)) / (c)) * (c))
