/*
libtap - Write tests in C
Copyright 2012 Jake Gelbman <gelbman@gmail.com>
This file is licensed under the LGPL
*/

#ifndef __TAP_H__
#define __TAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef va_copy
#ifdef __va_copy
#define va_copy __va_copy
#else
#define va_copy(d, s) ((d) = (s))
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

  int vok_at_loc(const char *file, int line, int test, const char *fmt,
                 va_list args);
  int ok_at_loc(const char *file, int line, int test, const char *fmt, ...);
  int is_at_loc(const char *file, int line, const char *got,
                const char *expected, const char *fmt, ...);
  int isnt_at_loc(const char *file, int line, const char *got,
                  const char *expected, const char *fmt, ...);
  int cmp_ok_at_loc(const char *file, int line, int a, const char *op,
                    int b, const char *fmt, ...);
  int fcmp_ok_at_loc(const char *file, int line, float range, float a,
                     const char *op, float b, const char *fmt, ...);
  int cmp_mem_at_loc(const char *file, int line, const void *got,
                     const void *expected, size_t n, const char *fmt, ...);
  int bail_out(int ignore, const char *fmt, ...);
  void tap_plan(int tests, const char *fmt, ...);
  int diag(const char *fmt, ...);
  int exit_status(void);
  void tap_skip(int n, const char *fmt, ...);
  void tap_todo(int ignore, const char *fmt, ...);
  void tap_end_todo(void);

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__
#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0
#define BITAND(x) PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y) 0
#define BITAND_1(y) y
#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t
#define CHECK_2(x, n, ...) n
#define CHECK_3(x, y, z, ...) z
#define CHECK(...) CHECK_2(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)
#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)
#define BOOL(x) COMPL(NOT(x))
#define IF(c) IIF(BOOL(c))
#define EMPTY()
#define DEFER(id) id EMPTY()
#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

#define FCMP_DEFAULT_RANGE 0.1
#define RANGE(x) ~, x
#undef FCMP
#define FCMP_DEFINED() EVAL(DEFER(CHECK_3)(FCMP, 1, 0, ))
#define FCMP_RANGE() IF(FCMP_DEFINED())(EVAL(DEFER(CHECK_2)(FCMP,)), FCMP_DEFAULT_RANGE)

#define NO_PLAN          -1
#define SKIP_ALL         -2
#define ok(...)          ok_at_loc(__FILE__, __LINE__, __VA_ARGS__, NULL)
#define is(...)          is_at_loc(__FILE__, __LINE__, __VA_ARGS__, NULL)
#define isnt(...)        isnt_at_loc(__FILE__, __LINE__, __VA_ARGS__, NULL)
#define cmp_ok(...)      cmp_ok_at_loc(__FILE__, __LINE__, __VA_ARGS__, NULL)
#define fcmp_ok(...)     fcmp_ok_at_loc(__FILE__, __LINE__, FCMP_RANGE(), __VA_ARGS__, NULL)
#define cmp_mem(...)     cmp_mem_at_loc(__FILE__, __LINE__, __VA_ARGS__, NULL)
#define plan(...)        tap_plan(__VA_ARGS__, NULL)
#define done_testing()   return exit_status()
#define BAIL_OUT(...)    bail_out(0, "" __VA_ARGS__, NULL)
#define pass(...)        ok(1, "" __VA_ARGS__)
#define fail(...)        ok(0, "" __VA_ARGS__)

#define skip(test, ...)  do {if (test) {tap_skip(__VA_ARGS__, NULL); break;}
#define end_skip         } while (0)

#define todo(...)        tap_todo(0, "" __VA_ARGS__, NULL)
#define end_todo         tap_end_todo()

#define dies_ok(...)     dies_ok_common(1, __VA_ARGS__)
#define lives_ok(...)    dies_ok_common(0, __VA_ARGS__)

#ifdef _WIN32
#define like(...)        tap_skip(1, "like is not implemented on Windows")
#define unlike           tap_skip(1, "unlike is not implemented on Windows")
#define dies_ok_common(...) \
                         tap_skip(1, "Death detection is not supported on Windows")
#else
#define like(...)        like_at_loc(1, __FILE__, __LINE__, __VA_ARGS__, NULL)
#define unlike(...)      like_at_loc(0, __FILE__, __LINE__, __VA_ARGS__, NULL)
  int like_at_loc(int for_match, const char *file, int line,
                  const char *got, const char *expected,
                  const char *fmt, ...);
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
  int tap_test_died(int status);
#define dies_ok_common(for_death, code, ...)                \
    do {                                                    \
        int cpid;                                           \
        int it_died;                                        \
        tap_test_died(1);                                   \
        cpid = fork();                                      \
        switch (cpid) {                                     \
        case -1:                                            \
            perror("fork error");                           \
            exit(1);                                        \
        case 0:                                             \
            close(1);                                       \
            close(2);                                       \
            code                                            \
            tap_test_died(0);                               \
            exit(0);                                        \
        }                                                   \
        if (waitpid(cpid, NULL, 0) < 0) {                   \
            perror("waitpid error");                        \
            exit(1);                                        \
        }                                                   \
        it_died = tap_test_died(0);                         \
        if (!it_died)                                       \
            {code}                                          \
        ok(for_death ? it_died : !it_died, "" __VA_ARGS__); \
    } while (0)
#endif

#ifdef __cplusplus
}
#endif
#endif
