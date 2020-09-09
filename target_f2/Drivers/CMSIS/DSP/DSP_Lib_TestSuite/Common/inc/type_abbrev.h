#ifndef _TYPE_ABBREV_H_
#define _TYPE_ABBREV_H_

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Expand the abbreviation for a type into the type itself.
 */
#define TYPE_FROM_ABBREV(abbrev)                \
    TYPE_ABBREV_##abbrev                        \

/**
 *  Expand the type to an abbreviation for that type.
 *
 *  Inverse of #TYPE_FROM_ABBREV().
 *
 *  @note Should be able to get a type back by writing.
 *  TYPE_FROM_ABBREV(ABBREV_FROM_TYPE(type))
 */
#define ABBREV_FROM_TYPE(type)                  \
    TYPE_SUFFIX_##type

#define TYPE_ABBREV_f64 float64_t
#define TYPE_ABBREV_f32 float32_t
#define TYPE_ABBREV_q31 q31_t
#define TYPE_ABBREV_q15 q15_t
#define TYPE_ABBREV_q7  q7_t

#define TYPE_SUFFIX_float64_t f64
#define TYPE_SUFFIX_float32_t f32
#define TYPE_SUFFIX_q31_t q31
#define TYPE_SUFFIX_q15_t q15
#define TYPE_SUFFIX_q7_t q7

#endif /* _TYPE_ABBREV_H_ */
