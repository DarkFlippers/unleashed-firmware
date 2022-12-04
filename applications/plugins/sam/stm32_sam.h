#include <furi.h>

#ifndef __STM32SAM__
#define __STM32SAM__

// SAM Text-To-Speech (TTS), ported from https://github.com/s-macke/SAM

class STM32SAM {
public:
    STM32SAM(uint32_t STM32SAM_SPEED);
    STM32SAM();

    void begin(void);

    void
        sam(const char* argv,
            unsigned char phonetic,
            unsigned char singmode,
            unsigned char pitch,
            unsigned char speed,
            unsigned char mouth,
            unsigned char throat);
    void
        sam(char* argv,
            unsigned char phonetic,
            unsigned char singmode,
            unsigned char pitch,
            unsigned char speed,
            unsigned char mouth,
            unsigned char throat);

    void say(const char* argv);
    void say(char* argv);
    void sing(const char* argv);
    void sing(char* argv);
    void sayPhonetic(const char* argv);
    void sayPhonetic(char* argv);
    void singPhonetic(const char* argv);
    void singPhonetic(char* argv);
    void setVoice(
        unsigned char _pitch = 64,
        unsigned char _speed = 72,
        unsigned char _mouth = 128,
        unsigned char _throat = 128);
    void setPitch(unsigned char _pitch = 64);
    void setSpeed(unsigned char _speed = 72);
    void setMouth(unsigned char _mouth = 128);
    void setThroat(unsigned char _throat = 128);

private:
    void SetAUDIO(unsigned char main_volume);

    void Output8BitAry(int index, unsigned char ary[5]);
    void Output8Bit(int index, unsigned char A);
    unsigned char Read(unsigned char p, unsigned char Y);
    void Write(unsigned char p, unsigned char Y, unsigned char value);
    void RenderSample(unsigned char* mem66);
    void Render();
    void AddInflection(unsigned char mem48, unsigned char phase1);
    void SetMouthThroat();
    unsigned char trans(unsigned char mem39212, unsigned char mem39213);
    void SetInput(char* _input);
    void Init();
    int SAMMain();
    void PrepareOutput();
    void Insert(
        unsigned char position /*var57*/,
        unsigned char mem60,
        unsigned char mem59,
        unsigned char mem58);
    void InsertBreath();
    void CopyStress();
    int Parser1();
    void SetPhonemeLength();
    void Code41240();
    void Parser2();
    void AdjustLengths();
    void Code47503(unsigned char mem52);
    void Code37055(unsigned char mem59);
    void Code37066(unsigned char mem58);
    unsigned char GetRuleByte(unsigned short mem62, unsigned char Y);
    int TextToPhonemes(unsigned char* input); // Code36484

    uint32_t _STM32SAM_SPEED;

    unsigned char speed;
    unsigned char pitch;
    unsigned char mouth;
    unsigned char throat;

    unsigned char phonetic;
    unsigned char singmode;

}; // STM32SAM class

#endif