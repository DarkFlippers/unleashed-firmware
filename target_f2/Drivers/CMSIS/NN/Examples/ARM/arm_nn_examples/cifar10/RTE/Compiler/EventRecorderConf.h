/*------------------------------------------------------------------------------
 * MDK - Component ::Event Recorder
 * Copyright (c) 2016 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    EventRecorderConf.h
 * Purpose: Event Recorder Configuration
 * Rev.:    V1.0.0
 *----------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>Event Recorder

//   <o>Number of Records
//     <8=>8 <16=>16 <32=>32 <64=>64 <128=>128 <256=>256 <512=>512 <1024=>1024
//     <2048=>2048 <4096=>4096 <8192=>8192 <16384=>16384 <32768=>32768
//     <65536=>65536 <131072=>131072 <262144=>262144 <524288=>524288
//     <1048576=>1048576
//   <i>Configure size of Event Record Buffer (each record is 16 bytes)
//   <i>Must be 2^n (min=8, max=1048576)
#define EVENT_RECORD_COUNT      64U

//   <o>Time Stamp Source
//      <0=> DWT Cycle Counter  <1=> SysTick
//      <3=> User Timer (Normal Reset)  <4=> User Timer (Power-On Reset)
//   <i>Selects source for 32-bit time stamp
#define EVENT_TIMESTAMP_SOURCE  1

//   <h>SysTick Configuration
//   <i>Configure values when Time Stamp Source is set to SysTick

//     <o>SysTick Input Clock Frequency [Hz] <1-1000000000>
//     <i>Defines SysTick input clock (typical identical with processor clock)
#define SYSTICK_CLOCK           100000000U

//     <o>SysTick Interrupt Period [us] <1-1000000000>
//     <i>Defines time period of the SysTick timer interrupt
#define SYSTICK_PERIOD_US       1000U

//   </h>

// </h>

//------------- <<< end of configuration section >>> ---------------------------
