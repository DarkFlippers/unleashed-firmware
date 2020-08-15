
#if defined (__CC_ARM)
  #if   (defined (ARM_MATH_CM0))
    #include "ARMCC\startup_armv6-m.s"
  #elif (defined (ARM_MATH_CM0P))
    #include "ARMCC\startup_armv6-m.s"
  #elif (defined (ARM_MATH_CM3))
    #include "ARMCC\startup_armv7-m.s"
  #elif (defined (ARM_MATH_CM4))
    #include "ARMCC\startup_armv7-m.s"
  #elif (defined (ARM_MATH_CM7))
    #include "ARMCC\startup_armv7-m.s"
  #elif (defined (ARM_MATH_ARMV8MBL))
    #include "ARMCC\startup_armv6-m.s"
  #elif (defined (ARM_MATH_ARMV8MML))
    #include "ARMCC\startup_armv7-m.s"
  #else
    #error "No appropriate startup file found!"
  #endif

#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #if   (defined (ARM_MATH_CM0))
    #include "ARMCLANG\startup_armv6-m.S"
  #elif (defined (ARM_MATH_CM0P))
    #include "ARMCLANG\startup_armv6-m.S"
  #elif (defined (ARM_MATH_CM3))
    #include "ARMCLANG\startup_armv7-m.S"
  #elif (defined (ARM_MATH_CM4))
    #include "ARMCLANG\startup_armv7-m.S"
  #elif (defined (ARM_MATH_CM7))
    #include "ARMCLANG\startup_armv7-m.S"
  #elif (defined (ARM_MATH_ARMV8MBL))
    #include "ARMCLANG\startup_armv6-m.S"
  #elif (defined (ARM_MATH_ARMV8MML))
    #include "ARMCLANG\startup_armv7-m.S"
  #else
    #error "No appropriate startup file found!"
  #endif

#elif defined (__GNUC__)
  #if   (defined (ARM_MATH_CM0))
    #include "GCC\startup_armv6-m.S"
  #elif (defined (ARM_MATH_CM0P))
    #include "GCC\startup_armv6-m.S"
  #elif (defined (ARM_MATH_CM3))
    #include "GCC\startup_armv7-m.S"
  #elif (defined (ARM_MATH_CM4))
    #include "GCC\startup_armv7-m.S"
  #elif (defined (ARM_MATH_CM7))
    #include "GCC\startup_armv7-m.S"
  #elif (defined (ARM_MATH_ARMV8MBL))
    #include "GCC\startup_armv6-m.S"
  #elif (defined (ARM_MATH_ARMV8MML))
    #include "GCC\startup_armv7-m.S"
  #else
    #error "No appropriate startup file found!"
  #endif

#else
  #error "Compiler not supported!"
#endif

