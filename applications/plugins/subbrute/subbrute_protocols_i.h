#pragma once

typedef enum {
    SubBruteAttackCAME12bit307,
    SubBruteAttackCAME12bit433,
    SubBruteAttackCAME12bit868,
    SubBruteAttackNICE12bit433,
    SubBruteAttackNICE12bit868,
    SubBruteAttackChamberlain9bit300,
    SubBruteAttackChamberlain9bit315,
    SubBruteAttackChamberlain9bit390,
    SubBruteAttackLinear10bit300,
    SubBruteAttackLinear10bit310,
    SubBruteAttackLoadFile,
    SubBruteAttackTotalCount,
} SubBruteAttacks;

const char* subbrute_protocol_name(SubBruteAttacks index);