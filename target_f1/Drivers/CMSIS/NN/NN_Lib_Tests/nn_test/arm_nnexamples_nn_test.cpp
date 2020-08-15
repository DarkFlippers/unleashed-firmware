/* ----------------------------------------------------------------------
* Copyright (C) 2010-2018 Arm Limited. All rights reserved.
*
*
* Project:       CMSIS NN Library
* Title:         arm_nnexamples_nn_test.cpp
*
* Description:   Example code for NN kernel testing.
*
* Target Processor: Cortex-M cores
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

#include "arm_nnexamples_nn_test.h"

//#define TEST_SIGMOID
//#define TEST_TANH
#define TEST_POOL
#define TEST_RELU
#define TEST_IP
#define TEST_CONV
#define TEST_NONSQUARE
#define TEST_NNMULT

int test_index = 0;
q7_t test_flags[50];
bool test_pass;

int main()
{
    printf("start tests\n");

    srand(1);

    // common pointers for testing data
    q7_t     *test1;
    q15_t    *test2;
    q7_t     *test3;
    q15_t    *test4;

    for (test_index = 0; test_index<50; test_index++) {
        test_flags[test_index] = -1;
    }
    test_index = 0;

#ifdef TEST_NNMULT
#define NNMULT_DIM 128
    test1 = new q7_t[NNMULT_DIM*2];
    test2 = new q15_t[NNMULT_DIM*2];
    test3 = new q7_t[NNMULT_DIM*2];
    test4 = new q15_t[NNMULT_DIM*2];

    q7_t * mult_out_q7 = test3;
    q7_t * mult_ref_q7 = test3 + NNMULT_DIM;
    q15_t * mult_out_q15 = test4;
    q15_t * mult_ref_q15 = test4 + NNMULT_DIM;

    for (int i=0;i<NNMULT_DIM*2;i++) {
        test1[i] = (rand() % 256 - 128);
        test2[i] = (rand() % 65536 - 32768);
    }

    // Test q7
    arm_nn_mult_q7(test1, test1+NNMULT_DIM, mult_out_q7, 5, NNMULT_DIM);

    arm_nn_mult_q7_ref(test1, test1+NNMULT_DIM, mult_ref_q7, 5, NNMULT_DIM);

    verify_results_q7(mult_out_q7, mult_ref_q7, NNMULT_DIM);

    arm_nn_mult_q7(test1, test1+NNMULT_DIM, mult_out_q7, 9, NNMULT_DIM);

    arm_nn_mult_q7_ref(test1, test1+NNMULT_DIM, mult_ref_q7, 9, NNMULT_DIM);

    verify_results_q7(mult_out_q7, mult_ref_q7, NNMULT_DIM);

    // Test q15
    arm_nn_mult_q15(test2, test2+NNMULT_DIM, mult_out_q15, 13, NNMULT_DIM);

    arm_nn_mult_q15_ref(test2, test2+NNMULT_DIM, mult_ref_q15, 13, NNMULT_DIM);

    verify_results_q15(mult_out_q15, mult_ref_q15, NNMULT_DIM);

    arm_nn_mult_q15(test2, test2+NNMULT_DIM, mult_out_q15, 18, NNMULT_DIM);

    arm_nn_mult_q15_ref(test2, test2+NNMULT_DIM, mult_ref_q15, 18, NNMULT_DIM);

    verify_results_q15(mult_out_q15, mult_ref_q15, NNMULT_DIM);

#endif

#ifdef TEST_SIGMOID

#define SIGMOID_DIM 128

    /* This part tests the running of sigmoid functions */

    test1 = new q7_t[SIGMOID_DIM];
    test2 = new q15_t[SIGMOID_DIM];
    test3 = new q7_t[SIGMOID_DIM];
    test4 = new q15_t[SIGMOID_DIM];

    srand(1);

    for (int i = 0; i < SIGMOID_DIM; i++)
    {
        test1[i] = (rand() % 256 - 128);
        test2[i] = (rand() % 65536 - 32768);
        test3[i] = test1[i];
        test4[i] = test2[i];
    }

    arm_nn_activations_direct_q7(test3, SIGMOID_DIM, 3, ARM_SIGMOID);

    for (int i = 0; i < SIGMOID_DIM; i++)
    {
        printf("in: %d  out: %d\n", test1[i], test3[i]);
    }

    printf("start testing q15_t sigmoid\n\n");

    arm_nn_activations_direct_q15(test4, SIGMOID_DIM, 3, ARM_SIGMOID);

    for (int i = 0; i < SIGMOID_DIM; i++)
    {
        printf("in: %d  out: %d\n", test2[i], test4[i]);
    }

    delete[]test1;
    delete[]test2;
    delete[]test3;
    delete[]test4;

#endif

#ifdef TEST_TANH

#define TANH_DIM 128

    /* This part tests the running of sigmoid functions */

    test1 = new q7_t[TANH_DIM];
    test2 = new q15_t[TANH_DIM];
    test3 = new q7_t[TANH_DIM];
    test4 = new q15_t[TANH_DIM];

    srand(1);

    for (int i = 0; i < TANH_DIM; i++)
    {
        test1[i] = (rand() % 256 - 128);
        test2[i] = (rand() % 65536 - 32768);
        test3[i] = test1[i];
        test4[i] = test2[i];
    }

    arm_nn_activations_direct_q7(test3, TANH_DIM, 3, ARM_TANH);

    printf("start testing q7_t tanh\n\n");

    for (int i = 0; i < TANH_DIM; i++)
    {
        printf("in: %d  out: %d\n", test1[i], test3[i]);
    }

    printf("start testing q15_t tanh\n\n");

    arm_nn_activations_direct_q15(test4, TANH_DIM, 3, ARM_TANH);

    for (int i = 0; i < TANH_DIM; i++)
    {
        printf("in: %d  out: %d\n", test2[i], test4[i]);
    }

    delete[]test1;
    delete[]test2;
    delete[]test3;
    delete[]test4;

#endif

#ifdef TEST_POOL

#define POOL_IM_DIM 32
#define POOL_IM_CH 8

    test1 = new q7_t[POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH * 2];
    test2 = new q15_t[POOL_IM_DIM * POOL_IM_CH];
    test3 = new q7_t[POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH];

    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        test1[i] = (rand() % 256 - 128);
    }

    q7_t     *img_in = test1 + POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH;
    q7_t     *pool_out_ref = test3;
    q7_t     *pool_out_opt = test3 + POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH / 2;

    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        test3[i] = 0;
    }

    // copy over the img input
    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        img_in[i] = test1[i];
    }

    initialize_results_q7(pool_out_ref, pool_out_opt, POOL_IM_DIM / 2 * POOL_IM_DIM / 2 * POOL_IM_CH);

    printf("Start maxpool reference implementation\n");

    arm_maxpool_q7_HWC_ref(img_in, POOL_IM_DIM, POOL_IM_CH, 3, 0, 2, POOL_IM_DIM / 2, (q7_t *) test2, pool_out_ref);

    // copy over the img input
    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        img_in[i] = test1[i];
    }

    printf("Start maxpool opt implementation\n");

    arm_maxpool_q7_HWC(img_in, POOL_IM_DIM, POOL_IM_CH, 3, 0, 2, POOL_IM_DIM / 2, (q7_t *) test2, pool_out_opt);

    verify_results_q7(pool_out_ref, pool_out_opt, POOL_IM_DIM / 2 * POOL_IM_DIM / 2 * POOL_IM_CH);

    // copy over the img input
    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        img_in[i] = test1[i];
    }

    // copy over the img input
    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        img_in[i] = test1[i];
    }

    printf("Start avepool ref implementation\n");

    arm_avepool_q7_HWC_ref(img_in, POOL_IM_DIM, POOL_IM_CH, 3, 0, 2, POOL_IM_DIM / 2, (q7_t *) test2, pool_out_ref);

    // copy over the img input
    for (int i = 0; i < POOL_IM_DIM * POOL_IM_DIM * POOL_IM_CH; i++)
    {
        img_in[i] = test1[i];
    }

    printf("Start avepool opt implementation\n");

    arm_avepool_q7_HWC(img_in, POOL_IM_DIM, POOL_IM_CH, 3, 0, 2, POOL_IM_DIM / 2, (q7_t *) test2, pool_out_opt);

    // special check here
    bool      if_ave_pool_match = true;
    for (int i = 0; i < POOL_IM_DIM / 2 * POOL_IM_DIM / 2 * POOL_IM_CH; i++)
    {
        // we tolerate at most difference of 1 here because of rounding errors
        if (pool_out_ref[i] - pool_out_opt[i] >= 2 || pool_out_opt[i] - pool_out_ref[i] >= 2)
        {
            printf("Output mismatch at %d, expected %d, actual %d\n", i, pool_out_ref[i], pool_out_opt[i]);
            if_ave_pool_match = false;
        }
    }
    if (if_ave_pool_match == true)
    {
        printf("Outputs match.\n");
    }

    delete[]test1;
    delete[]test2;
    delete[]test3;

#endif

#ifdef TEST_RELU

#define RELU_DIM 127

    test1 = new q7_t[RELU_DIM];
    test2 = new q15_t[RELU_DIM];
    test3 = new q7_t[RELU_DIM];
    test4 = new q15_t[RELU_DIM];

    for (int i = 0; i < RELU_DIM; i++)
    {
        test1[i] = (rand() % 256 - 128);
        test2[i] = (rand() % 65536 - 32768);
        test3[i] = test1[i];
        test4[i] = test2[i];
    }

    q7_t     *relu_ref_data_q7 = test1;
    q7_t     *relu_opt_data_q7 = test3;
    q15_t    *relu_ref_data_q15 = test2;
    q15_t    *relu_opt_data_q15 = test4;

    printf("Start ref relu q7 implementation\n");

    arm_relu_q7_ref(relu_ref_data_q7, RELU_DIM);

    printf("Start opt relu q7 implementation\n");

    arm_relu_q7(relu_opt_data_q7, RELU_DIM);

    verify_results_q7(relu_ref_data_q7, relu_opt_data_q7, RELU_DIM);

    printf("Start ref relu q15 implementation\n");

    arm_relu_q15_ref(relu_ref_data_q15, RELU_DIM);

    printf("Start opt relu q15 implementation\n");

    arm_relu_q15(relu_opt_data_q15, RELU_DIM);

    verify_results_q15(relu_ref_data_q15, relu_opt_data_q15, RELU_DIM);

    delete[]test1;
    delete[]test2;
    delete[]test3;
    delete[]test4;

#endif

#ifdef TEST_IP

#define IP_ROW_DIM 127
#define IP_COL_DIM 127

    q7_t      ip_weights[IP_ROW_DIM * IP_COL_DIM] = IP2_WEIGHT;
    q7_t      ip_q7_opt_weights[IP_ROW_DIM * IP_COL_DIM] = IP4_WEIGHT;
    q7_t      ip_q7_q15_opt_weights[IP_ROW_DIM * IP_COL_DIM] = IP4_q7_q15_WEIGHT;
    q15_t     ip_q15_weights[IP_ROW_DIM * IP_COL_DIM] = IP2_WEIGHT;
    q15_t     ip_q15_opt_weights[IP_ROW_DIM * IP_COL_DIM] = IP4_WEIGHT_Q15;

    test1 = new q7_t[IP_COL_DIM + IP_ROW_DIM];
    test2 = new q15_t[IP_COL_DIM];
    test3 = new q7_t[IP_ROW_DIM * 3];
    test4 = new q15_t[IP_COL_DIM + IP_ROW_DIM * 2];

    for (int i = 0; i < IP_ROW_DIM + IP_COL_DIM; i++)
    {
        test1[i] = rand() % 256 - 100;
    }
    for (int i = 0; i < IP_ROW_DIM * 3; i++)
    {
        test3[i] = 0;
    }

    q7_t     *ip_bias_q7 = test1 + IP_COL_DIM;

    q7_t     *ip_out_q7_ref = test3;
    q7_t     *ip_out_q7_opt = test3 + IP_ROW_DIM;
    q7_t     *ip_out_q7_opt_fast = test3 + 2 * IP_ROW_DIM;
    q15_t    *ip_out_q15_ref = test4 + IP_COL_DIM;
    q15_t    *ip_out_q15_opt = test4 + IP_COL_DIM + IP_ROW_DIM;

    initialize_results_q7(ip_out_q7_ref, ip_out_q7_opt, IP_ROW_DIM);
    initialize_results_q7(ip_out_q7_ref, ip_out_q7_opt_fast, IP_ROW_DIM);
    initialize_results_q7(ip_out_q7_ref, ip_out_q7_opt_fast, IP_ROW_DIM);

    printf("Start ref q7 implementation\n");

    arm_fully_connected_q7_ref(test1, ip_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7, ip_out_q7_ref, test2);

    printf("Start q7 implementation\n");

    arm_fully_connected_q7(test1, ip_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7, ip_out_q7_opt, test2);

    verify_results_q7(ip_out_q7_ref, ip_out_q7_opt, IP_ROW_DIM);

    printf("Start q7 ref opt implementation\n");

    arm_fully_connected_q7_opt_ref(test1, ip_q7_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7,
                                   ip_out_q7_opt_fast, test2);

    verify_results_q7(ip_out_q7_ref, ip_out_q7_opt_fast, IP_ROW_DIM);

    printf("Start q7 opt implementation\n");

    arm_fully_connected_q7_opt(test1, ip_q7_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7, ip_out_q7_opt_fast,
                               test2);

    verify_results_q7(ip_out_q7_ref, ip_out_q7_opt_fast, IP_ROW_DIM);

    for (int i = 0; i < IP_ROW_DIM + IP_COL_DIM; i++)
    {
        test4[i] = (rand() % 65536 - 32768);
    }

    initialize_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start ref q15 implementation\n");

    arm_fully_connected_q15_ref(test4, ip_q15_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, test2, ip_out_q15_ref, NULL);

    printf("Start q15 implementation\n");

    arm_fully_connected_q15(test4, ip_q15_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, test2, ip_out_q15_opt, NULL);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start ref opt q15 implementation\n");

    arm_fully_connected_q15_opt_ref(test4, ip_q15_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, test2, ip_out_q15_opt,
                                    NULL);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start opt q15 implementation\n");

    arm_fully_connected_q15_opt(test4, ip_q15_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, test2, ip_out_q15_opt, NULL);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    initialize_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start ref q7_q15 implementation\n");

    arm_fully_connected_mat_q7_vec_q15_ref(test4, ip_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7, ip_out_q15_ref,
                                           test2);

    printf("Start q7_q15 implementation\n");

    arm_fully_connected_mat_q7_vec_q15(test4, ip_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7, ip_out_q15_opt,
                                       test2);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start ref opt q7_q15 implementation\n");

    arm_fully_connected_mat_q7_vec_q15_opt_ref(test4, ip_q7_q15_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7,
                                               ip_out_q15_opt, test2);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    printf("Start opt q7_q15 implementation\n");

    arm_fully_connected_mat_q7_vec_q15_opt(test4, ip_q7_q15_opt_weights, IP_COL_DIM, IP_ROW_DIM, 1, 7, ip_bias_q7,
                                           ip_out_q15_opt, test2);

    verify_results_q15(ip_out_q15_ref, ip_out_q15_opt, IP_ROW_DIM);

    delete[]test1;
    delete[]test2;
    delete[]test3;
    delete[]test4;

#endif

#ifdef TEST_NONSQUARE

/* Use RCONV to differential with square CONV */

#define RCONV_IM_DIM_X 10
#define RCONV_IM_DIM_Y 8
#define RCONV_IM_CH 4
#define RCONV_KER_DIM_X 5
#define RCONV_KER_DIM_Y 3
#define RCONV_STRIDE_X 1
#define RCONV_STRIDE_Y 1
#define RCONV_PADDING_X 2
#define RCONV_PADDING_Y 1
#define RCONV_OUT_CH 4
#define RCONV_OUT_DIM_X 10
#define RCONV_OUT_DIM_Y 8

    test1 = new q7_t[RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH + RCONV_OUT_CH];
    test2 = new q15_t[2 * RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH];
    test3 =
        new q7_t[RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH + 2 * RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH];

    for (int i = 0; i < RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH + RCONV_OUT_CH; i++)
    {
        test1[i] = rand() % 256 - 100;
    }

    for (int i = 0;
         i < RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH + 2 * RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH; i++)
    {
        test3[i] = rand() % 256 - 100;
    }

    q7_t     *rconv_weight_q7 = test1;
    q7_t     *rconv_bias_q7 = test1 + RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH;

    q15_t    *rconv_buf = test2;

    q7_t     *rconv_im_in_q7 = test3;
    q7_t     *rconv_im_out_ref_q7 = test3 + RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH;
    q7_t     *rconv_im_out_opt_q7 =
        test3 + RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH + RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH;

    initialize_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    printf("start conv q7 nonsquare ref implementation\n");
    arm_convolve_HWC_q7_ref_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                      RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                      RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_ref_q7,
                                      RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    printf("start conv q7 nonsquare opt implementation\n");
    arm_convolve_HWC_q7_fast_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                       RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                       RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_opt_q7,
                                       RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    verify_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    initialize_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    printf("start conv q7 nonsquare ref implementation\n");
    arm_convolve_HWC_q7_ref_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                      RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                      RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_ref_q7,
                                      RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    printf("start conv q7 nonsquare basic implementation\n");
    arm_convolve_HWC_q7_basic_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                       RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                       RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_opt_q7,
                                       RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    verify_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    initialize_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    printf("start 1x1 conv q7 nonsquare fast implementation\n");
    arm_convolve_HWC_q7_fast_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                       RCONV_OUT_CH, 1, 1, 0, 0, RCONV_STRIDE_X,
                                       RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_ref_q7, RCONV_OUT_DIM_X,
                                       RCONV_OUT_DIM_Y, rconv_buf, NULL);

    printf("start 1x1 conv q7 nonsquare dedicated function implementation\n");
    arm_convolve_1x1_HWC_q7_fast_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q7,
                                           RCONV_OUT_CH, 1, 1, 0, 0, RCONV_STRIDE_X,
                                           RCONV_STRIDE_Y, rconv_bias_q7, 1, 7, rconv_im_out_opt_q7, RCONV_OUT_DIM_X,
                                           RCONV_OUT_DIM_Y, rconv_buf, NULL);

    verify_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    printf("start depthwise separable conv q7 nonsquare ref implementation\n");
    arm_depthwise_separable_conv_HWC_q7_ref_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH,
                                                      rconv_weight_q7, RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y,
                                                      RCONV_PADDING_X, RCONV_PADDING_Y, RCONV_STRIDE_X, RCONV_STRIDE_Y,
                                                      rconv_bias_q7, 1, 7, rconv_im_out_ref_q7, RCONV_OUT_DIM_X,
                                                      RCONV_OUT_DIM_Y, rconv_buf, NULL);

    printf("start depthwise separable conv q7 nonsquare opt implementation\n");
    arm_depthwise_separable_conv_HWC_q7_nonsquare(rconv_im_in_q7, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH,
                                                  rconv_weight_q7, RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y,
                                                  RCONV_PADDING_X, RCONV_PADDING_Y, RCONV_STRIDE_X, RCONV_STRIDE_Y,
                                                  rconv_bias_q7, 1, 7, rconv_im_out_opt_q7, RCONV_OUT_DIM_X,
                                                  RCONV_OUT_DIM_Y, rconv_buf, NULL);

    verify_results_q7(rconv_im_out_ref_q7, rconv_im_out_opt_q7, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    delete[]test1;
    delete[]test2;
    delete[]test3;
	
	test2 = new q15_t[RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH + RCONV_OUT_CH]; // weights + bias
	test4 = new q15_t[2 * RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH   //buffer
	         + RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH + 2 * RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH]; // i/o

    for (int i = 0; i < RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH + RCONV_OUT_CH; i++)
    {
        test2[i] = rand() % 256 - 100;
    }

    for (int i = 0;
         i < 2 * RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH
         + RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH + 2 * RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH;
        i++)
    {
        test4[i] = rand() % 256 - 100;
    }

    q15_t     *rconv_weight_q15 = test2;
    q15_t     *rconv_bias_q15 = test2 + RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH * RCONV_OUT_CH;

    rconv_buf = test4;

    q15_t     *rconv_im_in_q15 = test4 + 2 * RCONV_KER_DIM_Y * RCONV_KER_DIM_X * RCONV_IM_CH;
    q15_t     *rconv_im_out_ref_q15 = rconv_im_in_q15 + RCONV_IM_DIM_Y * RCONV_IM_DIM_X * RCONV_IM_CH;
    q15_t     *rconv_im_out_opt_q15 = rconv_im_out_ref_q15 + RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH;

    initialize_results_q15(rconv_im_out_ref_q15, rconv_im_out_opt_q15, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);

    printf("start conv q15 nonsquare ref implementation\n");
    arm_convolve_HWC_q15_nonsquare_ref(rconv_im_in_q15, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q15,
                                      RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                      RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q15, 1, 7, rconv_im_out_ref_q15,
                                      RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    printf("start conv q5 nonsquare opt implementation\n");
    arm_convolve_HWC_q15_fast_nonsquare(rconv_im_in_q15, RCONV_IM_DIM_X, RCONV_IM_DIM_Y, RCONV_IM_CH, rconv_weight_q15,
                                       RCONV_OUT_CH, RCONV_KER_DIM_X, RCONV_KER_DIM_Y, RCONV_PADDING_X, RCONV_PADDING_Y,
                                       RCONV_STRIDE_X, RCONV_STRIDE_Y, rconv_bias_q15, 1, 7, rconv_im_out_opt_q15,
                                       RCONV_OUT_DIM_X, RCONV_OUT_DIM_Y, rconv_buf, NULL);

    verify_results_q15(rconv_im_out_ref_q15, rconv_im_out_opt_q15, RCONV_OUT_DIM_Y * RCONV_OUT_DIM_X * RCONV_OUT_CH);
	
    delete [] test2;
    delete [] test4;
#endif

#ifdef TEST_CONV

#define CONV_IM_DIM 16
#define CONV_IM_CH 16
#define CONV_KER_DIM 5
#define CONV_OUT_CH 16
#define CONV_OUT_DIM 16

    test1 = new q7_t[CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH + CONV_OUT_CH];
    test2 =
        new q15_t[CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH +
                  2 * CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH + CONV_OUT_CH];
    test3 = new q7_t[CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + 2 * CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH];
    test4 = new q15_t[CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + 2 * CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH];

    for (int i = 0; i < CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH + CONV_OUT_CH; i++)
    {
        test1[i] = rand() % 256 - 100;
    }

    for (int i = 0;
         i <
         CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH +
         2 * CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH + CONV_OUT_CH; i++)
    {
        test2[i] = (rand() % 65536 - 32768);
    }

    for (int i = 0; i < CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + 2 * CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH; i++)
    {
        test3[i] = rand() % 256 - 100;
    }

    for (int i = 0; i < CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + 2 * CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH; i++)
    {
        test4[i] = (rand() % 65536 - 32768);
    }

    q7_t     *conv_weight_q7 = test1;
    q7_t     *conv_bias_q7 = test1 + CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH;

    q15_t    *conv_weight_q15 = test2;
    q15_t    *conv_buf = test2 + CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH;
    q15_t    *conv_bias_q15 =
        test2 + CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH +
        2 * CONV_KER_DIM * CONV_KER_DIM * CONV_IM_CH * CONV_OUT_CH;

    q7_t     *conv_im_in_q7 = test3;
    q7_t     *conv_im_out_ref_q7 = test3 + CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH;
    q7_t     *conv_im_out_opt_q7 =
        test3 + CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH;

    q15_t    *conv_im_in_q15 = test4;
    q15_t    *conv_im_out_ref_q15 = test4 + CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH;
    q15_t    *conv_im_out_opt_q15 =
        test4 + CONV_IM_DIM * CONV_IM_DIM * CONV_IM_CH + CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH;

    initialize_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q7 ref implementation\n");

    arm_convolve_HWC_q7_ref(conv_im_in_q7, CONV_IM_DIM, CONV_IM_CH, conv_weight_q7,
                            CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_ref_q7,
                            CONV_OUT_DIM, conv_buf, NULL);

    printf("start q7 basic implementation\n");

    arm_convolve_HWC_q7_basic(conv_im_in_q7, CONV_IM_DIM, CONV_IM_CH, conv_weight_q7,
                              CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_opt_q7,
                              CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q7 fast implementation\n");

    arm_convolve_HWC_q7_fast(conv_im_in_q7, CONV_IM_DIM, CONV_IM_CH, conv_weight_q7,
                             CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_opt_q7,
                             CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    // testing with RGB
    printf("start q7 ref implementation for RGB\n");

    arm_convolve_HWC_q7_ref(conv_im_in_q7, CONV_IM_DIM, 3, conv_weight_q7,
                            CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_ref_q7,
                            CONV_OUT_DIM, conv_buf, NULL);

    printf("start q7 basic implementation for RGB\n");

    arm_convolve_HWC_q7_basic(conv_im_in_q7, CONV_IM_DIM, 3, conv_weight_q7,
                              CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_opt_q7,
                              CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q7 RGB implementation for RGB\n");

    arm_convolve_HWC_q7_RGB(conv_im_in_q7, CONV_IM_DIM, 3, conv_weight_q7,
                            CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_opt_q7,
                            CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    // testing q15
    initialize_results_q15(conv_im_out_ref_q15, conv_im_out_opt_q15, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q15 ref implementation\n");

    arm_convolve_HWC_q15_ref(conv_im_in_q15, CONV_IM_DIM, CONV_IM_CH, conv_weight_q15,
                             CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q15, 0, 15, conv_im_out_ref_q15,
                             CONV_OUT_DIM, conv_buf, NULL);

    printf("start q15 basic implementation\n");

    arm_convolve_HWC_q15_basic(conv_im_in_q15, CONV_IM_DIM, CONV_IM_CH, conv_weight_q15,
                               CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q15, 0, 15, conv_im_out_opt_q15,
                               CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q15(conv_im_out_ref_q15, conv_im_out_opt_q15, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q15 fast implementation\n");

    arm_convolve_HWC_q15_fast(conv_im_in_q15, CONV_IM_DIM, CONV_IM_CH, conv_weight_q15,
                              CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q15, 0, 15, conv_im_out_opt_q15,
                              CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q15(conv_im_out_ref_q15, conv_im_out_opt_q15, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    // depthwise separable conv
    initialize_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    printf("start q7 depthwise_separable_conv ref implementation\n");

    arm_depthwise_separable_conv_HWC_q7_ref(conv_im_in_q7, CONV_IM_DIM, CONV_IM_CH, conv_weight_q7,
                                            CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_ref_q7,
                                            CONV_OUT_DIM, conv_buf, NULL);

    printf("start q7 depthwise_separable_conv implementation\n");

    arm_depthwise_separable_conv_HWC_q7(conv_im_in_q7, CONV_IM_DIM, CONV_IM_CH, conv_weight_q7,
                                        CONV_OUT_CH, CONV_KER_DIM, 2, 1, conv_bias_q7, 1, 7, conv_im_out_opt_q7,
                                        CONV_OUT_DIM, conv_buf, NULL);

    verify_results_q7(conv_im_out_ref_q7, conv_im_out_opt_q7, CONV_OUT_DIM * CONV_OUT_DIM * CONV_OUT_CH);

    delete[]test1;
    delete[]test2;
    delete[]test3;
    delete[]test4;

#endif

    test_pass = true;
    test_index = 0;
    while (test_flags[test_index] != -1) {
        if (test_flags[test_index]) {
             test_pass = false;
        }
        test_index ++;
    }
    if (test_pass) {
        printf("All tests passed\n");
    } else {
        printf("Test failed passed\n");
    }

    return 0;
}
