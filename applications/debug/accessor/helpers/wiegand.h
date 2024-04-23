#pragma once

class WIEGAND {
public:
    WIEGAND(void);
    void begin(void);
    void end(void);
    bool available(void);
    unsigned long getCode(void);
    unsigned long getCodeHigh(void);
    int getWiegandType(void);

    static void ReadD0(void);
    static void ReadD1(void);

private:
    static bool DoWiegandConversion(void);
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
