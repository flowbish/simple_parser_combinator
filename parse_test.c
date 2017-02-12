#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parse.h"

#define error(string, ...) fprintf (stderr, "[ERROR] "string "\n", ##__VA_ARGS__)
#define info(string, ...) fprintf (stderr, "[INFO] "string "\n", ##__VA_ARGS__)

#define run_test(test_fn)                               \
  do { bool success = test_fn();                        \
    if (!success) {                                     \
      error("Test \"" #test_fn "\" failed, aborting."); \
      return 1;                                         \
    } else {                                            \
      info("Test \"" #test_fn "\" succeeded.");         \
    }                                                   \
  } while (0)

bool check_parse(const char *input, parser p, const char *expected) {
  char *output = NULL;
  bool success = run(p, input, &output);
  if (!success && expected != NULL) {
    error("Parser failed to match.");
    return false;
  } else if (success && expected == NULL) {
    error("Parser matched erroneously.");
    return false;
  } else if (expected != NULL && strcmp(expected, output) != 0) {
    error("Parser failed to parse correctly:");
    error("Found: %s", output);
    error("Expected: %s", expected);
    return false;
  }
  return true;
}

bool test_blank() {
  return check_parse("", parser_create_blank(), "");
}

bool test_null() {
  return check_parse("", parser_create_null(), NULL);
}

bool test_char() {
  return check_parse("test", parser_create_char('t'), "t");
}

bool test_str_full() {
  return check_parse("test", parser_create_str("test"), "test");
}

bool test_str_partial() {
  return check_parse("testing", parser_create_str("test"), "test");
}

bool test_str_fail() {
  return check_parse("test", parser_create_str("something"), NULL);
}

bool test_or_first() {
  return check_parse("test", parser_create_or(parser_create_blank(), parser_create_null()), "");
}

bool test_or_second() {
  return check_parse("test", parser_create_or(parser_create_null(), parser_create_blank()), "");
}

bool test_or_strings() {
  return check_parse("that or this", parser_create_or(parser_create_str("this"), parser_create_str("that")), "that");
}

int main() {
  run_test(test_blank);
  run_test(test_null);
  run_test(test_char);
  run_test(test_str_full);
  run_test(test_str_partial);
  run_test(test_str_fail);
  run_test(test_or_first);
  run_test(test_or_second);
  run_test(test_or_strings);
  return 0;
}
