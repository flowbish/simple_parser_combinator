#pragma once

#include <stdbool.h>

struct error *assert_unsigned_equal(unsigned expected, unsigned actual); 

struct error *assert_int_equal(int expected, int actual);

struct error *assert_string_equal(char *expected, char *actual);
