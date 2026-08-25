#pragma once
#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void __clear_cache(void*, void*);
void* __aeabi_uldivmod(uint64_t, uint64_t);
double __aeabi_f2d(float);

/*
 * AEABI / libgcc compiler-runtime helpers exported through the firmware API
 * table. They have no public header of their own, so they are declared here to
 * be collected into api_symbols.csv. Apps that opt out of statically linking
 * libgcc (fap_exclude_libs=["gcc"]) resolve these from the firmware at load
 * time instead of each .fal carrying its own copy of the soft-float routines.
 */
double __adddf3(double, double);
void __aeabi_cdcmpeq(double, double);
void __aeabi_cdcmple(double, double);
void __aeabi_cdrcmple(double, double);
float __aeabi_d2f(double);
int __aeabi_d2iz(double);
long long __aeabi_d2lz(double);
unsigned int __aeabi_d2uiz(double);
unsigned long long __aeabi_d2ulz(double);
double __aeabi_dadd(double, double);
int __aeabi_dcmpeq(double, double);
int __aeabi_dcmpge(double, double);
int __aeabi_dcmpgt(double, double);
int __aeabi_dcmple(double, double);
int __aeabi_dcmplt(double, double);
int __aeabi_dcmpun(double, double);
double __aeabi_ddiv(double, double);
double __aeabi_dmul(double, double);
double __aeabi_drsub(double, double);
double __aeabi_dsub(double, double);
double __aeabi_i2d(int);
int __aeabi_idiv0(int);
double __aeabi_l2d(long long);
long long __aeabi_ldiv0(long long);
double __aeabi_ui2d(unsigned int);
double __aeabi_ul2d(unsigned long long);
int __cmpdf2(double, double);
double __divdf3(double, double);
int __eqdf2(double, double);
double __extendsfdf2(float);
long long __fixdfdi(double);
int __fixdfsi(double);
unsigned long long __fixunsdfdi(double);
unsigned int __fixunsdfsi(double);
double __floatdidf(long long);
double __floatsidf(int);
double __floatundidf(unsigned long long);
double __floatunsidf(unsigned int);
int __gedf2(double, double);
int __gtdf2(double, double);
int __ledf2(double, double);
int __ltdf2(double, double);
double __muldf3(double, double);
int __nedf2(double, double);
int __paritysi2(int);
int __popcountsi2(int);
double __subdf3(double, double);
float __truncdfsf2(double);
unsigned long long __udivmoddi4(unsigned long long, unsigned long long, unsigned long long*);
int __unorddf2(double, double);

#ifdef __cplusplus
}
#endif
