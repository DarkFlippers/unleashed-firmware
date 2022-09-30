#include "subbrute_protocols.h"

static const SubBruteProtocol subbrute_protocols[SubBruteAttackTotalCount] = {
    [SubBruteAttackCAME12bit307] =
        {307800000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
    [SubBruteAttackCAME12bit433] =
        {433920000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
    [SubBruteAttackCAME12bit868] =
        {868350000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
    [SubBruteAttackNICE12bit433] =
        {433920000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, NICEFileProtocol},
    [SubBruteAttackNICE12bit868] =
        {868350000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, NICEFileProtocol},
    [SubBruteAttackChamberlain9bit300] =
        {300000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
    [SubBruteAttackChamberlain9bit315] =
        {315000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
    [SubBruteAttackChamberlain9bit390] =
        {390000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
    [SubBruteAttackLinear10bit300] =
        {300000000, 10, 0, 5, FuriHalSubGhzPresetOok650Async, LinearFileProtocol},
    [SubBruteAttackLinear10bit310] =
        {300000000, 10, 0, 5, FuriHalSubGhzPresetOok650Async, LinearFileProtocol},
    [SubBruteAttackLoadFile] = {0, 0, 0, 3, FuriHalSubGhzPresetOok650Async, RAWFileProtocol},
};
//static const uint32_t subbrute_protocols[SubBruteAttackTotalCount][TotalProtocolFields] = {
//    [SubBruteAttackCAME12bit307] = {307800000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
//    [SubBruteAttackCAME12bit433] = {433920000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
//    [SubBruteAttackCAME12bit868] = {868350000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, CAMEFileProtocol},
//    [SubBruteAttackNICE12bit433] = {433920000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, NICEFileProtocol},
//    [SubBruteAttackNICE12bit868] = {868350000, 12, 0, 3, FuriHalSubGhzPresetOok650Async, NICEFileProtocol},
//    [SubBruteAttackChamberlain9bit300] = {300000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
//    [SubBruteAttackChamberlain9bit315] = {315000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
//    [SubBruteAttackChamberlain9bit390] = {390000000, 9, 0, 3, FuriHalSubGhzPresetOok650Async, ChamberlainFileProtocol},
//    [SubBruteAttackLinear10bit300] = {300000000, 10, 0, 5, FuriHalSubGhzPresetOok650Async, LinearFileProtocol},
//    [SubBruteAttackLinear10bit310] = {300000000, 10, 0, 5, FuriHalSubGhzPresetOok650Async, LinearFileProtocol},
//    [SubBruteAttackLoadFile] = {0, 0, 0, 3, FuriHalSubGhzPresetOok650Async, RAWFileProtocol},
//};

static const char* subbrute_protocol_names[] = {
    [SubBruteAttackCAME12bit307] = "CAME 12bit 307MHz",
    [SubBruteAttackCAME12bit433] = "CAME 12bit 433MHz",
    [SubBruteAttackCAME12bit868] = "CAME 12bit 868MHz",
    [SubBruteAttackNICE12bit433] = "NICE 12bit 433MHz",
    [SubBruteAttackNICE12bit868] = "NICE 12bit 868MHz",
    [SubBruteAttackChamberlain9bit300] = "Chamberlain 9bit 300MHz",
    [SubBruteAttackChamberlain9bit315] = "Chamberlain 9bit 315MHz",
    [SubBruteAttackChamberlain9bit390] = "Chamberlain 9bit 390MHz",
    [SubBruteAttackLinear10bit300] = "Linear 10bit 300MHz",
    [SubBruteAttackLinear10bit310] = "Linear 10bit 310MHz",
    [SubBruteAttackLoadFile] = "BF existing dump",
    [SubBruteAttackTotalCount] = "Total Count",
};

static const char* subbrute_protocol_presets[] = {
    [FuriHalSubGhzPresetIDLE] = "FuriHalSubGhzPresetIDLE",
    [FuriHalSubGhzPresetOok270Async] = "FuriHalSubGhzPresetOok270Async",
    [FuriHalSubGhzPresetOok650Async] = "FuriHalSubGhzPresetOok650Async",
    [FuriHalSubGhzPreset2FSKDev238Async] = "FuriHalSubGhzPreset2FSKDev238Async",
    [FuriHalSubGhzPreset2FSKDev476Async] = "FuriHalSubGhzPreset2FSKDev476Async",
    [FuriHalSubGhzPresetMSK99_97KbAsync] = "FuriHalSubGhzPresetMSK99_97KbAsync",
    [FuriHalSubGhzPresetGFSK9_99KbAsync] = "FuriHalSubGhzPresetGFSK9_99KbAsync",
};

static const char* subbrute_protocol_file_types[TotalFileProtocol] = {
    [CAMEFileProtocol] = "CAME",
    [NICEFileProtocol] = "Nice FLO",
    [ChamberlainFileProtocol] = "Cham_Code",
    [LinearFileProtocol] = "Linear",
    [PrincetonFileProtocol] = "Princeton",
    [RAWFileProtocol] = "RAW"};

SubBruteProtocol* subbrute_protocol_alloc(void) {
    SubBruteProtocol* protocol = malloc(sizeof(SubBruteProtocol));
    protocol->frequency = subbrute_protocols[SubBruteAttackLoadFile].frequency;
    protocol->repeat = subbrute_protocols[SubBruteAttackLoadFile].repeat;
    protocol->preset = subbrute_protocols[SubBruteAttackLoadFile].preset;
    protocol->file = subbrute_protocols[SubBruteAttackLoadFile].file;
    protocol->te = subbrute_protocols[SubBruteAttackLoadFile].te;
    protocol->bits = subbrute_protocols[SubBruteAttackLoadFile].bits;

    return protocol;
}

const char* subbrute_protocol_name(SubBruteAttacks index) {
    return subbrute_protocol_names[index];
}

SubBruteProtocol* subbrute_protocol(SubBruteAttacks index) {
    SubBruteProtocol* protocol = subbrute_protocol_alloc();
    protocol->frequency = subbrute_protocols[index].frequency;
    protocol->repeat = subbrute_protocols[index].repeat;
    protocol->preset = subbrute_protocols[index].preset;
    protocol->file = subbrute_protocols[index].file;
    protocol->te = subbrute_protocols[index].te;
    protocol->bits = subbrute_protocols[index].bits;

    return protocol;
}

const char* subbrute_protocol_preset(FuriHalSubGhzPreset preset) {
    return subbrute_protocol_presets[preset];
}

const char* subbrute_protocol_file(SubBruteFileProtocol protocol) {
    return subbrute_protocol_file_types[protocol];
}

FuriHalSubGhzPreset subbrute_protocol_convert_preset(string_t preset_name) {
    for(size_t i = FuriHalSubGhzPresetIDLE; i<FuriHalSubGhzPresetCustom;i++) {
        if(string_cmp_str(preset_name, subbrute_protocol_presets[i]) == 0) {
            return i;
        }
    }

    return FuriHalSubGhzPresetIDLE;
}

SubBruteFileProtocol subbrute_protocol_file_protocol_name(string_t name) {
    for(size_t i = CAMEFileProtocol; i<TotalFileProtocol-1;i++) {
        if(string_cmp_str(name, subbrute_protocol_file_types[i]) == 0) {
            return i;
        }
    }

    return RAWFileProtocol;
}