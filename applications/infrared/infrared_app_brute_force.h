/**
  * @file infrared_app_brute_force.h
  * Infrared: Brute Force class description
  */
#pragma once

#include <unordered_map>
#include <memory>
#include <flipper_format/flipper_format.h>

/** Class handles brute force mechanic */
class InfraredAppBruteForce {
    /** Universal database filename */
    const char* universal_db_filename;

    /** Current record name (POWER, MUTE, VOL+, etc).
     * This is the name of signal to brute force. */
    std::string current_record;

    /** Flipper File Format instance */
    FlipperFormat* ff;

    /** Data about every record - index in button panel view
     * and amount of signals, which is need for correct
     * progress bar displaying. */
    typedef struct {
        /** Index of record in button panel view model */
        int index;
        /** Amount of signals of that type (POWER, MUTE, etc) */
        int amount;
    } Record;

    /** Container to hold Record info.
     * 'key' is record name, because we have to search by both, index and name,
     * but index search has place once per button press, and should not be
     * noticed, but name search should occur during entering universal menu,
     * and will go through container for every record in file, that's why
     * more critical to have faster search by record name.
     */
    std::unordered_map<std::string, Record> records;

public:
    /** Calculate messages. Walk through the file ('universal_db_name')
     * and calculate amount of records of certain type. */
    bool calculate_messages();

    /** Start brute force */
    bool start_bruteforce(int index, int& record_amount);

    /** Stop brute force */
    void stop_bruteforce();

    /** Send next signal during brute force */
    bool send_next_bruteforce();

    /** Add record to container of records */
    void add_record(int index, const char* name);

    /** Initialize class, set db file */
    InfraredAppBruteForce(const char* filename)
        : universal_db_filename(filename) {
    }

    /** Deinitialize class */
    ~InfraredAppBruteForce() {
    }
};
