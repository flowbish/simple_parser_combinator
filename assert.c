#include <stdbool.h>
#include <string.h>

#include "assert.h"
#include "error.h"
#include "log.h"

struct error *
assert(bool actual)
{
   if (!actual)
     raise("Actual was false");
   return NULL;
}

struct error *
assert_not_null(void *actual)
{
   if (actual == NULL)
     raise("Actual was NULL");
   return NULL;
}

struct error *
assert_null(void *actual)
{
   if (actual != NULL)
     raise("Actual was not NULL");
   return NULL;
}

struct error *
assert_error(struct error *error)
{
    if (error == NULL)
     raise("No error was raised!");
    free(error);
    return NULL;
}

struct error *
assert_error_with_message(struct error *error, const char *message)
{
    if (error == NULL) {
        raise("No error was raised!");
    } else if (strstr(error->message, message) == NULL) {
        raise(
            "Error message incorrect, expected: %s, actual: %s",
            message,
            error->message);
    }
    free(error);
    return NULL;
}

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
assert_uint8_equal(uint8_t expected, uint8_t actual)
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
