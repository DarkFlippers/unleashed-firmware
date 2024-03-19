#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Early initialization */
void furi_hal_dma_init_early(void);

/** Early de-initialization */
void furi_hal_dma_deinit_early(void);

#ifdef __cplusplus
}
#endif
