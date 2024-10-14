#include "gallagher/gallagher_util.h"
#include "mosgortrans/mosgortrans_util.h"

/* 
 * A list of app's private functions and objects to expose for plugins.
 * It is used to generate a table of symbols for import resolver to use.
 * TBD: automatically generate this table from app's header files
 */
static constexpr auto nfc_app_api_table = sort(create_array_t<sym_entry>(
    API_METHOD(
        gallagher_deobfuscate_and_parse_credential,
        void,
        (GallagherCredential * credential, const uint8_t* cardholder_data_obfuscated)),
    API_VARIABLE(GALLAGHER_CARDAX_ASCII, const uint8_t*),
    API_METHOD(
        mosgortrans_parse_transport_block,
        bool,
        (const MfClassicBlock* block, FuriString* result)),
    API_METHOD(
        render_section_header,
        void,
        (FuriString * str,
         const char* name,
         uint8_t prefix_separator_cnt,
         uint8_t suffix_separator_cnt))));
