#include "dfu_file.h"

#include <furi_hal.h>
#include <toolbox/crc32_calc.h>

#define VALID_WHOLE_FILE_CRC 0xFFFFFFFF
#define DFU_SUFFIX_VERSION   0x011A
#define DFU_SIGNATURE        "DfuSe"

bool dfu_file_validate_crc(File* dfuf, const DfuPageTaskProgressCb progress_cb, void* context) {
    uint32_t file_crc = crc32_calc_file(dfuf, progress_cb, context);

    /* Last 4 bytes of DFU file = CRC of previous file contents, inverted
     * If we calculate whole file CRC32, incl. embedded CRC,
     * that should give us 0xFFFFFFFF 
     */
    return file_crc == VALID_WHOLE_FILE_CRC;
}

uint8_t dfu_file_validate_headers(File* dfuf, const DfuValidationParams* reference_params) {
    furi_assert(reference_params);

    DfuPrefix dfu_prefix = {0};
    DfuSuffix dfu_suffix = {0};
    size_t bytes_read = 0;

    if(!storage_file_is_open(dfuf) || !storage_file_seek(dfuf, 0, true)) {
        return 0;
    }

    const uint32_t dfu_suffix_offset = storage_file_size(dfuf) - sizeof(DfuSuffix);

    bytes_read = storage_file_read(dfuf, &dfu_prefix, sizeof(DfuPrefix));
    if(bytes_read != sizeof(DfuPrefix)) {
        return 0;
    }

    if(memcmp(dfu_prefix.szSignature, DFU_SIGNATURE, sizeof(dfu_prefix.szSignature)) != 0) {
        return 0;
    }

    if((dfu_prefix.bVersion != 1) || (dfu_prefix.DFUImageSize != dfu_suffix_offset)) {
        return 0;
    }

    if(!storage_file_seek(dfuf, dfu_suffix_offset, true)) {
        return 0;
    }

    bytes_read = storage_file_read(dfuf, &dfu_suffix, sizeof(DfuSuffix));
    if(bytes_read != sizeof(DfuSuffix)) {
        return 0;
    }

    if((dfu_suffix.bLength != sizeof(DfuSuffix)) || (dfu_suffix.bcdDFU != DFU_SUFFIX_VERSION)) {
        return 0;
    }
    /* TODO FL-3561: check DfuSignature?.. */

    if((dfu_suffix.idVendor != reference_params->vendor) ||
       (dfu_suffix.idProduct != reference_params->product) ||
       (dfu_suffix.bcdDevice != reference_params->device)) {
        return 0;
    }

    return dfu_prefix.bTargets;
}

/* Assumes file is open, valid and read pointer is set at the start of image data
 */
static DfuUpdateBlockResult dfu_file_perform_task_for_update_pages(
    const DfuUpdateTask* task,
    File* dfuf,
    const ImageElementHeader* header) {
    furi_assert(task);
    furi_assert(header);
    task->progress_cb(0, task->context);
    const size_t FLASH_PAGE_SIZE = furi_hal_flash_get_page_size();
    const size_t FLASH_PAGE_ALIGNMENT_MASK = FLASH_PAGE_SIZE - 1;
    if((header->dwElementAddress & FLASH_PAGE_ALIGNMENT_MASK) != 0) {
        /* start address is not aligned by page boundary -- we don't support that. Yet. */
        return UpdateBlockResult_Failed;
    }

    if(task->address_cb && (!task->address_cb(header->dwElementAddress) ||
                            !task->address_cb(header->dwElementAddress + header->dwElementSize))) {
        storage_file_seek(dfuf, header->dwElementSize, false);
        task->progress_cb(100, task->context);
        return UpdateBlockResult_Skipped;
    }

    uint8_t* fw_block = malloc(FLASH_PAGE_SIZE);
    size_t bytes_read = 0;
    uint32_t element_offs = 0;

    while(element_offs < header->dwElementSize) {
        uint32_t n_bytes_to_read = FLASH_PAGE_SIZE;
        if((element_offs + n_bytes_to_read) > header->dwElementSize) {
            n_bytes_to_read = header->dwElementSize - element_offs;
        }

        bytes_read = storage_file_read(dfuf, fw_block, n_bytes_to_read);
        if(bytes_read == 0) {
            break;
        }

        int16_t i_page = furi_hal_flash_get_page_number(header->dwElementAddress + element_offs);
        if(i_page < 0) {
            break;
        }

        if(!task->task_cb(i_page, fw_block, bytes_read)) {
            break;
        }

        element_offs += bytes_read;
        task->progress_cb(element_offs * 100 / header->dwElementSize, task->context);
    }

    free(fw_block);
    return (element_offs == header->dwElementSize) ? UpdateBlockResult_OK :
                                                     UpdateBlockResult_Failed;
}

bool dfu_file_process_targets(const DfuUpdateTask* task, File* dfuf, const uint8_t n_targets) {
    TargetPrefix target_prefix = {0};
    ImageElementHeader image_element = {0};
    size_t bytes_read = 0;

    if(!storage_file_seek(dfuf, sizeof(DfuPrefix), true)) {
        return UpdateBlockResult_Failed;
    };

    for(uint8_t i_target = 0; i_target < n_targets; ++i_target) {
        bytes_read = storage_file_read(dfuf, &target_prefix, sizeof(TargetPrefix));
        if(bytes_read != sizeof(TargetPrefix)) {
            return UpdateBlockResult_Failed;
        }

        /* TODO FL-3562: look into TargetPrefix and validate/filter?.. */
        for(uint32_t i_element = 0; i_element < target_prefix.dwNbElements; ++i_element) {
            bytes_read = storage_file_read(dfuf, &image_element, sizeof(ImageElementHeader));
            if(bytes_read != sizeof(ImageElementHeader)) {
                return UpdateBlockResult_Failed;
            }

            if(dfu_file_perform_task_for_update_pages(task, dfuf, &image_element) ==
               UpdateBlockResult_Failed) {
                return false;
            }
        }
    }

    return true;
}
