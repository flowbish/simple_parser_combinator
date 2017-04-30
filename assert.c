#include <stdbool.h>
#include <string.h>

#include "assert.h"
#include "log.h"

bool
assert_unsigned_equal(unsigned expected, unsigned actual)
{
  if (expected != actual) {
    error("Expected: %u, Actual: %u", expected, actual);
    return false;
  }
  return true;
}

bool
assert_string_equal(char *expected, char *actual)
{
  if (strcmp(expected, actual) != 0) {
    error("Expected: \"%s\", Actual: \"%s\"", expected, actual);
    return false;
  }
  return true;
}
