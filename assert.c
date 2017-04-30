#include <stdbool.h>
#include <string.h>

#include "assert.h"
#include "error.h"
#include "log.h"

struct error *
assert_unsigned_equal(unsigned expected, unsigned actual)
{
  if (expected != actual)
    raise("Expected: %u, Actual: %u", expected, actual);
  return NULL;
}

struct error *
assert_int_equal(int expected, int actual)
{
  if (expected != actual)
    raise("Expected: %u, Actual: %u", expected, actual);
  return NULL;
}

struct error *
assert_string_equal(char *expected, char *actual)
{
  if (strcmp(expected, actual) != 0)
    raise("Expected: \"%s\", Actual: \"%s\"", expected, actual);
  return NULL;
}
