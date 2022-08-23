#pragma once
#include <toolbox/protocols/protocol_dict.h>
#include "protocols/lfrfid_protocols.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Save protocol from dictionary to file
 * 
 * @param dict 
 * @param protocol 
 * @param filename 
 * @return true 
 * @return false 
 */
bool lfrfid_dict_file_save(ProtocolDict* dict, ProtocolId protocol, const char* filename);

/**
 * @brief Load protocol from file to dictionary
 * 
 * @param dict 
 * @param filename 
 * @return ProtocolId 
 */
ProtocolId lfrfid_dict_file_load(ProtocolDict* dict, const char* filename);

#ifdef __cplusplus
}
#endif