#include "felica_listener.h"

#include <nfc/protocols/nfc_generic_event.h>

#define FELICA_LISTENER_READ_BLOCK_COUNT_MAX  (4U)
#define FELICA_LISTENER_READ_BLOCK_COUNT_MIN  (1U)
#define FELICA_LISTENER_WRITE_BLOCK_COUNT_MAX (2U)
#define FELICA_LISTENER_WRITE_BLOCK_COUNT_MIN (1U)

typedef enum {
    Felica_ListenerStateIdle,
    Felica_ListenerStateActivated,
} FelicaListenerState;

/** Generic Felica request same for both read and write operations. */
typedef struct {
    uint8_t length;
    FelicaCommandHeader header;
} FelicaListenerGenericRequest;

/** Generic request but with list of requested elements. */
typedef struct {
    FelicaListenerGenericRequest base;
    FelicaBlockListElement list[];
} FelicaListenerRequest;

typedef FelicaListenerRequest FelicaListenerReadRequest;
typedef FelicaListenerRequest FelicaListenerWriteRequest;

/** Struct for write request data.
 *
 * All data are placed right after @ref FelicaListenerRequest data.
 */
typedef struct {
    FelicaBlockData blocks[FELICA_LISTENER_WRITE_BLOCK_COUNT_MAX];
} FelicaListenerWriteBlockData;

/** Write command handler signature.
 *
 * Depending on requested block write behaviour can be different, therefore
 * different handlers are used.
 */
typedef void (*FelicaCommandWriteBlockHandler)(
    FelicaListener* instance,
    const uint8_t block_number,
    const FelicaBlockData* data_block);

/** Read command handler signature.
 *
 * Depending on requested block read behaviour can be different, therefore
 * different handlers are used.
 */
typedef void (*FelicaCommanReadBlockHandler)(
    FelicaListener* instance,
    const uint8_t block_number,
    const uint8_t resp_data_index,
    FelicaListenerReadCommandResponse* response);

struct FelicaListener {
    Nfc* nfc;
    FelicaData* data;
    FelicaListenerState state;
    FelicaAuthentication auth;
    FelicaBlockData mc_shadow;

    uint8_t request_size_buf;
    uint8_t block_list_size;
    uint8_t requested_blocks[FELICA_LISTENER_READ_BLOCK_COUNT_MAX];
    uint8_t mac_calc_start;
    bool rc_written;

    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent generic_event;
    NfcGenericCallback callback;
    void* context;
};

/** Resets card authentication state after field off, also resets session key and
 * perform post process operations with some blocks.
 *
 * @param      instance  pointer to the listener instance to be used.
 */
void felica_listener_reset(FelicaListener* instance);

/** Performs WCNT increasing in case of write operation.
 *
 * @param      data  pointer to Felica card data.
 */
void felica_wcnt_increment(FelicaData* data);

/** Compares IDm of card loaded for emulation with IDm from request.
 *
 * @param      instance     pointer to the listener instance to be used.
 * @param      request_idm  pointer to IDm block from request structure.
 *
 * @return     True if IDms' are equal, otherwise false.
 */
bool felica_listener_check_idm(const FelicaListener* instance, const FelicaIDm* request_idm);

/** This is the first request validation function.
 *
 * Its main aim is to check whether the input request is valid in general by
 * counting the amount of data in request. Function iterates through block list
 * elements and data, counts real amount of elements and real amount of
 * coresponding elements data (in case of write operation). If block list
 * element amount is invalid or request data is missing, such request will be
 * ignored.
 *
 * @param      instance  pointer to the listener instance to be used.
 * @param      request   pointer to received request.
 *
 * @return     True if block element list contains valid amount of data,
 *             otherwise false.
 */
bool felica_listener_check_block_list_size(
    FelicaListener* instance,
    FelicaListenerGenericRequest* request);

/** Used to take first element from block element list.
 *
 * Each element in block list can be 2 or 3-bytes length and they can be mixed
 * together. Therefore before returning the element, function checks whether
 * there is enough bytes in the request for this element, in order to avoid
 * memory casting behind the request block.
 *
 * @param      instance  pointer to the listener instance to be used.
 * @param      request   pointer to received request.
 *
 * @return     Pointer to the first element of the list or NULL if there is not
 *             enough bytes in the request.
 */
const FelicaBlockListElement* felica_listener_block_list_item_get_first(
    FelicaListener* instance,
    const FelicaListenerRequest* request);

/** Used to take next element from block element list.
 *
 * It uses pointer to the previous element, takes its length and calculates
 * pointer to the next element if there is enough bytes left.
 *
 * @param      instance   pointer to the listener instance to be used.
 * @param      prev_item  pointer to the previous item taken.
 *
 * @return     Pointer to the next element of the list or NULL if there is not
 *             enough bytes in the request.
 */
const FelicaBlockListElement* felica_listener_block_list_item_get_next(
    FelicaListener* instance,
    const FelicaBlockListElement* prev_item);

/** Calculates pointer to data blocks in case of write operation, because block
 * list elements size can vary.
 *
 * For calculation it uses previously calculated block list size and
 * FelicaListenerGenericRequest size.
 *
 * @param      instance         pointer to the listener instance to be used.
 * @param      generic_request  pointer to received request.
 *
 * @return     Pointer to data blocks for write operation.
 */
const FelicaListenerWriteBlockData* felica_listener_get_write_request_data_pointer(
    const FelicaListener* const instance,
    const FelicaListenerGenericRequest* const generic_request);

/** Function validates write request data and sets Felica SF1 and SF2 flags
 * directly to the response.
 *
 * When something is wrong with data in the request (for example non-existing
 * block is requested) this function sets proper SF1 and SF2 flags which will be
 * then send to reader. When every thing is fine SF1 and SF2 are 0. Also this
 * function performs authentiocation checks by MAC_A calculation if request
 * contains MAC_A.
 *
 * @param      instance  pointer to the listener instance to be used.
 * @param      request   pointer to received write request.
 * @param      data      pointer to data which request wants to write.
 * @param      response  pointer to the response to where SF1 and SF2 flags will
 *                       be populated.
 *
 * @return     True if request is fine also SF1 and SF2 will be 0. False if
 *             something is wrong and SF1, SF2 will provide more detailed
 *             information about the error.
 */
bool felica_listener_validate_write_request_and_set_sf(
    FelicaListener* instance,
    const FelicaListenerWriteRequest* const request,
    const FelicaListenerWriteBlockData* const data,
    FelicaListenerWriteCommandResponse* response);

/** Function validates read request data and sets Felica SF1 and SF2 flags
 * directly to the response.
 *
 * When something is wrong with data in the request (for example non-existing
 * block is requested) this function sets proper SF1 and SF2 flags which will be
 * then send to reader. In such case there will be no any data in the request,
 * only flags. In case when request is fine SF1 and SF2 will be 0 and data will
 * be populated to the response after them. Attention! This function doesn't
 * populate any data, it only allows further processing where those data will be
 * populated.
 *
 * @param      instance     pointer to the listener instance to be used.
 * @param      request      pointer to received write request.
 * @param      resp_header  pointer to the response to where SF1 and SF2 and
 *                          data flags will be populated.
 *
 * @return     True if request is fine also SF1 and SF2 will be 0. False if
 *             something is wrong and SF1, SF2 will provide more detailed
 *             information about the error.
 */
bool felica_listener_validate_read_request_and_set_sf(
    FelicaListener* instance,
    const FelicaListenerReadRequest* const request,
    FelicaCommandResponseHeader* resp_header);

/** Function returns appropiate block handler for processing read operation
 * depending on number of block.
 *
 * @param      block_number  number of block.
 *
 * @return     pointer to block handler function.
 */
FelicaCommanReadBlockHandler felica_listener_get_read_block_handler(const uint8_t block_number);

/** Function returns appropiate block handler for processing write operation
 * depending on number of block.
 *
 * @param      block_number  number of block.
 *
 * @return     pointer to block handler function.
 */
FelicaCommandWriteBlockHandler felica_listener_get_write_block_handler(const uint8_t block_number);

/** Sends response data from buffer to reader.
 *
 * @param      instance   pointer to the listener instance to be used.
 * @param      tx_buffer  buffer with response data.
 *
 * @return     error code or FelicaErrorNone.
 */
FelicaError
    felica_listener_frame_exchange(const FelicaListener* instance, const BitBuffer* tx_buffer);
