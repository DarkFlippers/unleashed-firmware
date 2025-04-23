#ifndef MFKEY_H
#define MFKEY_H

#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <inttypes.h>
#include <toolbox/keys_dict.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

struct Crypto1State {
    uint32_t odd, even;
};
struct Msb {
    int tail;
    uint32_t states[768];
};

typedef enum {
    MissingNonces,
    ZeroNonces,
    InsufficientRAM,
} MFKeyError;

typedef enum {
    Ready,
    Initializing,
    DictionaryAttack,
    MFKeyAttack,
    Complete,
    Error,
    Help,
} MFKeyState;

// TODO: Can we eliminate any of the members of this struct?
typedef struct {
    FuriMutex* mutex;
    MFKeyError err;
    MFKeyState mfkey_state;
    int cracked;
    int unique_cracked;
    int num_completed;
    int num_candidates;
    int total;
    int dict_count;
    int search;
    int eta_timestamp;
    int eta_total;
    int eta_round;
    bool mfkey32_present;
    bool nested_present;
    bool close_thread_please;
    FuriThread* mfkeythread;
    KeysDict* cuid_dict;
} ProgramState;

typedef enum {
    mfkey32,
    static_nested,
    static_encrypted
} AttackType;

typedef struct {
    AttackType attack;
    MfClassicKey key; // key
    uint32_t uid; // serial number
    uint32_t nt0; // tag challenge first
    uint32_t nt1; // tag challenge second
    uint32_t uid_xor_nt0; // uid ^ nt0
    uint32_t uid_xor_nt1; // uid ^ nt1
    union {
        // Mfkey32
        struct {
            uint32_t p64; // 64th successor of nt0
            uint32_t p64b; // 64th successor of nt1
            uint32_t nr0_enc; // first encrypted reader challenge
            uint32_t ar0_enc; // first encrypted reader response
            uint32_t nr1_enc; // second encrypted reader challenge
            uint32_t ar1_enc; // second encrypted reader response
        };
        // Nested
        struct {
            uint32_t ks1_1_enc; // first encrypted keystream
            uint32_t ks1_2_enc; // second encrypted keystream
            char par_1_str[5]; // first parity bits (string representation)
            char par_2_str[5]; // second parity bits (string representation)
            uint8_t par_1; // first parity bits
            uint8_t par_2; // second parity bits
        };
    };
} MfClassicNonce;

typedef struct {
    Stream* stream;
    uint32_t total_nonces;
    MfClassicNonce* remaining_nonce_array;
    size_t remaining_nonces;
} MfClassicNonceArray;

struct KeysDict {
    Stream* stream;
    size_t key_size;
    size_t key_size_symbols;
    size_t total_keys;
};

#endif // MFKEY_H
