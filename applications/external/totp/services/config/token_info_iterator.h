#pragma once

#include "../../types/token_info.h"
#include <flipper_format/flipper_format.h>
#include "constants.h"

typedef int TotpIteratorUpdateTokenResult;

typedef TotpIteratorUpdateTokenResult (
    *TOTP_ITERATOR_UPDATE_TOKEN_ACTION)(TokenInfo* const token_info, const void* context);

typedef struct TokenInfoIteratorContext TokenInfoIteratorContext;

enum TotpIteratorUpdateTokenResults {

    /**
     * @brief Token successfully updated
     */
    TotpIteratorUpdateTokenResultSuccess = 0,

    /**
     * @brief An error ocurred during updating config file
     */
    TotpIteratorUpdateTokenResultFileUpdateFailed = -1
};

/**
 * @brief Initializes a new token info iterator
 * @param storage storage reference
 * @param config_file config file to use
 * @param crypto_settings crypto settings
 * @return Token info iterator context
 */
TokenInfoIteratorContext* totp_token_info_iterator_alloc(
    Storage* storage,
    FlipperFormat* config_file,
    CryptoSettings* crypto_settings);

/**
 * @brief Navigates iterator to the token with given index
 * @param context token info iterator context
 * @param token_index token index to navigate to
 * @return \c true if navigation succeeded; \c false otherwise
 */
bool totp_token_info_iterator_go_to(TokenInfoIteratorContext* context, size_t token_index);

/**
 * @brief Moves current token to a given new index
 * @param context token info iterator context
 * @param new_index new token index to move current token to
 * @return \c true if operation succeeded; \c false otherwise
 */
bool totp_token_info_iterator_move_current_token_info(
    TokenInfoIteratorContext* context,
    size_t new_index);

/**
 * @brief Updates current token info using given update action
 * @param context token info iterator context
 * @param update action which is responsible to make all the necessary updates to token info
 * @param update_context update action context
 * @return \c true if operation succeeded; \c false otherwise
 */
TotpIteratorUpdateTokenResult totp_token_info_iterator_update_current_token(
    TokenInfoIteratorContext* context,
    TOTP_ITERATOR_UPDATE_TOKEN_ACTION update,
    const void* update_context);

/**
 * @brief Adds new token info to the end of the list using given update action
 * @param context token info iterator context
 * @param update action which is responsible to make all the necessary updates to token info
 * @param update_context update action context
 * @return \c true if operation succeeded; \c false otherwise
 */
TotpIteratorUpdateTokenResult totp_token_info_iterator_add_new_token(
    TokenInfoIteratorContext* context,
    TOTP_ITERATOR_UPDATE_TOKEN_ACTION update,
    const void* update_context);

/**
 * @brief Remvoves current token info
 * @param context token info iterator context
 * @return \c true if operation succeeded; \c false otherwise
 */
bool totp_token_info_iterator_remove_current_token_info(TokenInfoIteratorContext* context);

/**
 * @brief Disposes token info iterator and releases all the resources
 * @param context token info iterator context
 */
void totp_token_info_iterator_free(TokenInfoIteratorContext* context);

/**
 * @brief Gets current token info
 * @param context token info iterator context
 * @return current token info
 */
const TokenInfo*
    totp_token_info_iterator_get_current_token(const TokenInfoIteratorContext* context);

/**
 * @brief Gets current token info index
 * @param context token info iterator context
 * @return current token info index
 */
size_t totp_token_info_iterator_get_current_token_index(const TokenInfoIteratorContext* context);

/**
 * @brief Gets total amount of token infos found
 * @param context token info iterator context
 * @return amount of token infos found
 */
size_t totp_token_info_iterator_get_total_count(const TokenInfoIteratorContext* context);

/**
 * @brief Attaches token info iterator to another config file
 * @param context token info iterator context
 * @param config_file config file reference to attach token info iterator to
 */
void totp_token_info_iterator_attach_to_config_file(
    TokenInfoIteratorContext* context,
    FlipperFormat* config_file);
