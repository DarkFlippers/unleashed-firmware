#include "available_fonts.h"
#include "712serif/712serif.h"
#include "bedstead/bedstead.h"
#include "dpcomic/dpcomic.h"
#include "funclimbing/funclimbing.h"
#include "graph35pix/graph35pix.h"
#include "karma_future/karma_future.h"
#include "mode_nine/mode_nine.h"
#include "pixelflag/pixelflag.h"
#include "redhat_mono/redhat_mono.h"
#include "zector/zector.h"

const FONT_INFO* const available_fonts[AVAILABLE_FONTS_COUNT] = {
    &modeNine_15ptFontInfo,
    &_712Serif_24ptFontInfo,
    &bedstead_17ptFontInfo,
    &dPComic_18ptFontInfo,
    &funclimbingDemo_18ptFontInfo,
    &graph35pix_12ptFontInfo,
    &karmaFuture_14ptFontInfo,
    &pixelFlag_18ptFontInfo,
    &redHatMono_16ptFontInfo,
    &zector_18ptFontInfo};
