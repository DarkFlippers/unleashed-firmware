#pragma once
#include <core/record.h>

/**
 * @brief Class for opening, casting, holding and closing records
 * 
 * @tparam TRecordClass record class
 */
template <typename TRecordClass>
class RecordController {
public:
    /**
     * @brief Construct a new Record Controller object for record with record name
     * 
     * @param record_name record name
     */
    RecordController(const char* record_name) {
        name = record_name;
        value = static_cast<TRecordClass*>(furi_record_open(name));
    };

    ~RecordController() {
        furi_record_close(name);
    }

    /**
     * @brief Record getter
     * 
     * @return TRecordClass* record value
     */
    TRecordClass* get() {
        return value;
    }

    /**
     * @brief Record getter (by cast)
     * 
     * @return TRecordClass* record value
     */
    operator TRecordClass*() const {
        return value;
    }

private:
    const char* name;
    TRecordClass* value;
};
