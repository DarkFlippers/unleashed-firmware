#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Skylanders"

static const uint64_t skylanders_key = 0x4b0b20107ccb;

bool skylanders_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 0;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(skylanders_key, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool skylanders_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(skylanders_key, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(skylanders_key, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = mf_classic_is_card_read(data);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static uint8_t fill_name(const uint16_t id, FuriString* name) {
    // USED RESEARCH FROM https://github.com/silicontrip/SkyReader/blob/master/toynames.cpp#L15C1-L163C1
    // AND https://github.com/bettse/Solarbreeze/blob/master/Solarbreeze/ThePoster.swift#L438C1-L681C1
    switch(id) {
    case 0x0000:
        furi_string_cat_printf(name, "Whirlwind");
        break;
    case 0x0001:
        furi_string_cat_printf(name, "Sonic Boom");
        break;
    case 0x0002:
        furi_string_cat_printf(name, "Warnado");
        break;
    case 0x0003:
        furi_string_cat_printf(name, "Lightning Rod");
        break;
    case 0x0004:
    case 0x0194:
        furi_string_cat_printf(name, "Bash");
        break;
    case 0x0005:
        furi_string_cat_printf(name, "Terrafin");
        break;
    case 0x0006:
        furi_string_cat_printf(name, "Dino-Rang");
        break;
    case 0x0007:
        furi_string_cat_printf(name, "Prism Break");
        break;
    case 0x0008:
        furi_string_cat_printf(name, "Sunburn");
        break;
    case 0x0009:
        furi_string_cat_printf(name, "Eruptor");
        break;
    case 0x000A:
        furi_string_cat_printf(name, "Ignitor");
        break;
    case 0x000B:
        furi_string_cat_printf(name, "Flameslinger");
        break;
    case 0x000C:
        furi_string_cat_printf(name, "Zap");
        break;
    case 0x000D:
        furi_string_cat_printf(name, "Wham-Shell");
        break;
    case 0x000E:
        furi_string_cat_printf(name, "Gill Grunt");
        break;
    case 0x000F:
        furi_string_cat_printf(name, "Slam Bam");
        break;
    case 0x0010:
    case 0x01A0:
        furi_string_cat_printf(name, "Spyro");
        break;
    case 0x0011:
        furi_string_cat_printf(name, "Voodood");
        break;
    case 0x0012:
        furi_string_cat_printf(name, "Double Trouble");
        break;
    case 0x0013:
    case 0x01A3:
        furi_string_cat_printf(name, "Trigger Happy");
        break;
    case 0x0014:
        furi_string_cat_printf(name, "Drobot");
        break;
    case 0x0015:
        furi_string_cat_printf(name, "Drill Sergeant");
        break;
    case 0x0016:
        furi_string_cat_printf(name, "Boomer");
        break;
    case 0x0017:
        furi_string_cat_printf(name, "Wrecking Ball");
        break;
    case 0x0018:
        furi_string_cat_printf(name, "Camo");
        break;
    case 0x0019:
        furi_string_cat_printf(name, "Zook");
        break;
    case 0x001A:
        furi_string_cat_printf(name, "Stealth Elf");
        break;
    case 0x001B:
        furi_string_cat_printf(name, "Stump Smash");
        break;
    case 0x001C:
        furi_string_cat_printf(name, "Dark Spyro");
        break;
    case 0x001D:
        furi_string_cat_printf(name, "Hex");
        break;
    case 0x001E:
    case 0x01AE:
        furi_string_cat_printf(name, "Chop Chop");
        break;
    case 0x001F:
        furi_string_cat_printf(name, "Ghost Roaster");
        break;
    case 0x0020:
        furi_string_cat_printf(name, "Cynder");
        break;
    case 0x0064:
        furi_string_cat_printf(name, "Jet Vac");
        break;
    case 0x0065:
        furi_string_cat_printf(name, "Swarm");
        break;
    case 0x0066:
        furi_string_cat_printf(name, "Crusher");
        break;
    case 0x0067:
        furi_string_cat_printf(name, "Flashwing");
        break;
    case 0x0068:
        furi_string_cat_printf(name, "Hot Head");
        break;
    case 0x0069:
        furi_string_cat_printf(name, "Hot Dog");
        break;
    case 0x006A:
        furi_string_cat_printf(name, "Chill");
        break;
    case 0x006B:
        furi_string_cat_printf(name, "Thumpback");
        break;
    case 0x006C:
        furi_string_cat_printf(name, "Pop Fizz");
        break;
    case 0x006D:
        furi_string_cat_printf(name, "Ninjini");
        break;
    case 0x006E:
        furi_string_cat_printf(name, "Bouncer");
        break;
    case 0x006F:
        furi_string_cat_printf(name, "Sprocket");
        break;
    case 0x0070:
        furi_string_cat_printf(name, "Tree Rex");
        break;
    case 0x0071:
        furi_string_cat_printf(name, "Shroomboom");
        break;
    case 0x0072:
        furi_string_cat_printf(name, "Eye-Brawl");
        break;
    case 0x0073:
        furi_string_cat_printf(name, "Fright Rider");
        break;
    case 0x00C8:
        furi_string_cat_printf(name, "Anvil Rain");
        break;
    case 0x00C9:
        furi_string_cat_printf(name, "Treasure Chest");
        break;
    case 0x00CA:
        furi_string_cat_printf(name, "Healing Elixer");
        break;
    case 0x00CB:
        furi_string_cat_printf(name, "Ghost Swords");
        break;
    case 0x00CC:
        furi_string_cat_printf(name, "Time Twister");
        break;
    case 0x00CD:
        furi_string_cat_printf(name, "Sky-Iron Shield");
        break;
    case 0x00CE:
        furi_string_cat_printf(name, "Winged Boots");
        break;
    case 0x00CF:
        furi_string_cat_printf(name, "Sparx Dragonfly");
        break;
    case 0x00D0:
        furi_string_cat_printf(name, "Dragonfire Cannon");
        break;
    case 0x00D1:
        furi_string_cat_printf(name, "Scorpion Striker Catapult");
        break;
    case 0x00D2:
        furi_string_cat_printf(name, "Trap - Magic");
        break;
    case 0x00D3:
        furi_string_cat_printf(name, "Trap - Water");
        break;
    case 0x00D4:
        furi_string_cat_printf(name, "Trap - Air");
        break;
    case 0x00D5:
        furi_string_cat_printf(name, "Trap - Undead");
        break;
    case 0x00D6:
        furi_string_cat_printf(name, "Trap - Tech");
        break;
    case 0x00D7:
        furi_string_cat_printf(name, "Trap - Fire");
        break;
    case 0x00D8:
        furi_string_cat_printf(name, "Trap - Earth");
        break;
    case 0x00D9:
        furi_string_cat_printf(name, "Trap - Life");
        break;
    case 0x00DA:
        furi_string_cat_printf(name, "Trap - Light");
        break;
    case 0x00DB:
        furi_string_cat_printf(name, "Trap - Dark");
        break;
    case 0x00DC:
        furi_string_cat_printf(name, "Trap - Kaos");
        break;
    case 0x00E6:
        furi_string_cat_printf(name, "Hand Of Fate");
        break;
    case 0x00E7:
        furi_string_cat_printf(name, "Piggy Bank");
        break;
    case 0x00E8:
        furi_string_cat_printf(name, "Rocket Ram");
        break;
    case 0x00E9:
        furi_string_cat_printf(name, "Tiki Speaky");
        break;
    case 0x00EB:
        furi_string_cat_printf(name, "Imaginite Mystery Chest");
        break;
    case 0x012C:
        furi_string_cat_printf(name, "Dragons Peak");
        break;
    case 0x012D:
        furi_string_cat_printf(name, "Empire of Ice");
        break;
    case 0x012E:
        furi_string_cat_printf(name, "Pirate Seas");
        break;
    case 0x012F:
        furi_string_cat_printf(name, "Darklight Crypt");
        break;
    case 0x0130:
        furi_string_cat_printf(name, "Volcanic Vault");
        break;
    case 0x0131:
        furi_string_cat_printf(name, "Mirror Of Mystery");
        break;
    case 0x0132:
        furi_string_cat_printf(name, "Nightmare Express");
        break;
    case 0x0133:
        furi_string_cat_printf(name, "Sunscraper Spire");
        break;
    case 0x0134:
        furi_string_cat_printf(name, "Midnight Museum");
        break;
    case 0x01C2:
        furi_string_cat_printf(name, "Gusto");
        break;
    case 0x01C3:
        furi_string_cat_printf(name, "Thunderbolt");
        break;
    case 0x01C4:
        furi_string_cat_printf(name, "Fling Kong");
        break;
    case 0x01C5:
        furi_string_cat_printf(name, "Blades");
        break;
    case 0x01C6:
        furi_string_cat_printf(name, "Wallop");
        break;
    case 0x01C7:
        furi_string_cat_printf(name, "Head Rush");
        break;
    case 0x01C8:
        furi_string_cat_printf(name, "Fist Bump");
        break;
    case 0x01C9:
        furi_string_cat_printf(name, "Rocky Roll");
        break;
    case 0x01CA:
        furi_string_cat_printf(name, "Wildfire");
        break;
    case 0x01CB:
        furi_string_cat_printf(name, "Ka Boom");
        break;
    case 0x01CC:
        furi_string_cat_printf(name, "Trail Blazer");
        break;
    case 0x01CD:
        furi_string_cat_printf(name, "Torch");
        break;
    case 0x01CE:
        furi_string_cat_printf(name, "Snap Shot");
        break;
    case 0x01CF:
        furi_string_cat_printf(name, "Lob Star");
        break;
    case 0x01D0:
        furi_string_cat_printf(name, "Flip Wreck");
        break;
    case 0x01D1:
        furi_string_cat_printf(name, "Echo");
        break;
    case 0x01D2:
        furi_string_cat_printf(name, "Blastermind");
        break;
    case 0x01D3:
        furi_string_cat_printf(name, "Enigma");
        break;
    case 0x01D4:
        furi_string_cat_printf(name, "Deja Vu");
        break;
    case 0x01D5:
        furi_string_cat_printf(name, "Cobra Cadabra");
        break;
    case 0x01D6:
        furi_string_cat_printf(name, "Jawbreaker");
        break;
    case 0x01D7:
        furi_string_cat_printf(name, "Gearshift");
        break;
    case 0x01D8:
        furi_string_cat_printf(name, "Chopper");
        break;
    case 0x01D9:
        furi_string_cat_printf(name, "Tread Head");
        break;
    case 0x01DA:
        furi_string_cat_printf(name, "Bushwhack");
        break;
    case 0x01DB:
        furi_string_cat_printf(name, "Tuff Luck");
        break;
    case 0x01DC:
        furi_string_cat_printf(name, "Food Fight");
        break;
    case 0x01DD:
        furi_string_cat_printf(name, "High Five");
        break;
    case 0x01DE:
        furi_string_cat_printf(name, "Krypt King");
        break;
    case 0x01DF:
        furi_string_cat_printf(name, "Short Cut");
        break;
    case 0x01E0:
        furi_string_cat_printf(name, "Bat Spin");
        break;
    case 0x01E1:
        furi_string_cat_printf(name, "Funny Bone");
        break;
    case 0x01E2:
        furi_string_cat_printf(name, "Knight light");
        break;
    case 0x01E3:
        furi_string_cat_printf(name, "Spotlight");
        break;
    case 0x01E4:
        furi_string_cat_printf(name, "Knight Mare");
        break;
    case 0x01E5:
        furi_string_cat_printf(name, "Blackout");
        break;
    case 0x01F6:
        furi_string_cat_printf(name, "Bop");
        break;
    case 0x01F7:
        furi_string_cat_printf(name, "Spry");
        break;
    case 0x01F8:
        furi_string_cat_printf(name, "Hijinx");
        break;
    case 0x01F9:
        furi_string_cat_printf(name, "Terrabite");
        break;
    case 0x01FA:
        furi_string_cat_printf(name, "Breeze");
        break;
    case 0x01FB:
        furi_string_cat_printf(name, "Weeruptor");
        break;
    case 0x01FC:
        furi_string_cat_printf(name, "Pet Vac");
        break;
    case 0x01FD:
        furi_string_cat_printf(name, "Small Fry");
        break;
    case 0x01FE:
        furi_string_cat_printf(name, "Drobit");
        break;
    case 0x0202:
        furi_string_cat_printf(name, "Gill Runt");
        break;
    case 0x0207:
        furi_string_cat_printf(name, "Trigger Snappy");
        break;
    case 0x020E:
        furi_string_cat_printf(name, "Whisper Elf");
        break;
    case 0x021C:
        furi_string_cat_printf(name, "Barkley");
        break;
    case 0x021D:
        furi_string_cat_printf(name, "Thumpling");
        break;
    case 0x021E:
        furi_string_cat_printf(name, "Mini Jini");
        break;
    case 0x021F:
        furi_string_cat_printf(name, "Eye Small");
        break;
    case 0x0259:
        furi_string_cat_printf(name, "King Pen");
        break;
    case 0x0265:
        furi_string_cat_printf(name, "Golden Queen");
        break;
    case 0x02AD:
        furi_string_cat_printf(name, "Fire Acorn");
        break;
    case 0x03E8:
        furi_string_cat_printf(name, "(Boom) Jet");
        break;
    case 0x03E9:
        furi_string_cat_printf(name, "(Free) Ranger");
        break;
    case 0x03EA:
        furi_string_cat_printf(name, "(Rubble) Rouser");
        break;
    case 0x03EB:
        furi_string_cat_printf(name, "(Doom) Stone");
        break;
    case 0x03EC:
        furi_string_cat_printf(name, "Blast Zone");
        break;
    case 0x03ED:
        furi_string_cat_printf(name, "(Fire) Kraken");
        break;
    case 0x03EE:
        furi_string_cat_printf(name, "(Stink) Bomb");
        break;
    case 0x03EF:
        furi_string_cat_printf(name, "(Grilla) Drilla");
        break;
    case 0x03F0:
        furi_string_cat_printf(name, "(Hoot) Loop");
        break;
    case 0x03F1:
        furi_string_cat_printf(name, "(Trap) Shadow");
        break;
    case 0x03F2:
        furi_string_cat_printf(name, "(Magna) Charge");
        break;
    case 0x03F3:
        furi_string_cat_printf(name, "(Spy) Rise");
        break;
    case 0x03F4:
        furi_string_cat_printf(name, "(Night) Shift");
        break;
    case 0x03F5:
        furi_string_cat_printf(name, "(Rattle) Shake");
        break;
    case 0x03F6:
        furi_string_cat_printf(name, "(Freeze) Blade");
        break;
    case 0x03F7:
        furi_string_cat_printf(name, "Wash Buckler");
        break;
    case 0x07D0:
        furi_string_cat_printf(name, "Boom (Jet)");
        break;
    case 0x07D1:
        furi_string_cat_printf(name, "Free (Ranger)");
        break;
    case 0x07D2:
        furi_string_cat_printf(name, "Rubble (Rouser)");
        break;
    case 0x07D3:
        furi_string_cat_printf(name, "Doom (Stone)");
        break;
    case 0x07D4:
        furi_string_cat_printf(name, "Blast Zone (Head)");
        break;
    case 0x07D5:
        furi_string_cat_printf(name, "Fire (Kraken)");
        break;
    case 0x07D6:
        furi_string_cat_printf(name, "Stink (Bomb)");
        break;
    case 0x07D7:
        furi_string_cat_printf(name, "Grilla (Drilla)");
        break;
    case 0x07D8:
        furi_string_cat_printf(name, "Hoot (Loop)");
        break;
    case 0x07D9:
        furi_string_cat_printf(name, "Trap (Shadow)");
        break;
    case 0x07DA:
        furi_string_cat_printf(name, "Magna (Charge)");
        break;
    case 0x07DB:
        furi_string_cat_printf(name, "Spy (Rise)");
        break;
    case 0x07DC:
        furi_string_cat_printf(name, "Night (Shift)");
        break;
    case 0x07DD:
        furi_string_cat_printf(name, "Rattle (Shake)");
        break;
    case 0x07DE:
        furi_string_cat_printf(name, "Freeze (Blade)");
        break;
    case 0x07DF:
        furi_string_cat_printf(name, "Wash Buckler (Head)");
        break;
    case 0x0BB8:
        furi_string_cat_printf(name, "Scratch");
        break;
    case 0x0BB9:
        furi_string_cat_printf(name, "Pop Thorn");
        break;
    case 0x0BBA:
        furi_string_cat_printf(name, "Slobber Tooth");
        break;
    case 0x0BBB:
        furi_string_cat_printf(name, "Scorp");
        break;
    case 0x0BBC:
        furi_string_cat_printf(name, "Fryno");
        break;
    case 0x0BBD:
        furi_string_cat_printf(name, "Smolderdash");
        break;
    case 0x0BBE:
        furi_string_cat_printf(name, "Bumble Blast");
        break;
    case 0x0BBF:
        furi_string_cat_printf(name, "Zoo Lou");
        break;
    case 0x0BC0:
        furi_string_cat_printf(name, "Dune Bug");
        break;
    case 0x0BC1:
        furi_string_cat_printf(name, "Star Strike");
        break;
    case 0x0BC2:
        furi_string_cat_printf(name, "Countdown");
        break;
    case 0x0BC3:
        furi_string_cat_printf(name, "Wind Up");
        break;
    case 0x0BC4:
        furi_string_cat_printf(name, "Roller Brawl");
        break;
    case 0x0BC5:
        furi_string_cat_printf(name, "Grim Creeper");
        break;
    case 0x0BC6:
        furi_string_cat_printf(name, "Rip Tide");
        break;
    case 0x0BC7:
        furi_string_cat_printf(name, "Punk Shock");
        break;
    case 0x0C80:
        furi_string_cat_printf(name, "Battle Hammer");
        break;
    case 0x0C81:
        furi_string_cat_printf(name, "Sky Diamond");
        break;
    case 0x0C82:
        furi_string_cat_printf(name, "Platinum Sheep");
        break;
    case 0x0C83:
        furi_string_cat_printf(name, "Groove Machine");
        break;
    case 0x0C84:
        furi_string_cat_printf(name, "UFO Hat");
        break;
    case 0x0C94:
        furi_string_cat_printf(name, "Jet Stream");
        break;
    case 0x0C95:
        furi_string_cat_printf(name, "Tomb Buggy");
        break;
    case 0x0C96:
        furi_string_cat_printf(name, "Reef Ripper");
        break;
    case 0x0C97:
        furi_string_cat_printf(name, "Burn Cycle");
        break;
    case 0x0C98:
        furi_string_cat_printf(name, "Hot Streak");
        break;
    case 0x0C99:
        furi_string_cat_printf(name, "Shark Tank");
        break;
    case 0x0C9A:
        furi_string_cat_printf(name, "Thump Truck");
        break;
    case 0x0C9B:
        furi_string_cat_printf(name, "Crypt Crusher");
        break;
    case 0x0C9C:
        furi_string_cat_printf(name, "Stealth Stinger");
        break;
    case 0x0C9F:
        furi_string_cat_printf(name, "Dive Bomber");
        break;
    case 0x0CA0:
        furi_string_cat_printf(name, "Sky Slicer");
        break;
    case 0x0CA1:
        furi_string_cat_printf(name, "Clown Cruiser");
        break;
    case 0x0CA2:
        furi_string_cat_printf(name, "Gold Rusher");
        break;
    case 0x0CA3:
        furi_string_cat_printf(name, "Shield Striker");
        break;
    case 0x0CA4:
        furi_string_cat_printf(name, "Sun Runner");
        break;
    case 0x0CA5:
        furi_string_cat_printf(name, "Sea Shadow");
        break;
    case 0x0CA6:
        furi_string_cat_printf(name, "Splatter Splasher");
        break;
    case 0x0CA7:
        furi_string_cat_printf(name, "Soda Skimmer");
        break;
    case 0x0CA8:
        furi_string_cat_printf(name, "Barrel Blaster");
        break;
    case 0x0CA9:
        furi_string_cat_printf(name, "Buzz Wing");
        break;
    case 0x0CE4:
        furi_string_cat_printf(name, "Sheep Wreck Island");
        break;
    case 0x0CE5:
        furi_string_cat_printf(name, "Tower of Time");
        break;
    case 0x0CE6:
        furi_string_cat_printf(name, "Fiery Forge");
        break;
    case 0x0CE7:
        furi_string_cat_printf(name, "Arkeyan Crossbow");
        break;
    case 0x0D48:
        furi_string_cat_printf(name, "Fiesta");
        break;
    case 0x0D49:
        furi_string_cat_printf(name, "High Volt");
        break;
    case 0x0D4A:
        furi_string_cat_printf(name, "Splat");
        break;
    case 0x0D4E:
        furi_string_cat_printf(name, "Stormblade");
        break;
    case 0x0D53:
        furi_string_cat_printf(name, "Smash It");
        break;
    case 0x0D54:
        furi_string_cat_printf(name, "Spitfire");
        break;
    case 0x0D55:
        furi_string_cat_printf(name, "Hurricane Jet-Vac");
        break;
    case 0x0D56:
        furi_string_cat_printf(name, "Double Dare Trigger Happy");
        break;
    case 0x0D57:
        furi_string_cat_printf(name, "Super Shot Stealth Elf");
        break;
    case 0x0D58:
        furi_string_cat_printf(name, "Shark Shooter Terrafin");
        break;
    case 0x0D59:
        furi_string_cat_printf(name, "Bone Bash Roller Brawl");
        break;
    case 0x0D5C:
        furi_string_cat_printf(name, "Big Bubble Pop Fizz");
        break;
    case 0x0D5D:
        furi_string_cat_printf(name, "Lava Lance Eruptor");
        break;
    case 0x0D5E:
        furi_string_cat_printf(name, "Deep Dive Gill Grunt");
        break;
    case 0x0D5F:
        furi_string_cat_printf(name, "Turbo Charge Donkey Kong");
        break;
    case 0x0D60:
        furi_string_cat_printf(name, "Hammer Slam Bowser");
        break;
    case 0x0D61:
        furi_string_cat_printf(name, "Dive-Clops");
        break;
    case 0x0D62:
        furi_string_cat_printf(name, "Astroblast");
        break;
    case 0x0D63:
        furi_string_cat_printf(name, "Nightfall");
        break;
    case 0x0D64:
        furi_string_cat_printf(name, "Thrillipede");
        break;
    case 0x0DAC:
        furi_string_cat_printf(name, "Sky Trophy");
        break;
    case 0x0DAD:
        furi_string_cat_printf(name, "Land Trophy");
        break;
    case 0x0DAE:
        furi_string_cat_printf(name, "Sea Trophy");
        break;
    case 0x0DAF:
        furi_string_cat_printf(name, "Kaos Trophy");
        break;
    default:
        furi_string_cat_printf(name, "Unknown");
        break;
    }

    return true;
}

static bool skylanders_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;
    FuriString* name = furi_string_alloc();

    do {
        // verify key
        const uint8_t verify_sector = 0;
        MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, verify_sector);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != skylanders_key) break;

        const uint16_t id = (uint16_t)*data->block[1].data;
        if(id == 0) break;

        bool success = fill_name(id, name);
        if(!success) break;

        furi_string_printf(parsed_data, "\e#Skylanders\n%s", furi_string_get_cstr(name));

        parsed = true;

    } while(false);

    furi_string_free(name);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin skylanders_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = skylanders_verify,
    .read = skylanders_read,
    .parse = skylanders_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor skylanders_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &skylanders_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* skylanders_plugin_ep(void) {
    return &skylanders_plugin_descriptor;
}
