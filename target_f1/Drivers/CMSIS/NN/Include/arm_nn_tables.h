/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_nn_tables.h
 * Description:  Extern declaration for NN tables
 *
 * $Date:        17. January 2018
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ARM_NN_TABLES_H
#define _ARM_NN_TABLES_H

#include "arm_math.h"

/**
* @brief tables for various activation functions
*
*/

extern const q15_t sigmoidTable_q15[256];
extern const q7_t sigmoidTable_q7[256];

extern const q7_t tanhTable_q7[256];
extern const q15_t tanhTable_q15[256];

  /**
   * @brief 2-way tables for various activation functions
   *
   * 2-way table, H table for value larger than 1/4
   * L table for value smaller than 1/4, H table for remaining
   * We have this only for the q15_t version. It does not make
   * sense to have it for q7_t type
   */
extern const q15_t sigmoidHTable_q15[192];
extern const q15_t sigmoidLTable_q15[128];

extern const q15_t sigmoidLTable_q15[128];
extern const q15_t sigmoidHTable_q15[192];

#endif                          /*  ARM_NN_TABLES_H */
