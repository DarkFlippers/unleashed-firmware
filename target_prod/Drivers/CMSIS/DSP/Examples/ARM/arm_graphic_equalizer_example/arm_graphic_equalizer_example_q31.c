/* ----------------------------------------------------------------------
* Copyright (C) 2010-2012 ARM Limited. All rights reserved.
*
* $Date:         17. January 2013
* $Revision:     V1.4.0
*
* Project:       CMSIS DSP Library
* Title:         arm_graphic_equalizer_example_q31.c
*
* Description:   Example showing an audio graphic equalizer constructed
*                out of Biquad filters.
*
* Target Processor: Cortex-M4/Cortex-M3
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

/**
 * @ingroup groupExamples
 */

/**
 * @defgroup GEQ5Band Graphic Audio Equalizer Example
 *
 * \par Description:
 * \par
 * This example demonstrates how a 5-band graphic equalizer can be constructed
 * using the Biquad cascade functions.
 * A graphic equalizer is used in audio applications to vary the tonal quality
 * of the audio.
 *
 * \par Block Diagram:
 * \par
 * The design is based on a cascade of 5 filter sections.
 * \image html GEQ_signalflow.gif
 * Each filter section is 4th order and consists of a cascade of two Biquads.
 * Each filter has a nominal gain of 0 dB (1.0 in linear units) and
 * boosts or cuts signals within a specific frequency range.
 * The edge frequencies between the 5 bands are 100, 500, 2000, and 6000 Hz.
 * Each band has an adjustable boost or cut in the range of +/- 9 dB.
 * For example, the band that extends from 500 to 2000 Hz has the response shown below:
 * \par
 * \image html GEQ_bandresponse.gif
 * \par
 * With 1 dB steps, each filter has a total of 19 different settings.
 * The filter coefficients for all possible 19 settings were precomputed
 * in MATLAB and stored in a table.  With 5 different tables, there are
 * a total of 5 x 19 = 95 different 4th order filters.
 * All 95 responses are shown below:
 * \par
 * \image html GEQ_allbandresponse.gif
 * \par
 * Each 4th order filter has 10 coefficents for a grand total of 950 different filter
 * coefficients that must be tabulated. The input and output data is in Q31 format.
 * For better noise performance, the two low frequency bands are implemented using the high
 * precision 32x64-bit Biquad filters. The remaining 3 high frequency bands use standard
 * 32x32-bit Biquad filters. The input signal used in the example is a logarithmic chirp.
 * \par
 * \image html GEQ_inputchirp.gif
 * \par
 * The array <code>bandGains</code> specifies the gain in dB to apply in each band.
 * For example, if <code>bandGains={0, -3, 6, 4, -6};</code> then the output signal will be:
 * \par
 * \image html GEQ_outputchirp.gif
 * \par
 * \note The output chirp signal follows the gain or boost of each band.
 * \par
 *
 * \par Variables Description:
 * \par
 * \li \c testInput_f32 points to the input data
 * \li \c testRefOutput_f32 points to the reference output data
 * \li \c testOutput points to the test output data
 * \li \c inputQ31 temporary input buffer
 * \li \c outputQ31 temporary output buffer
 * \li \c biquadStateBand1Q31 points to state buffer for band1
 * \li \c biquadStateBand2Q31 points to state buffer for band2
 * \li \c biquadStateBand3Q31 points to state buffer for band3
 * \li \c biquadStateBand4Q31 points to state buffer for band4
 * \li \c biquadStateBand5Q31 points to state buffer for band5
 * \li \c coeffTable points to coefficient buffer for all bands
 * \li \c gainDB gain buffer which has gains applied for all the bands
 *
 * \par CMSIS DSP Software Library Functions Used:
 * \par
 * - arm_biquad_cas_df1_32x64_init_q31()
 * - arm_biquad_cas_df1_32x64_q31()
 * - arm_biquad_cascade_df1_init_q31()
 * - arm_biquad_cascade_df1_q31()
 * - arm_scale_q31()
 * - arm_scale_f32()
 * - arm_float_to_q31()
 * - arm_q31_to_float()
 *
 * <b> Refer  </b>
 * \link arm_graphic_equalizer_example_q31.c \endlink
 *
 */


/** \example arm_graphic_equalizer_example_q31.c
 */


#include "arm_math.h"
#include "math_helper.h"

/* Length of the overall data in the test */
#define TESTLENGTH 320

/* Block size for the underlying processing */
#define BLOCKSIZE 32

/* Total number of blocks to run */
#define NUMBLOCKS (TESTLENGTH/BLOCKSIZE)

/* Number of 2nd order Biquad stages per filter */
#define NUMSTAGES 2

#define SNR_THRESHOLD_F32  98

/* -------------------------------------------------------------------
 * External Declarations for Input and Output buffers
 * ------------------------------------------------------------------- */

extern float32_t testInput_f32[TESTLENGTH];
static float32_t testOutput[TESTLENGTH];

extern float32_t testRefOutput_f32[TESTLENGTH];

/* ----------------------------------------------------------------------
** Q31 state buffers for Band1, Band2, Band3, Band4, Band5
** ------------------------------------------------------------------- */

static q63_t biquadStateBand1Q31[4 * 2];
static q63_t biquadStateBand2Q31[4 * 2];
static q31_t biquadStateBand3Q31[4 * 2];
static q31_t biquadStateBand4Q31[4 * 2];
static q31_t biquadStateBand5Q31[4 * 2];

/* ----------------------------------------------------------------------
** Q31 input and output buffers
** ------------------------------------------------------------------- */

q31_t inputQ31[BLOCKSIZE];
q31_t outputQ31[BLOCKSIZE];

/* ----------------------------------------------------------------------
** Entire coefficient table.  There are 10 coefficients per 4th order Biquad
** cascade filter.  The first 10 coefficients correspond to the -9 dB gain
** setting of band 1; the next 10 coefficient correspond to the -8 dB gain
** setting of band 1; and so on.  There are 10*19=190 coefficients in total
** for band 1 (gains = -9, -8, -7, ..., 9).  After this come the 190 coefficients
** for band 2.
**
** The coefficients are in Q29 format and require a postShift of 2.
** ------------------------------------------------------------------- */

const q31_t coeffTable[950] = {

  /* Band 1, -9 dB gain */
  535576962, -1071153923, 535576962, 1073741824, -536870912, 535576962, -1063501998, 527979313, 1060865294, -524146981,
  /* Band 1, -8 dB gain */
  535723226, -1071446451, 535723226, 1073741824, -536870912, 535723226, -1063568947, 527903217, 1061230578, -524503778,
  535868593, -1071737186, 535868593, 1073741824, -536870912, 535868593, -1063627467, 527819780, 1061585502, -524850686,
  536013181, -1072026363, 536013181, 1073741824, -536870912, 536013181, -1063677598, 527728935, 1061930361, -525187972,
  536157109, -1072314217, 536157109, 1073741824, -536870912, 536157109, -1063719372, 527630607, 1062265438, -525515897,
  536300492, -1072600983, 536300492, 1073741824, -536870912, 536300492, -1063752815, 527524720, 1062591011, -525834716,
  536443447, -1072886894, 536443447, 1073741824, -536870912, 536443447, -1063777945, 527411186, 1062907350, -526144676,
  536586091, -1073172183, 536586091, 1073741824, -536870912, 536586091, -1063794775, 527289917, 1063214717, -526446017,
  536728541, -1073457082, 536728541, 1073741824, -536870912, 536728541, -1063803308, 527160815, 1063513366, -526738975,
  536870912, -1073741824, 536870912, 1073741824, -536870912, 536870912, -1063803543, 527023777, 1063803543, -527023777,
  537013321, -1074026642, 537013321, 1073741824, -536870912, 537013321, -1063795470, 526878696, 1064085490, -527300648,
  537155884, -1074311768, 537155884, 1073741824, -536870912, 537155884, -1063779073, 526725455, 1064359439, -527569803,
  537298718, -1074597435, 537298718, 1073741824, -536870912, 537298718, -1063754328, 526563934, 1064625617, -527831454,
  537441939, -1074883878, 537441939, 1073741824, -536870912, 537441939, -1063721205, 526394005, 1064884245, -528085806,
  537585666, -1075171331, 537585666, 1073741824, -536870912, 537585666, -1063679666, 526215534, 1065135536, -528333059,
  537730015, -1075460030, 537730015, 1073741824, -536870912, 537730015, -1063629666, 526028380, 1065379699, -528573409,
  537875106, -1075750212, 537875106, 1073741824, -536870912, 537875106, -1063571152, 525832396, 1065616936, -528807045,
  538021057, -1076042114, 538021057, 1073741824, -536870912, 538021057, -1063504065, 525627429, 1065847444, -529034151,
  538167989, -1076335977, 538167989, 1073741824, -536870912, 538167989, -1063428338, 525413317, 1066071412, -529254907,

  /* Band 2, -9 dB gain */
  531784976, -1055497692, 523873415, 1066213307, -529420241, 531784976, -1040357886, 509828014, 1028908252, -494627367,
  /* Band 2, -8 dB gain */
  532357636, -1056601982, 524400080, 1066115844, -529326645, 532357636, -1040623406, 509562600, 1030462237, -496062122,
  532927392, -1057707729, 524931110, 1066024274, -529239070, 532927392, -1040848253, 509262081, 1031969246, -497457090,
  533494678, -1058816094, 525467240, 1065939047, -529157961, 533494678, -1041032161, 508925950, 1033429976, -498812573,
  534059929, -1059928204, 526009170, 1065860582, -529083734, 534059929, -1041174868, 508553717, 1034845124, -500128887,
  534623580, -1061045148, 526557561, 1065789260, -529016764, 534623580, -1041276126, 508144920, 1036215393, -501406373,
  535186068, -1062167969, 527113032, 1065725420, -528957385, 535186068, -1041335703, 507699125, 1037541500, -502645399,
  535747827, -1063297666, 527676151, 1065669351, -528905879, 535747827, -1041353386, 507215934, 1038824183, -503846368,
  536309295, -1064435183, 528247436, 1065621289, -528862476, 536309295, -1041328990, 506694984, 1040064203, -505009724,
  536870912, -1065581413, 528827349, 1065581413, -528827349, 536870912, -1041262354, 506135953, 1041262354, -506135953,
  537433117, -1066737194, 529416295, 1065549847, -528800610, 537433117, -1041153346, 505538564, 1042419457, -507225588,
  537996352, -1067903307, 530014622, 1065526651, -528782316, 537996352, -1041001864, 504902578, 1043536370, -508279208,
  538561061, -1069080480, 530622620, 1065511830, -528772462, 538561061, -1040807833, 504227800, 1044613981, -509297437,
  539127690, -1070269387, 531240527, 1065505333, -528770987, 539127690, -1040571205, 503514074, 1045653211, -510280946,
  539696690, -1071470656, 531868525, 1065507054, -528777778, 539696690, -1040291951, 502761277, 1046655011, -511230450,
  540268512, -1072684867, 532506750, 1065516837, -528792672, 540268512, -1039970063, 501969320, 1047620358, -512146700,
  540843613, -1073912567, 533155297, 1065534483, -528815459, 540843613, -1039605542, 501138139, 1048550251, -513030484,
  541422451, -1075154268, 533814224, 1065559750, -528845892, 541422451, -1039198394, 500267687, 1049445708, -513882621,
  542005489, -1076410460, 534483561, 1065592362, -528883686, 542005489, -1038748624, 499357932, 1050307760, -514703956,
  518903861, -1001986830, 486725277, 1037235801, -502367695, 518903861, -945834422, 446371043, 902366163, -400700571,
  520899989, -1005630916, 488289126, 1036926846, -502147311, 520899989, -946490935, 445581846, 907921945, -404936158,
  522893209, -1009290002, 489869792, 1036650484, -501961419, 522893209, -947006359, 444685310, 913306106, -409075225,
  524884763, -1012968199, 491470256, 1036407567, -501810737, 524884763, -947377809, 443679533, 918521018, -413116221,
  526875910, -1016669649, 493093518, 1036198712, -501695739, 526875910, -947602324, 442562672, 923569247, -417057897,
  528867927, -1020398503, 494742575, 1036024293, -501616651, 528867927, -947676875, 441332970, 928453558, -420899319,
  530862111, -1024158905, 496420407, 1035884447, -501573457, 530862111, -947598385, 439988777, 933176909, -424639872,
  532859778, -1027954970, 498129955, 1035779077, -501565907, 532859778, -947363742, 438528571, 937742446, -428279254,
  534862260, -1031790763, 499874098, 1035707863, -501593525, 534862260, -946969823, 436950987, 942153486, -431817474,
  536870912, -1035670279, 501655630, 1035670279, -501655630, 536870912, -946413508, 435254839, 946413508, -435254839,
  538887107, -1039597419, 503477238, 1035665609, -501751354, 538887107, -945691703, 433439146, 950526127, -438591937,
  540912240, -1043575967, 505341475, 1035692963, -501879659, 540912240, -944801359, 431503152, 954495080, -441829621,
  542947726, -1047609569, 507250741, 1035751307, -502039364, 542947726, -943739490, 429446349, 958324201, -444968987,
  544995000, -1051701717, 509207261, 1035839473, -502229165, 544995000, -942503190, 427268492, 962017400, -448011351,
  547055523, -1055855728, 511213065, 1035956193, -502447657, 547055523, -941089647, 424969617, 965578640, -450958226,
  549130774, -1060074734, 513269973, 1036100110, -502693359, 549130774, -939496155, 422550049, 969011913, -453811298,
  551222259, -1064361672, 515379585, 1036269804, -502964731, 551222259, -937720119, 420010407, 972321228, -456572401,
  553331507, -1068719280, 517543273, 1036463810, -503260192, 553331507, -935759057, 417351601, 975510582, -459243495,
  555460072, -1073150100, 519762181, 1036680633, -503578144, 555460072, -933610600, 414574832, 978583948, -461826644,
  494084017, -851422604, 404056273, 930151631, -423619864, 494084017, -673714108, 339502486, 561843007, -265801750,
  498713542, -859177141, 406587077, 929211656, -423786402, 498713542, -673274906, 338185129, 573719128, -272222942,
  503369016, -867012190, 409148384, 928362985, -424054784, 503369016, -672533059, 336693984, 585290277, -278599028,
  508052536, -874935599, 411746438, 927604291, -424422151, 508052536, -671478538, 335026905, 596558312, -284920289,
  512766286, -882955583, 414387826, 926933782, -424885216, 512766286, -670100998, 333182045, 607525792, -291177811,
  517512534, -891080712, 417079474, 926349262, -425440318, 517512534, -668389789, 331157902, 618195914, -297363485,
  522293635, -899319903, 419828635, 925848177, -426083491, 522293635, -666333963, 328953368, 628572440, -303470012,
  527112032, -907682405, 422642886, 925427679, -426810526, 527112032, -663922286, 326567785, 638659631, -309490882,
  531970251, -916177781, 425530105, 925084675, -427617023, 531970251, -661143261, 324000998, 648462180, -315420352,
  536870912, -924815881, 428498454, 924815881, -428498454, 536870912, -657985147, 321253420, 657985147, -321253420,
  541816719, -933606817, 431556352, 924617870, -429450209, 541816719, -654435997, 318326093, 667233900, -326985786,
  546810467, -942560921, 434712438, 924487114, -430467639, 546810467, -650483688, 315220754, 676214053, -332613816,
  551855042, -951688708, 437975532, 924420027, -431546101, 551855042, -646115970, 311939896, 684931422, -338134495,
  556953421, -961000826, 441354588, 924413001, -432680993, 556953421, -641320513, 308486839, 693391970, -343545389,
  562108672, -970508005, 444858642, 924462435, -433867780, 562108672, -636084967, 304865786, 701601770, -348844597,
  567323959, -980220994, 448496743, 924564764, -435102022, 567323959, -630397020, 301081886, 709566963, -354030710,
  572602539, -990150500, 452277894, 924716482, -436379394, 572602539, -624244471, 297141281, 717293726, -359102767,
  577947763, -1000307125, 456210977, 924914158, -437695705, 577947763, -617615296, 293051155, 724788245, -364060214,
  583363084, -1010701292, 460304674, 925154455, -439046908, 583363084, -610497723, 288819761, 732056685, -368902865,
  387379495, -506912469, 196933274, 840112184, -347208270, 387379495, 506912469, 196933274, -840112184, -347208270,
  401658082, -532275898, 207149427, 833765363, -343175316, 401658082, 532275898, 207149427, -833765363, -343175316,
  416472483, -558722695, 217902617, 827270154, -339107319, 416472483, 558722695, 217902617, -827270154, -339107319,
  431841949, -586290861, 229212798, 820624988, -335007540, 431841949, 586290861, 229212798, -820624988, -335007540,
  447786335, -615019650, 241100489, 813828443, -330879528, 447786335, 615019650, 241100489, -813828443, -330879528,
  464326111, -644949597, 253586805, 806879270, -326727141, 464326111, 644949597, 253586805, -806879270, -326727141,
  481482377, -676122557, 266693475, 799776409, -322554559, 481482377, 676122557, 266693475, -799776409, -322554559,
  499276882, -708581728, 280442865, 792519013, -318366296, 499276882, 708581728, 280442865, -792519013, -318366296,
  517732032, -742371685, 294857996, 785106465, -314167221, 517732032, 742371685, 294857996, -785106465, -314167221,
  536870912, -777538408, 309962566, 777538408, -309962566, 536870912, 777538408, 309962566, -777538408, -309962566,
  556717294, -814129313, 325780968, 769814766, -305757943, 556717294, 814129313, 325780968, -769814766, -305757943,
  577295658, -852193284, 342338310, 761935777, -301559360, 577295658, 852193284, 342338310, -761935777, -301559360,
  598631206, -891780698, 359660433, 753902014, -297373230, 598631206, 891780698, 359660433, -753902014, -297373230,
  620749877, -932943463, 377773927, 745714425, -293206383, 620749877, 932943463, 377773927, -745714425, -293206383,
  643678365, -975735041, 396706151, 737374355, -289066077, 643678365, 975735041, 396706151, -737374355, -289066077,
  667444134, -1020210487, 416485252, 728883588, -284960004, 667444134, 1020210487, 416485252, -728883588, -284960004,
  692075438, -1066426476, 437140179, 720244375, -280896294, 692075438, 1066426476, 437140179, -720244375, -280896294,
  717601336, -1114441339, 458700704, 711459472, -276883515, 717601336, 1114441339, 458700704, -711459472, -276883515,
  744051710, -1164315096, 481197437, 702532174, -272930673, 744051710, 1164315096, 481197437, -702532174, -272930673

};

/* ----------------------------------------------------------------------
** Desired gains, in dB, per band
** ------------------------------------------------------------------- */

int gainDB[5] = {0, -3, 6, 4, -6};

float32_t snr;


/* ----------------------------------------------------------------------
 * Graphic equalizer Example
 * ------------------------------------------------------------------- */

int32_t main(void)
{
  float32_t  *inputF32, *outputF32;
  arm_biquad_cas_df1_32x64_ins_q31 S1;
  arm_biquad_cas_df1_32x64_ins_q31 S2;
  arm_biquad_casd_df1_inst_q31 S3;
  arm_biquad_casd_df1_inst_q31 S4;
  arm_biquad_casd_df1_inst_q31 S5;
  int i;
  int32_t status;

  inputF32 = &testInput_f32[0];
  outputF32 = &testOutput[0];

  /* Initialize the state and coefficient buffers for all Biquad sections */

  arm_biquad_cas_df1_32x64_init_q31(&S1, NUMSTAGES,
            (q31_t *) &coeffTable[190*0 + 10*(gainDB[0] + 9)],
            &biquadStateBand1Q31[0], 2);

  arm_biquad_cas_df1_32x64_init_q31(&S2, NUMSTAGES,
            (q31_t *) &coeffTable[190*1 + 10*(gainDB[1] + 9)],
            &biquadStateBand2Q31[0], 2);

  arm_biquad_cascade_df1_init_q31(&S3, NUMSTAGES,
          (q31_t *) &coeffTable[190*2 + 10*(gainDB[2] + 9)],
          &biquadStateBand3Q31[0], 2);

  arm_biquad_cascade_df1_init_q31(&S4, NUMSTAGES,
          (q31_t *) &coeffTable[190*3 + 10*(gainDB[3] + 9)],
          &biquadStateBand4Q31[0], 2);

  arm_biquad_cascade_df1_init_q31(&S5, NUMSTAGES,
          (q31_t *) &coeffTable[190*4 + 10*(gainDB[4] + 9)],
          &biquadStateBand5Q31[0], 2);


  /* Call the process functions and needs to change filter coefficients
     for varying the gain of each band */

  for(i=0; i < NUMBLOCKS; i++)
  {

    /* ----------------------------------------------------------------------
    ** Convert block of input data from float to Q31
    ** ------------------------------------------------------------------- */

    arm_float_to_q31(inputF32 + (i*BLOCKSIZE), inputQ31, BLOCKSIZE);

    /* ----------------------------------------------------------------------
    ** Scale down by 1/8.  This provides additional headroom so that the
    ** graphic EQ can apply gain.
    ** ------------------------------------------------------------------- */

    arm_scale_q31(inputQ31, 0x7FFFFFFF, -3, inputQ31, BLOCKSIZE);

    /* ----------------------------------------------------------------------
    ** Call the Q31 Biquad Cascade DF1 32x64 process function for band1, band2
    ** ------------------------------------------------------------------- */

    arm_biquad_cas_df1_32x64_q31(&S1, inputQ31, outputQ31, BLOCKSIZE);
    arm_biquad_cas_df1_32x64_q31(&S2, outputQ31, outputQ31, BLOCKSIZE);

    /* ----------------------------------------------------------------------
    ** Call the Q31 Biquad Cascade DF1 process function for band3, band4, band5
    ** ------------------------------------------------------------------- */

    arm_biquad_cascade_df1_q31(&S3, outputQ31, outputQ31, BLOCKSIZE);
    arm_biquad_cascade_df1_q31(&S4, outputQ31, outputQ31, BLOCKSIZE);
    arm_biquad_cascade_df1_q31(&S5, outputQ31, outputQ31, BLOCKSIZE);

    /* ----------------------------------------------------------------------
    ** Convert Q31 result back to float
    ** ------------------------------------------------------------------- */

    arm_q31_to_float(outputQ31, outputF32 + (i * BLOCKSIZE), BLOCKSIZE);

    /* ----------------------------------------------------------------------
    ** Scale back up
    ** ------------------------------------------------------------------- */

    arm_scale_f32(outputF32 + (i * BLOCKSIZE), 8.0f, outputF32 + (i * BLOCKSIZE), BLOCKSIZE);
  };

  snr = arm_snr_f32(testRefOutput_f32, testOutput, TESTLENGTH);

  if (snr < SNR_THRESHOLD_F32)
  {
    status = ARM_MATH_TEST_FAILURE;
  }
  else
  {
    status = ARM_MATH_SUCCESS;
  }

  /* ----------------------------------------------------------------------
  ** Loop here if the signal does not match the reference output.
  ** ------------------------------------------------------------------- */

  if ( status != ARM_MATH_SUCCESS)
  {
    while (1);
  }

  while (1);                             /* main function does not return */
}

/** \endlink */



