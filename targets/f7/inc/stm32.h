#ifndef _STM32_H_
#define _STM32_H_

/* modify bitfield */
#define _BMD(reg, msk, val) (reg) = (((reg) & ~(msk)) | (val))
/* set bitfield */
#define _BST(reg, bits)     (reg) = ((reg) | (bits))
/* clear bitfield */
#define _BCL(reg, bits)     (reg) = ((reg) & ~(bits))
/* wait until bitfield set */
#define _WBS(reg, bits)     while(((reg) & (bits)) == 0)
/* wait until bitfield clear */
#define _WBC(reg, bits)     while(((reg) & (bits)) != 0)
/* wait for bitfield value */
#define _WVL(reg, msk, val) while(((reg) & (msk)) != (val))
/* bit value */
#define _BV(bit)            (0x01 << (bit))

#include "stm32wbxx.h"

#endif // _STM32_H_
