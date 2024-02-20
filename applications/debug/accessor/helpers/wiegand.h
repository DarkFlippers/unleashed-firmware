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
    static unsigned long
        GetCardId(unsigned long* codehigh, unsigned long* codelow, char bitlength);

    static unsigned long _cardTempHigh;
    static unsigned long _cardTemp;
    static unsigned long _lastWiegand;
    static int _bitCount;
    static int _wiegandType;
    static unsigned long _code;
    static unsigned long _codeHigh;
};
