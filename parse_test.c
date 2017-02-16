#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "linker_set.h"
#include "parse.h"
#include "log.h"

typedef bool (test_case_fn_t)(void);
LINKERSET_DECLARE(test_case_fn);
typedef char *test_case_name_t;
LINKERSET_DECLARE(test_case_name);

#define declare_test(name)                            \
  bool name(void);                                    \
  LINKERSET_ADD_ITEM(test_case_fn, name);                      \
  const char *test_case_##name##_str = #name;         \
  LINKERSET_ADD_ITEM(test_case_name, test_case_##name##_str);  \

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
  bool result = true;
  if (!success && expected != NULL) {
    error("Parser failed to match.");
    result = false;
  } else if (success && expected == NULL) {
    error("Parser matched erroneously.");
    error("Found: %s", output);
    result = false;
  } else if (success && expected != NULL && strcmp(expected, output) != 0) {
    error("Parser failed to parse correctly:");
    error("Found: %s", output);
    error("Expected: %s", expected);
    result = false;
  }
  free(output);
  parser_free(p);
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
  *(int *)total = strlen(x);
  return true;
}

new_test(test_exe_pass) {
  int total = 0;
  parser parse_set = or(try(and(exe(ch('a'), set_int, &total),
                                ch('b'))),
                        (ch('a')));
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

  return check_parse("XVII", parse_roman, "XVII") && total == 17;
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

bool add_value(char *letter, void *total) {
  *(size_t *)total += value(*letter);
  return true;
}

bool sub_value(char *letter, void *total) {
  *(size_t *)total -= value(*letter);
  return true;
}

parser pair(char a, char b, void *total) {
  return and(exe(ch(a), sub_value, total),
             exe(ch(b), add_value, total));
}

parser single(char a, size_t *total) {
  return optional(many(exe(ch(a), add_value, total)));
}

parser roman_numeral(size_t *total) {
  parser parse_m = single('M', total);
  parser parse_d = single('D', total);
  parser parse_c = or3(try(pair('C', 'M', total)),
                       try(pair('C', 'D', total)),
                       single('C', total));
  parser parse_l = single('L', total);
  parser parse_x = or3(try(pair('X', 'C', total)),
                       try(pair('X', 'L', total)),
                       single('X', total));
  parser parse_v = single('V', total);
  parser parse_i = or3(try(pair('I', 'X', total)),
                       try(pair('I', 'V', total)),
                       single('I', total));
  return and8(parse_m, parse_d, parse_c, parse_l, parse_x,
              parse_v, parse_i, eof);
}

new_test(test_roman_numeral_3) {
  size_t total = 0;
  return check_parse("XCII", roman_numeral(&total), "XCII") && total == 92;
}

new_test(test_roman_numeral_4) {
  size_t total = 0;
  return check_parse("MDCCXCVII", roman_numeral(&total), "MDCCXCVII") && total == 1797;
}

bool test_should_run(char *test_name, char **valid_names, size_t num_valid_names) {
  for (size_t i = 0; i < num_valid_names; i += 1) {
    if (strcmp(test_name, valid_names[i]) == 0) {
      return true;
    }
  }
  return false;
}

int main(int argc, char **argv) {
  size_t pass = 0, fail = 0;
  for (size_t i = 0; i < LINKERSET_SIZE(test_case_fn, size_t); i++) {
    LINKERSET_GET(test_case_fn, test, i);
    LINKERSET_GET(test_case_name, name, i);
    // Run the test if test name is supplied as an argument (or if there are no
    // arguments)
    if (argc == 1 || test_should_run(*name, &argv[1], argc - 1)) {
      debug("Running test \"%s\"", *name);
      bool success = (*test)();
      if (success) {
        pass += 1;
      } else {
        error("Test \"%s\" failed.", *name);
        fail += 1;
      }
    }
  }

  info("%zu passes, %zu failures", pass, fail);
  return 0;
}
