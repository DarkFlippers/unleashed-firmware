/**
 * @file compress.h
 * LZSS based compression HAL API
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Compress Icon control structure */
typedef struct CompressIcon CompressIcon;

/** Initialize icon compressor
 *
 * @param[in]  decode_buf_size  The icon buffer size for decoding. Ensure that
 *                              it's big enough for any icons that you are
 *                              planning to decode with it.
 *
 * @return     Compress Icon instance
 */
CompressIcon* compress_icon_alloc(size_t decode_buf_size);

/** Free icon compressor
 *
 * @param      instance  The Compress Icon instance
 */
void compress_icon_free(CompressIcon* instance);

/** Decompress icon
 *
 * @warning    output pointer set by this function is valid till next
 *             `compress_icon_decode` or `compress_icon_free` call
 *
 * @param      instance   The Compress Icon instance
 * @param      icon_data  pointer to icon data.
 * @param[in]  output     pointer to decoded buffer pointer. Data in buffer is
 *                        valid till next call. If icon data was not compressed,
 *                        pointer within icon_data is returned
 */
void compress_icon_decode(CompressIcon* instance, const uint8_t* icon_data, uint8_t** output);

//////////////////////////////////////////////////////////////////////////

/** Compress control structure */
typedef struct Compress Compress;

/** Supported compression types */
typedef enum {
    CompressTypeHeatshrink = 0,
} CompressType;

/** Configuration for heatshrink compression */
typedef struct {
    uint16_t window_sz2;
    uint16_t lookahead_sz2;
    uint16_t input_buffer_sz;
} CompressConfigHeatshrink;

/** Default configuration for heatshrink compression. Used for image assets. */
extern const CompressConfigHeatshrink compress_config_heatshrink_default;

/** Allocate encoder and decoder
 *
 * @param      type     Compression type
 * @param[in]  config   Configuration for compression, specific to type
 *
 * @return     Compress instance
 */
Compress* compress_alloc(CompressType type, const void* config);

/** Free encoder and decoder
 *
 * @param      compress  Compress instance
 */
void compress_free(Compress* compress);

/** Encode data
 *
 * @param      compress       Compress instance
 * @param      data_in        pointer to input data
 * @param      data_in_size   size of input data
 * @param      data_out       maximum size of output data
 * @param[in]  data_out_size  The data out size
 * @param      data_res_size  pointer to result output data size
 *
 * @note       Prepends compressed stream with a header. If data is not compressible,
 *             it will be stored as is after the header.
 * @return     true on success
 */
bool compress_encode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

/** Decode data
 *
 * @param      compress       Compress instance
 * @param      data_in        pointer to input data
 * @param      data_in_size   size of input data
 * @param      data_out       maximum size of output data
 * @param[in]  data_out_size  The data out size
 * @param      data_res_size  pointer to result output data size
 *
 * @note       Expects compressed stream with a header, as produced by `compress_encode`.
 * @return     true on success
 */
bool compress_decode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

/** I/O callback for streamed compression/decompression
 * 
 * @param context user context
 * @param buffer buffer to read/write
 * @param size size of buffer
 * 
 * @return number of bytes read/written, 0 on end of stream, negative on error
 */
typedef int32_t (*CompressIoCallback)(void* context, uint8_t* buffer, size_t size);

/** Decompress streamed data
 *
 * @param      compress       Compress instance
 * @param      read_cb        read callback
 * @param      read_context   read callback context
 * @param      write_cb       write callback
 * @param      write_context  write callback context
 *
 * @note       Does not expect a header, just compressed data stream.
 * @return     true on success
 */
bool compress_decode_streamed(
    Compress* compress,
    CompressIoCallback read_cb,
    void* read_context,
    CompressIoCallback write_cb,
    void* write_context);

//////////////////////////////////////////////////////////////////////////

/** CompressStreamDecoder control structure */
typedef struct CompressStreamDecoder CompressStreamDecoder;

/** Allocate stream decoder
 *
 * @param      type          Compression type
 * @param[in]  config        Configuration for compression, specific to type
 * @param      read_cb       The read callback for input (compressed) data
 * @param      read_context  The read context
 *
 * @return     CompressStreamDecoder instance
 */
CompressStreamDecoder* compress_stream_decoder_alloc(
    CompressType type,
    const void* config,
    CompressIoCallback read_cb,
    void* read_context);

/** Free stream decoder
 *
 * @param      instance  The CompressStreamDecoder instance
 */
void compress_stream_decoder_free(CompressStreamDecoder* instance);

/** Read uncompressed data chunk from stream decoder
 *
 * @param      instance       The CompressStreamDecoder instance
 * @param      data_out       The data out
 * @param[in]  data_out_size  The data out size
 *
 * @return     true on success
 */
bool compress_stream_decoder_read(
    CompressStreamDecoder* instance,
    uint8_t* data_out,
    size_t data_out_size);

/** Seek to position in uncompressed data stream
 *
 * @param      instance   The CompressStreamDecoder instance
 * @param[in]  position   The position
 * 
 * @return     true on success
 * @warning    Backward seeking is not supported
 */
bool compress_stream_decoder_seek(CompressStreamDecoder* instance, size_t position);

/** Get current position in uncompressed data stream
 *
 * @param      instance  The CompressStreamDecoder instance
 *
 * @return     current position
 */
size_t compress_stream_decoder_tell(CompressStreamDecoder* instance);

/** Reset stream decoder to the beginning
 * @warning    Read callback must be repositioned by caller separately
 *
 * @param      instance  The CompressStreamDecoder instance
 *
 * @return     true on success
 */
bool compress_stream_decoder_rewind(CompressStreamDecoder* instance);

#ifdef __cplusplus
}
#endif
