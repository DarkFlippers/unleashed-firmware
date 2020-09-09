#ifndef _UTIL_H_
#define _UTIL_H_

/*--------------------------------------------------------------------------------*/
/* Macros and Defines */
/*--------------------------------------------------------------------------------*/

/**
 *  Convert a symbol to a string and add a 'NewLine'.
 */
#define STR_NL(x)  STR1_NL(x)
#define STR1_NL(x) (STR2_NL(x)"\n")
#define STR2_NL(x) #x

/**
 *  Convert a symbol to a string.
 */
#define STR(x)  STR1(x)
#define STR1(x) STR2(x)
#define STR2(x) #x

/**
 *  Concatenate two symbols.
 */
#define CONCAT(a, b)  CONCAT1(a, b)
#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a##b


/**
 *  Place curly braces around a varaible number of macro arguments.
 */
#define CURLY(...) {__VA_ARGS__}

/**
 *  Place parenthesis around a variable number of macro arguments.
 */
#define PAREN(...) (__VA_ARGS__)

/* Standard min/max macros. */
#define MIN(x,y) (((x) < (y)) ? (x) : (y) )
#define MAX(x,y) (((x) > (y)) ? (x) : (y) )

/**
 *  Bound value using low and high limits.
 *
 *  Evaluate to a number in the range, endpoint inclusive.
 */
#define BOUND(low, high, value)                 \
    MAX(MIN(high, value), low)

#endif /* _UTIL_H_ */
