#define NUM_CHANNELS 132
#define NUM_CHUNKS 6
#define CHUNK_SIZE (NUM_CHANNELS / NUM_CHUNKS)

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
 * ultranarrow mode: 2 MHz on screen, 19 kHz per channel
 * precise mode: 400 KHz on screen, 3.92 kHz per channel
 */
#define WIDE 0
#define NARROW 1
#define ULTRAWIDE 2
#define ULTRANARROW 3
#define PRECISE 4

/* channel spacing in Hz */
#define WIDE_SPACING 196078
#define NARROW_SPACING 39215
#define ULTRAWIDE_SPACING 784313
#define ULTRANARROW_SPACING 19607
#define PRECISE_SPACING 3921

/* vertical scrolling */
#define VERTICAL_SHORT_STEP 16
#define MAX_VSCROLL 120
#define MIN_VSCROLL 0
#define DEFAULT_VSCROLL 48

/* frequencies in KHz */
#define DEFAULT_FREQ 440000
#define WIDE_STEP 5000
#define NARROW_STEP 1000
#define ULTRAWIDE_STEP 20000
#define ULTRANARROW_STEP 500
#define PRECISE_STEP 100

/* margin in KHz */
#define WIDE_MARGIN 13000
#define NARROW_MARGIN 3000
#define ULTRAWIDE_MARGIN 42000
#define ULTRANARROW_MARGIN 1000
#define PRECISE_MARGIN 200

/* frequency bands supported by device */
#define BAND_300 0
#define BAND_400 1
#define BAND_900 2

/* band limits in KHz */
#define MIN_300 281000
#define CEN_300 315000
#define MAX_300 361000
#define MIN_400 378000
#define CEN_400 435000
#define MAX_400 481000
#define MIN_900 749000
#define CEN_900 855000
#define MAX_900 962000

/* band transition points in KHz */
#define EDGE_400 369000
#define EDGE_900 615000

/* VCO transition points in Hz */
#define MID_300 318000000
#define MID_400 424000000
#define MID_900 848000000

#define UPPER(a, b, c) ((((a) - (b) + ((c) / 2)) / (c)) * (c))
#define LOWER(a, b, c) ((((a) + (b)) / (c)) * (c))