/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_iir_lattice_init_f32.c
 * Description:  Floating-point IIR lattice filter initialization function
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
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

#include "arm_math.h"

/**
 * @ingroup groupFilters
 */

/**
 * @addtogroup IIR_Lattice
 * @{
 */

/**
 * @brief Initialization function for the floating-point IIR lattice filter.
 * @param[in] *S points to an instance of the floating-point IIR lattice structure.
 * @param[in] numStages number of stages in the filter.
 * @param[in] *pkCoeffs points to the reflection coefficient buffer.  The array is of length numStages.
 * @param[in] *pvCoeffs points to the ladder coefficient buffer.  The array is of length numStages+1.
 * @param[in] *pState points to the state buffer.  The array is of length numStages+blockSize.
 * @param[in] blockSize number of samples to process.
 * @return none.
 */

void arm_iir_lattice_init_f32(
  arm_iir_lattice_instance_f32 * S,
  uint16_t numStages,
  float32_t * pkCoeffs,
  float32_t * pvCoeffs,
  float32_t * pState,
  uint32_t blockSize)
{
  /* Assign filter taps */
  S->numStages = numStages;

  /* Assign reflection coefficient pointer */
  S->pkCoeffs = pkCoeffs;

  /* Assign ladder coefficient pointer */
  S->pvCoeffs = pvCoeffs;

  /* Clear state buffer and size is always blockSize + numStages */
  memset(pState, 0, (numStages + blockSize) * sizeof(float32_t));

  /* Assign state pointer */
  S->pState = pState;


}

  /**
   * @} end of IIR_Lattice group
   */
