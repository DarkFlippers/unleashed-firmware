#include "jtest_fw.h"

/**
 *  Dump the JTEST_FW.str_buffer the Keil framework in pieces.
 *
 *  The JTEST_FW.str_buffer contains more characters than the Keil framework can
 *  dump at once. This function dumps them in blocks.
 */
void jtest_dump_str_segments(void)
{
    uint32_t seg_idx      = 0;
    uint32_t memmove_idx = 0;
    uint32_t seg_cnt  =
        (strlen(JTEST_FW.str_buffer) / JTEST_STR_MAX_OUTPUT_SIZE) + 1;

    for( seg_idx = 0; seg_idx < seg_cnt; ++seg_idx)
    {
        JTEST_TRIGGER_ACTION(dump_str);

        if (seg_idx < JTEST_STR_MAX_OUTPUT_SEGMENTS)
        {
            memmove_idx = 0;
            while (memmove_idx < (seg_cnt - seg_idx -1) )
            {
                memmove(
                    JTEST_FW.str_buffer+
                    (memmove_idx* JTEST_STR_MAX_OUTPUT_SIZE),
                    JTEST_FW.str_buffer+
                    ((memmove_idx+1)*JTEST_STR_MAX_OUTPUT_SIZE),
                    JTEST_BUF_SIZE);
                ++memmove_idx;
            }
        }
    }
    return;
}
