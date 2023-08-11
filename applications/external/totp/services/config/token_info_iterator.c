#include "token_info_iterator.h"

#include <flipper_format/flipper_format_i.h>
#include <flipper_format/flipper_format_stream.h>
#include <toolbox/stream/file_stream.h>
#include "../../types/common.h"
#include "../../types/crypto_settings.h"

#define CONFIG_FILE_PART_FILE_PATH CONFIG_FILE_DIRECTORY_PATH "/totp.conf.part"
#define STREAM_COPY_BUFFER_SIZE (128)

struct TokenInfoIteratorContext {
    size_t total_count;
    size_t current_index;
    size_t last_seek_offset;
    size_t last_seek_index;
    TokenInfo* current_token;
    FlipperFormat* config_file;
    CryptoSettings* crypto_settings;
    Storage* storage;
};

static bool
    flipper_format_seek_to_siblinig_token_start(Stream* stream, StreamDirection direction) {
    char buffer[sizeof(TOTP_CONFIG_KEY_TOKEN_NAME) + 1];
    bool found = false;
    while(!found) {
        if(!stream_seek_to_char(stream, '\n', direction)) {
            break;
        }

        size_t buffer_read_size;
        if((buffer_read_size = stream_read(stream, (uint8_t*)&buffer[0], sizeof(buffer))) == 0) {
            break;
        }

        if(!stream_seek(stream, -(int32_t)buffer_read_size, StreamOffsetFromCurrent)) {
            break;
        }

        if(strncmp(buffer, "\n" TOTP_CONFIG_KEY_TOKEN_NAME ":", sizeof(buffer)) == 0) {
            found = true;
        }
    }

    return found;
}

static bool seek_to_token(size_t token_index, TokenInfoIteratorContext* context) {
    furi_check(context != NULL && context->config_file != NULL);
    if(token_index >= context->total_count) {
        return false;
    }

    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    long token_index_diff = (long)token_index - (long)context->last_seek_index;
    size_t token_index_diff_weight = (size_t)labs(token_index_diff);
    StreamDirection direction = token_index_diff >= 0 ? StreamDirectionForward :
                                                        StreamDirectionBackward;
    if(token_index_diff_weight > token_index || context->last_seek_offset == 0) {
        context->last_seek_offset = 0;
        context->last_seek_index = 0;
        token_index_diff = token_index + 1;
        direction = StreamDirectionForward;
    } else if(token_index_diff_weight > (context->total_count - token_index - 1)) {
        context->last_seek_offset = stream_size(stream);
        context->last_seek_index = context->total_count - 1;
        token_index_diff = -(long)(context->total_count - token_index);
        direction = StreamDirectionBackward;
    }

    if(!stream_seek(stream, context->last_seek_offset, StreamOffsetFromStart)) {
        return false;
    }

    if(token_index_diff != 0) {
        long i = 0;
        long i_inc = token_index_diff >= 0 ? 1 : -1;
        do {
            if(!flipper_format_seek_to_siblinig_token_start(stream, direction)) {
                break;
            }

            i += i_inc;
        } while((i_inc > 0 && i < token_index_diff) || (i_inc < 0 && i > token_index_diff));

        if((i_inc > 0 && i < token_index_diff) || (i_inc < 0 && i > token_index_diff)) {
            context->last_seek_offset = 0;
            FURI_LOG_D(LOGGING_TAG, "Was not able to move");
            return false;
        }

        context->last_seek_offset = stream_tell(stream);
        context->last_seek_index = token_index;
    }

    return true;
}

static bool stream_insert_stream(Stream* dst, Stream* src) {
    uint8_t buffer[STREAM_COPY_BUFFER_SIZE];
    size_t buffer_read_size;
    while((buffer_read_size = stream_read(src, buffer, sizeof(buffer))) != 0) {
        if(!stream_insert(dst, buffer, buffer_read_size)) {
            return false;
        }
    }

    return true;
}

static bool ensure_stream_ends_with_lf(Stream* stream) {
    uint8_t last_char;
    size_t original_pos = stream_tell(stream);
    if(!stream_seek(stream, -1, StreamOffsetFromEnd) || stream_read(stream, &last_char, 1) < 1) {
        return false;
    }

    const uint8_t lf = '\n';
    if(last_char != lf && !stream_write(stream, &lf, 1)) {
        return false;
    }

    if(!stream_seek(stream, original_pos, StreamOffsetFromStart)) {
        return false;
    }

    return true;
}

static bool
    totp_token_info_iterator_save_current_token_info_changes(TokenInfoIteratorContext* context) {
    bool is_new_token = context->current_index >= context->total_count;
    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    if(is_new_token) {
        if(!ensure_stream_ends_with_lf(stream) ||
           !flipper_format_seek_to_end(context->config_file)) {
            return false;
        }
    } else {
        if(!seek_to_token(context->current_index, context)) {
            return false;
        }
    }

    size_t offset_start = stream_tell(stream);

    size_t offset_end;
    if(is_new_token) {
        offset_end = offset_start;
    } else if(context->current_index + 1 >= context->total_count) {
        offset_end = stream_size(stream);
    } else if(seek_to_token(context->current_index + 1, context)) {
        offset_end = stream_tell(stream);
    } else {
        return false;
    }

    FlipperFormat* temp_ff = flipper_format_file_alloc(context->storage);
    if(!flipper_format_file_open_always(temp_ff, CONFIG_FILE_PART_FILE_PATH)) {
        flipper_format_free(temp_ff);
        return false;
    }

    TokenInfo* token_info = context->current_token;
    bool result = false;

    do {
        if(!flipper_format_write_string(temp_ff, TOTP_CONFIG_KEY_TOKEN_NAME, token_info->name)) {
            break;
        }

        if(!flipper_format_write_hex(
               temp_ff,
               TOTP_CONFIG_KEY_TOKEN_SECRET,
               token_info->token,
               token_info->token_length)) {
            break;
        }

        uint32_t tmp_uint32 = token_info->algo;
        if(!flipper_format_write_uint32(temp_ff, TOTP_CONFIG_KEY_TOKEN_ALGO, &tmp_uint32, 1)) {
            break;
        }

        tmp_uint32 = token_info->digits;
        if(!flipper_format_write_uint32(temp_ff, TOTP_CONFIG_KEY_TOKEN_DIGITS, &tmp_uint32, 1)) {
            break;
        }

        tmp_uint32 = token_info->duration;
        if(!flipper_format_write_uint32(temp_ff, TOTP_CONFIG_KEY_TOKEN_DURATION, &tmp_uint32, 1)) {
            break;
        }

        tmp_uint32 = token_info->automation_features;
        if(!flipper_format_write_uint32(
               temp_ff, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, &tmp_uint32, 1)) {
            break;
        }

        Stream* temp_stream = flipper_format_get_raw_stream(temp_ff);

        if(!stream_rewind(temp_stream)) {
            break;
        }

        if(!stream_seek(stream, offset_start, StreamOffsetFromStart)) {
            break;
        }

        if(offset_end != offset_start && !stream_delete(stream, offset_end - offset_start)) {
            break;
        }

        if(!is_new_token && !stream_write_char(stream, '\n')) {
            break;
        }

        if(!stream_insert_stream(stream, temp_stream)) {
            break;
        }

        if(is_new_token) {
            context->total_count++;
        }

        result = true;
    } while(false);

    flipper_format_free(temp_ff);
    storage_common_remove(context->storage, CONFIG_FILE_PART_FILE_PATH);

    stream_seek(stream, offset_start, StreamOffsetFromStart);
    context->last_seek_offset = offset_start;
    context->last_seek_index = context->current_index;

    return result;
}

TokenInfoIteratorContext* totp_token_info_iterator_alloc(
    Storage* storage,
    FlipperFormat* config_file,
    CryptoSettings* crypto_settings) {
    Stream* stream = flipper_format_get_raw_stream(config_file);
    stream_rewind(stream);
    size_t tokens_count = 0;
    while(true) {
        if(!flipper_format_seek_to_siblinig_token_start(stream, StreamDirectionForward)) {
            break;
        }

        tokens_count++;
    }

    TokenInfoIteratorContext* context = malloc(sizeof(TokenInfoIteratorContext));
    furi_check(context != NULL);

    context->total_count = tokens_count;
    context->current_token = token_info_alloc();
    context->config_file = config_file;
    context->crypto_settings = crypto_settings;
    context->storage = storage;
    return context;
}

void totp_token_info_iterator_free(TokenInfoIteratorContext* context) {
    if(context == NULL) return;
    token_info_free(context->current_token);
    free(context);
}

bool totp_token_info_iterator_remove_current_token_info(TokenInfoIteratorContext* context) {
    if(!seek_to_token(context->current_index, context)) {
        return false;
    }

    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    size_t begin_offset = stream_tell(stream);
    size_t end_offset;
    if(!ensure_stream_ends_with_lf(stream)) {
        return false;
    }

    if(context->current_index >= context->total_count - 1) {
        end_offset = stream_size(stream) - 1;
    } else if(seek_to_token(context->current_index + 1, context)) {
        end_offset = stream_tell(stream);
    } else {
        return false;
    }

    if(!stream_seek(stream, begin_offset, StreamOffsetFromStart) ||
       !stream_delete(stream, end_offset - begin_offset)) {
        return false;
    }

    context->total_count--;
    if(context->current_index >= context->total_count) {
        context->current_index = context->total_count - 1;
    }

    return true;
}

bool totp_token_info_iterator_move_current_token_info(
    TokenInfoIteratorContext* context,
    size_t new_index) {
    if(context->current_index == new_index) return true;

    Stream* stream = flipper_format_get_raw_stream(context->config_file);

    if(!ensure_stream_ends_with_lf(stream)) {
        return false;
    }

    if(!seek_to_token(context->current_index, context)) {
        return false;
    }

    size_t begin_offset = stream_tell(stream);
    size_t end_offset;
    if(context->current_index >= context->total_count - 1) {
        end_offset = stream_size(stream) - 1;
    } else if(seek_to_token(context->current_index + 1, context)) {
        end_offset = stream_tell(stream);
    } else {
        return false;
    }

    Stream* temp_stream = file_stream_alloc(context->storage);
    if(!file_stream_open(
           temp_stream, CONFIG_FILE_PART_FILE_PATH, FSAM_READ_WRITE, FSOM_CREATE_ALWAYS)) {
        stream_free(temp_stream);
        return false;
    }

    size_t moving_size = end_offset - begin_offset;

    bool result = false;
    do {
        if(!stream_seek(stream, begin_offset, StreamOffsetFromStart)) {
            break;
        }

        if(stream_copy(stream, temp_stream, moving_size) < moving_size) {
            break;
        }

        if(!stream_rewind(temp_stream)) {
            break;
        }

        if(!stream_seek(stream, begin_offset, StreamOffsetFromStart)) {
            break;
        }

        if(!stream_delete(stream, moving_size)) {
            break;
        }

        context->last_seek_offset = 0;
        context->last_seek_index = 0;
        if(new_index >= context->total_count - 1) {
            if(!stream_seek(stream, stream_size(stream) - 1, StreamOffsetFromStart)) {
                break;
            }
        } else if(!seek_to_token(new_index, context)) {
            break;
        }

        result = stream_insert_stream(stream, temp_stream);
    } while(false);

    stream_free(temp_stream);
    storage_common_remove(context->storage, CONFIG_FILE_PART_FILE_PATH);

    context->last_seek_offset = 0;
    context->last_seek_index = 0;

    return result;
}

TotpIteratorUpdateTokenResult totp_token_info_iterator_update_current_token(
    TokenInfoIteratorContext* context,
    TOTP_ITERATOR_UPDATE_TOKEN_ACTION update,
    const void* update_context) {
    TotpIteratorUpdateTokenResult result = update(context->current_token, update_context);
    if(result == TotpIteratorUpdateTokenResultSuccess) {
        if(!totp_token_info_iterator_save_current_token_info_changes(context)) {
            result = TotpIteratorUpdateTokenResultFileUpdateFailed;
        }

        return result;
    }

    totp_token_info_iterator_go_to(context, context->current_index);
    return result;
}

TotpIteratorUpdateTokenResult totp_token_info_iterator_add_new_token(
    TokenInfoIteratorContext* context,
    TOTP_ITERATOR_UPDATE_TOKEN_ACTION update,
    const void* update_context) {
    size_t previous_index = context->current_index;
    context->current_index = context->total_count;
    token_info_set_defaults(context->current_token);
    TotpIteratorUpdateTokenResult result = update(context->current_token, update_context);
    if(result == TotpIteratorUpdateTokenResultSuccess &&
       !totp_token_info_iterator_save_current_token_info_changes(context)) {
        result = TotpIteratorUpdateTokenResultFileUpdateFailed;
    }

    if(result != TotpIteratorUpdateTokenResultSuccess) {
        totp_token_info_iterator_go_to(context, previous_index);
    }

    return result;
}

bool totp_token_info_iterator_go_to(TokenInfoIteratorContext* context, size_t token_index) {
    furi_check(context != NULL);
    context->current_index = token_index;
    if(!seek_to_token(context->current_index, context)) {
        return false;
    }

    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    size_t original_offset = stream_tell(stream);

    if(!flipper_format_read_string(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_NAME, context->current_token->name)) {
        stream_seek(stream, original_offset, StreamOffsetFromStart);
        return false;
    }

    uint32_t secret_bytes_count;
    if(!flipper_format_get_value_count(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, &secret_bytes_count)) {
        secret_bytes_count = 0;
    }
    TokenInfo* tokenInfo = context->current_token;
    bool token_update_needed = false;
    if(tokenInfo->token != NULL) {
        free(tokenInfo->token);
        tokenInfo->token_length = 0;
    }

    if(secret_bytes_count == 1) { // Plain secret key
        FuriString* temp_str = furi_string_alloc();

        if(flipper_format_read_string(
               context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str)) {
            if(token_info_set_secret(
                   tokenInfo,
                   furi_string_get_cstr(temp_str),
                   furi_string_size(temp_str),
                   PlainTokenSecretEncodingBase32,
                   context->crypto_settings)) {
                FURI_LOG_W(
                    LOGGING_TAG,
                    "Token \"%s\" has plain secret",
                    furi_string_get_cstr(tokenInfo->name));
                token_update_needed = true;
            } else {
                tokenInfo->token = NULL;
                tokenInfo->token_length = 0;
                FURI_LOG_W(
                    LOGGING_TAG,
                    "Token \"%s\" has invalid secret",
                    furi_string_get_cstr(tokenInfo->name));
            }
        } else {
            tokenInfo->token = NULL;
            tokenInfo->token_length = 0;
        }

        furi_string_free(temp_str);
    } else { // encrypted
        tokenInfo->token_length = secret_bytes_count;
        if(secret_bytes_count > 0) {
            tokenInfo->token = malloc(tokenInfo->token_length);
            furi_check(tokenInfo->token != NULL);
            if(!flipper_format_read_hex(
                   context->config_file,
                   TOTP_CONFIG_KEY_TOKEN_SECRET,
                   tokenInfo->token,
                   tokenInfo->token_length)) {
                free(tokenInfo->token);
                tokenInfo->token = NULL;
                tokenInfo->token_length = 0;
            }
        } else {
            tokenInfo->token = NULL;
        }
    }

    uint32_t temp_data32;
    if(!flipper_format_read_uint32(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_ALGO, &temp_data32, 1) ||
       !token_info_set_algo_from_int(tokenInfo, temp_data32)) {
        tokenInfo->algo = TokenHashAlgoDefault;
    }

    if(!flipper_format_read_uint32(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &temp_data32, 1) ||
       !token_info_set_digits_from_int(tokenInfo, temp_data32)) {
        tokenInfo->digits = TokenDigitsCountSix;
    }

    if(!flipper_format_read_uint32(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_DURATION, &temp_data32, 1) ||
       !token_info_set_duration_from_int(tokenInfo, temp_data32)) {
        tokenInfo->duration = TokenDurationDefault;
    }

    if(flipper_format_read_uint32(
           context->config_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, &temp_data32, 1)) {
        tokenInfo->automation_features = temp_data32;
    } else {
        tokenInfo->automation_features = TokenAutomationFeatureNone;
    }

    stream_seek(stream, original_offset, StreamOffsetFromStart);

    if(token_update_needed && !totp_token_info_iterator_save_current_token_info_changes(context)) {
        return false;
    }

    return true;
}

const TokenInfo*
    totp_token_info_iterator_get_current_token(const TokenInfoIteratorContext* context) {
    return context->current_token;
}

size_t totp_token_info_iterator_get_current_token_index(const TokenInfoIteratorContext* context) {
    return context->current_index;
}

size_t totp_token_info_iterator_get_total_count(const TokenInfoIteratorContext* context) {
    return context->total_count;
}

void totp_token_info_iterator_attach_to_config_file(
    TokenInfoIteratorContext* context,
    FlipperFormat* config_file) {
    context->config_file = config_file;
    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    stream_seek(stream, context->last_seek_offset, StreamOffsetFromStart);
}