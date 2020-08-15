#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "arm_math.h"

#include "arm_nnfunctions.h"
#include "ref_functions.h"

extern int test_index;
extern q7_t test_flags[50];

void initialize_results_q7(q7_t * ref, q7_t * opt, int length)
{
    arm_fill_q7(0, ref, length);
    arm_fill_q7(37, opt, length);
}

void initialize_results_q15(q15_t * ref, q15_t * opt, int length)
{
    arm_fill_q15(0, ref, length);
    arm_fill_q15(0x5F5, opt, length);
}

void verify_results_q7(q7_t * ref, q7_t * opt, int length)
{

    bool      if_match = true;

    for (int i = 0; i < length; i++)
    {
        if (ref[i] != opt[i])
        {
            printf("Output mismatch at %d, expected %d, actual %d\r\n", i, ref[i], opt[i]);

            if_match = false;
        }
    }

    if (if_match == true)
    {
        printf("Outputs match.\r\n\r\n");
        test_flags[test_index++] = 0;
    } else {
        test_flags[test_index++] = 1;
    }

}

void verify_results_q15(q15_t * ref, q15_t * opt, int length)
{

    bool      if_match = true;

    for (int i = 0; i < length; i++)
    {
        if (ref[i] != opt[i])
        {
            printf("Output mismatch at %d, expected %d, actual %d\r\n", i, ref[i], opt[i]);

            if_match = false;
        }
    }

    if (if_match == true)
    {
        printf("Outputs match.\r\n\r\n");
        test_flags[test_index++] = 0;
    } else {
        test_flags[test_index++] = 1;
    }

}

#endif
