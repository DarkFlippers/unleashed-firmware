#pragma once

class WIEGAND {
public:
    WIEGAND();
    void begin();
    void end();
    bool available();
    unsigned long getCode();
    unsigned long getCodeHigh();
    int getWiegandType();

    static void ReadD0();
    static void ReadD1();

private:
    static bool DoWiegandConversion();
    static unsigned long GetCardId(
        volatile unsigned long* codehigh,
        volatile unsigned long* codelow,
        char bitlength);

    static volatile unsigned long _cardTempHigh;
    static volatile unsigned long _cardTemp;
    static volatile unsigned long _lastWiegand;
    static volatile int _bitCount;
    static int _wiegandType;
    static unsigned long _code;
    static unsigned long _codeHigh;
};
