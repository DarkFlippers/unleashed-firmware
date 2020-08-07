
#if   (defined (ARMCM0))
  #include "system_ARMCM0.c"

#elif (defined (ARMCM0P))
  #include "system_ARMCM0plus.c"

#elif (defined (ARMCM3))
  #include "system_ARMCM3.c"

#elif (defined (ARMCM4) || defined (ARMCM4_FP))
  #include "system_ARMCM4.c"

#elif (defined (ARMCM7) || defined (ARMCM7_SP) || defined (ARMCM7_DP))
  #include "system_ARMCM7.c"

#elif defined (ARMv8MBL)
  #include "system_ARMv8MBL.c"

#elif (defined (ARMv8MML)    || defined (ARMv8MML_DSP)    || \
       defined (ARMv8MML_SP) || defined (ARMv8MML_DSP_SP) || \
       defined (ARMv8MML_DP) || defined (ARMv8MML_DSP_DP)   )
  #include "system_ARMv8MML.c"

#else
  #error "No appropriate system file found!"
#endif
