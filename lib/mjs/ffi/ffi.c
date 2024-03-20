/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#include "ffi.h"

#define IS_W(arg) ((arg).ctype == FFI_CTYPE_WORD)
#define IS_D(arg) ((arg).ctype == FFI_CTYPE_DOUBLE)
#define IS_F(arg) ((arg).ctype == FFI_CTYPE_FLOAT)

#define W(arg) ((ffi_word_t)(arg).v.i)
#define D(arg) ((arg).v.d)
#define F(arg) ((arg).v.f)

void ffi_set_word(struct ffi_arg* arg, ffi_word_t v) {
    arg->ctype = FFI_CTYPE_WORD;
    arg->v.i = v;
}

void ffi_set_bool(struct ffi_arg* arg, bool v) {
    arg->ctype = FFI_CTYPE_BOOL;
    arg->v.i = v;
}

void ffi_set_ptr(struct ffi_arg* arg, void* v) {
    ffi_set_word(arg, (ffi_word_t)v);
}

void ffi_set_double(struct ffi_arg* arg, double v) {
    arg->ctype = FFI_CTYPE_DOUBLE;
    arg->v.d = v;
}

void ffi_set_float(struct ffi_arg* arg, float v) {
    arg->ctype = FFI_CTYPE_FLOAT;
    arg->v.f = v;
}

/*
 * The ARM ABI uses only 4 32-bit registers for paramter passing.
 * Xtensa call0 calling-convention (as used by Espressif) has 6.
 *
 * Focusing only on implementing FFI with registers means we can simplify a lot.
 *
 * ARM has some quasi-alignment rules when mixing double and integers as
 * arguments. Only:
 *   a) double, int32_t, int32_t
 *   b) int32_t, double
 * would fit in 4 registers. (the same goes for uint64_t).
 *
 * In order to simplify further, when a double-width argument is present, we
 * allow only two arguments.
 */

/*
 * We need to support x86_64 in order to support local tests.
 * x86_64 has more and wider registers, but unlike the two main
 * embedded platforms we target it has a separate register file for
 * integer values and for floating point values (both for passing args and
 * return values). E.g. if a double value is passed as a second argument
 * it gets passed in the first available floating point register.
 *
 * I.e, the compiler generates exactly the same code for:
 *
 * void foo(int a, double b) {...}
 *
 * and
 *
 * void foo(double b, int a) {...}
 *
 *
 */

typedef ffi_word_t (*w4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*w5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*w6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);

typedef ffi_word_t (*wdw_t)(double, ffi_word_t);
typedef ffi_word_t (*wwd_t)(ffi_word_t, double);
typedef ffi_word_t (*wdd_t)(double, double);

typedef ffi_word_t (*wwwd_t)(ffi_word_t, ffi_word_t, double);
typedef ffi_word_t (*wwdw_t)(ffi_word_t, double, ffi_word_t);
typedef ffi_word_t (*wwdd_t)(ffi_word_t, double, double);
typedef ffi_word_t (*wdww_t)(double, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*wdwd_t)(double, ffi_word_t, double);
typedef ffi_word_t (*wddw_t)(double, double, ffi_word_t);
typedef ffi_word_t (*wddd_t)(double, double, double);

typedef ffi_word_t (*wfw_t)(float, ffi_word_t);
typedef ffi_word_t (*wwf_t)(ffi_word_t, float);
typedef ffi_word_t (*wff_t)(float, float);

typedef ffi_word_t (*wwwf_t)(ffi_word_t, ffi_word_t, float);
typedef ffi_word_t (*wwfw_t)(ffi_word_t, float, ffi_word_t);
typedef ffi_word_t (*wwff_t)(ffi_word_t, float, float);
typedef ffi_word_t (*wfww_t)(float, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*wfwf_t)(float, ffi_word_t, float);
typedef ffi_word_t (*wffw_t)(float, float, ffi_word_t);
typedef ffi_word_t (*wfff_t)(float, float, float);

typedef bool (*b4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef bool (*b5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef bool (*b6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef bool (*bdw_t)(double, ffi_word_t);
typedef bool (*bwd_t)(ffi_word_t, double);
typedef bool (*bdd_t)(double, double);

typedef bool (*bwwd_t)(ffi_word_t, ffi_word_t, double);
typedef bool (*bwdw_t)(ffi_word_t, double, ffi_word_t);
typedef bool (*bwdd_t)(ffi_word_t, double, double);
typedef bool (*bdww_t)(double, ffi_word_t, ffi_word_t);
typedef bool (*bdwd_t)(double, ffi_word_t, double);
typedef bool (*bddw_t)(double, double, ffi_word_t);
typedef bool (*bddd_t)(double, double, double);

typedef bool (*bfw_t)(float, ffi_word_t);
typedef bool (*bwf_t)(ffi_word_t, float);
typedef bool (*bff_t)(float, float);

typedef bool (*bwwf_t)(ffi_word_t, ffi_word_t, float);
typedef bool (*bwfw_t)(ffi_word_t, float, ffi_word_t);
typedef bool (*bwff_t)(ffi_word_t, float, float);
typedef bool (*bfww_t)(float, ffi_word_t, ffi_word_t);
typedef bool (*bfwf_t)(float, ffi_word_t, float);
typedef bool (*bffw_t)(float, float, ffi_word_t);
typedef bool (*bfff_t)(float, float, float);

typedef double (*d4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef double (*d5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef double (*d6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef double (*ddw_t)(double, ffi_word_t);
typedef double (*dwd_t)(ffi_word_t, double);
typedef double (*ddd_t)(double, double);

typedef double (*dwwd_t)(ffi_word_t, ffi_word_t, double);
typedef double (*dwdw_t)(ffi_word_t, double, ffi_word_t);
typedef double (*dwdd_t)(ffi_word_t, double, double);
typedef double (*ddww_t)(double, ffi_word_t, ffi_word_t);
typedef double (*ddwd_t)(double, ffi_word_t, double);
typedef double (*dddw_t)(double, double, ffi_word_t);
typedef double (*dddd_t)(double, double, double);

typedef float (*f4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef float (*f5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef float (*f6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef float (*ffw_t)(float, ffi_word_t);
typedef float (*fwf_t)(ffi_word_t, float);
typedef float (*fff_t)(float, float);

typedef float (*fwwf_t)(ffi_word_t, ffi_word_t, float);
typedef float (*fwfw_t)(ffi_word_t, float, ffi_word_t);
typedef float (*fwff_t)(ffi_word_t, float, float);
typedef float (*ffww_t)(float, ffi_word_t, ffi_word_t);
typedef float (*ffwf_t)(float, ffi_word_t, float);
typedef float (*fffw_t)(float, float, ffi_word_t);
typedef float (*ffff_t)(float, float, float);

int ffi_call_mjs(ffi_fn_t* func, int nargs, struct ffi_arg* res, struct ffi_arg* args) {
    int i, doubles = 0, floats = 0;

    if(nargs > 6) return -1;
    for(i = 0; i < nargs; i++) {
        doubles += (IS_D(args[i]));
        floats += (IS_F(args[i]));
    }

    /* Doubles and floats are not supported together atm */
    if(doubles > 0 && floats > 0) {
        return -1;
    }

    switch(res->ctype) {
    case FFI_CTYPE_WORD: { /* {{{ */
        ffi_word_t r;
        if(doubles == 0) {
            if(floats == 0) {
                /*
           * No double and no float args: we currently support up to 6
           * word-sized arguments
           */
                if(nargs <= 4) {
                    w4w_t f = (w4w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]));
                } else if(nargs == 5) {
                    w5w_t f = (w5w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]));
                } else if(nargs == 6) {
                    w6w_t f = (w6w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]), W(args[5]));
                } else {
                    abort();
                }
            } else {
                /* There are some floats */
                switch(nargs) {
                case 0:
                case 1:
                case 2:
                    if(IS_F(args[0]) && IS_F(args[1])) {
                        wff_t f = (wff_t)func;
                        r = f(F(args[0]), F(args[1]));
                    } else if(IS_F(args[0])) {
                        wfw_t f = (wfw_t)func;
                        r = f(F(args[0]), W(args[1]));
                    } else {
                        wwf_t f = (wwf_t)func;
                        r = f(W(args[0]), F(args[1]));
                    }
                    break;

                case 3:
                    if(IS_W(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                        wwwf_t f = (wwwf_t)func;
                        r = f(W(args[0]), W(args[1]), F(args[2]));
                    } else if(IS_W(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                        wwfw_t f = (wwfw_t)func;
                        r = f(W(args[0]), F(args[1]), W(args[2]));
                    } else if(IS_W(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                        wwff_t f = (wwff_t)func;
                        r = f(W(args[0]), F(args[1]), F(args[2]));
                    } else if(IS_F(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                        wfww_t f = (wfww_t)func;
                        r = f(F(args[0]), W(args[1]), W(args[2]));
                    } else if(IS_F(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                        wfwf_t f = (wfwf_t)func;
                        r = f(F(args[0]), W(args[1]), F(args[2]));
                    } else if(IS_F(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                        wffw_t f = (wffw_t)func;
                        r = f(F(args[0]), F(args[1]), W(args[2]));
                    } else if(IS_F(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                        wfff_t f = (wfff_t)func;
                        r = f(F(args[0]), F(args[1]), F(args[2]));
                    } else {
                        // The above checks should be exhaustive
                        abort();
                    }
                    break;
                default:
                    return -1;
                }
            }
        } else {
            /* There are some doubles */
            switch(nargs) {
            case 0:
            case 1:
            case 2:
                if(IS_D(args[0]) && IS_D(args[1])) {
                    wdd_t f = (wdd_t)func;
                    r = f(D(args[0]), D(args[1]));
                } else if(IS_D(args[0])) {
                    wdw_t f = (wdw_t)func;
                    r = f(D(args[0]), W(args[1]));
                } else {
                    wwd_t f = (wwd_t)func;
                    r = f(W(args[0]), D(args[1]));
                }
                break;

            case 3:
                if(IS_W(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    wwwd_t f = (wwwd_t)func;
                    r = f(W(args[0]), W(args[1]), D(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    wwdw_t f = (wwdw_t)func;
                    r = f(W(args[0]), D(args[1]), W(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    wwdd_t f = (wwdd_t)func;
                    r = f(W(args[0]), D(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                    wdww_t f = (wdww_t)func;
                    r = f(D(args[0]), W(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    wdwd_t f = (wdwd_t)func;
                    r = f(D(args[0]), W(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    wddw_t f = (wddw_t)func;
                    r = f(D(args[0]), D(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    wddd_t f = (wddd_t)func;
                    r = f(D(args[0]), D(args[1]), D(args[2]));
                } else {
                    // The above checks should be exhaustive
                    abort();
                }
                break;
            default:
                return -1;
            }
        }
        res->v.i = (uint64_t)r;
    } break; /* }}} */
    case FFI_CTYPE_BOOL: { /* {{{ */
        ffi_word_t r;
        if(doubles == 0) {
            if(floats == 0) {
                /*
           * No double and no float args: we currently support up to 6
           * word-sized arguments
           */
                if(nargs <= 4) {
                    b4w_t f = (b4w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]));
                } else if(nargs == 5) {
                    b5w_t f = (b5w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]));
                } else if(nargs == 6) {
                    b6w_t f = (b6w_t)func;
                    r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]), W(args[5]));
                } else {
                    abort();
                }
            } else {
                /* There are some floats */
                switch(nargs) {
                case 0:
                case 1:
                case 2:
                    if(IS_F(args[0]) && IS_F(args[1])) {
                        bff_t f = (bff_t)func;
                        r = f(F(args[0]), F(args[1]));
                    } else if(IS_F(args[0])) {
                        bfw_t f = (bfw_t)func;
                        r = f(F(args[0]), W(args[1]));
                    } else {
                        bwf_t f = (bwf_t)func;
                        r = f(W(args[0]), F(args[1]));
                    }
                    break;

                case 3:
                    if(IS_W(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                        bwwf_t f = (bwwf_t)func;
                        r = f(W(args[0]), W(args[1]), F(args[2]));
                    } else if(IS_W(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                        bwfw_t f = (bwfw_t)func;
                        r = f(W(args[0]), F(args[1]), W(args[2]));
                    } else if(IS_W(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                        bwff_t f = (bwff_t)func;
                        r = f(W(args[0]), F(args[1]), F(args[2]));
                    } else if(IS_F(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                        bfww_t f = (bfww_t)func;
                        r = f(F(args[0]), W(args[1]), W(args[2]));
                    } else if(IS_F(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                        bfwf_t f = (bfwf_t)func;
                        r = f(F(args[0]), W(args[1]), F(args[2]));
                    } else if(IS_F(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                        bffw_t f = (bffw_t)func;
                        r = f(F(args[0]), F(args[1]), W(args[2]));
                    } else if(IS_F(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                        bfff_t f = (bfff_t)func;
                        r = f(F(args[0]), F(args[1]), F(args[2]));
                    } else {
                        // The above checks should be exhaustive
                        abort();
                    }
                    break;
                default:
                    return -1;
                }
            }
        } else {
            /* There are some doubles */
            switch(nargs) {
            case 0:
            case 1:
            case 2:
                if(IS_D(args[0]) && IS_D(args[1])) {
                    bdd_t f = (bdd_t)func;
                    r = f(D(args[0]), D(args[1]));
                } else if(IS_D(args[0])) {
                    bdw_t f = (bdw_t)func;
                    r = f(D(args[0]), W(args[1]));
                } else {
                    bwd_t f = (bwd_t)func;
                    r = f(W(args[0]), D(args[1]));
                }
                break;

            case 3:
                if(IS_W(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    bwwd_t f = (bwwd_t)func;
                    r = f(W(args[0]), W(args[1]), D(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    bwdw_t f = (bwdw_t)func;
                    r = f(W(args[0]), D(args[1]), W(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    bwdd_t f = (bwdd_t)func;
                    r = f(W(args[0]), D(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                    bdww_t f = (bdww_t)func;
                    r = f(D(args[0]), W(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    bdwd_t f = (bdwd_t)func;
                    r = f(D(args[0]), W(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    bddw_t f = (bddw_t)func;
                    r = f(D(args[0]), D(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    bddd_t f = (bddd_t)func;
                    r = f(D(args[0]), D(args[1]), D(args[2]));
                } else {
                    // The above checks should be exhaustive
                    abort();
                }
                break;
            default:
                return -1;
            }
        }
        res->v.i = (uint64_t)r;
    } break; /* }}} */
    case FFI_CTYPE_DOUBLE: { /* {{{ */
        double r;
        if(doubles == 0) {
            /* No double args: we currently support up to 6 word-sized arguments
         */
            if(nargs <= 4) {
                d4w_t f = (d4w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]));
            } else if(nargs == 5) {
                d5w_t f = (d5w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]));
            } else if(nargs == 6) {
                d6w_t f = (d6w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]), W(args[5]));
            } else {
                abort();
            }
        } else {
            switch(nargs) {
            case 0:
            case 1:
            case 2:
                if(IS_D(args[0]) && IS_D(args[1])) {
                    ddd_t f = (ddd_t)func;
                    r = f(D(args[0]), D(args[1]));
                } else if(IS_D(args[0])) {
                    ddw_t f = (ddw_t)func;
                    r = f(D(args[0]), W(args[1]));
                } else {
                    dwd_t f = (dwd_t)func;
                    r = f(W(args[0]), D(args[1]));
                }
                break;

            case 3:
                if(IS_W(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    dwwd_t f = (dwwd_t)func;
                    r = f(W(args[0]), W(args[1]), D(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    dwdw_t f = (dwdw_t)func;
                    r = f(W(args[0]), D(args[1]), W(args[2]));
                } else if(IS_W(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    dwdd_t f = (dwdd_t)func;
                    r = f(W(args[0]), D(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                    ddww_t f = (ddww_t)func;
                    r = f(D(args[0]), W(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_W(args[1]) && IS_D(args[2])) {
                    ddwd_t f = (ddwd_t)func;
                    r = f(D(args[0]), W(args[1]), D(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_W(args[2])) {
                    dddw_t f = (dddw_t)func;
                    r = f(D(args[0]), D(args[1]), W(args[2]));
                } else if(IS_D(args[0]) && IS_D(args[1]) && IS_D(args[2])) {
                    dddd_t f = (dddd_t)func;
                    r = f(D(args[0]), D(args[1]), D(args[2]));
                } else {
                    // The above checks should be exhaustive
                    abort();
                }
                break;
            default:
                return -1;
            }
        }
        res->v.d = r;
    } break; /* }}} */
    case FFI_CTYPE_FLOAT: { /* {{{ */
        double r;
        if(floats == 0) {
            /* No float args: we currently support up to 6 word-sized arguments
         */
            if(nargs <= 4) {
                f4w_t f = (f4w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]));
            } else if(nargs == 5) {
                f5w_t f = (f5w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]));
            } else if(nargs == 6) {
                f6w_t f = (f6w_t)func;
                r = f(W(args[0]), W(args[1]), W(args[2]), W(args[3]), W(args[4]), W(args[5]));
            } else {
                abort();
            }
        } else {
            /* There are some float args */
            switch(nargs) {
            case 0:
            case 1:
            case 2:
                if(IS_F(args[0]) && IS_F(args[1])) {
                    fff_t f = (fff_t)func;
                    r = f(F(args[0]), F(args[1]));
                } else if(IS_F(args[0])) {
                    ffw_t f = (ffw_t)func;
                    r = f(F(args[0]), W(args[1]));
                } else {
                    fwf_t f = (fwf_t)func;
                    r = f(W(args[0]), F(args[1]));
                }
                break;

            case 3:
                if(IS_W(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                    fwwf_t f = (fwwf_t)func;
                    r = f(W(args[0]), W(args[1]), F(args[2]));
                } else if(IS_W(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                    fwfw_t f = (fwfw_t)func;
                    r = f(W(args[0]), F(args[1]), W(args[2]));
                } else if(IS_W(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                    fwff_t f = (fwff_t)func;
                    r = f(W(args[0]), F(args[1]), F(args[2]));
                } else if(IS_F(args[0]) && IS_W(args[1]) && IS_W(args[2])) {
                    ffww_t f = (ffww_t)func;
                    r = f(F(args[0]), W(args[1]), W(args[2]));
                } else if(IS_F(args[0]) && IS_W(args[1]) && IS_F(args[2])) {
                    ffwf_t f = (ffwf_t)func;
                    r = f(F(args[0]), W(args[1]), F(args[2]));
                } else if(IS_F(args[0]) && IS_F(args[1]) && IS_W(args[2])) {
                    fffw_t f = (fffw_t)func;
                    r = f(F(args[0]), F(args[1]), W(args[2]));
                } else if(IS_F(args[0]) && IS_F(args[1]) && IS_F(args[2])) {
                    ffff_t f = (ffff_t)func;
                    r = f(F(args[0]), F(args[1]), F(args[2]));
                } else {
                    // The above checks should be exhaustive
                    abort();
                }
                break;
            default:
                return -1;
            }
        }
        res->v.f = r;
    } break; /* }}} */
    }

    return 0;
}
