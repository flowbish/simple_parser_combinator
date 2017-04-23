#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "linker_set.h"
#include "log.h"

typedef bool (test_case_fn_t)(void);
typedef char *test_case_name_t;

LINKERSET_DECLARE(test_case_fn);
LINKERSET_DECLARE(test_case_name);

#define declare_test(name)                                    \
  bool name(void);                                            \
  LINKERSET_ADD_ITEM(test_case_fn, name);                     \
  const char *test_case_##name##_str = #name;                 \
  LINKERSET_ADD_ITEM(test_case_name, test_case_##name##_str)

#define define_test(name)                       \
  bool name()

#define new_test(name)                          \
  declare_test(name);                           \
  define_test(name)

#define run_test(name, test_fn)                    \
  do { bool success = (test_fn)();                  \
    if (!success) {                                 \
      error("Test \"%s\" failed, aborting.", name); \
      return 1;                                     \
    }                                               \
  } while (0)

#define num_tests() LINKERSET_SIZE(test_case_fn, size_t)
#define get_test_fn(fn, i) LINKERSET_GET(test_case_fn, fn, i)
#define get_test_name(name, i) LINKERSET_GET(test_case_name, name, i)

bool test_should_run(char *test_name, char **valid_names, size_t num_valid_names);
