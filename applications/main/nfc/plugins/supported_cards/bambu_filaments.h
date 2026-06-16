// Ported from https://github.com/uzyn/flipper-bambu (GPL-3.0, (c) U-Zyn Chua)
//
// Bambu Lab Filament Lookup Table
// Maps variant IDs to filament codes and color names
// Source: https://github.com/queengooborg/Bambu-Lab-RFID-Library
//
// This file can be updated independently as new filaments are released.
// To add a new filament: add an entry to bambu_filament_table[] with:
//   { "VARIANT_ID", "5-DIGIT-CODE", "Color Name" }

#ifndef BAMBU_FILAMENTS_H
#define BAMBU_FILAMENTS_H

typedef struct {
    const char* variant_id;     // e.g., "A00-R3"
    const char* filament_code;  // e.g., "10204"
    const char* color_name;     // e.g., "Hot Pink"
} BambuFilamentInfo;

// Lookup table - sorted by variant_id for easier maintenance
static const BambuFilamentInfo bambu_filament_table[] = {
    // PLA Basic (A00-xxx) - Material ID: GFA00
    {"A00-A0", "10300", "Orange"},
    {"A00-A1", "10301", "Pumpkin Orange"},
    {"A00-B1", "10602", "Blue Grey"},
    {"A00-B3", "10604", "Cobalt Blue"},
    {"A00-B4", "10601", "Blue"},
    {"A00-B5", "10605", "Turquoise"},
    {"A00-B8", "10603", "Cyan"},
    {"A00-D0", "10103", "Gray"},
    {"A00-D1", "10102", "Silver"},
    {"A00-D2", "10104", "Light Gray"},
    {"A00-D3", "10105", "Dark Gray"},
    {"A00-G1", "10501", "Bambu Green"},
    {"A00-G2", "10502", "Mistletoe Green"},
    {"A00-G3", "10503", "Bright Green"},
    {"A00-G6", "10501", "Bambu Green"},
    {"A00-K0", "10101", "Black"},
    {"A00-M0", "10900", "Arctic Whisper"},
    {"A00-M1", "10901", "Solar Breeze"},
    {"A00-M2", "10902", "Ocean to Meadow"},
    {"A00-M3", "10903", "Pink Citrus"},
    {"A00-M4", "10904", "Mint Lime"},
    {"A00-M5", "10905", "Blueberry Bubblegum"},
    {"A00-M6", "10906", "Dusk Glare"},
    {"A00-M7", "10907", "Cotton Candy Cloud"},
    {"A00-N0", "10800", "Brown"},
    {"A00-N1", "10802", "Cocoa Brown"},
    {"A00-P0", "10201", "Beige"},
    {"A00-P2", "10701", "Indigo Purple"},
    {"A00-P5", "10700", "Purple"},
    {"A00-P6", "10202", "Magenta"},
    {"A00-P7", "10203", "Pink"},
    {"A00-R0", "10200", "Red"},
    {"A00-R2", "10205", "Maroon Red"},
    {"A00-R3", "10204", "Hot Pink"},
    {"A00-W1", "10100", "Jade White"},
    {"A00-Y0", "10400", "Yellow"},
    {"A00-Y2", "10402", "Sunflower Yellow"},
    {"A00-Y3", "10801", "Bronze"},
    {"A00-Y4", "10401", "Gold"},

    // PLA Matte (A01-xxx) - Material ID: GFA01
    {"A01-A2", "11300", "Mandarin Orange"},
    {"A01-B0", "11603", "Sky Blue"},
    {"A01-B3", "11600", "Marine Blue"},
    {"A01-B4", "11601", "Ice Blue"},
    {"A01-B6", "11602", "Dark Blue"},
    {"A01-D0", "11104", "Nardo Gray"},
    {"A01-D3", "11102", "Ash Gray"},
    {"A01-G0", "11502", "Apple Green"},
    {"A01-G1", "11500", "Grass Green"},
    {"A01-G7", "11501", "Dark Green"},
    {"A01-K1", "11101", "Charcoal"},
    {"A01-N0", "11802", "Dark Chocolate"},
    {"A01-N1", "11800", "Latte Brown"},
    {"A01-N2", "11801", "Dark Brown"},
    {"A01-N3", "11803", "Caramel"},
    {"A01-P3", "11201", "Sakura Pink"},
    {"A01-P4", "11700", "Lilac Purple"},
    {"A01-R1", "11200", "Scarlet Red"},
    {"A01-R2", "11203", "Terracotta"},
    {"A01-R3", "11204", "Plum"},
    {"A01-R4", "11202", "Dark Red"},
    {"A01-W2", "11100", "Ivory White"},
    {"A01-W3", "11103", "Bone White"},
    {"A01-Y2", "11400", "Lemon Yellow"},
    {"A01-Y3", "11401", "Desert Tan"},

    // PLA Metal (A02-xxx) - Material ID: GFA02
    {"A02-B2", "13600", "Cobalt Blue Metallic"},
    {"A02-D2", "13100", "Iron Gray Metallic"},
    {"A02-G2", "13500", "Oxide Green Metallic"},
    {"A02-N3", "13800", "Copper Brown Metallic"},
    {"A02-Y1", "13400", "Iridium Gold Metallic"},

    // PLA Silk+ (A06-xxx) - Material ID: GFA06
    {"A06-B0", "13603", "Baby Blue"},
    {"A06-B1", "13604", "Blue"},
    {"A06-D0", "13108", "Titan Gray"},
    {"A06-D1", "13109", "Silver"},
    {"A06-G0", "13506", "Candy Green"},
    {"A06-G1", "13507", "Mint"},
    {"A06-P0", "13702", "Purple"},
    {"A06-R0", "13205", "Candy Red"},
    {"A06-R1", "13206", "Rose Gold"},
    {"A06-R2", "13207", "Pink"},
    {"A06-W0", "13110", "White"},
    {"A06-Y0", "13404", "Champagne"},
    {"A06-Y1", "13405", "Gold"},

    // PLA Silk Multi-Color (A05-xxx) - Material ID: GFA05
    {"A05-M1", "13906", "South Beach"},
    {"A05-M4", "13909", "Aurora Purple"},
    {"A05-M8", "13912", "Dawn Radiance"},
    {"A05-T1", "13901", "Gilded Rose"},
    {"A05-T2", "13902", "Midnight Blaze"},
    {"A05-T3", "13903", "Neon City"},
    {"A05-T4", "13904", "Blue Hawaii"},
    {"A05-T5", "13905", "Velvet Eclipse"},

    // PLA Marble (A07-xxx) - Material ID: GFA07
    {"A07-D4", "13103", "White Marble"},
    {"A07-R5", "13201", "Red Granite"},

    // PLA Sparkle (A08-xxx) - Material ID: GFA08
    {"A08-B7", "13700", "Royal Purple Sparkle"},
    {"A08-D5", "13102", "Slate Gray Sparkle"},
    {"A08-G3", "13501", "Alpine Green Sparkle"},
    {"A08-K2", "13101", "Onyx Black Sparkle"},
    {"A08-R2", "13200", "Crimson Red Sparkle"},
    {"A08-Y1", "13402", "Classic Gold Sparkle"},

    // PLA Tough (A09-xxx) - Material ID: GFA09
    {"A09-A0", "12002", "Orange"},
    {"A09-B4", "12004", "Light Blue"},
    {"A09-B5", "12005", "Lavender Blue"},
    {"A09-D1", "12001", "Silver"},
    {"A09-R3", "12003", "Vermilion Red"},
    {"A09-Y0", "12000", "Yellow"},

    // PLA Tough+ (A10-xxx) - Material ID: GFA10
    {"A10-D0", "12105", "Gray"},
    {"A10-K0", "12104", "Black"},
    {"A10-W0", "12107", "White"},

    // PLA Aero (A11-xxx) - Material ID: GFA11
    {"A11-K0", "14103", "Black"},
    {"A11-W0", "14102", "White"},

    // PLA Glow (A12-xxx) - Material ID: GFA12
    {"A12-A0", "15300", "Orange"},
    {"A12-B0", "15600", "Blue"},
    {"A12-G0", "15500", "Green"},
    {"A12-R0", "15200", "Pink"},
    {"A12-Y0", "15400", "Yellow"},

    // PLA Galaxy (A15-xxx) - Material ID: GFA15
    {"A15-B0", "13602", "Purple"},
    {"A15-G0", "13503", "Green"},
    {"A15-G1", "13504", "Nebulae"},
    {"A15-R0", "13203", "Brown"},

    // PLA Wood (A16-xxx) - Material ID: GFA16
    {"A16-G0", "13505", "Classic Birch"},
    {"A16-K0", "13107", "Black Walnut"},
    {"A16-N0", "13801", "Clay Brown"},
    {"A16-R0", "13204", "Rosewood"},
    {"A16-W0", "13106", "White Oak"},
    {"A16-Y0", "13403", "Ochre Yellow"},

    // PLA Translucent (A17-xxx) - Material ID: GFA17
    {"A17-A0", "13301", "Orange"},
    {"A17-B1", "13611", "Blue"},
    {"A17-G0", "13510", "Light Jade"},
    {"A17-P0", "13710", "Purple"},
    {"A17-P1", "13711", "Lavender"},
    {"A17-R0", "13210", "Red"},
    {"A17-R1", "13211", "Cherry Pink"},
    {"A17-Y0", "13410", "Mellow Yellow"},

    // PLA Lite (A18-xxx) - Material ID: GFA18
    {"A18-B0", "16600", "Cyan"},
    {"A18-B1", "16601", "Blue"},
    {"A18-D0", "16101", "Gray"},
    {"A18-K0", "16100", "Black"},
    {"A18-P0", "16602", "Matte Beige"},
    {"A18-R0", "16200", "Red"},
    {"A18-W0", "16103", "White"},
    {"A18-Y0", "16400", "Yellow"},

    // PLA-CF (A50-xxx) - Material ID: GFA50
    {"A50-D6", "14101", "Lava Gray"},
    {"A50-K0", "14100", "Black"},

    // ABS (B00-xxx) - Material ID: GFB00
    {"B00-A0", "40300", "Orange"},
    {"B00-B0", "40600", "Blue"},
    {"B00-B4", "40601", "Azure"},
    {"B00-B6", "40602", "Navy Blue"},
    {"B00-D0", "20101", "Gray"},
    {"B00-D1", "40102", "Silver"},
    {"B00-G6", "40500", "Bambu Green"},
    {"B00-G7", "40502", "Olive"},
    {"B00-K0", "40101", "Black"},
    {"B00-R0", "40200", "Red"},
    {"B00-W0", "40100", "White"},
    {"B00-Y1", "40402", "Tangerine Yellow"},

    // ASA (B01-xxx) - Material ID: GFB01
    {"B01-D0", "45102", "Gray"},
    {"B01-K0", "45101", "Black"},
    {"B01-R0", "45200", "Red"},
    {"B01-W0", "45100", "White"},

    // ASA Aero (B02-xxx) - Material ID: GFB02
    {"B02-W0", "46100", "White"},

    // ASA-CF (B51-xxx) - Material ID: GFB51
    {"B51-K0", "46101", "Black"},

    // ABS-GF (B50-xxx) - Material ID: GFB50
    {"B50-A0", "41300", "Orange"},
    {"B50-K0", "41101", "Black"},

    // PC (C00-xxx) - Material ID: GFC00
    {"C00-C0", "60102", "Clear Black"},
    {"C00-C1", "60103", "Transparent"},
    {"C00-K0", "60101", "Black"},
    {"C00-W0", "60100", "White"},

    // PC FR (C01-xxx) - Material ID: GFC01
    {"C01-D0", "63102", "Gray"},
    {"C01-K0", "63100", "Black"},
    {"C01-W0", "63101", "White"},

    // PETG Translucent (G01-xxx) - Material ID: GFG01
    {"G01-A0", "32300", "Translucent Orange"},
    {"G01-B0", "32600", "Translucent Light Blue"},
    {"G01-C0", "32101", "Clear"},
    {"G01-D0", "32100", "Translucent Gray"},
    {"G01-G0", "32500", "Translucent Olive"},
    {"G01-G1", "32501", "Translucent Teal"},
    {"G01-N0", "32800", "Translucent Brown"},
    {"G01-P0", "32700", "Translucent Purple"},
    {"G01-P1", "32200", "Translucent Pink"},

    // PETG HF (G02-xxx) - Material ID: GFG02
    {"G02-A0", "33300", "Orange"},
    {"G02-B0", "33600", "Blue"},
    {"G02-B1", "33601", "Lake Blue"},
    {"G02-D0", "33101", "Gray"},
    {"G02-D1", "33103", "Dark Gray"},
    {"G02-G0", "33500", "Green"},
    {"G02-G1", "33501", "Lime Green"},
    {"G02-G2", "33502", "Forest Green"},
    {"G02-K0", "33102", "Black"},
    {"G02-N1", "33801", "Peanut Brown"},
    {"G02-R0", "33200", "Red"},
    {"G02-W0", "33100", "White"},
    {"G02-Y0", "33400", "Yellow"},
    {"G02-Y1", "33401", "Cream"},

    // PETG-CF (G50-xxx) - Material ID: GFG50
    {"G50-D6", "31101", "Titan Gray"},
    {"G50-G7", "31500", "Malachite Green"},
    {"G50-K0", "31100", "Black"},
    {"G50-P7", "31700", "Violet Purple"},

    // PAHT-CF (N04-xxx) - Material ID: GFN04
    {"N04-K0", "70100", "Black"},

    // PA6-GF (N08-xxx) - Material ID: GFN08
    {"N08-K0", "72104", "Black"},

    // Support for PLA/PETG (S02-xxx) - Material ID: GFS02
    {"S02-W0", "65102", "Nature"},
    {"S02-W1", "65104", "White"},

    // Support for PA/PET (S03-xxx) - Material ID: GFS03
    {"S03-G1", "65500", "Green"},

    // PVA (S04-xxx) - Material ID: GFS04
    {"S04-Y0", "66400", "Clear"},

    // Support (S05-xxx) - Material ID: GFS05
    {"S05-C0", "65103", "Black"},

    // Support for ABS (S06-xxx) - Material ID: GFS06
    {"S06-W0", "66100", "White"},

    // TPU for AMS (U02-xxx) - Material ID: GFU02
    {"U02-B0", "53600", "Blue"},
    {"U02-D0", "53102", "Gray"},
    {"U02-K0", "53101", "Black"},
};

#define BAMBU_FILAMENT_TABLE_SIZE (sizeof(bambu_filament_table) / sizeof(bambu_filament_table[0]))

// Normalize variant IDs that contain padded zeros, e.g.:
// - "A00-K00" -> "A00-K0"
// - "A00-G06" -> "A00-G6"
static inline void
bambu_normalize_variant_id(const char* variant_id, char* normalized, size_t normalized_size) {
    if(normalized_size == 0) {
        return;
    }

    if(variant_id == NULL) {
        normalized[0] = '\0';
        return;
    }

    size_t src_len = strlen(variant_id);
    if(src_len >= normalized_size) {
        src_len = normalized_size - 1;
    }
    memcpy(normalized, variant_id, src_len);
    normalized[src_len] = '\0';

    const char* dash = strchr(normalized, '-');
    if(dash == NULL) {
        return;
    }

    size_t dash_pos = (size_t)(dash - normalized);
    if(dash_pos + 2 >= normalized_size || normalized[dash_pos + 1] == '\0') {
        return;
    }

    char* suffix = &normalized[dash_pos + 2];
    if(*suffix == '\0') {
        return;
    }

    int digits_only = 1;
    for(size_t i = 0; suffix[i] != '\0'; i++) {
        if(suffix[i] < '0' || suffix[i] > '9') {
            digits_only = 0;
            break;
        }
    }
    if(!digits_only) {
        return;
    }

    size_t leading_zeros = 0;
    while(suffix[leading_zeros] == '0' && suffix[leading_zeros + 1] != '\0') {
        leading_zeros++;
    }

    if(leading_zeros > 0) {
        size_t suffix_len = strlen(suffix);
        memmove(suffix, suffix + leading_zeros, suffix_len - leading_zeros + 1);
    }
}

// Lookup function: Find filament info by variant_id
// Returns NULL if not found
static inline const BambuFilamentInfo* bambu_lookup_filament(const char* variant_id) {
    if(variant_id == NULL || variant_id[0] == '\0') {
        return NULL;
    }

    for(size_t i = 0; i < BAMBU_FILAMENT_TABLE_SIZE; i++) {
        if(strcmp(bambu_filament_table[i].variant_id, variant_id) == 0) {
            return &bambu_filament_table[i];
        }
    }

    char normalized_variant_id[16];
    bambu_normalize_variant_id(variant_id, normalized_variant_id, sizeof(normalized_variant_id));

    if(strcmp(normalized_variant_id, variant_id) != 0) {
        for(size_t i = 0; i < BAMBU_FILAMENT_TABLE_SIZE; i++) {
            if(strcmp(bambu_filament_table[i].variant_id, normalized_variant_id) == 0) {
                return &bambu_filament_table[i];
            }
        }
    }

    return NULL;
}

#endif // BAMBU_FILAMENTS_H
