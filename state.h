#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

struct parse_state {
  const char *input;
  size_t input_len;
  size_t pos;
  char *output;
  size_t num_outputs;
  bool (**handlers)(char *, void *);
  char **strings;
  void **args;
};

bool state_getc(struct parse_state *state, char *c);

void state_create(struct parse_state *state, const char *input);

void state_copy(struct parse_state *dest, struct parse_state *src);

void state_destroy(struct parse_state *target);

bool state_execute(struct parse_state *state);

bool state_finished(struct parse_state *state);

/**
 * Undo the action of reading a single character. Always returns false for
 * convenience.
 */
bool state_rewind(struct parse_state *state);

/**
 * Undo the action of reading n characters. Will never rewind past the beginning
 * of the input. Always returns false for convenience.
 */
bool state_rewind_n(struct parse_state *state, size_t n);

bool state_success(struct parse_state *state, char c);

bool state_success_blank(struct parse_state *state);

bool state_output_append_str(struct parse_state *state, char *str);

bool state_add_handler(struct parse_state *state, bool (*handler)(char *, void *), char *string, void *arg);
