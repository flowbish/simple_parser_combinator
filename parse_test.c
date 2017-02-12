#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "linker_set.h"
#include "parse.h"

/* Printing macros */
#define error(string, ...) fprintf (stderr, "[ERROR] "string "\n", ##__VA_ARGS__)
#define info(string, ...) fprintf (stderr, "[INFO] "string "\n", ##__VA_ARGS__)

SET_DECLARE(test_case_fn, void);
SET_DECLARE(test_case_name, char*);

#define declare_test(name)                            \
  bool name(void);                                    \
  SET_ENTRY(test_case_fn, name);                      \
  const char *test_case_##name##_str = #name;         \
  SET_ENTRY(test_case_name, test_case_##name##_str);  \

#define define_test(name)                       \
  bool name()

#define new_test(name)                          \
  declare_test(name);                           \
  define_test(name)

#define run_test(name, test_fn)                     \
  do { bool success = (test_fn)();                  \
    if (!success) {                                 \
      error("Test \"%s\" failed, aborting.", name); \
      return 1;                                     \
    } else {                                        \
      info("Test \"%s\" succeeded.", name);         \
    }                                               \
  } while (0)

bool check_parse(const char *input, parser p, const char *expected) {
  char *output = NULL;
  bool success = run(p, input, &output);
  if (!success && expected != NULL) {
    error("Parser failed to match.");
    return false;
  } else if (success && expected == NULL) {
    error("Parser matched erroneously.");
    error("Found: %s", output);
    return false;
  } else if (expected != NULL && strcmp(expected, output) != 0) {
    error("Parser failed to parse correctly:");
    error("Found: %s", output);
    error("Expected: %s", expected);
    return false;
  }
  return true;
}

new_test(test_blank) {
  return check_parse("", parser_create_blank(), "");
}

new_test(test_null) {
  return check_parse("", parser_create_null(), NULL);
}

new_test(test_char) {
  return check_parse("test", parser_create_char('t'), "t");
}

new_test(test_str_full) {
  return check_parse("test", parser_create_str("test"), "test");
}

new_test(test_str_partial) {
  return check_parse("testing", parser_create_str("test"), "test");
}

new_test(test_str_fail) {
  return check_parse("test", parser_create_str("something"), NULL);
}

new_test(test_or_first) {
  return check_parse("test", parser_create_or(parser_create_blank(), parser_create_null()), "");
}

new_test(test_or_second) {
  return check_parse("test", parser_create_or(parser_create_null(), parser_create_blank()), "");
}

new_test(test_or_string) {
  return check_parse("that or this", parser_create_or(parser_create_str("this"), parser_create_str("that")), "that");
}

new_test(test_and_first_fail) {
  return check_parse("test", parser_create_and(parser_create_null(), parser_create_blank()), NULL);
}

new_test(test_and_second_fail) {
  return check_parse("test", parser_create_and(parser_create_blank(), parser_create_null()), NULL);
}

new_test(test_and_second_char) {
  return check_parse("test", parser_create_and(parser_create_char('t'), parser_create_char('e')), "te");
}

int main() {
  for (int i = 0; i < SET_COUNT(test_case_fn); i++) {
    void *test = SET_ITEM(test_case_fn, i);
    char **name = SET_ITEM(test_case_name, i);
    run_test(*name, (bool (*)(void))test);
  }
  return 0;
}
