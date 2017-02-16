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
  } else if (success && expected != NULL && strcmp(expected, output) != 0) {
    error("Parser failed to parse correctly:");
    error("Found: %s", output);
    error("Expected: %s", expected);
    return false;
  }
  return true;
}

new_test(test_blank) {
  return check_parse("", blank, "");
}

new_test(test_blank_with_input) {
  return check_parse("test", blank, "");
}

new_test(test_null) {
  return check_parse("", null, NULL);
}

new_test(test_eof_pass) {
  return check_parse("", eof, "");
}

new_test(test_eof_fail) {
  return check_parse("a", eof, NULL);
}

new_test(test_char) {
  return check_parse("test", ch('t'), "t");
}

new_test(test_str_full) {
  return check_parse("test", str("test"), "test");
}

new_test(test_str_partial) {
  return check_parse("testing", str("test"), "test");
}

new_test(test_str_fail) {
  return check_parse("test", str("something"), NULL);
}

new_test(test_or_first) {
  return check_parse("test", or(blank, null), "");
}

new_test(test_or_second) {
  return check_parse("test", or(null, blank), "");
}

/*
 * We should see that the first parser in the or(), str("this"), consumes input
 * until it fails, at which point the next parser, str("that"), picks up where
 * it left off. This means that the first few letters in common must be repeated
 * for this to parse successful. This is why wrapping the first term in a try()
 * is reccommended.
 */
new_test(test_or_string) {
  return check_parse("ththat", or(str("this"), str("that")), "ththat");
}

/*
 * As explained above, if the first term is wrapped in try(), then our parse
 * behaves exactly as we expect, with both branches of the or starting from the
 * same character regardless of whether the first consumed any input.
 */
new_test(test_try_or_string) {
  return check_parse("that", or(try(str("this")), str("that")), "that");
}

new_test(test_and_first_fail) {
  return check_parse("test", and(null, blank), NULL);
}

new_test(test_and_second_fail) {
  return check_parse("test", and(blank, null), NULL);
}

new_test(test_and_second_char) {
  return check_parse("test", and(ch('t'), ch('e')), "te");
}

new_test(test_many_no_match) {
  return check_parse("aaabbb", many(null), "");
}

new_test(test_many_single_match) {
  return check_parse("ab", many(ch('a')), "a");
}

new_test(test_many_matches) {
  return check_parse("aaabbb", many(ch('a')), "aaa");
}

bool set_int(char *x, void *total) {
  *(int *)total += strlen(x);
  return true;
}

new_test(test_exe_pass) {
  int total = 0;
  parser parse_set = or(try(and(exe(ch('a'), set_int, &total), ch('b'))), (ch('a')));
  return check_parse("ab", parse_set, "ab") && total == 1;
}

new_test(test_exe_fail) {
  int total = 0;
  parser parse_set = or(try(and(exe(ch('a'), set_int, &total),
                                ch('b'))),
                        (ch('a')));
  return check_parse("a", parse_set, "a") && total == 0;
}

bool parsed_x(char *xs, void *total) {
  *(int *)total += 10 * strlen(xs);
  return true;
}

bool parsed_v(char *vs, void *total) {
  *(int *)total += 5 * strlen(vs);
  return true;
}

bool parsed_i(char *is, void *total) {
  *(int *)total += strlen(is);
  return true;
}

bool parsed_iv(char *iv, void *total) {
  (void)iv;
  *(int *)total += 4;
  return true;
}

new_test(test_roman_numeral) {
  int total = 0;
  return check_parse("XVII", and4(exe(many(ch('X')), parsed_x, &total),
                                  exe(many(ch('V')), parsed_v, &total),
                                  exe(many(ch('I')), parsed_i, &total),
                                  eof), "XVII")
    && total == 17;
}

new_test(test_roman_numeral_2) {
  int total = 0;

  parser parse_xs = exe(many(ch('X')), parsed_x, &total);
  parser parse_vs = exe(many(ch('V')), parsed_v, &total);
  parser parse_is = or(try(exe(str("IV"), parsed_iv, &total)),
                       exe(many(ch('I')), parsed_i, &total));
  parser parse_roman = and4(parse_xs, parse_vs, parse_is, eof);

  return check_parse("XVII", parse_roman, "XVII") && total == 17
    && check_parse("IV", parse_roman, "IV") && total == 21;
}

int value(char c) {
  switch(c) {
  case 'I':
    return 1;
  case 'V':
    return 5;
  case 'X':
    return 10;
  case 'L':
    return 50;
  case 'C':
    return 100;
  case 'D':
    return 500;
  case 'M':
    return 1000;
  default:
    return -1;
  }
}

/* bool add_value(char *letter, void *total) { */
/*   *(int *)total += value(*letter); */
/* } */

/* bool set_value(char *letter, void *total) { */
/*   *(int *)total -= value(*letter); */
/* } */

/* struct pair { */
/*   int first; */
/*   int second; */
/* } */

/* parser pair(char a, char b, void *total) { */
/*   return and(ch(a), ch(b)) */
/* } */

bool parsed_single(char *letter, void *total) {
  *(int *)total += value(*letter) * strlen(letter);
  return true;
}

parser single(char a, int *total) {
  return exe(many(ch(a)), parsed_single, total);
}

new_test(test_roman_numeral_3) {
  int total = 0;
  parser parse_m = single('M', &total);
  parser parse_d = single('D', &total);
  parser parse_c = single('C', &total);
  parser parse_l = single('L', &total);
  parser parse_x = single('X', &total);
  parser parse_v = single('V', &total);
  parser parse_i = single('I', &total);
  parser parse_roman = and8(parse_m, parse_d, parse_c, parse_l, parse_x,
                            parse_v, parse_i, eof);
  return check_parse("MCLXVII", parse_roman, "MCLXVII") && total == 1167;
}

int main() {
  size_t total = SET_COUNT(test_case_fn), pass = 0, fail = 0;
  info("Running %zu tests.", total);
  for (int i = 0; i < SET_COUNT(test_case_fn); i++) {
    bool (*test)(void) = (bool (*)(void))SET_ITEM(test_case_fn, i);
    char **name = SET_ITEM(test_case_name, i);
    bool success = test();
    if (success) {
      pass += 1;
    } else {
      error("Test \"%s\" failed.", *name);
      fail += 1;
    }
  }

  info("%zu passes, %zu failures", pass, fail);
  return 0;
}
