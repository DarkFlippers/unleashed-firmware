
#include "jtest_fw.h"

void test_start    (void) {
//  ;
  JTEST_FW.test_start++;
}

void test_end      (void) {
//  ;
  JTEST_FW.test_end++;
}

void group_start   (void) {
//  ;
  JTEST_FW.group_start++;
}

void group_end     (void) {
//  ;
  JTEST_FW.group_end++;
}

void dump_str      (void) {
//  ;
  JTEST_FW.dump_str++;
}

void dump_data     (void) {
//  ;
  JTEST_FW.dump_data++;
}

void exit_fw       (void) {
//  ;
  JTEST_FW.exit_fw++;
}
