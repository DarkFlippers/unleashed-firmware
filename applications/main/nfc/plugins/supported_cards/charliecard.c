/*
 * Parser for MBTA CharlieCard (Boston, MA, USA).
 *
 * Copyright 2024 Zachary Weiss <me@zachary.ws>
 * 
 * Public security research on the MBTA's fare system stretches back to 2008,
 * starting with Russel Ryan, Zack Anderson, and Alessandro Chiesa's 
 * "Anatomy of a Subway Hack", for which they were famously issued a gag order. 
 * A thorough history of research & researchers deserving of credit is 
 * detailed by @bobbyrsec in his 2022 blog post (& presentation):
 * "Operation Charlie: Hacking the MBTA CharlieCard from 2008 to Present" 
 * https://medium.com/@bobbyrsec/operation-charlie-hacking-the-mbta-charliecard-from-2008-to-present-24ea9f0aaa38
 * 
 * Fare gate IDs, card types, and general assistance courtesy of the
 * minds behind DEFCON 31's "Boston Infinite Money Glitch" presentation:
 * — Matthew Harris; mattyharris.net <matty@mattyharris.net>
 * — Zachary Bertocchi; zackbertocchi.com <zach@zachbertocci.com>
 * — Scott Campbell; josephscottcampbell.com <scott@josephscottcampbell.com>
 * — Noah Gibson; <noahgibson06@proton.me>
 * Talk available at: https://www.youtube.com/watch?v=1JT_lTfK69Q
 * 
 * TODOs:
 * — Reverse engineer passes (sectors 4 & 5?), impl.
 * — Infer transaction flag meanings
 * — Infer remaining unknown bytes in the balance sectors (2 & 3)
 * – ASCII art &/or unified read function for the balance sectors, 
 *   to improve readability / interpretability by others?
 * — Improve string output formatting, esp. of transaction log
 * — Continually gather data on fare gate ID mappings, update as collected;
 *   check locations this might be scrapable / inferrable from:
 *   [X] MBTA GTFS spec (https://www.mbta.com/developers/gtfs) features & IDs 
 *       seem too-coarse-grained & uncorrelated
 *   [X] MBTA ArcGIS (https://mbta-massdot.opendata.arcgis.com/) & Tableau (https://public.tableau.com/app/profile/mbta.office.of.performance.management.and.innovation/vizzes) 
 *       files don't seem to have anything of that resolution (only down to ridership by station)
 *   [X] (skim of) MBTA public GitHub (https://github.com/mbta) repos make no reference to fare-gate-level data
 *   [X] (skim of) MBTA public engineering docs (https://www.mbta.com/engineering) unfruitful;
 *       Closest mention spotted is 2014 "Ridership and Service Statistics" (https://cdn.mbta.com/sites/default/files/fmcb-meeting-docs/reports-policies/2014-07-mbta-bluebook-ed14.pdf)
 *       where on pg.40, "Equipment at Stations" is enumerated, and fare gates counts are given,
 *       listed as "AFC Gates" (presumably standing for "Automated Fare Control")
 *   [X] Josiah Zachery criminal trial public evidence — convicted partially on 
 *       data on his CharlieCard, appeals partially on basis of legality of this search.
 *       Prev. court case (gag order mentioned in preamble) leaked some data in the files
 *       entered into evidence. Seemingly did not happen here; fare gate IDs unmentioned,
 *       only ever the nature of stored/saved data and methods of retrieval.
 *       Appelate case dockets 2019-P-0401, SJC-12952, SJ-2017-0390 (https://www.ma-appellatecourts.org/party)
 *       Trial court case 04/02/2015 #1584CR10265 @Suffolk County Criminal Superior Court (https://www.masscourts.org/eservices/home.page.16)
 *   [ ] FOIA / public records request? (https://massachusettsdot.mycusthelp.com/WEBAPP/_rs/(S(tbcygdlm0oojy35p1wv0y2y5))/supporthome.aspx)
 *   [ ] MBTA data blog? (https://www.massdottracker.com/datablog/)
 *   [ ] Other?
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>
#include <datetime.h>
#include <furi_hal.h>
#include <locale/locale.h>

#define TAG "CharlieCard"

// starts Wednesday 2003/1/1 @ midnight
#define CHARLIE_EPOCH          \
    (DateTime) {               \
        0, 0, 0, 1, 1, 2003, 4 \
    }
// timestep is one minute
#define CHARLIE_TIME_DELTA_SECS 60
#define CHARLIE_END_VALID_DELTA_SECS 60 * 8
#define CHARLIE_N_TRIP_HISTORY 10

enum CharlieActiveSector {
    CHARLIE_ACTIVE_SECTOR_2,
    CHARLIE_ACTIVE_SECTOR_3,
};

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

// always from the same set of keys (cf. default keys dict for list w/o multiplicity)
// we only care about the data in the first half of the sectors
// second half sectors keys seemingly change position sometimes across cards?
// no data stored there, but might want to impl some custom read function
// accounting for this such that reading is faster (else it seems to fall back on dict
// approach for remaining keys)...
static const MfClassicKeyPair charliecard_1k_keys[] = {
    {.a = 0x3060206F5B0A, .b = 0xF1B9F5669CC8},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x5EC39B022F2B, .b = 0xF662248E7E89},
    {.a = 0x3A09594C8587, .b = 0x62387B8D250D},
    {.a = 0xF238D78FF48F, .b = 0x9DC282D46217},
    {.a = 0xAFD0BA94D624, .b = 0x92EE4DC87191},
    {.a = 0xB35A0E4ACC09, .b = 0x756EF55E2507},
    {.a = 0x447AB7FD5A6B, .b = 0x932B9CB730EF},
    {.a = 0x1F1A0A111B5B, .b = 0xAD9E0A1CA2F7},
    {.a = 0xD58023BA2BDC, .b = 0x62CED42A6D87},
    {.a = 0x2548A443DF28, .b = 0x2ED3B15E7C0F},
};

typedef struct {
    uint16_t dollars;
    uint8_t cents;
} Money;

#define FARE_BUS \
    (Money) {    \
        1, 70    \
    }
#define FARE_SUB \
    (Money) {    \
        2, 40    \
    }

typedef struct {
    DateTime date;
    uint16_t gate;
    uint8_t g_flag;
    Money fare;
    uint8_t f_flag;
} Trip;

// IdMapping approach borrowed from Jeremy Cooper's 'clipper.c'
typedef struct {
    uint16_t id;
    const char* name;
} IdMapping;

// this should be a complete accounting of types,
static const IdMapping charliecard_types[] = {
    // Regular card types
    {.id = 367, .name = "Adult"},
    {.id = 366, .name = "SV Adult"},
    {.id = 418, .name = "Student"},
    {.id = 419, .name = "Senior"},
    {.id = 420, .name = "Tap"},
    {.id = 417, .name = "Blind"},
    {.id = 426, .name = "Child"},
    {.id = 410, .name = "Employee ID Without Passback"},
    {.id = 414, .name = "Employee ID With Passback"},
    {.id = 415, .name = "Retiree"},
    {.id = 416, .name = "Police/Fire"},

    // Passes
    {.id = 135, .name = "30 Day Local Bus Pass"},
    {.id = 136, .name = "30 Day Inner Express Bus Pass"}, //
    {.id = 137, .name = "30 Day Outer Express Bus Pass"}, //
    {.id = 138, .name = "30 Day LinkPass"}, //
    {.id = 139, .name = "30 Day Senior LinkPass"}, //
    {.id = 148, .name = "30 Day TAP LinkPass"},
    {.id = 150, .name = "Monthly Student LinkPass"},
    {.id = 424, .name = "Monthly TAP LinkPass"}, // 0b0110101000
    {.id = 425, .name = "Monthly Senior LinkPass"}, // 0b0110101001
    {.id = 421, .name = "Senior TAP/Permit"}, // 0b0110100101
    {.id = 422, .name = "Senior TAP/Permit 30 Days"}, // 0b0110100110

    // Commuter rail passes
    {.id = 166, .name = "30 Day Commuter Rail Zone 1A Pass"},
    {.id = 167, .name = "30 Day Commuter Rail Zone 1 Pass"},
    {.id = 168, .name = "30 Day Commuter Rail Zone 2 Pass"},
    {.id = 169, .name = "30 Day Commuter Rail Zone 3 Pass"},
    {.id = 170, .name = "30 Day Commuter Rail Zone 4 Pass"},
    {.id = 171, .name = "30 Day Commuter Rail Zone 5 Pass"},
    {.id = 172, .name = "30 Day Commuter Rail Zone 6 Pass"},
    {.id = 173, .name = "30 Day Commuter Rail Zone 7 Pass"},
    {.id = 174, .name = "30 Day Commuter Rail Zone 8 Pass"},
    {.id = 175, .name = "30 Day Interzone 1 Pass"},
    {.id = 176, .name = "30 Day Interzone 2 Pass"},
    {.id = 177, .name = "30 Day Interzone 3 Pass"},
    {.id = 178, .name = "30 Day Interzone 4 Pass"},
    {.id = 179, .name = "30 Day Interzone 5 Pass"},
    {.id = 180, .name = "30 Day Interzone 6 Pass"},
    {.id = 181, .name = "30 Day Interzone 7 Pass"},
    {.id = 182, .name = "30 Day Interzone 8 Pass"},

    {.id = 140, .name = "One Way Interzone Adult 1 Zone"},
    {.id = 141, .name = "One Way Interzone Adult 2 Zones"},
    {.id = 142, .name = "One Way Interzone Adult 3 Zones"},
    {.id = 143, .name = "One Way Interzone Adult 4 Zones"},
    {.id = 144, .name = "One Way Interzone Adult 5 Zones"},
    {.id = 145, .name = "One Way Interzone Adult 6 Zones"},
    {.id = 146, .name = "One Way Interzone Adult 7 Zones"},
    {.id = 147, .name = "One Way Interzone Adult 8 Zones"},

    {.id = 428, .name = "One Way Half Fare Zone 1"},
    {.id = 429, .name = "One Way Half Fare Zone 2"},
    {.id = 430, .name = "One Way Half Fare Zone 3"},
    {.id = 431, .name = "One Way Half Fare Zone 4"},
    {.id = 432, .name = "One Way Half Fare Zone 5"},
    {.id = 433, .name = "One Way Half Fare Zone 6"},
    {.id = 434, .name = "One Way Half Fare Zone 7"},
    {.id = 435, .name = "One Way Half Fare Zone 8"},
    {.id = 436, .name = "One Way Interzone Half Fare 1 Zone"},
    {.id = 437, .name = "One Way Interzone Half Fare 2 Zones"},
    {.id = 438, .name = "One Way Interzone Half Fare 3 Zones"},
    {.id = 439, .name = "One Way Interzone Half Fare 4 Zones"},
    {.id = 440, .name = "One Way Interzone Half Fare 5 Zones"},
    {.id = 441, .name = "One Way Interzone Half Fare 6 Zones"},
    {.id = 442, .name = "One Way Interzone Half Fare 7 Zones"},
    {.id = 443, .name = "One Way Interzone Half Fare 8 Zones"},

    {.id = 509, .name = "Group Interzone 1 Zones"},
    {.id = 510, .name = "Group Interzone 2 Zones"},
    {.id = 511, .name = "Group Interzone 3 Zones"},
    {.id = 512, .name = "Group Interzone 4 Zones"},
    {.id = 513, .name = "Group Interzone 5 Zones"},
    {.id = 514, .name = "Group Interzone 6 Zones"},
    {.id = 515, .name = "Group Interzone 7 Zones"},
    {.id = 516, .name = "Group Interzone 8 Zones"},

    {.id = 952, .name = "Zone 1 Student Monthly Pass"},
    {.id = 953, .name = "Zone 2 Student Monthly Pass"},
    {.id = 954, .name = "Zone 3 Student Monthly Pass"},
    {.id = 955, .name = "Zone 4 Student Monthly Pass"},
    {.id = 956, .name = "Zone 5 Student Monthly Pass"},
    {.id = 957, .name = "Zone 6 Student Monthly Pass"},
    {.id = 958, .name = "Zone 7 Student Monthly Pass"},
    {.id = 959, .name = "Zone 8 Student Monthly Pass"},
    {.id = 960, .name = "Zone 9 Student Monthly Pass"},
    {.id = 961, .name = "Zone 10 Student Monthly Pass"},

    {.id = 963, .name = "Interzone 1 Zone Student Monthly Pass"},
    {.id = 964, .name = "Interzone 2 Zone Student Monthly Pass"},
    {.id = 965, .name = "Interzone 3 Zone Student Monthly Pass"},
    {.id = 966, .name = "Interzone 4 Zone Student Monthly Pass"},
    {.id = 967, .name = "Interzone 5 Zone Student Monthly Pass"},
    {.id = 968, .name = "Interzone 6 Zone Student Monthly Pass"},
    {.id = 969, .name = "Interzone 7 Zone Student Monthly Pass"},
    {.id = 970, .name = "Interzone 8 Zone Student Monthly Pass"},
    {.id = 971, .name = "Interzone 9 Zone Student Monthly Pass"},
    {.id = 972, .name = "Interzone 10 Zone Student Monthly Pass"},
};
static const size_t kNumTypes = COUNT_OF(charliecard_types);

// Incomplete, and subject to change
// Only covers Orange & Blue line stations
// Gathered manually, and provided courtesy of, DEFCON31 researchers
// as cited above.
static const IdMapping charliecard_fare_gate_ids[] = {
    // Davis
    {.id = 6766, .name = "Davis"},
    {.id = 6767, .name = "Davis"},
    {.id = 6768, .name = "Davis"},
    {.id = 6769, .name = "Davis"},
    {.id = 6770, .name = "Davis"},
    {.id = 6771, .name = "Davis"},
    {.id = 6772, .name = "Davis"},
    {.id = 2167, .name = "Davis"},
    {.id = 7020, .name = "Davis"},
    // Porter
    {.id = 6781, .name = "Porter"},
    {.id = 6780, .name = "Porter"},
    {.id = 6779, .name = "Porter"},
    {.id = 6778, .name = "Porter"},
    {.id = 6777, .name = "Porter"},
    {.id = 6776, .name = "Porter"},
    {.id = 6775, .name = "Porter"},
    {.id = 2168, .name = "Porter"},
    {.id = 7021, .name = "Porter"},
    {.id = 6782, .name = "Porter"},
    // Oak Grove
    {.id = 6640, .name = "Oak Grove"},
    {.id = 6641, .name = "Oak Grove"},
    {.id = 6639, .name = "Oak Grove"},
    {.id = 2036, .name = "Oak Grove"},
    {.id = 6642, .name = "Oak Grove"},
    {.id = 6979, .name = "Oak Grove"},
    // Downtown Crossing
    {.id = 2091, .name = "Downtown Crossing"},
    {.id = 6995, .name = "Downtown Crossing"},
    {.id = 6699, .name = "Downtown Crossing"},
    {.id = 6700, .name = "Downtown Crossing"},
    {.id = 1926, .name = "Downtown Crossing"},
    {.id = 2084, .name = "Downtown Crossing"},
    {.id = 6994, .name = "Downtown Crossing"},
    {.id = 6695, .name = "Downtown Crossing"},
    {.id = 6694, .name = "Downtown Crossing"},
    {.id = 6696, .name = "Downtown Crossing"},
    {.id = 2336, .name = "Downtown Crossing"},
    {.id = 1056, .name = "Downtown Crossing"},
    {.id = 6814, .name = "Downtown Crossing"},
    {.id = 6813, .name = "Downtown Crossing"},
    {.id = 2212, .name = "Downtown Crossing"},
    {.id = 7038, .name = "Downtown Crossing"},
    // State
    {.id = 7092, .name = "State"},
    {.id = 1844, .name = "State"},
    {.id = 6689, .name = "State"},
    {.id = 6988, .name = "State"},
    {.id = 6991, .name = "State"},
    {.id = 2083, .name = "State"},
    {.id = 6688, .name = "State"},
    {.id = 6687, .name = "State"},
    {.id = 6686, .name = "State"},
    {.id = 2078, .name = "State"},
    {.id = 6987, .name = "State"},
    {.id = 7090, .name = "State"},
    {.id = 1842, .name = "State"},
    // Haymarket
    {.id = 6684, .name = "Haymarket"},
    {.id = 6683, .name = "Haymarket"},
    {.id = 6682, .name = "Haymarket"},
    {.id = 6681, .name = "Haymarket"},
    {.id = 2073, .name = "Haymarket"},
    {.id = 7074, .name = "Haymarket"},
    {.id = 6883, .name = "Haymarket"},
    {.id = 6884, .name = "Haymarket"},
    {.id = 6885, .name = "Haymarket"},
    {.id = 6886, .name = "Haymarket"},
    {.id = 2303, .name = "Haymarket"},
    {.id = 6986, .name = "Haymarket"},
    // North Station
    {.id = 6985, .name = "North Station"},
    {.id = 2063, .name = "North Station"},
    {.id = 6671, .name = "North Station"},
    {.id = 6672, .name = "North Station"},
    {.id = 6673, .name = "North Station"},
    {.id = 6674, .name = "North Station"},
    {.id = 6675, .name = "North Station"},
    {.id = 6676, .name = "North Station"},
    {.id = 6677, .name = "North Station"},
    {.id = 6678, .name = "North Station"},
    {.id = 6984, .name = "North Station"},
    {.id = 2062, .name = "North Station"},
    {.id = 6668, .name = "North Station"},
    {.id = 6667, .name = "North Station"},
    {.id = 6666, .name = "North Station"},
    {.id = 6665, .name = "North Station"},
    {.id = 6664, .name = "North Station"},
    // Sullivan Square
    {.id = 6654, .name = "Sullivan Square"},
    {.id = 6655, .name = "Sullivan Square"},
    {.id = 6656, .name = "Sullivan Square"},
    {.id = 6657, .name = "Sullivan Square"},
    {.id = 6658, .name = "Sullivan Square"},
    {.id = 6659, .name = "Sullivan Square"},
    {.id = 2053, .name = "Sullivan Square"},
    {.id = 6982, .name = "Sullivan Square"},
    // Community College
    {.id = 6661, .name = "Community College"},
    {.id = 6662, .name = "Community College"},
    {.id = 2056, .name = "Community College"},
    {.id = 6983, .name = "Community College"},
    // Assembly
    {.id = 3876, .name = "Assembly"},
    {.id = 3875, .name = "Assembly"},
    {.id = 6957, .name = "Assembly"},
    {.id = 6956, .name = "Assembly"},
    {.id = 6955, .name = "Assembly"},
    {.id = 6954, .name = "Assembly"},
    {.id = 6953, .name = "Assembly"},
    {.id = 7101, .name = "Assembly"},
    {.id = 3873, .name = "Assembly"},
    {.id = 3872, .name = "Assembly"},
    // Wellington
    {.id = 6981, .name = "Wellington"},
    {.id = 2042, .name = "Wellington"},
    {.id = 6650, .name = "Wellington"},
    {.id = 6651, .name = "Wellington"},
    {.id = 6652, .name = "Wellington"},
    {.id = 6653, .name = "Wellington"},
    // Malden
    {.id = 6980, .name = "Malden Center"},
    {.id = 2037, .name = "Malden Center"},
    {.id = 6645, .name = "Malden Center"},
    {.id = 6646, .name = "Malden Center"},
    {.id = 6647, .name = "Malden Center"},
    {.id = 6648, .name = "Malden Center"},
    // Chinatown
    {.id = 6704,
     .name =
         "Malden Center"}, // Entry error? Placed after "Chinatown" divider, but with name Malden Center
    {.id = 6705, .name = "Chinatown"},
    {.id = 2099, .name = "Chinatown"},
    {.id = 7003, .name = "Chinatown"},
    {.id = 7002, .name = "Chinatown"},
    {.id = 2096, .name = "Chinatown"},
    {.id = 6702, .name = "Chinatown"},
    {.id = 6701, .name = "Chinatown"},
    // Tufts Medical Center
    {.id = 6707, .name = "Tufts Medical Center"},
    {.id = 6708, .name = "Tufts Medical Center"},
    {.id = 6709, .name = "Tufts Medical Center"},
    {.id = 6710, .name = "Tufts Medical Center"},
    {.id = 6711, .name = "Tufts Medical Center"},
    {.id = 2105, .name = "Tufts Medical Center"},
    {.id = 7004, .name = "Tufts Medical Center"},
    {.id = 1941, .name = "Tufts Medical Center"},
    {.id = 7006, .name = "Tufts Medical Center"},
    // Back Bay
    {.id = 7007, .name = "Back Bay"},
    {.id = 1480, .name = "Back Bay"},
    {.id = 6714, .name = "Back Bay"},
    {.id = 6715, .name = "Back Bay"},
    {.id = 6716, .name = "Back Bay"},
    {.id = 6717, .name = "Back Bay"},
    {.id = 6718, .name = "Back Bay"},
    {.id = 6719, .name = "Back Bay"},
    {.id = 6720, .name = "Back Bay"},
    {.id = 1801, .name = "Back Bay"},
    {.id = 7009, .name = "Back Bay"},
    // Massachusetts Avenue
    {.id = 7010, .name = "Massachusetts Avenue"},
    {.id = 2118, .name = "Massachusetts Avenue"},
    {.id = 6724, .name = "Massachusetts Avenue"},
    {.id = 6723, .name = "Massachusetts Avenue"},
    {.id = 6722, .name = "Massachusetts Avenue"},
    {.id = 6721, .name = "Massachusetts Avenue"},
    // Ruggles
    {.id = 6726, .name = "Ruggles"},
    {.id = 6727, .name = "Ruggles"},
    {.id = 6728, .name = "Ruggles"},
    {.id = 2122, .name = "Ruggles"},
    {.id = 2123, .name = "Ruggles"},
    {.id = 2124, .name = "Ruggles"},
    {.id = 1804, .name = "Ruggles"},
    // Roxbury Crossing
    {.id = 6737, .name = "Roxbury Crossing"},
    {.id = 6736, .name = "Roxbury Crossing"},
    {.id = 6735, .name = "Roxbury Crossing"},
    {.id = 6734, .name = "Roxbury Crossing"},
    {.id = 6733, .name = "Roxbury Crossing"},
    {.id = 2125, .name = "Roxbury Crossing"},
    {.id = 7012, .name = "Roxbury Crossing"},
    // Jackson Square
    {.id = 6741, .name = "Jackson Square"},
    {.id = 6740, .name = "Jackson Square"},
    {.id = 6739, .name = "Jackson Square"},
    {.id = 2131, .name = "Jackson Square"},
    {.id = 7013, .name = "Jackson Square"},
    {.id = 7014, .name = "Jackson Square"},
    {.id = 2135, .name = "Jackson Square"},
    {.id = 6743, .name = "Jackson Square"},
    {.id = 6744, .name = "Jackson Square"},
    {.id = 6745, .name = "Jackson Square"},
    // Green Street
    {.id = 6746, .name = "Green Street"},
    {.id = 6747, .name = "Green Street"},
    {.id = 6748, .name = "Green Street"},
    {.id = 2142, .name = "Green Street"},
    {.id = 7015, .name = "Green Street"},
    // Forest Hills
    {.id = 6750, .name = "Forest Hills"},
    {.id = 6751, .name = "Forest Hills"},
    {.id = 6752, .name = "Forest Hills"},
    {.id = 6753, .name = "Forest Hills"},
    {.id = 6754, .name = "Forest Hills"},
    {.id = 6755, .name = "Forest Hills"},
    {.id = 2150, .name = "Forest Hills"},
    {.id = 7016, .name = "Forest Hills"},
    {.id = 6950, .name = "Forest Hills"},
    {.id = 6951, .name = "Forest Hills"},
    {.id = 604, .name = "Forest Hills"}, // Entry error?
    {.id = 7096, .name = "Forest Hills"},
    // South Station
    {.id = 7039, .name = "South Station"},
    {.id = 2215, .name = "South Station"},
    {.id = 6816, .name = "South Station"},
    {.id = 6817, .name = "South Station"},
    {.id = 6818, .name = "South Station"},
    {.id = 6819, .name = "South Station"},
    {.id = 6820, .name = "South Station"},
    {.id = 6821, .name = "South Station"},
    {.id = 6822, .name = "South Station"},
    {.id = 6823, .name = "South Station"},
    {.id = 7040, .name = "South Station"},
    {.id = 2228, .name = "South Station"},
    {.id = 6827, .name = "South Station"},
    {.id = 6826, .name = "South Station"},
    {.id = 6825, .name = "South Station"},
    {.id = 6824, .name = "South Station"},
    // Courthouse
    {.id = 6929, .name = "Courthouse"},
    {.id = 2357, .name = "Courthouse"},
    {.id = 7079, .name = "Courthouse"},
    {.id = 6933, .name = "Courthouse"},
    {.id = 6932, .name = "Courthouse"},
    {.id = 2358, .name = "Courthouse"},
    {.id = 6792, .name = "Courthouse"},
    // Bowdoin
    {.id = 6937, .name = "Bowdoin"},
    {.id = 2367, .name = "Bowdoin"},
    {.id = 7085, .name = "Bowdoin"},
    // Government Center
    {.id = 6963, .name = "Government Center"},
    {.id = 6962, .name = "Government Center"},
    {.id = 6961, .name = "Government Center"},
    {.id = 6960, .name = "Government Center"},
    {.id = 6959, .name = "Government Center"},
    {.id = 6958, .name = "Government Center"},
    {.id = 5298, .name = "Government Center"},
    // Aquarium
    {.id = 6609, .name = "Aquarium"},
    {.id = 6608, .name = "Aquarium"},
    {.id = 1877, .name = "Aquarium"},
    {.id = 6965, .name = "Aquarium"},
    {.id = 6610, .name = "Aquarium"},
    {.id = 1880, .name = "Aquarium"},
    {.id = 1871, .name = "Aquarium"},
    {.id = 6966, .name = "Aquarium"},
    // Maverick
    {.id = 7088, .name = "Maverick"},
    {.id = 6944, .name = "Maverick"},
    {.id = 4384, .name = "Maverick"},
    {.id = 6946, .name = "Maverick"},
    {.id = 6947, .name = "Maverick"},
    {.id = 6948, .name = "Maverick"},
    {.id = 6949, .name = "Maverick"},
    {.id = 1840, .name = "Maverick"},
    {.id = 7083, .name = "Maverick"},
    // Airport
    {.id = 6613, .name = "Airport"},
    {.id = 6612, .name = "Airport"},
    {.id = 6611, .name = "Airport"},
    {.id = 6968, .name = "Airport"},
    {.id = 2009, .name = "Airport"},
    {.id = 6616, .name = "Airport"},
    {.id = 6615, .name = "Airport"},
    {.id = 6614, .name = "Airport"},
    {.id = 6970, .name = "Airport"},
    {.id = 1847, .name = "Airport"},
    // Wood Island
    {.id = 6618, .name = "Wood Island"},
    {.id = 6619, .name = "Wood Island"},
    {.id = 2010, .name = "Wood Island"},
    {.id = 6971, .name = "Wood Island"},
    // Orient Heights
    {.id = 6621, .name = "Orient Heights"}, // marked as needs checking
    {.id = 6622, .name = "Orient Heights"},
    {.id = 6623, .name = "Orient Heights"},
    {.id = 2014, .name = "Orient Heights"},
    {.id = 6972, .name = "Orient Heights"},
    {.id = 6974, .name = "Orient Heights"},
    {.id = 1868, .name = "Orient Heights"},
    // Suffolk Downs
    {.id = 6625, .name = "Suffolk Downs"},
    {.id = 6626, .name = "Suffolk Downs"},
    {.id = 2017, .name = "Suffolk Downs"},
    {.id = 6975, .name = "Suffolk Downs"},
    // Beachmont
    {.id = 6628, .name = "Beachmont"},
    {.id = 6629, .name = "Beachmont"},
    {.id = 6630, .name = "Beachmont"},
    {.id = 2021, .name = "Beachmont"},
    {.id = 6976, .name = "Beachmont"},
    // Revere Beach
    {.id = 6632, .name = "Revere Beach"},
    {.id = 6633, .name = "Revere Beach"},
    {.id = 2024, .name = "Revere Beach"},
    {.id = 6977, .name = "Revere Beach"},
    // Wonderland
    {.id = 6638, .name = "Wonderland"},
    {.id = 6637, .name = "Wonderland"},
    {.id = 6636, .name = "Wonderland"},
    {.id = 2025, .name = "Wonderland"},
    {.id = 6978, .name = "Wonderland"},
};
static const size_t kNumFareGateIds = COUNT_OF(charliecard_fare_gate_ids);

static const uint8_t*
    pos_to_ptr(const MfClassicData* data, uint8_t sector_num, uint8_t block_num, uint8_t byte_num) {
    uint8_t block_offset = mf_classic_get_first_block_num_of_sector(sector_num);
    return &data->block[block_offset + block_num].data[byte_num];
}

static uint64_t pos_to_num(
    const MfClassicData* data,
    uint8_t sector_num,
    uint8_t block_num,
    uint8_t byte_num,
    uint8_t byte_len) {
    return bit_lib_bytes_to_num_be(pos_to_ptr(data, sector_num, block_num, byte_num), byte_len);
}

static DateTime dt_delta(DateTime dt, uint64_t delta_secs) {
    DateTime dt_shifted = {0};
    datetime_timestamp_to_datetime(datetime_datetime_to_timestamp(&dt) + delta_secs, &dt_shifted);

    return dt_shifted;
}

static bool get_map_item(uint16_t id, const IdMapping* map, size_t sz, const char** out) {
    // code borrowed from Jeremy Cooper's 'clipper.c'. Used as follows:
    // const char* s; if(!get_map_item(_,_,_,&s)) {s="Default str";}
    // TODO: change to furistring out?
    for(size_t i = 0; i < sz; i++) {
        if(map[i].id == id) {
            *out = map[i].name;
            return true;
        }
    }

    return false;
}

static bool charliecard_verify(Nfc* nfc) {
    // does this suffice? Or should I check add'l keys/data/etc?
    bool verified = false;

    do {
        const uint8_t verify_sector = 1;
        const uint8_t verify_block = mf_classic_get_first_block_num_of_sector(verify_sector) + 1;
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(
            charliecard_1k_keys[verify_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, verify_block, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", verify_block, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool charliecard_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        if(type != MfClassicType1k) break;

        MfClassicDeviceKeys keys = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                charliecard_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                charliecard_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

uint32_t time_now() {
    return furi_hal_rtc_get_timestamp();
}

static Money money_parse(
    const MfClassicData* data,
    uint8_t sector_num,
    uint8_t block_num,
    uint8_t byte_num) {
    // CharlieCards store all money values in two bytes as half-cents
    // bitmask removes sign/flag, bitshift converts half-cents to cents, div & mod yield dollars & cents
    uint16_t amt = (pos_to_num(data, sector_num, block_num, byte_num, 2) & 0x7FFF) >> 1;
    return (Money){amt / 100, amt % 100};
}

static DateTime
    date_parse(const MfClassicData* data, uint8_t sector_num, uint8_t block_num, uint8_t byte_num) {
    // Dates are 3 bytes, in minutes since 2003/1/1 ("CHARLIE_EPOCH")
    uint32_t ts_charlie = pos_to_num(data, sector_num, block_num, byte_num, 3);
    return dt_delta(CHARLIE_EPOCH, ts_charlie * CHARLIE_TIME_DELTA_SECS);
}

static DateTime
    end_validity_parse(const MfClassicData* data, enum CharlieActiveSector active_sec) {
    // End validity field is a bit odd; shares byte 1 with another variable (the card type field),
    // occupying only the last 3 bits (and subsequent two bytes), hence bitmask
    // TODO; what are the add'l 3 bits between type & end validity fields?
    uint32_t ts_charlie_ev =
        pos_to_num(data, (active_sec == CHARLIE_ACTIVE_SECTOR_2) ? 2 : 3, 1, 1, 3);
    ts_charlie_ev = ts_charlie_ev & 0x1FFFFF;

    // additionally, instead of minute deltas, is in 8 minute increments
    // relative to CHARLIE_EPOCH (2003/1/1), per DEFCON31 researcher's work
    return dt_delta(CHARLIE_EPOCH, ts_charlie_ev * CHARLIE_END_VALID_DELTA_SECS);
}

static Trip
    trip_parse(const MfClassicData* data, uint8_t sector_num, uint8_t block_num, uint8_t byte_num) {
    /* This function parses individual trips. Each trip packs 7 bytes, stored as follows:

           0    1    2    3    4    5    6    
           +----.----.----+----.--+-+----.----+
           |     date     |   loc |f|   amt   |
           +----.----.----+----.--+-+----.----+

    Where date is in the typical format, loc represents the fare gate tapped, and amt is the fare amount.
    Amount appears to contain some flag bits, however, it is unclear what precisely their function is.

    Gate ID ("loc") is only the first 13 bits of 0x3:0x5, the final three bits appear to be flags ("f").
    Least significant flag bit (ie "loc & 0x1") seems to indicate:
    — When 0, fare (the amount by which balance is decremented)
    — When 1, refill (the amount by which balance is incremented)

    On monthly pass cards, MSB of amt will be set: 0x8000 (negative zero)
    Seemingly randomly (irrespective of card type, last trip, etc) 0x0001 will be set on amt in addition to 
    whatever the regular fare is (a half cent more). I am uncertain what this flag indicates.
    */
    const DateTime date = date_parse(data, sector_num, block_num, byte_num);
    const uint16_t gate = pos_to_num(data, sector_num, block_num, byte_num + 3, 2) >> 3;
    const uint8_t g_flag = pos_to_num(data, sector_num, block_num, byte_num + 3, 2) & 0b111;
    const Money fare = money_parse(data, sector_num, block_num, byte_num + 5);
    const uint8_t f_flag = pos_to_num(data, sector_num, block_num, byte_num + 5, 2) & 0x8001;
    return (Trip){date, gate, g_flag, fare, f_flag};
}

static bool date_ge(DateTime dt1, DateTime dt2) {
    return datetime_datetime_to_timestamp(&dt1) >= datetime_datetime_to_timestamp(&dt2);
}

static Trip* trips_parse(const MfClassicData* data) {
    /* Sectors 6 & 7 store the last 10 trips. Overall layout as follows:

           0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
           +----.----.----.----.----.----.----+----.----.----.----.----.----.----+----.----+
    0x180  |               trip0              |               trip1              |   crc1  |
           +----.----.----.----.----.----.----+----.----.----.----.----.----.----+----.----+
    ...                     ...                                ...                   ...
           +----.----.----.----.----.----.----+----.----.----.----.----.----.----+----.----+
    0x1D0  |               trip8              |               trip9              |   crc5  |
           +----.----.----.----.----.----.----+----.----.----.----.----.----.----+----.----+
    0x1E0  |                                empty                                |   crc6  |
           +----.----.----.----.----.----.----.----.----.----.----.----.----.----+----.----+
    
    "empty" is all 0s. Trips are not sorted, rather, appear to get overwritten sequentially. (eg, sorted modulo array rotation)
    */
    Trip* trips = malloc(sizeof(Trip) * CHARLIE_N_TRIP_HISTORY);

    // Parse each trip field using some modular math magic to get the offsets:
    // move from sector 6 -> 7 after the first 6 trips
    // move a block within a given sector every 2 trips, reset every 3 blocks (as sector has changed)
    // alternate between a start byte of 0 and 7 with every iteration
    for(size_t i = 0; i < CHARLIE_N_TRIP_HISTORY; i++) {
        trips[i] = trip_parse(data, 6 + (i / 6), (i / 2) % 3, (i % 2) * 7);
    }

    // Iterate through the array to find the maximum (newest) date value
    int max_idx = 0;
    for(int i = 1; i < CHARLIE_N_TRIP_HISTORY; i++) {
        if(date_ge(trips[i].date, trips[max_idx].date)) {
            max_idx = i;
        }
    }

    // Sort by rotating
    for(int r = 0; r < (max_idx + 1); r++) {
        // Store the first element
        Trip temp = trips[0];
        // Shift elements to the left
        for(int i = 0; i < CHARLIE_N_TRIP_HISTORY - 1; i++) {
            trips[i] = trips[i + 1];
        }
        // Move the first element to the last
        trips[CHARLIE_N_TRIP_HISTORY - 1] = temp;
    }

    // Reverse order, such that newest is first, oldest last
    for(int i = 0; i < CHARLIE_N_TRIP_HISTORY / 2; i++) {
        // Swap elements at index i and size - i - 1
        Trip temp = trips[i];
        trips[i] = trips[CHARLIE_N_TRIP_HISTORY - i - 1];
        trips[CHARLIE_N_TRIP_HISTORY - i - 1] = temp;
    }

    return trips;
}

static uint16_t n_uses(const MfClassicData* data, const enum CharlieActiveSector active_sector) {
    /* First two bytes of applicable block (sector 1, block 1 or 2 depending on active_sector)
    The *lower* of the two values *minus one* is the true use count,
    per DEFCON31 researcher's findings
    */
    return pos_to_num(data, 1, 1 + active_sector, 0, 2) - 1;
}

static enum CharlieActiveSector get_active_sector(const MfClassicData* data) {
    /* Card has two transaction sectors (2 & 3) containing balance data, with two
    corresponding trip counters in 0x50:0x51 & 0x60:0x61 (sector 1, byte 0:1 of blocks 1 & 2).

    The *lower* count variable corresponds to the active sector 
    (0x5_ lower -> 2 active, 0x6_ lower -> 3 active)

    Sectors 2 & 3 are (largely) identical, save for trip data.
    Card seems to alternate between the two, with active sector storing 
    the current balance & recent trip/transaction, & the inactive sector storing 
    the N-1 trip/transaction version of the same data.

    Here I check both the trip count and the stored transaction date,
    for my own sanity, to confirm the active sector.
    */

    // active sector based on trip counters
    const bool active_trip = n_uses(data, CHARLIE_ACTIVE_SECTOR_2) <=
                             n_uses(data, CHARLIE_ACTIVE_SECTOR_3);

    // active sector based on transaction date
    DateTime ds2 = date_parse(data, 2, 0, 1);
    DateTime ds3 = date_parse(data, 3, 0, 1);
    const bool active_date = datetime_datetime_to_timestamp(&ds2) >=
                             datetime_datetime_to_timestamp(&ds3);

    // with all tested cards so far, this has been true
    furi_assert(active_trip == active_date);

    return active_trip ? CHARLIE_ACTIVE_SECTOR_2 : CHARLIE_ACTIVE_SECTOR_3;
}

static uint16_t type_parse(const MfClassicData* data) {
    /* Card type data stored in the first 10bits of block 1 of sectors 2 & 3 (Block 9 & Block 13, from card start)
    To my knowledge, card type should never change, so we can check either
    without caring which is active. For my sanity, I check both, and assert equal.
    */

    // bitshift (2bytes = 16 bits) by 6bits for just first 10bits
    const uint16_t type1 = pos_to_num(data, 2, 1, 0, 2) >> 6;
    const uint16_t type2 = pos_to_num(data, 3, 1, 0, 2) >> 6;
    furi_assert(type1 == type2);

    return type1;
}

/*
static DateTime expiry(DateTime iss) {
    // Per Metrodroid CharlieCard parser (https://github.com/metrodroid/metrodroid/blob/master/src/commonMain/kotlin/au/id/micolous/metrodroid/transit/charlie/CharlieCardTransitData.kt)
    // Expiry not explicitly stored in card data; rather, calculated from date of issue
    // Cards were first issued in 2006, expired in 5 years, w/ no printed expiry date
    // Cards issued after 2011 expire in 10 years
    //
    // Per DEFCON31 researcher's work (cited above):
    // Student cards last one school year and expire at the end of August the following year
    // Pre-2011 issued cards expire in 7 years, not 5 as claimed by Metrodroid
    // Post-2011 expire in 10 years, less one day
    // Redundant function given the existance of the end validity field?
    // Any important distinctions between the two?
    

    // perhaps additionally clipping to 2030-12-__ in anticipation of upcoming system migration?
    // need to get a new card to confirm.

    // TODO add card type logic for student card expiry
    DateTime exp;
    if(iss.year < 2011) {
        // add 7 years; assumes average year of 8766 hrs (to account for leap years)
        // may be off by a few hours as a result
        exp = dt_delta(iss, 7 * 8766 * 60 * 60);
    } else {
        // add 10 years, subtract a day. Same assumption as above
        exp = dt_delta(iss, ((10 * 8766) - 24) * 60 * 60);
    }

    return exp;
}*/

static bool expired(DateTime expiry, DateTime last_trip) {
    // if a card has sat unused for >2 years, expired (verify this claim?)
    // else expired if current date > expiry date

    uint32_t ts_exp = datetime_datetime_to_timestamp(&expiry);
    uint32_t ts_last = datetime_datetime_to_timestamp(&last_trip);
    uint32_t ts_now = time_now();

    return (ts_exp <= ts_now) | ((ts_now - ts_last) >= (2 * 365 * 24 * 60 * 60));
}

void locale_format_dt_cat(FuriString* out, const DateTime* dt) {
    // helper to print datetimes
    FuriString* s = furi_string_alloc();

    LocaleDateFormat date_format = locale_get_date_format();
    const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";
    locale_format_date(s, dt, date_format, separator);
    furi_string_cat(out, s);
    locale_format_time(s, dt, locale_get_time_format(), false);
    furi_string_cat_printf(out, "  ");
    furi_string_cat(out, s);

    furi_string_free(s);
}

void type_format_cat(FuriString* out, uint16_t type) {
    const char* s;
    if(!get_map_item(type, charliecard_types, kNumTypes, &s)) {
        s = "";
        furi_string_cat_printf(out, "Unknown-%u", type);
    }

    furi_string_cat_str(out, s);
}

void money_format_cat(FuriString* out, Money money) {
    furi_string_cat_printf(out, "$%u.%02u", money.dollars, money.cents);
}

void trip_format_cat(FuriString* out, Trip trip) {
    const char* sep = "   ";
    const char* sta;

    locale_format_dt_cat(out, &trip.date);
    furi_string_cat_printf(out, "\n%s", !!(trip.g_flag & 0x1) ? "-" : "+");
    money_format_cat(out, trip.fare);
    if(!!(trip.g_flag & 0x1) && (trip.fare.dollars == FARE_BUS.dollars) &&
       (trip.fare.cents == FARE_BUS.cents)) {
        // if not a refill, and the fare amount is equal to bus fare (any better approach? flag bits for modality?)
        // format for bus (gate ID on busses = posted bus #)
        furi_string_cat_printf(out, "%sBus#%u", sep, trip.gate);
    } else if(get_map_item(trip.gate, charliecard_fare_gate_ids, kNumFareGateIds, &sta)) {
        // station found in fare gate ID map, append station name
        furi_string_cat_str(out, sep);
        furi_string_cat_str(out, sta);
    } else {
        // no found station in fare gate ID map & not a bus, just print ID w/o add'l info
        furi_string_cat_printf(out, "%s%u", sep, trip.gate);
    }
    // print flags for debugging purposes
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        furi_string_cat_printf(out, "%s%u%s%u", sep, trip.g_flag, sep, trip.f_flag);
    }
}

static bool charliecard_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        if(data->type != MfClassicType1k) break;

        // Verify key
        // arbitrary sector in the main data portion
        const uint8_t verify_sector = 3;
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);

        const uint64_t key_a =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        const uint64_t key_b =
            bit_lib_bytes_to_num_be(sec_tr->key_b.data, COUNT_OF(sec_tr->key_b.data));
        if(key_a != charliecard_1k_keys[verify_sector].a) break;
        if(key_b != charliecard_1k_keys[verify_sector].b) break;

        // TODO: Verify add'l?

        const enum CharlieActiveSector active_sec_enum = get_active_sector(data);
        const uint8_t active_sector = (active_sec_enum == CHARLIE_ACTIVE_SECTOR_2) ? 2 : 3;

        furi_string_cat_printf(parsed_data, "\e#CharlieCard");

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        uint32_t card_number = bit_lib_bytes_to_num_be(uid, 4);
        furi_string_cat_printf(parsed_data, "\nSerial: 5-%lu", card_number);

        Money bal = money_parse(data, active_sector, 1, 5);
        furi_string_cat_printf(parsed_data, "\nBal: ");
        money_format_cat(parsed_data, bal);

        const uint16_t type = type_parse(data);
        furi_string_cat_printf(parsed_data, "\nType: ");
        type_format_cat(parsed_data, type);

        const uint16_t n_trips = n_uses(data, active_sec_enum);
        furi_string_cat_printf(parsed_data, "\nTrip Count: %u", n_trips);

        const DateTime iss = date_parse(data, active_sector, 0, 6);
        furi_string_cat_printf(parsed_data, "\nIssued: ");
        locale_format_dt_cat(parsed_data, &iss);

        const DateTime e_v = end_validity_parse(data, active_sec_enum);
        furi_string_cat_printf(parsed_data, "\nExpiry: ");
        locale_format_dt_cat(parsed_data, &e_v);

        DateTime last = date_parse(data, active_sector, 0, 1);
        furi_string_cat_printf(parsed_data, "\nExpired: %s", expired(e_v, last) ? "Yes" : "No");

        Trip* trips = trips_parse(data);
        furi_string_cat_printf(parsed_data, "\nTransactions:");
        for(size_t i = 0; i < CHARLIE_N_TRIP_HISTORY; i++) {
            furi_string_cat_printf(parsed_data, "\n");
            trip_format_cat(parsed_data, trips[i]);
            furi_string_cat_printf(parsed_data, "\n");
        }
        free(trips);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin charliecard_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = charliecard_verify,
    .read = charliecard_read,
    .parse = charliecard_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor charliecard_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &charliecard_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* charliecard_plugin_ep(void) {
    return &charliecard_plugin_descriptor;
}
