#include "../include/tap.h"
#include <string.h>


#define T(func) run_test(argc, argv, #func, func)


void
run_test(int argc, char **argv, char *test_name, void (*test)(void)) {
  int i, should_run;

  should_run = 0;

  if (argc == 1) {
    /* Always run if no arguments present */
    should_run = 1;

  } else {
    /* Otherwise, check if the test name matches any arguments given */

    for (i = 1; i < argc; ++i) {
      if (strncmp(test_name, argv[i], strlen(argv[i])) == 0) {
        should_run = 1;
      }
    }
  }

  if (should_run) {
    test();
  }
}
