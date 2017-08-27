#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "assert.h"
#include "error.h"
#include "test.h"
#include "parse.h"
#include "log.h"

struct error*
check_parse(const char *input, struct parser *p, const char *expected)
{
  char *output = NULL;
  bool success = run(p, input, &output);
  struct error *error = NULL;

  if (!success && expected != NULL) {
    error_to(error, "Parser failed to match.");
  } else if (success && expected == NULL) {
    error_to(error, "Parser matched erroneously.\nFound: %s", output);
  } else if (success && expected != NULL && strcmp(expected, output) != 0) {
    error_to(error,
             "Parser failed to parse correctly:\n"
             "Expected: %s\n"
             "Found: %s", expected, output);
  }

  free(output);
  parser_free(p);
  return error;
}

new_test(test_blank)
{
  return check_parse("", blank, "");
}

new_test(test_blank_with_input)
{
  return check_parse("test", blank, "");
}

new_test(test_null)
{
  return check_parse("", null, NULL);
}

new_test(test_eof_pass)
{
  return check_parse("", eof, "");
}

new_test(test_eof_fail)
{
  return check_parse("a", eof, NULL);
}

new_test(test_char)
{
  return check_parse("test", ch('t'), "t");
}

new_test(test_str_full)
{
  return check_parse("test", str("test"), "test");
}

new_test(test_str_partial)
{
  return check_parse("testing", str("test"), "test");
}

new_test(test_str_fail)
{
  return check_parse("test", str("something"), NULL);
}

new_test(test_or_first)
{
  return check_parse("test", or(blank, null), "");
}

new_test(test_or_second)
{
  return check_parse("test", or(null, blank), "");
}

/*
 * We should see that the first parser in the or(), str("this"), consumes input
 * until it fails, at which point the next parser, str("that"), picks up where
 * it left off. This means that the first few letters in common must be repeated
 * for this to parse successful. This is why wrapping the first term in a try()
 * is reccommended.
 */
new_test(test_or_string)
{
  return check_parse("ththat", or(str("this"), str("that")), "ththat");
}

/*
 * As explained above, if the first term is wrapped in try(), then our parse
 * behaves exactly as we expect, with both branches of the or starting from the
 * same character regardless of whether the first consumed any input.
 */
new_test(test_try_or_string)
{
  return check_parse("that", or(try(str("this")), str("that")), "that");
}

new_test(test_until)
{
  return check_parse("111one", until(str("one")), "111");
}

new_test(test_until_empty)
{
  return check_parse("", until(str("one")), "");
}

new_test(test_until_eof)
{
  return check_parse("11111", until(str("one")), "11111");
}

new_test(test_and_first_fail)
{
  return check_parse("test", and(null, blank), NULL);
}

new_test(test_and_second_fail)
{
  return check_parse("test", and(blank, null), NULL);
}

new_test(test_and_second_char)
{
  return check_parse("test", and(ch('t'), ch('e')), "te");
}

new_test(test_and_second_char2)
{
  return check_parse("test", and(ch('t'), ch('e'), ch('s')), "tes");
}

new_test(test_many_no_match)
{
  return check_parse("aaabbb", many(null), "");
}

new_test(test_many_single_match)
{
  return check_parse("ab", many(ch('a')), "a");
}

new_test(test_many_matches)
{
  return check_parse("aaabbb", many(ch('a')), "aaa");
}

bool
set_int(char *x, void *total)
{
  *(int *)total = strlen(x);
  return true;
}

new_test(test_exe_pass)
{
  int total = 0;
  struct parser *parse_set = or(try(and(exe(ch('a'), set_int, &total),
                                ch('b'))),
                        (ch('a')));
  error_try(check_parse("ab", parse_set, "ab"));
  error_try(assert_int_equal(1, total));
  return NULL;
}

new_test(test_exe_fail)
{
  int total = 0;
  struct parser *parse_set = or(try(and(exe(ch('a'), set_int, &total),
                                ch('b'))),
                        (ch('a')));
  error_try(check_parse("a", parse_set, "a"));
  error_try(assert_int_equal(0, total));
  return NULL;
}

static bool
parsed_x(char *xs, void *total)
{
  *(int *)total += 10 * strlen(xs);
  return true;
}

static bool
parsed_v(char *vs, void *total)
{
  *(int *)total += 5 * strlen(vs);
  return true;
}

static bool
parsed_i(char *is, void *total)
{
  *(int *)total += strlen(is);
  return true;
}

static bool
parsed_iv(char *iv, void *total)
{
  (void)iv;
  *(int *)total += 4;
  return true;
}

new_test(test_roman_numeral)
{
  int total = 0;
  error_try(check_parse("XVII", and(exe(many(ch('X')), parsed_x, &total),
                                  exe(many(ch('V')), parsed_v, &total),
                                  exe(many(ch('I')), parsed_i, &total),
                                   eof), "XVII"));
  error_try(assert_int_equal(17, total));
  return NULL;
}

new_test(test_roman_numeral_2)
{
  int total = 0;

  struct parser *parse_xs = exe(many(ch('X')), parsed_x, &total);
  struct parser *parse_vs = exe(many(ch('V')), parsed_v, &total);
  struct parser *parse_is = or(try(exe(str("IV"), parsed_iv, &total)),
                       exe(many(ch('I')), parsed_i, &total));
  struct parser *parse_roman = and(parse_xs, parse_vs, parse_is, eof);

  error_try(check_parse("XVII", parse_roman, "XVII"));
  error_try(assert_int_equal(17, total));
  return NULL;
}

int
value(char c)
{
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

bool
add_value(char *letter, void *total)
{
  *(size_t *)total += value(*letter);
  return true;
}

bool
sub_value(char *letter, void *total)
{
  *(size_t *)total -= value(*letter);
  return true;
}

struct parser *
pair(char a, char b, void *total)
{
  return and(exe(ch(a), sub_value, total),
             exe(ch(b), add_value, total));
}

struct parser *
single(char a, size_t *total)
{
  return optional(many(exe(ch(a), add_value, total)));
}

struct parser *
roman_numeral(size_t *total)
{
  struct parser *parse_m = single('M', total);
  struct parser *parse_d = single('D', total);
  struct parser *parse_c = or(try(pair('C', 'M', total)),
                      try(pair('C', 'D', total)),
                      single('C', total));
  struct parser *parse_l = single('L', total);
  struct parser *parse_x = or(try(pair('X', 'C', total)),
                      try(pair('X', 'L', total)),
                      single('X', total));
  struct parser *parse_v = single('V', total);
  struct parser *parse_i = or(try(pair('I', 'X', total)),
                      try(pair('I', 'V', total)),
                      single('I', total));
  return and(parse_m, parse_d, parse_c, parse_l, parse_x,
              parse_v, parse_i, eof);
}

new_test(test_roman_numeral_3)
{
  size_t total = 0;
  error_try(check_parse("XCII", roman_numeral(&total), "XCII"));
  error_try(assert_int_equal(92, total));
  return NULL;
}

new_test(test_roman_numeral_4)
{
  size_t total = 0;
  error_try(check_parse("MDCCXCVII", roman_numeral(&total), "MDCCXCVII"));
  error_try(assert_int_equal(1797, total));
  return NULL;
}

static bool
capture_string(char *capture, void *dest)
{
  // Gross
  *((char **)dest) = malloc(strlen(capture) + 1);
  strcpy(*((char **)dest), capture);
  return true;
}

static struct parser *
delim_parser(char left, char right, char **inner)
{
  return and(ch(left),
              exe(until(ch(right)),
                  capture_string,
                  inner),
              ch(right));
}

new_test(test_delims)
{
  char *inner = NULL;
  error_try(check_parse("{test}", delim_parser('{', '}', &inner), "{test}"));
  error_try(assert_string_equal("test", inner));
  return NULL;
}
