#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "state.h"

bool
state_getc(struct parse_state *state, char *c)
{
  if (state->pos >= state->input_len) {
    return false;
  }
  if (c) {
    *c = state->input[state->pos];
  }
  return true;
}

void
state_create(struct parse_state *state, const char *input)
{
  memset(state, 0, sizeof(struct parse_state));
  state->input = input;
  state->input_len = strlen(input);
  state->pos = 0;
}

void
state_copy(struct parse_state *dest, struct parse_state *src)
{
  dest->input = src->input;
  dest->input_len = src->input_len;
  dest->pos = src->pos;
  dest->num_outputs = src->num_outputs;
  if (src->output) {
    dest->output = malloc(strlen(src->output) + 1);
    strcpy(dest->output, src->output);
  } else {
    dest->output = NULL;
  }
  dest->strings = malloc(src->num_outputs * sizeof(char *));
  for (size_t i = 0; i < src->num_outputs; i += 1) {
    if (src->strings[i]) {
      dest->strings[i] = malloc(strlen(src->strings[i]) + 1);
      strcpy(dest->strings[i], src->strings[i]);
    } else {
      dest->strings[i] = NULL;
    }
  }
  dest->args = malloc(src->num_outputs * sizeof(void *));
  memcpy(dest->args, src->args, src->num_outputs * sizeof(void *));
  dest->handlers = malloc(src->num_outputs * sizeof(bool (*)(char *, void *)));
  memcpy(dest->handlers, src->handlers, src->num_outputs * sizeof(bool (*)(char *, void *)));
}

void
state_destroy(struct parse_state *target)
{
  free(target->output);
  for (size_t i = 0; i < target->num_outputs; i += 1) {
    free(target->strings[i]);
  }
  free(target->handlers);
  free(target->strings);
  free(target->args);
}

bool
state_execute(struct parse_state *state)
{
  bool success = true;
  for (size_t i = 0; i < state->num_outputs; i += 1) {
    success = success && (*state->handlers[i])(state->strings[i], state->args[i]);
  }
  return success;
}

bool
state_finished(struct parse_state *state)
{
  return state->pos == state->input_len;
}

/**
 * Undo the action of reading a single character. Always returns false for
 * convenience.
 */
bool
state_rewind(struct parse_state *state)
{
  // pass
  (void)state;
  return false;
}

/**
 * Undo the action of reading n characters. Will never rewind past the beginning
 * of the input. Always returns false for convenience.
 */
bool
state_rewind_n(struct parse_state *state, size_t n)
{
  if (n > 1) {
    if (n > state->pos) {
      state->pos = 0;
    }
    state->pos -= n;
  }
  return false;
}

/**
 * Allocates a free-able output string to the address given. No-op if o is NULL.
 */
bool
output_string_create(char **o)
{
  if (o == NULL) {
    return false;
  } else if (*o == NULL) {
    *o = malloc(1);
    **o = '\0';
  }
  return true;
}

bool
output_string_append_char(char **o, char c)
{
  if (o == NULL) {
    return false;
  } else {
    char buffer[] = {c, '\0'};
    *o = realloc(*o, strlen(*o) + 2);
    strcat(*o, buffer);
    return true;
  }
}

bool
output_string_append_str(char **o, char *s)
{
  if (o == NULL || s == NULL) {
    return false;
  } else {
    *o = realloc(*o, strlen(*o) + strlen(s) + 1);
    strcat(*o, s);
    return true;
  }
}

bool
state_success(struct parse_state *state, char c)
{
  if (state->output == NULL) {
    output_string_create(&state->output);
  }
  output_string_append_char(&state->output, c);
  state->pos += 1;
  return true;
}

bool
state_success_blank(struct parse_state *state)
{
  if (state->output == NULL) {
    output_string_create(&state->output);
  }
  return true;
}

bool
state_output_append_str(struct parse_state *state, char *str)
{
  return output_string_append_str(&state->output, str);
}

bool
state_add_handler(
    struct parse_state *state,
    bool (*handler)(char *, void *),
    char *string,
    void *arg)
{
  state->num_outputs += 1;

  state->strings = realloc(state->strings, (state->num_outputs) * sizeof(char *) );
  state->strings[state->num_outputs - 1] = malloc(strlen(string) + 1);
  strcpy(state->strings[state->num_outputs - 1], string);

  state->args = realloc(state->args, (state->num_outputs) * sizeof(void *));
  state->args[state->num_outputs - 1] = arg;

  state->handlers = realloc(state->handlers, (state->num_outputs) * sizeof(bool (*)(char *, void *)));
  state->handlers[state->num_outputs - 1] = handler;
  return true;
}
