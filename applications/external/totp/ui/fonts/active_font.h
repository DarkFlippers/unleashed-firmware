#pragma once

#include "../../features_config.h"
#include "font_info.h"

#if TOTP_FONT == TOTP_FONT_MODENINE
#include "mode_nine/mode_nine.h"
#define TOTP_CODE_FONT_INFO modeNine_15ptFontInfo
#elif TOTP_FONT == TOTP_FONT_REDHATMONO
#include "redhat_mono/redhat_mono.h"
#define TOTP_CODE_FONT_INFO redHatMono_16ptFontInfo
#elif TOTP_FONT == TOTP_FONT_BEDSTEAD
#include "bedstead/bedstead.h"
#define TOTP_CODE_FONT_INFO bedstead_17ptFontInfo
#elif TOTP_FONT == TOTP_FONT_ZECTOR
#include "zector/zector.h"
#define TOTP_CODE_FONT_INFO zector_18ptFontInfo
#elif TOTP_FONT == TOTP_FONT_712SERIF
#include "712serif/712serif.h"
#define TOTP_CODE_FONT_INFO _712Serif_24ptFontInfo
#elif TOTP_FONT == TOTP_FONT_GRAPH35PIX
#include "graph35pix/graph35pix.h"
#define TOTP_CODE_FONT_INFO graph35pix_12ptFontInfo
#elif TOTP_FONT == TOTP_FONT_KARMAFUTURE
#include "karma_future/karma_future.h"
#define TOTP_CODE_FONT_INFO karmaFuture_14ptFontInfo
#endif