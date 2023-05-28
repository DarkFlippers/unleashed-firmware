#include <furi.h>
#include <furi_hal_console.h>
#include <furi_hal_gpio.h>
#include <furi_hal_power.h>
#include <furi_hal_uart.h>
#include <gui/canvas_i.h>
#include <gui/gui.h>
#include <input/input.h>
#include <math.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stdlib.h>

#include "FlipperZeroWiFiModuleDefines.h"

#define WIFI_APP_DEBUG 0

#if WIFI_APP_DEBUG
#define APP_NAME_TAG "WiFi_Scanner"
#define WIFI_APP_LOG_I(format, ...) FURI_LOG_I(APP_NAME_TAG, format, ##__VA_ARGS__)
#define WIFI_APP_LOG_D(format, ...) FURI_LOG_D(APP_NAME_TAG, format, ##__VA_ARGS__)
#define WIFI_APP_LOG_E(format, ...) FURI_LOG_E(APP_NAME_TAG, format, ##__VA_ARGS__)
#else
#define WIFI_APP_LOG_I(format, ...)
#define WIFI_APP_LOG_D(format, ...)
#define WIFI_APP_LOG_E(format, ...)
#endif // WIFI_APP_DEBUG

#define DISABLE_CONSOLE !WIFI_APP_DEBUG

#define ENABLE_MODULE_POWER 1
#define ENABLE_MODULE_DETECTION 1

#define ANIMATION_TIME 350

typedef enum EChunkArrayData {
    EChunkArrayData_Context = 0,
    EChunkArrayData_SSID,
    EChunkArrayData_EncryptionType,
    EChunkArrayData_RSSI,
    EChunkArrayData_BSSID,
    EChunkArrayData_Channel,
    EChunkArrayData_IsHidden,
    EChunkArrayData_CurrentAPIndex,
    EChunkArrayData_TotalAps,
    EChunkArrayData_ENUM_MAX
} EChunkArrayData;

typedef enum EEventType // app internally defined event types
{ EventTypeKey // flipper input.h type
} EEventType;

typedef struct SPluginEvent {
    EEventType m_type;
    InputEvent m_input;
} SPluginEvent;

typedef struct EAccessPointDesc {
    FuriString* m_accessPointName;
    int16_t m_rssi;
    FuriString* m_secType;
    FuriString* m_bssid;
    unsigned short m_channel;
    bool m_isHidden;
} EAccessPointDesc;

typedef enum EAppContext {
    Undefined,
    WaitingForModule,
    Initializing,
    ScanMode,
    MonitorMode,
    ScanAnimation,
    MonitorAnimation
} EAppContext;

typedef enum EWorkerEventFlags {
    WorkerEventReserved = (1 << 0), // Reserved for StreamBuffer internal event
    WorkerEventStop = (1 << 1),
    WorkerEventRx = (1 << 2),
} EWorkerEventFlags;

typedef struct SWiFiScannerApp {
    FuriMutex* mutex;
    Gui* m_gui;
    FuriThread* m_worker_thread;
    NotificationApp* m_notification;
    FuriStreamBuffer* m_rx_stream;

    bool m_wifiModuleInitialized;
    bool m_wifiModuleAttached;

    EAppContext m_context;

    EAccessPointDesc m_currentAccesspointDescription;

    unsigned short m_totalAccessPoints;
    unsigned short m_currentIndexAccessPoint;

    uint32_t m_prevAnimationTime;
    uint32_t m_animationTime;
    uint8_t m_animtaionCounter;
} SWiFiScannerApp;

/*
  Fontname: -FreeType-Inconsolata LGC-Bold-R-Normal--36-360-72-72-P-176-ISO10646-1
  Copyright: Original Roman version created by Raph Levien using his own tools and FontForge. Copyright 2006 Raph Levien. Hellenisation of the Roman font, by Dimosthenis Kaponis, using FontForge. Hellenic glyphs Copyright 2010-2012 Dimosthenis Kaponis. Released under the SIL Open Font License, http://scripts.sil.org/OFL.    Cyrillic glyphs added by MihailJP, using FontForge. Cyrillic glyphs Copyright 2012 MihailJP. Released under the SIL Open Font License, http://scripts.sil.org/OFL.    Emboldened by MihailJP.    Some glyphs modified by Greg Omelaenko, using FontForge.
  Glyphs: 95/658
  BBX Build Mode: 2
*/
const uint8_t u8g2_font_inb27_mr[4414] =
    "_\2\5\4\5\6\1\4\6\26%\0\370\33\371\35\371\5e\13\313\21! \13\266\14\366\207\377\377\377"
    "\20\0!\30\266\14\366\7`\310\22\353E\42\351\177r|(\220h\240-\222\17\37\42%\266\14v\37"
    "d,\62\310\42\203,\62\310\42\203,\62\310\42\203,\62\310\42\203,\62\310\42\203|\370\377\307\1#"
    "M\266\14\366\7u\220\261\310 \213\14\262\310 \213\14\262\310 k\20b\36 \343\1\62\36 \3;"
    "d\220E\6Yd\220\65\10Y\203\220\365\306\3c<@\306\3\344\220A\26\31d\221A\326 d\15"
    "\62\330 \203\15\62\26!\343\303\63\0$A\266\14\366\7\200H\352\251\325\320;$\220P\14\21d\210"
    "C\4\31\1\21AX\11\244e.\271\325\64F\202YD\224E\6Yd\20\23\10\31\244\214AD!"
    "&\220P\314Cm\251G$\365\341\42\0%I\266\14\366\7\261\34RN!%\21B\226 \205\14\42"
    "H!\203\4b\310H\346!\227N \253\204\22I,\221\222d\10F\302Q\15\221\220N\32\304\220@"
    "\6)%\220A\12\21d\20R\304\42\204$R\312)\344\224\17/\3&F\266\14\366\7\264\274\304\326"
    "b\251\214\222H!\211\24\222H!\212\14\262\310 \214\4\322\220;\16\65E\302!\202\220A\250A\221"
    "\22\310 &\15r\16!\250\224bL)\5\225\7Fa\241\224\64\310\61F|x\30\0'\17\266\14"
    "\366\7aH\372\357\303\377\177\1\0((\266\14v\14sD\363\216\63\317@\22\13,\221\304\22\211$"
    "\261D\42\351\223ER\262\304\42K,\262\212v\261L\241\1)$\266\14\266\26\263D\233\254\305\42K"
    ",\222\310\42\351_$\222\304\22I,\260\304\2\313\263\301\22\307\7\7\0*/\266\14\366\207\207\210\244"
    "\261PH\11\205\14\62F\61a\10C\36\30\306\261\2\317C\215\10\262\310(\251\20\202\212!h\234\261"
    "\2\12\37\376,\0+\33\266\14\366\207\207\210\244\267\36\30\344\201A\36\30\344\201\261\210\244\337\207\277\14"
    "\0,\30\266\14\366\207\377\377\300\220%\32hI\42\307\34rH\42\7\15\15\0-\26\266\14\366\207\177"
    "\352\201`\36\10\346\201`\36\10\37\376\277\4\0.\21\266\14\366\207\377\377\0\211\6\332\42\371p\22\0"
    "/.\266\14\366\33\223\304\22I,\221H\22\211$\221H\22\211$\221H\22K$\261D\22K$\261"
    "D\42I$\222D\42\211\34\65|x\17\0\60I\266\14\366\7\266<\324\324b\251\220\202\210!\247\230"
    "b\310)\206\30SJA\204\24\22\10!e\10B\10\31\203\20\62(B\306 \224\30\205\20\22\206!"
    "\4\31R\214)\245\34b\312!\247\224r\12)\211-\325\20,\37^\7\61\26\266\14\366\7\226@\363"
    "NKL\265!\210\244\377\377\367\341y\0\62'\266\14\366\7\325\70\265Xr\247\24s\306)($\42"
    "i\261D\22\13\254\277H\311g\36\10\346\201`\36\10\37\36\6\63-\266\14\366\7\324\70\265Xri"
    "\230\222\2\42\222\26\313\63,\265\363RS\261\310\42i) \202\304)\207\224r\36jj\265\363\341y"
    "\0\64\65\266\14\366\7\232H\22\13\264\336q\310\215@\32\11\204\21A\330\30d\221A\24!$\221B"
    "\22)\4\21C\316\3\203<\60\310\3\203<\60\36\221\364}x\32\0\65*\266\14\366\7\361\235\337!"
    "\222N\20\306TK\16\25b\316\70%\5T$\275#P)\4\21cJ\61\17\265\245\234\371\360:\0"
    "\66>\266\14\366\7\327\70\265Xj\250\224\221\310\11\211H\22K$\243(\22\22z\347\201`L)\246"
    "\234R\10*\205$RH\42\205$R\12\42\206\234b\212!\250\220\202\232b+=\363\341q\0\67-"
    "\266\14\366\7\360\201`\36\10\346\201`\236$\261D\22K$\222\304\22I,\221H\22K$\222\304\22"
    "I\254\42\211U$\261|\370\4\0\70;\266\14\366\7\325\70\265Xj\250\224r\212!\207\356\24C\20"
    ")$\225Q\324b\251)U\204I\244\224C\31\202J!\211\24\222H!\211\224r\212)\245\234\207\232"
    "Z\355|x\35\0\71\71\266\14\366\7\265\274\304\326b\251\220\202\210!\247\230b\10\42\206 b\10\42"
    "\206 b\312\241L\71\205\30\344\251\23\10\33\203H\42\207$*\34\222D)\251%\266\322+\37>:"
    "\27\266\14\366\207\77A\242\201\266H>|\210D\3m\221|\70\11\0;\34\266\14\366\207\77A\242\201"
    "\266H>|\210D\3\255X$\221C\222HM\361\1\1< \266\14\366\207\271\60G,\357\264\304\360"
    "\265\363J<\20\301\23/\210\301\3\215$T|\370\25\0= \266\14\366\207\317=\60\310\3\203<\60"
    "\310\3\343\303\352\3\203<\60\310\3\203<\60>\374g\1>\36\266\14\366\207\266\240D\32\210\37<\261"
    "\274\323\20\303\332\325P;\257\304\61\305\207\277\6\0\77%\266\14\266\67N\251\206\236\61\245\34r\312\21"
    "\211H*\226Hb\201u\221H\352C\226D\3m\221|x\36\0@G\266\14\366\7\266\274\264\32r"
    "\247\230b\10\42\205(B\306Ad\24\65\6ac\20\66\306(\204\214\61H!c\14R\310\30\203\24"
    "\62\306\250\306\30m\220\301\310 \213\20R\2!d\222YNH.\265\245\234\371\360\70\0A<\266\14"
    "\366\7\67PA\307\34\262\304\2\15<\357\70\22H#\201\264!\10#\203,\62\310\32e(RHj"
    "\311\235\357\220S\12I\244\220T\6Yd\220E\6Y%\220F><\13\0BD\266\14\366\7\217)"
    "\207\36\10\346\201`\310\61\205\240RH\42\205$RH\42\205\240R\310)\346\201`\336\371@\60\4\225"
    "BR!D\21B\24!D\21B\24!$\25B\220!\17\210\362@\60\357\264\17\217\3C/\266\14"
    "\366\7\327\264\245\32z\306\24S\312)\244$B\210\22\206\304\22\211\244'K,\222\310\22\213\12\247\240"
    "a\214)\346\241\246V\63\37\36\7DC\266\14\366\7o\255\226\336y\207\30c\10*\205\240RH\42"
    "\205\244B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\204\244BH\42\205\240"
    "R\10\42\206\30c\336q\250\245\365\341y\0E'\266\14\366\7\357\1Q\36\20\345\1Q\36\20\205H"
    "\372\344;\277C$\375\344\3\242< \312\3\242< ><\14\0F\35\266\14\366\7\360\201`\36\10"
    "\346\201`\36\10\206H\372\244C>D$\375\357\303W\0G:\266\14\366\7\326\70\245\32z\306\24S"
    "\312)\244$a\210$\261D\42i&\15b\322 &\15b\322(\212\20\242\10!\212\220\222H)\210"
    "\24c\212y \34\227\30;\37\36\7HM\266\14\366\7\217$RH\42\205$RH\42\205$RH"
    "\42\205$RH\42\205$RH\42\205$R\36\20\345\1Q\36\20\345\1QH\42\205$RH\42\205"
    "$RH\42\205$RH\42\205$RH\42\205$RH\42\205$\362\341a\0I\24\266\14\366\7\320"
    "!_#\222\376\377k\357\374>\274\14\0J\33\266\14\366\7\264\245\236#\222\376\377\222\60%\215BR"
    "S\213\245W>|\1\0KI\266\14\366\7\257\244\62H*\204\240R\310)\206\230rH)\210\24\222"
    "\10!\212\14\262\210(\213\204\302RKM\261\22\312\42\203,\62\212\42\244$RH\42\245 b\312!"
    "\247\30r\212!\250\24\222\12!\251\20\242\312\207g\1L\32\266\14\366\7\260D\42\351\377\377\311\7D"
    "y@\224\7Dy@|x\27\0MU\266\14\366\7\217,\62\310\42\243\244\62J*\303\34\63\314\61"
    "\343\224\63N\71\3\33$\214\61\2\31$\320\6\21#\14A\6\21#\14A\6\31eP\243\14\32!"
    "\203\220A\310 \213\14\262\310 \213\14\262\310 \213\14\262\310 \213\14\262\310 \213\14\262\310\207g\1"
    "NS\266\14\366\7\217(BJ\42\244$B\14\42\304 B\316!\344\34B\220!\204\4R\10!\201"
    "\24B\210 \204\22\204P\203\42dP\204\22\204P\202\20RH \204\230\21\10!\6\21r\16!\347"
    "\20\202\14!\310\20\222\12!\251\20\242\10!\212|x\27\0OC\266\14\366\7\325\70\265Xr\247\224"
    "r\310)\245 RH*\204(\62\212\42\203,\62\310\42\203,\62\310\42\203,\62\310\42\203,\62\212"
    "\42\204(BH*\244 b\310)\246\224\202\134bK\71\363\341u\0P\60\266\14\366\7\257\245w\36"
    "\10\346\1Q\10*\205\244B\210\42\204(B\210\42\204(BH*\204\240R\36\20\345\201`\34b\212"
    "H\372\357\303g\0QJ\266\14\366\7\325\70\265Xr\247\224r\310)\245 RH*\204\244\62\212\42"
    "\203,\62\310\42\203,\62\310\42\203,\62\310\42\203,\62\212\42\204\244BH*\244 b\310)\246\224"
    "\202\134b+\271\23\211$R\265\344\20\64\37@\0RC\266\14\366\7\217)\207\336y \30rL!"
    "\250\24\222H!\211\24\222H!\211\24\202J!\247\230\7\202y\307!\246H!\211\24\222H)\210\30"
    "\202\210)\207v\212!\210\30\202J!\211\24\222\312\207w\1S\61\266\14\366\7\365\264\245\32z\207\230"
    "\12\211C\222\70DV\321D\364\262\207\242\221ER&,b\302\42e\244B\214)\345\1a^b\354"
    "|x\35\0T\27\266\14\366\7\356\1\62\36 \343\1\62\36 \212H\372\377\377>|UH\266\14\366"
    "\7\257$B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\204"
    "(B\210\42\204(B\210\42\204(B\210\42\204(B\210\42\244\240B\12*\245\230b\36rJ\71\363"
    "\341u\0V>\266\14\366\7\216\60\42\10#\203(B\210\42\204(R\10\42\206 b\10\42\207\30\202"
    "\210!\210\30\222H\31\212\20\242\10!\213\214\301\210 \214\10\322H\30\16{\6Z\221H\42\7\25\37"
    ">W\134\266\14\366\7n\22\303\15A\212(D\220\62\10\21\244\14B\4!\204\214A\10!\23)c"
    "\20\62\312\30\204\10\63\6!\302\10B\210\70\201\20\42N\30f\204!F\30\346\210\21\206\71\342\230#"
    "\216\61\304\30C\12*\244\240B\12\42\206 b\10\42\206\240q\206\22H(\201\304\207\247\1XB\266"
    "\14\366\7\217(BH\42\206 b\212!\210\224\202\12!\212\214\242\212 \214\204\302\222;\320\300\22K"
    "\64\357<\324\24#\202,\62\212\42\204$R\12\42\206\34rJ)\210\24\222\312(\252|x\26\0Y"
    "\60\266\14\366\7\216\254\42\212\42\204\244B\12\42\206 b\212!\210\30\202\12!\212\20\242\212 \214\10"
    "\322H\30\16\275\363L,\221H\372\357\303\363\0Z\63\266\14\366\7\360\1Q\36\20\345\1Q\36\10\222"
    "D\22K\244b\211$\226H\305\22I,\260D\22\13,\261\300\22\37\20\344\201A\36\30\344\201\361\341"
    "]\0[\23\266\14\366n\251\227\210\244\377\377\77\331R\357C\10\0\134+\266\14\66\21\262H\42\213$"
    "\262H\42\213$\262Hj\22I&\221d\22Id\221D\26Id\221D\26IM\42\311\14\37\36\1"
    "]\20\266\14vn\251'\351\377\377_\352\373\220\2^!\266\14\366\7WL\42I\64\357<\324\210 "
    "\213\14\262\10!\211\24\222\210!I\34\361\341\377\17\2_\26\266\14\366\207\377\377\247\36 \343\1\62\36"
    " \343\1\362!\3\0`\33\266\14\266\11UPA\307$\222\310\22\213,\261\310\42\251\71Q\361\341\377"
    "g\0a\60\266\14\366\207\217\236\266\222C.\15S$\325Rj\347\231\7\202)\207\224\202H!\211\24"
    "\202J)\345\230\7\202y \34\25H\62\203|x\30\0b;\266\14\66.\221H\372\215\242HH\350"
    "\235\7\202\61\305\224rJ!\251\20\222\12!\212\20\242\10!\212\20\242\10!\212\20\222\12)\210\24c"
    "Jy \230w\206Hh\220\362\341u\0c(\266\14\366\207\257\32\247\224;\17\4S\225\202\206!I"
    "\230\22\211\244\311\42\211,H\240b\310y \34\247\224\63\37\36\7d;\266\14\366\7\207H\372\251\62"
    "\10J\201\234g\36\10\245\30S\10*\244\240BH*\204(B\210\42\204(B\210\42\244\240B\12*"
    "\245\234R,\363@\70\17\245@T\31\344\303\273\0e,\266\14\366\207\217\236\266TC\317T\206 b"
    "H\42\344\201A\36\30\344\201A\36\30\204H\42\253\24\216\61\344<\344\322r\346\303\343\0f\35\266\14"
    "\366\7\302\70\265Xj\250\230\201\312\11\211H\312\71\344cD\322\377\367\341\23\0g=\266\14\366\207o"
    "\26R\314\3\241< \10!j\224R\16}\247\224r\314(\251\251\265\324\42a\64\42[z \234\7"
    "\202y@\20\222\312 \213\14\262\310(\251\214\7Fy \34\267\220\7h<\266\14v.\221H\372\215"
    "\242\210@\210\4u\336\71\244\30c\210)\207\30\202\210!\210\30\202\210!\210\30\202\210!\210\30\202\210"
    "!\210\30\202\210!\210\30\202\210!\210\30\202\310\207\207\1i\30\266\14\366\7`\310\22+\71>$S"
    "\313I\372\277\326R\357\303\323\0j\37\266\14\366\7d\310\22+\71>\364\230\342I\372\377\227\204)\210"
    "\220\202\134bK\71\363A\0k\71\266\14v.\221H\372\241R\310)\206\230rH)\210\220\222\310("
    "\212\210\262H(Lck\25Q\24!%\221R\20)\5\21S\16\71\305\20T\12A\206\220d>\274"
    "\12\0l\21\266\14vVL'\351\377\377\330;\277\17/\3m@\266\14\366\207\257\221PF!+\234"
    "\361\0\31\17\24QF\21\325 \204\10B(A\10%\10\241\4!\224 \204\22\204P\202\20J\20B"
    "\11B(A\10%\10\241\4!\224 \204\22\204P\37\236\5n:\266\14\366\207\357\221Q\24\21\10\275"
    "\363\316!\305\224CL\71\304\20D\14A\304\20D\14A\304\20D\14A\304\20D\14A\304\20D\14"
    "A\304\20D\14A\304\20D><\14\0o\62\266\14\366\207\257\32\247TC\357\224bJ\71\245\220T"
    "FIe\220E\6Yd\220E\6Yd\24E\10I\244\224SL)\306<\324\324r\346\303\343\0p"
    "=\266\14\366\207\317\221Q\24\11\11=\20\314\3\301\230bJ\71\245\220T\10I\205\20E\10Q\204\20"
    "E\10Q\204\20E\10I\205\24D\212\61\245<\20\314;$$DFQD\322\367\301\1q\71\266\14"
    "\366\207\217\32AP\12\344<\363@(\305\230BP!\5\25B\24!D\21B\24!D\21B\24!"
    "%\221BP)\345\224b\231\7\302y(\5\242\312 \222~\30\0r\37\266\14\366\207\17\22R\22\21"
    "\351<\20\314\3\301\34#\216\71\342\224X\42\221\364\277\17\37\1s,\266\14\366\207\217\232\306\222C\317"
    "\24C\16A\343\24\24R\211\351)\227\340\221\305\10E\214P\244\224S\312\3\341\270\304\332\371\360:\0"
    "t\35\266\14\366\207\234\220\364\332;\277E$\375\223\305\4U\212XL\261\245\232\371\360\66\0u;\266"
    "\14\366\207\317\221D\12I\244\220D\12I\244\220D\12I\244\220D\12I\244\220D\12I\244\220D\12"
    "I\244\220DJ\71\245\224SL)\306<\20\316C)\20U\6\371\360\60\0v\63\266\14\366\207\257\25"
    "E\10Q\204\20E\12I\303\20DL\71\3\21C\20\61$\221\62\24!D\21\62\30\21\204\21A\32"
    "\11\303!w\240\201E\22\71>|wH\266\14\366\207\217Q\202\60\42H\31\204\214Q\6!c\224A"
    "\310 \243\14j\224A\215\62F\31\303\210Q\206\30a\210QH\230\304($Lb\24\22\204@\306\14"
    "s\314\60\307\14sJ)\247\224\202H!\211\24\362\341i\0x\63\266\14\366\207\317\221DJ\71\305\24"
    "C\20)D\221QT\21\204%\207\334\201%\232w\36j$\224U\4Q\205\220DJ\71\305\224R\20"
    ")$\225\17\357\2y<\266\14\366\207\317\225T\10I\244\220DJA\304\220CC\304\20D\12I\205"
    "\20E\310Xd\20F\4aD\14\67\2q\347\35h`\221T$,\24\262\4!L-\325\320+\37"
    "\24\0z&\266\14\366\207\357=\20\314\3\301<\20\314\213$\26X\213$\26X\27\13|@\220\7\6"
    "y`\220\7\306\207w\1{$\266\14\366\7\342\264\304\324Z\253\300\22\211\244_$\357<\3\15<\262"
    "H\372'K,r\255\305\224C\32\0|\16\266\14\366\7\200H\372\377\377\377\357\3}#\266\14\266\66"
    "\20\271\324\224,\222\376d\221d\36h\336q\310\221H$\375\213e)\246Xj\347\3\1\0~\31\266"
    "\14\366\207/\232\23\214\42d<\60\306\3\303\210\202`\371\360\377\303\0\0\0\0\4\377\377\0";

/*
  Fontname: open_iconic_arrow_2x
  Copyright: https://github.com/iconic/open-iconic, SIL OPEN FONT LICENSE
  Glyphs: 28/28
  BBX Build Mode: 0
*/
const uint8_t u8g2_font_open_iconic_arrow_2x_t[644] =
    "\34\0\4\4\5\5\4\4\6\20\20\0\0\16\0\16\0\0\17\0\0\2g@\17\352i\302$P\376\221"
    "\12\64\246\310\11\2A\22O%\303\24Z\360X\242\17^\20\36-<(\0B\22O!\303\32\134\364"
    "`\22\17\236\222\35,:\20\0C\17\352)\302$\216\224\31\24\212\4\312\77\2D\37\20\42\302eP"
    "\25!\62\205\212\24*a\210#\304E\17\222 F\244P\231\42\245\24\232\2E \20\42\302ePU"
    "\11\63E\216\220\71Ah-a\302e\15\241 s\244\310\231\22\246\24\232\2F\37\20\42\302eP\225"
    "\211\62G\212\234!\201\310laj\27\221\70C\344H\31\23\245\24\232\2G\37\20\42\302ePU\221"
    "\62\205\212\20#A\360haD\34\231(T\244P\31B\244\24\232\2H\21\12.\302C\214\376\215\212"
    "$\207\212\15\14\4\0I\23P\341\302\25\134\364`\27\17\36\204xBz\270P\0J\23P\341\302*"
    "|\64\221\27\17\36\204p<Zx(\0K\21\12.\302\24pX\241#)\324\20\243\177\3\0L\16"
    "\356d\303\340\202\215*t&\311\12\3M\24\307u\302\26J\320\30\42%\36\224\60R\206\320(a\1"
    "N\25\307q\302\20L\324 \62EL<(Q\204\314 Q\301\0O\15\356$\303&\226\244\71Tj"
    "X\70P\24.%\303!P\4\261D%\212\224Q\205\316$Ya\0Q\20\311m\302&\214P\231:"
    "*UW\304D\0R\17\311m\302!\214T=*SG\304\204\1S\24.%\303&\226\244\71Tj"
    "\212\224(\224\214\204@\21\0T\23\20\42\302F\230^\252Ci\226\264x\230>x \0U\23\20\42"
    "\302\360\201x\230\212&k\22\235J\302\364\14\0V\34\320\241\302\34\36\200\30\27/\204\12\21\32\36\302"
    "\240B\204\212x\341F<\200\300\0W\32\220\241\302&\230\354hq\202\304\221\21\306L\14\71A\342D"
    "\217%,\14\0X\37\20\42\302\302\210\221@A\2\205K$\314\210\70\42\344\204\30\23$\134BA\2"
    "\5q\2\0Y$\17\42\302\34\134\304\270TF\306\214\20\64bH\260\262\203\211\226\34\42$\320\230\21"
    "\42H\245+,:\10\0Z\30\220!\303\32\36\200\360\321\4S\61\351\206\324 q\202\4\206\12\17\0"
    "[\34\320\241\302`\324X\30q&D-J!&\211\230`a\204\313P\220@A\34\1\0\0\0\4"
    "\377\377\0";

/*
  Fontname: -Adobe-Courier-Bold-R-Normal--11-80-100-100-M-60-ISO10646-1
  Copyright: Copyright (c) 1984, 1987 Adobe Systems Incorporated. All Rights Reserved. Copyright (c) 1988, 1991 Digital Equipment Corporation. All Rights Reserved.
  Glyphs: 18/873
  BBX Build Mode: 0
*/
const uint8_t u8g2_font_courB08_tn[199] =
    "\22\0\3\3\3\4\2\3\4\6\11\0\376\6\376\7\377\0\0\0\0\0\252 \5\0\353\0*\11$\357"
    "\212P$\241\0+\12-\353\12\206J\301\20\0,\7\233\345\221$\1-\5\15\357(.\5\212\351\20"
    "/\14\304\347K\212\205b\241X\14\0\60\12=\351\231Hx\221L\0\61\10>\351\22\21u\62\62\11"
    "=\351\231\250\211\264\34\63\14=\351\231\250I\206\24\311\4\0\64\14>\351\223\215\42ZlB\11\0\65"
    "\12=\351\270Q\324F\26\0\66\14=\351\231Hh\24\11E\62\1\67\13=\351\270\310D\62\221L\4"
    "\70\15=\351\231H(\222\211\204\42\231\0\71\14=\351\231H(\22\32E\62\1:\6\242\351\20\12\0"
    "\0\0\4\377\377\0";

/*
  Fontname: -Misc-Fixed-Bold-R-Normal--13-120-75-75-C-70-ISO10646-1
  Copyright: Public domain font.  Share and enjoy.
  Glyphs: 95/1003
  BBX Build Mode: 0
*/
const uint8_t u8g2_font_7x13B_tr[1083] =
    "_\0\3\3\3\4\3\5\4\6\15\0\376\11\376\11\0\1}\2\330\4\36 \5\0\356\7!\7J\303"
    "\307\241D\42\10\235\332\207\204E\0#\20\315\302OR$r\230\244\34&I\221\10\0$\17N\302\227"
    "\214\22\321F\223\250Dh\42\0%\17N\302\307H\22\251\4e\212\221JD\64&\17N\302\317H\242"
    "\247\221$\62\251\210&\1'\7\42\327\307!\0(\14\314\302\227D$\21\251\211d\2)\15\314\302\207"
    "L$\23\251\210$\42\0*\15\66\306O(&:\224d\241\10\0+\13\66\306\227Pt(\11E\0"
    ",\10\244\276\317\212\22\0-\6\16\316\207\1.\10\234\276\217\204\42\1/\14N\302\247\232P\246(\23"
    "\12\1\60\16N\302\227,\24\21\361$\11\305D\0\61\13N\302\227l\24\21\352\311\0\62\16N\302\17"
    "ED\22\212F\62\241\320\0\63\15N\302\207Q\246F\25\222$\24\0\64\15N\302\247lD\221\220H"
    "\207\240\2\65\16N\302\307!(\254\210\204B\222\204\2\66\16N\302\17ED\24VDL\22\12\0\67"
    "\15N\302\207QM(\23\312\204\62\0\70\16N\302\17E\304$\241\210\230$\24\0\71\16N\302\17E"
    "\304$)\12I\22\12\0:\14\304\276\217\204\42\207I(\22\0;\13\304\276\217\204\42\236L\224\0<"
    "\10N\302\247LW\35=\7&\312\207\35j>\11N\302\207T\67\35\1\77\16N\302\17ED\22\212"
    "fr\230P\4@\17N\302\17%\266R\211L\252\61\11\5\0A\13N\302\17E\304t\30q\22B"
    "\14N\302GE\304t\21\61]\0C\13N\302\17ED\324\223\204\2D\12N\302GE\304O\27\0"
    "E\13N\302\307!\250X\21*\32F\13N\302\307!\250X\21j\4G\14N\302\17EDT)\61"
    "I\12H\13N\302\207\210\323a\304I\0I\11N\302\207I\250O\6J\12N\302\247>\222$\24\0"
    "K\17N\302\207lD\221\220f$\211\22-\0L\10N\302\207P\77\32M\13N\302Gpt\70\210"
    "x\22N\15N\302\207\210T\251\34&M$\1O\13N\302\17E\304O\22\12\0P\13N\302GE"
    "\304t\21j\4Q\14V\276\17E\304S\205\62\241\12R\15N\302GE\304t!I\224h\1S\16"
    "N\302\17ED\224R\205$\11\5\0T\11N\302\207I\250\237\0U\12N\302\207\210\77I(\0V"
    "\15N\302\207\210I\22\312D\23*\1W\13N\302\207\210\247\303A\64\14X\17N\302Gp$\11\205"
    "h\62R\212h\30Y\14N\302\207\210$!\321\204:\1Z\12N\302\207QMG\241\1[\10\314\302"
    "\207I\237\10\134\14N\302\207P*\224*J\205\2]\10\314\302\7I\237\14^\11&\326\227\214\42\32"
    "\6_\7\26\276\307\241\0`\7\234\336\207L\1a\12\66\302\17Ur\42I\12b\13N\302\207P\261"
    "\42\342t\1c\13\66\302\17EDT\222P\0d\12N\302\247\226\23'I\1e\14\66\302\17Et"
    "\30\212$\24\0f\14N\302\327H\242(\243\11\265\1g\16F\272\317\22IB\221RD\22\12\0h"
    "\13N\302\207P\261\42\342I\0i\12N\302\227P\16\32\352dj\14^\272\247:L\250#IB\1"
    "k\15N\302\207P\23EB\42I\224\4l\10N\302\317P\77\31m\12\66\302\207Dr\70\61\11n"
    "\11\66\302GE\304\223\0o\12\66\302\17E\304IB\1p\13F\272GE\304t\21*\2q\12F"
    "\272\317\211IR\324\0r\11\66\302GED\324\10s\15\66\302\17E$\21KD\22\12\0t\14F"
    "\302\217PV\22j\21M\0u\11\66\302\207\210'I\1v\13\66\302\207\210IB\242\211\0w\13\66"
    "\302\207\210\351p\11E\0x\14\66\302\207\210$!QD$\1y\14F\272\207\210\223\244H\222P\0"
    "z\12\66\302\207Q&\222\11\15{\14\314\302OI&\221ID\262\1|\7J\303\307\3\1}\15\314"
    "\302\307L$\221Id\242\12\0~\11\36\332\217\350\20\222\0\0\0\0\4\377\377\0";

/////// INIT STATE ///////
static void wifi_scanner_app_init(SWiFiScannerApp* const app) {
    app->m_context = Undefined;

    app->m_totalAccessPoints = 0;
    app->m_currentIndexAccessPoint = 0;

    app->m_currentAccesspointDescription.m_accessPointName = furi_string_alloc();
    furi_string_set(app->m_currentAccesspointDescription.m_accessPointName, "N/A\n");
    app->m_currentAccesspointDescription.m_channel = 0;
    app->m_currentAccesspointDescription.m_bssid = furi_string_alloc();
    furi_string_set(app->m_currentAccesspointDescription.m_bssid, "N/A\n");
    app->m_currentAccesspointDescription.m_secType = furi_string_alloc();
    furi_string_set(app->m_currentAccesspointDescription.m_secType, "N/A\n");
    app->m_currentAccesspointDescription.m_rssi = 0;
    app->m_currentAccesspointDescription.m_isHidden = false;

    app->m_prevAnimationTime = 0;
    app->m_animationTime = ANIMATION_TIME;
    app->m_animtaionCounter = 0;

    app->m_wifiModuleInitialized = false;

#if ENABLE_MODULE_DETECTION
    app->m_wifiModuleAttached = false;
#else
    app->m_wifiModuleAttached = true;
#endif
}

int16_t dBmtoPercentage(int16_t dBm) {
    const int16_t RSSI_MAX = -50; // define maximum strength of signal in dBm
    const int16_t RSSI_MIN = -100; // define minimum strength of signal in dBm

    int16_t quality;
    if(dBm <= RSSI_MIN) {
        quality = 0;
    } else if(dBm >= RSSI_MAX) {
        quality = 100;
    } else {
        quality = 2 * (dBm + 100);
    }

    return quality;
}

void DrawSignalStrengthBar(Canvas* canvas, int rssi, int x, int y, int width, int height) {
    int16_t percents = dBmtoPercentage(rssi);

    //u8g2_DrawHLine(&canvas->fb, x, y, width);
    //u8g2_DrawHLine(&canvas->fb, x, y + height, width);

    if(rssi != NA && height > 0) {
        uint8_t barHeight = floor((height / 100.f) * percents);
        canvas_draw_box(canvas, x, y + height - barHeight, width, barHeight);
    }
}

static void wifi_module_render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    SWiFiScannerApp* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    canvas_clear(canvas);

    {
        switch(app->m_context) {
        case Undefined: {
            canvas_set_font(canvas, FontPrimary);

            const char* strError = "Something wrong";
            canvas_draw_str(
                canvas,
                (128 / 2) - (canvas_string_width(canvas, strError) / 2),
                (64 / 2) /* - (canvas_current_font_height(canvas) / 2)*/,
                strError);
        } break;
        case WaitingForModule:
#if ENABLE_MODULE_DETECTION
            furi_assert(!app->m_wifiModuleAttached);
            if(!app->m_wifiModuleAttached) {
                canvas_set_font(canvas, FontSecondary);

                const char* strConnectModule = "Attach WiFi scanner module";
                canvas_draw_str(
                    canvas,
                    (128 / 2) - (canvas_string_width(canvas, strConnectModule) / 2),
                    (64 / 2) /* - (canvas_current_font_height(canvas) / 2)*/,
                    strConnectModule);
            }
#endif
            break;
        case Initializing: {
            furi_assert(!app->m_wifiModuleInitialized);
            if(!app->m_wifiModuleInitialized) {
                canvas_set_font(canvas, FontPrimary);

                const char* strInitializing = "Initializing...";
                canvas_draw_str(
                    canvas,
                    (128 / 2) - (canvas_string_width(canvas, strInitializing) / 2),
                    (64 / 2) - (canvas_current_font_height(canvas) / 2),
                    strInitializing);
            }
        } break;
        case ScanMode: {
            uint8_t offsetY = 0;
            uint8_t offsetX = 0;
            canvas_draw_frame(canvas, 0, 0, 128, 64);

            //canvas_set_font(canvas, FontPrimary);
            canvas_set_custom_u8g2_font(canvas, u8g2_font_7x13B_tr);
            uint8_t fontHeight = canvas_current_font_height(canvas);

            offsetX += 5;
            offsetY += fontHeight;
            canvas_draw_str(
                canvas,
                offsetX,
                offsetY,
                app->m_currentAccesspointDescription.m_isHidden ?
                    "(Hidden SSID)" :
                    furi_string_get_cstr(app->m_currentAccesspointDescription.m_accessPointName));

            offsetY += fontHeight;

            canvas_draw_str(
                canvas,
                offsetX,
                offsetY,
                furi_string_get_cstr(app->m_currentAccesspointDescription.m_bssid));

            canvas_set_font(canvas, FontSecondary);
            //canvas_set_custom_u8g2_font(canvas, u8g2_font_tinytim_tf);
            fontHeight = canvas_current_font_height(canvas);

            offsetY += fontHeight + 1;

            char string[15];
            snprintf(
                string, sizeof(string), "RSSI: %d", app->m_currentAccesspointDescription.m_rssi);
            canvas_draw_str(canvas, offsetX, offsetY, string);

            offsetY += fontHeight + 1;

            snprintf(
                string, sizeof(string), "CHNL: %d", app->m_currentAccesspointDescription.m_channel);
            canvas_draw_str(canvas, offsetX, offsetY, string);

            offsetY += fontHeight + 1;

            snprintf(
                string,
                sizeof(string),
                "ENCR: %s",
                furi_string_get_cstr(app->m_currentAccesspointDescription.m_secType));
            canvas_draw_str(canvas, offsetX, offsetY, string);

            offsetY += fontHeight;
            offsetY -= fontHeight;

            canvas_set_custom_u8g2_font(canvas, u8g2_font_courB08_tn);
            snprintf(
                string,
                sizeof(string),
                "%d/%d",
                app->m_currentIndexAccessPoint,
                app->m_totalAccessPoints);
            offsetX = 128 - canvas_string_width(canvas, string) - 5;
            canvas_draw_str(canvas, offsetX, offsetY, string);

            canvas_draw_frame(
                canvas, offsetX - 6, offsetY - canvas_current_font_height(canvas) - 3, 128, 64);

            canvas_set_custom_u8g2_font(canvas, u8g2_font_open_iconic_arrow_2x_t);
            if(app->m_currentIndexAccessPoint != app->m_totalAccessPoints) {
                //canvas_draw_triangle(canvas, offsetX - 5 - 20, offsetY + 5, 4, 4, CanvasDirectionBottomToTop);
                canvas_draw_str(canvas, offsetX - 0 - 35, offsetY + 5, "\x4C");
            }

            if(app->m_currentIndexAccessPoint != 1) {
                //canvas_draw_triangle(canvas, offsetX - 6 - 35, offsetY + 5, 4, 4, CanvasDirectionTopToBottom);
                canvas_draw_str(canvas, offsetX - 4 - 20, offsetY + 5, "\x4F");
            }
        } break;
        case MonitorMode: {
            uint8_t offsetY = 0;
            uint8_t offsetX = 0;

            canvas_draw_frame(canvas, 0, 0, 128, 64);

            //canvas_set_font(canvas, FontBigNumbers);
            //canvas_set_custom_u8g2_font(canvas, u8g2_font_inb27_mr);
            canvas_set_custom_u8g2_font(canvas, u8g2_font_inb27_mr);
            uint8_t fontHeight = canvas_current_font_height(canvas);
            uint8_t fontWidth = canvas_current_font_width(canvas);

            if(app->m_currentAccesspointDescription.m_rssi == NA) {
                offsetX += floor(128 / 2) - fontWidth - 10;
                offsetY += fontHeight - 5;

                canvas_draw_str(canvas, offsetX, offsetY, "N/A");
            } else {
                offsetX += floor(128 / 2) - 2 * fontWidth;
                offsetY += fontHeight - 5;

                char rssi[8];
                snprintf(rssi, sizeof(rssi), "%d", app->m_currentAccesspointDescription.m_rssi);
                canvas_draw_str(canvas, offsetX, offsetY, rssi);
            }

            //canvas_set_font(canvas, FontPrimary);
            canvas_set_custom_u8g2_font(canvas, u8g2_font_7x13B_tr);
            fontHeight = canvas_current_font_height(canvas);
            fontWidth = canvas_current_font_width(canvas);

            offsetX = 5;
            offsetY = 64 - 7 - fontHeight;
            canvas_draw_str(
                canvas,
                offsetX,
                offsetY,
                furi_string_get_cstr(app->m_currentAccesspointDescription.m_accessPointName));

            offsetY += fontHeight + 2;

            canvas_draw_str(
                canvas,
                offsetX,
                offsetY,
                furi_string_get_cstr(app->m_currentAccesspointDescription.m_bssid));

            DrawSignalStrengthBar(
                canvas, app->m_currentAccesspointDescription.m_rssi, 5, 5, 12, 25);
            DrawSignalStrengthBar(
                canvas, app->m_currentAccesspointDescription.m_rssi, 128 - 5 - 12, 5, 12, 25);
        } break;
        case ScanAnimation: {
            uint32_t currentTime = furi_get_tick();
            if(currentTime - app->m_prevAnimationTime > app->m_animationTime) {
                app->m_prevAnimationTime = currentTime;
                app->m_animtaionCounter += 1;
                app->m_animtaionCounter = app->m_animtaionCounter % 3;
            }

            uint8_t offsetX = 10;
            uint8_t mutliplier = 2;

            for(uint8_t i = 0; i < 3; ++i) {
                canvas_draw_disc(
                    canvas,
                    offsetX + 30 + 25 * i,
                    64 / 2 - 7,
                    5 * (app->m_animtaionCounter == i ? mutliplier : 1));
            }

            canvas_set_custom_u8g2_font(canvas, u8g2_font_7x13B_tr);
            //canvas_set_font(canvas, FontPrimary);
            const char* message = "Scanning";
            canvas_draw_str(
                canvas, 128 / 2 - canvas_string_width(canvas, message) / 2, 55, message);
        } break;
        case MonitorAnimation: {
            uint32_t currentTime = furi_get_tick();
            if(currentTime - app->m_prevAnimationTime > app->m_animationTime) {
                app->m_prevAnimationTime = currentTime;
                app->m_animtaionCounter += 1;
                app->m_animtaionCounter = app->m_animtaionCounter % 2;
            }

            uint8_t offsetX = 10;
            uint8_t mutliplier = 2;

            canvas_draw_disc(
                canvas,
                offsetX + 30,
                64 / 2 - 7,
                5 * (app->m_animtaionCounter == 0 ? mutliplier : 1));
            canvas_draw_disc(
                canvas,
                offsetX + 55,
                64 / 2 - 7,
                5 * (app->m_animtaionCounter == 1 ? mutliplier : 1));
            canvas_draw_disc(
                canvas,
                offsetX + 80,
                64 / 2 - 7,
                5 * (app->m_animtaionCounter == 0 ? mutliplier : 1));

            canvas_set_custom_u8g2_font(canvas, u8g2_font_7x13B_tr);
            //canvas_set_font(canvas, FontPrimary);
            const char* message = "Monitor Mode";
            canvas_draw_str(
                canvas, 128 / 2 - canvas_string_width(canvas, message) / 2, 55, message);
        } break;
        default:
            break;
        }
    }
    furi_mutex_release(app->mutex);
}

static void wifi_module_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    SPluginEvent event = {.m_type = EventTypeKey, .m_input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void uart_on_irq_cb(UartIrqEvent ev, uint8_t data, void* context) {
    furi_assert(context);

    SWiFiScannerApp* app = context;

    WIFI_APP_LOG_I("uart_echo_on_irq_cb");

    if(ev == UartIrqEventRXNE) {
        WIFI_APP_LOG_I("ev == UartIrqEventRXNE");
        furi_stream_buffer_send(app->m_rx_stream, &data, 1, 0);
        furi_thread_flags_set(furi_thread_get_id(app->m_worker_thread), WorkerEventRx);
    }
}

static int32_t uart_worker(void* context) {
    furi_assert(context);

    SWiFiScannerApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    if(app == NULL) {
        return 1;
    }

    FuriStreamBuffer* rx_stream = app->m_rx_stream;

    furi_mutex_release(app->mutex);

    while(true) {
        uint32_t events = furi_thread_flags_wait(
            WorkerEventStop | WorkerEventRx, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);

        if(events & WorkerEventStop) break;
        if(events & WorkerEventRx) {
            size_t length = 0;
            FuriString* receivedString;
            receivedString = furi_string_alloc();
            do {
                uint8_t data[64];
                length = furi_stream_buffer_receive(rx_stream, data, 64, 25);
                if(length > 0) {
                    WIFI_APP_LOG_I("Received Data - length: %i", length);

                    for(uint16_t i = 0; i < length; i++) {
                        furi_string_push_back(receivedString, data[i]);
                    }

                    //notification_message(app->notification, &sequence_set_only_red_255);
                }
            } while(length > 0);
            if(furi_string_size(receivedString) > 0) {
                FuriString* chunk;
                chunk = furi_string_alloc();
                size_t begin = 0;
                size_t end = 0;
                size_t stringSize = furi_string_size(receivedString);

                WIFI_APP_LOG_I("Received string: %s", furi_string_get_cstr(receivedString));

                FuriString* chunksArray[EChunkArrayData_ENUM_MAX];
                for(uint8_t i = 0; i < EChunkArrayData_ENUM_MAX; ++i) {
                    chunksArray[i] = furi_string_alloc();
                }

                uint8_t index = 0;
                do {
                    end = furi_string_search_char(receivedString, '+', begin);

                    if(end == FURI_STRING_FAILURE) {
                        end = stringSize;
                    }

                    WIFI_APP_LOG_I("size: %i, begin: %i, end: %i", stringSize, begin, end);

                    furi_string_set_strn(
                        chunk, &furi_string_get_cstr(receivedString)[begin], end - begin);

                    WIFI_APP_LOG_I("String chunk: %s", furi_string_get_cstr(chunk));

                    furi_string_set(chunksArray[index++], chunk);

                    begin = end + 1;
                } while(end < stringSize);
                furi_string_free(chunk);

                app = context;
                furi_mutex_acquire(app->mutex, FuriWaitForever);
                if(app == NULL) {
                    return 1;
                }

                if(!app->m_wifiModuleInitialized) {
                    if(furi_string_cmp_str(
                           chunksArray[EChunkArrayData_Context], MODULE_CONTEXT_INITIALIZATION) ==
                       0) {
                        app->m_wifiModuleInitialized = true;
                        app->m_context = ScanAnimation;
                    }

                } else {
                    if(furi_string_cmp_str(
                           chunksArray[EChunkArrayData_Context], MODULE_CONTEXT_MONITOR) == 0) {
                        app->m_context = MonitorMode;
                    } else if(
                        furi_string_cmp_str(
                            chunksArray[EChunkArrayData_Context], MODULE_CONTEXT_SCAN) == 0) {
                        app->m_context = ScanMode;
                    } else if(
                        furi_string_cmp_str(
                            chunksArray[EChunkArrayData_Context], MODULE_CONTEXT_SCAN_ANIMATION) ==
                        0) {
                        app->m_context = ScanAnimation;
                    } else if(
                        furi_string_cmp_str(
                            chunksArray[EChunkArrayData_Context],
                            MODULE_CONTEXT_MONITOR_ANIMATION) == 0) {
                        app->m_context = MonitorAnimation;
                    }

                    if(app->m_context == MonitorMode || app->m_context == ScanMode) {
                        furi_string_set(
                            app->m_currentAccesspointDescription.m_accessPointName,
                            chunksArray[EChunkArrayData_SSID]);
                        furi_string_set(
                            app->m_currentAccesspointDescription.m_secType,
                            chunksArray[EChunkArrayData_EncryptionType]);
                        app->m_currentAccesspointDescription.m_rssi =
                            atoi(furi_string_get_cstr(chunksArray[EChunkArrayData_RSSI]));
                        furi_string_set(
                            app->m_currentAccesspointDescription.m_bssid,
                            chunksArray[EChunkArrayData_BSSID]);
                        app->m_currentAccesspointDescription.m_channel =
                            atoi(furi_string_get_cstr(chunksArray[EChunkArrayData_Channel]));
                        app->m_currentAccesspointDescription.m_isHidden =
                            atoi(furi_string_get_cstr(chunksArray[EChunkArrayData_IsHidden]));

                        app->m_currentIndexAccessPoint = atoi(
                            furi_string_get_cstr(chunksArray[EChunkArrayData_CurrentAPIndex]));
                        app->m_totalAccessPoints =
                            atoi(furi_string_get_cstr(chunksArray[EChunkArrayData_TotalAps]));
                    }
                }

                furi_mutex_release(app->mutex);

                // Clear string array
                for(index = 0; index < EChunkArrayData_ENUM_MAX; ++index) {
                    furi_string_free(chunksArray[index]);
                }
            }
            furi_string_free(receivedString);
        }
    }

    return 0;
}

typedef enum ESerialCommand {
    ESerialCommand_Next,
    ESerialCommand_Previous,
    ESerialCommand_Scan,
    ESerialCommand_MonitorMode,
    ESerialCommand_Restart
} ESerialCommand;

void send_serial_command(ESerialCommand command) {
#if !DISABLE_CONSOLE
    return;
#endif

    uint8_t data[1] = {0};

    switch(command) {
    case ESerialCommand_Next:
        data[0] = MODULE_CONTROL_COMMAND_NEXT;
        break;
    case ESerialCommand_Previous:
        data[0] = MODULE_CONTROL_COMMAND_PREVIOUS;
        break;
    case ESerialCommand_Scan:
        data[0] = MODULE_CONTROL_COMMAND_SCAN;
        break;
    case ESerialCommand_MonitorMode:
        data[0] = MODULE_CONTROL_COMMAND_MONITOR;
        break;
    case ESerialCommand_Restart:
        data[0] = MODULE_CONTROL_COMMAND_RESTART;
        break;
    default:
        return;
    };

    furi_hal_uart_tx(FuriHalUartIdUSART1, data, 1);
}

int32_t wifi_scanner_app(void* p) {
    UNUSED(p);

    WIFI_APP_LOG_I("Init");

    // FuriTimer* timer = furi_timer_alloc(blink_test_update, FuriTimerTypePeriodic, event_queue);
    // furi_timer_start(timer, furi_kernel_get_tick_frequency());

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(SPluginEvent));

    SWiFiScannerApp* app = malloc(sizeof(SWiFiScannerApp));

    wifi_scanner_app_init(app);

#if ENABLE_MODULE_DETECTION
    furi_hal_gpio_init(
        &gpio_ext_pc0,
        GpioModeInput,
        GpioPullUp,
        GpioSpeedLow); // Connect to the Flipper's ground just to be sure
    //furi_hal_gpio_add_int_callback(pinD0, input_isr_d0, this);
    app->m_context = WaitingForModule;
#else
    app->m_context = Initializing;
#if ENABLE_MODULE_POWER
    uint8_t attempts = 0;
    while(!furi_hal_power_is_otg_enabled() && attempts++ < 5) {
        furi_hal_power_enable_otg();
        furi_delay_ms(10);
    }
    furi_delay_ms(200);
#endif // ENABLE_MODULE_POWER
#endif // ENABLE_MODULE_DETECTION

    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    if(!app->mutex) {
        WIFI_APP_LOG_E("cannot create mutex\r\n");
        free(app);
        return 255;
    }

    WIFI_APP_LOG_I("Mutex created");

    app->m_notification = furi_record_open(RECORD_NOTIFICATION);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, wifi_module_render_callback, app);
    view_port_input_callback_set(view_port, wifi_module_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    //notification_message(app->notification, &sequence_set_only_blue_255);

    app->m_rx_stream = furi_stream_buffer_alloc(1 * 1024, 1);

    app->m_worker_thread = furi_thread_alloc();
    furi_thread_set_name(app->m_worker_thread, "WiFiModuleUARTWorker");
    furi_thread_set_stack_size(app->m_worker_thread, 1024);
    furi_thread_set_context(app->m_worker_thread, app);
    furi_thread_set_callback(app->m_worker_thread, uart_worker);
    furi_thread_start(app->m_worker_thread);
    WIFI_APP_LOG_I("UART thread allocated");

    // Enable uart listener
#if DISABLE_CONSOLE
    furi_hal_console_disable();
#endif
    furi_hal_uart_set_br(FuriHalUartIdUSART1, FLIPPERZERO_SERIAL_BAUD);
    furi_hal_uart_set_irq_cb(FuriHalUartIdUSART1, uart_on_irq_cb, app);
    WIFI_APP_LOG_I("UART Listener created");

    // Because we assume that module was on before we launched the app. We need to ensure that module will be in initial state on app start
    send_serial_command(ESerialCommand_Restart);

    SPluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(app->mutex, FuriWaitForever);

#if ENABLE_MODULE_DETECTION
        if(!app->m_wifiModuleAttached) {
            if(furi_hal_gpio_read(&gpio_ext_pc0) == false) {
                WIFI_APP_LOG_I("Module Attached");
                app->m_wifiModuleAttached = true;
                app->m_context = Initializing;
#if ENABLE_MODULE_POWER
                uint8_t attempts2 = 0;
                while(!furi_hal_power_is_otg_enabled() && attempts2++ < 3) {
                    furi_hal_power_enable_otg();
                    furi_delay_ms(10);
                }

#endif
            }
        }
#endif // ENABLE_MODULE_DETECTION

        if(event_status == FuriStatusOk) {
            if(event.m_type == EventTypeKey) {
                if(app->m_wifiModuleInitialized) {
                    if(app->m_context == ScanMode) {
                        switch(event.m_input.key) {
                        case InputKeyUp:
                        case InputKeyLeft:
                            if(event.m_input.type == InputTypeShort) {
                                WIFI_APP_LOG_I("Previous");
                                send_serial_command(ESerialCommand_Previous);
                            } else if(event.m_input.type == InputTypeRepeat) {
                                WIFI_APP_LOG_I("Previous Repeat");
                                send_serial_command(ESerialCommand_Previous);
                            }
                            break;
                        case InputKeyDown:
                        case InputKeyRight:
                            if(event.m_input.type == InputTypeShort) {
                                WIFI_APP_LOG_I("Next");
                                send_serial_command(ESerialCommand_Next);
                            } else if(event.m_input.type == InputTypeRepeat) {
                                WIFI_APP_LOG_I("Next Repeat");
                                send_serial_command(ESerialCommand_Next);
                            }
                            break;
                        default:
                            break;
                        }
                    }

                    switch(event.m_input.key) {
                    case InputKeyOk:
                        if(event.m_input.type == InputTypeShort) {
                            if(app->m_context == ScanMode) {
                                WIFI_APP_LOG_I("Monitor Mode");
                                send_serial_command(ESerialCommand_MonitorMode);
                            }
                        } else if(event.m_input.type == InputTypeLong) {
                            WIFI_APP_LOG_I("Scan");
                            send_serial_command(ESerialCommand_Scan);
                        }
                        break;
                    case InputKeyBack:
                        if(event.m_input.type == InputTypeShort) {
                            switch(app->m_context) {
                            case MonitorMode:
                                send_serial_command(ESerialCommand_Scan);
                                break;
                            case ScanMode:
                                processing = false;
                                break;
                            default:
                                break;
                            }
                        } else if(event.m_input.type == InputTypeLong) {
                            processing = false;
                        }
                        break;
                    default:
                        break;
                    }
                } else {
                    if(event.m_input.key == InputKeyBack) {
                        if(event.m_input.type == InputTypeShort ||
                           event.m_input.type == InputTypeLong) {
                            processing = false;
                        }
                    }
                }
            }
        }

#if ENABLE_MODULE_DETECTION
        if(app->m_wifiModuleAttached && furi_hal_gpio_read(&gpio_ext_pc0) == true) {
            WIFI_APP_LOG_D("Module Disconnected - Exit");
            processing = false;
            app->m_wifiModuleAttached = false;
            app->m_wifiModuleInitialized = false;
        }
#endif

        view_port_update(view_port);
        furi_mutex_release(app->mutex);
    }

    WIFI_APP_LOG_I("Start exit app");

    furi_thread_flags_set(furi_thread_get_id(app->m_worker_thread), WorkerEventStop);
    furi_thread_join(app->m_worker_thread);
    furi_thread_free(app->m_worker_thread);

    WIFI_APP_LOG_I("Thread Deleted");

    // Reset GPIO pins to default state
    furi_hal_gpio_init(&gpio_ext_pc0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

#if DISABLE_CONSOLE
    furi_hal_console_enable();
#endif

    view_port_enabled_set(view_port, false);

    gui_remove_view_port(gui, view_port);

    // Close gui record
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    app->m_gui = NULL;

    view_port_free(view_port);

    furi_message_queue_free(event_queue);

    furi_stream_buffer_free(app->m_rx_stream);

    furi_mutex_free(app->mutex);

    // Free rest
    free(app);

    WIFI_APP_LOG_I("App freed");

#if ENABLE_MODULE_POWER
    if(furi_hal_power_is_otg_enabled()) {
        furi_hal_power_disable_otg();
    }
#endif

    return 0;
}