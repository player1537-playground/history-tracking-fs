#include "../include/tap.h"
#include "common.c"


void
test_foo__example(void) {
  cmp_ok(3, "==", 3, "3 should be 3");
}


int
main(int argc, char **argv) {
  plan(NO_PLAN);
  T(test_foo__example);
  done_testing();
}
