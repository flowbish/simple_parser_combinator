#pragma once

#include <stdbool.h>
#include <stdint.h>

struct error *assert(bool actual);

struct error *assert_not_null(void *actual);

struct error *assert_null(void *actual);

/*
 * Asserts that an error was returned. This also frees the error received
 * (if any).
 */
struct error *assert_error(struct error *error);

/*
 * Asserts that an error was returned and contains the specified text. This
 * also frees the error received (if any).
 */
struct error *assert_error_with_message(struct error *error, const char *message);

struct error *assert_unsigned_equal(unsigned expected, unsigned actual); 

struct error *assert_int_equal(int expected, int actual);

struct error *assert_uint8_equal(uint8_t expected, uint8_t actual);

struct error *assert_string_equal(char *expected, char *actual);
