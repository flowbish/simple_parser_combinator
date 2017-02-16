#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "parse.h"

#ifdef DEBUG
#define debug(string, ...) fprintf (stderr, "[DEBUG] "string "\n", ##__VA_ARGS__)
#else
#define debug(...)
#endif

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

struct parser {
  bool (*run)(const struct parser*, struct parse_state*);
  void (*free)(struct parser*);
};

typedef bool (*parser_run_fn)(const struct parser*, struct parse_state*, char **o);
typedef void (*parser_free_fn)(struct parser*);

bool state_getc(struct parse_state *state, char *c) {
  if (state->pos >= state->input_len) {
    return false;
  }
  if (c) {
    *c = state->input[state->pos];
  }
  return true;
}

void state_create(struct parse_state *state, const char *input) {
  memset(state, 0, sizeof(struct parse_state));
  state->input = input;
  state->input_len = strlen(input);
  state->pos = 0;
}

void state_copy(struct parse_state *dest, struct parse_state *src) {
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

void state_destroy(struct parse_state *target) {
  free(target->output);
  for (size_t i = 0; i < target->num_outputs; i += 1) {
    free(target->strings[i]);
  }
  free(target->handlers);
  free(target->strings);
  free(target->args);
}

bool state_execute(struct parse_state *state) {
  bool success = true;
  for (size_t i = 0; i < state->num_outputs; i += 1) {
    debug("executing handler for '%s'", state->strings[i]);
    success = success && (*state->handlers[i])(state->strings[i], state->args[i]);
  }
  return success;
}

bool state_finished(struct parse_state *state) {
  return state->pos == state->input_len;
}

/**
 * Undo the action of reading a single character. Always returns false for
 * convenience.
 */
bool state_rewind(struct parse_state *state) {
  // pass
  (void)state;
  return false;
}

/**
 * Undo the action of reading n characters. Will never rewind past the beginning
 * of the input. Always returns false for convenience.
 */
bool state_rewind_n(struct parse_state *state, size_t n) {
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
bool output_string_create(char **o) {
  if (o == NULL) {
    return false;
  } else if (*o == NULL) {
    *o = malloc(1);
    **o = '\0';
  }
  return true;
}

bool output_string_append_char(char **o, char c) {
  if (o == NULL) {
    return false;
  } else {
    char buffer[] = {c, '\0'};
    *o = realloc(*o, strlen(*o) + 2);
    strcat(*o, buffer);
    return true;
  }
}

bool output_string_append_str(char **o, char *s) {
  if (o == NULL || s == NULL) {
    return false;
  } else {
    *o = realloc(*o, strlen(*o) + strlen(s) + 1);
    strcat(*o, s);
    return true;
  }
}

bool output_string_replace(char *new, char **o) {
  if (o == NULL) {
    return false;
  } else {
    free(*o);
    *o = new;
    return true;
  }
}

bool state_success(struct parse_state *state, char c) {
  if (state->output == NULL) {
    output_string_create(&state->output);
  }
  output_string_append_char(&state->output, c);
  state->pos += 1;
  return true;
}

bool state_success_blank(struct parse_state *state) {
  if (state->output == NULL) {
    output_string_create(&state->output);
  }
  return true;
}

bool state_output_append_str(struct parse_state *state, char *str) {
  if (state->output == NULL) {
    output_string_create(&state->output);
  }
  return output_string_append_str(&state->output, str);
}

bool state_add_handler(struct parse_state *state, bool (*handler)(char *, void *), char *string, void *arg) {
  state->num_outputs += 1;

  debug("added handler for '%s' index %zu", string, state->num_outputs - 1);
  state->strings = realloc(state->strings, (state->num_outputs) * sizeof(char *) );
  state->strings[state->num_outputs - 1] = malloc(strlen(string) + 1);
  strcpy(state->strings[state->num_outputs - 1], string);

  state->args = realloc(state->args, (state->num_outputs) * sizeof(void *));
  state->args[state->num_outputs - 1] = arg;

  state->handlers = realloc(state->handlers, (state->num_outputs) * sizeof(bool (*)(char *, void *)));
  state->handlers[state->num_outputs - 1] = handler;
  return true;
}

/**
 * Generic interface to executing a parser.
 */
bool parser_run(const struct parser *p, struct parse_state *state) {
  return (p->run)(p, state);
}

/**
 * Default free function for a parser, used if no other free'd data is needed.
 */
void parser_free(struct parser *p) {
  return (p->free)(p);
}

void parser_free_default(parser p) {
  free(p);
}

/**
 * Default parser run method, consumes no input and doesn't fail.
 */

bool parser_run_default(const struct parser *p, struct parse_state *state) {
  (void)p;
  (void)state;
  return true;
}

void parser_set_defaults(struct parser *p) {
  p->free = parser_free_default;
  p->run = parser_run_default;
}

/**
 * Blank parser, will consume no input and always succeed.
 */

struct parser_blank {
  struct parser parser;
};

bool parser_run_blank(const struct parser *p, struct parse_state *state) {
  (void)p;
  return state_success_blank(state);
}

struct parser *parser_create_blank() {
  struct parser_blank *parser = malloc(sizeof(struct parser_blank));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_blank;
  return (struct parser *)parser;
}

/**
 * Null parser, will consume no input and always succeed.
 */

struct parser_null {
  struct parser parser;
};

bool parser_run_null(const struct parser *p, struct parse_state *state) {
  (void)p;
  (void)state;
  return false;
}

struct parser *parser_create_null() {
  struct parser_null *parser = malloc(sizeof(struct parser_null));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_null;
  return (struct parser *)parser;
}

/**
 * EOF parser, will only pass if EOF.
 */

struct parser_eof {
  struct parser parser;
};

bool parser_run_eof(const struct parser *p, struct parse_state *state) {
  (void)p;
  if (state_finished(state)) {
    state_success_blank(state);
    return true;
  }
  return false;
}

struct parser *parser_create_eof() {
  struct parser_eof *parser = malloc(sizeof(struct parser_eof));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_eof;
  return (struct parser *)parser;
}

/**
 * Single character parser. Will attmept to parse a single, specified character
 * from the input.
 */

struct parser_char {
  struct parser parser;
  char c;
};

bool parser_run_char(const struct parser *p, struct parse_state *state) {
  char b, c = ((struct parser_char *)p)->c;
  if (state_getc(state, &b) && b == c) {
    return state_success(state, b);
  }
  return state_rewind(state);
}

struct parser *parser_create_char(char c) {
  struct parser_char *parser = malloc(sizeof(struct parser_char));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_char;
  parser->c = c;
  return (struct parser *)parser;
}

/**
 * Parse a specified string.
 */

struct parser_str {
  struct parser parser;
  char *str;
};

bool parser_run_str(const struct parser *p, struct parse_state *state) {
  char *str = ((struct parser_str *)p)->str;
  char cur;
  while (*str && state_getc(state, &cur)) {
    if (cur != *(str++)) {
      return state_rewind(state);
    }
    state_success(state, cur);
  }
  return true;
}

void parser_free_str(parser p) {
  free(((struct parser_str *)p)->str);
  parser_free_default(p);
}

struct parser *parser_create_str(char *str) {
  struct parser_str *parser = malloc(sizeof(struct parser_str));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_str;
  parser->parser.run = parser_run_str;
  parser->str = strdup(str);
  return (struct parser *)parser;
}

/**
 * Union of parsers. Will attempt to execute the given parsers in order,
 * succeeding if either succeed.
 */

struct parser_or {
  struct parser parser;
  parser first;
  parser second;
};

bool parser_run_or(const struct parser *p, struct parse_state *state) {
  bool first = parser_run(((struct parser_or *)p)->first, state);
  if (first) {
    return true;
  }
  bool second = parser_run(((struct parser_or *)p)->second, state);
  if (second) {
    return true;
  }
  return false;
}

void parser_free_or(parser p) {
  parser_free(((struct parser_or *)p)->first);
  parser_free(((struct parser_or *)p)->second);
  parser_free_default(p);
}

struct parser *parser_create_or(parser first, parser second) {
  struct parser_or *parser = malloc(sizeof(struct parser_or));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_or;
  parser->parser.run = parser_run_or;
  parser->first = first;
  parser->second = second;
  return (struct parser *)parser;
}

/**
 * Concatenation of parser. Will attempt to execute the given parses in order,
 * succeeding only if both succeed.
 */

struct parser_and {
  struct parser parser;
  parser first;
  parser second;
};

bool parser_run_and(const struct parser *p, struct parse_state *state) {
  bool first = parser_run(((struct parser_and *)p)->first, state);
  if (!first) {
    return false;
  }
  bool second = parser_run(((struct parser_and *)p)->second, state);
  if (!second) {
    return false;
  }
  return true;
}

void parser_free_and(parser p) {
  parser_free(((struct parser_and *)p)->first);
  parser_free(((struct parser_and *)p)->second);
  parser_free_default(p);
}

struct parser *parser_create_and(parser first, parser second) {
  struct parser_and *parser = malloc(sizeof(struct parser_and));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_and;
  parser->parser.run = parser_run_and;
  parser->first = first;
  parser->second = second;
  return (struct parser *)parser;
}

/**
 * Kleene star. Will attempt to match the first parser as many times as
 * possible.
 */

struct parser_many {
  struct parser parser;
  parser target;
};

bool parser_run_many(const struct parser *p, struct parse_state *state) {
  state_success_blank(state);
  bool success = true;
  parser target = ((struct parser_many *)p)->target;
  do {
    success = parser_run(target, state);
  } while (success == true);
  return true;
}

void parser_free_many(parser p) {
  parser_free(((struct parser_many *)p)->target);
  parser_free_default(p);
}

struct parser *parser_create_many(parser target) {
  struct parser_many *parser = malloc(sizeof(struct parser_many));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_many;
  parser->parser.run = parser_run_many;
  parser->target = target;
  return (struct parser *)parser;
}

/**
 * Try to apply a given parser, rolling back input if a parsing error occurs.
 */

struct parser_try {
  struct parser parser;
  parser target;
};

bool parser_run_try(const struct parser *p, struct parse_state *state) {
  struct parse_state _state;
  state_copy(&_state, state);
  bool success = parser_run(((struct parser_try *)p)->target, state);
  if (!success) {
    state_destroy(state);
    *state = _state;
  } else {
    state_destroy(&_state);
  }
  return success;
}

void parser_free_try(parser p) {
  parser_free(((struct parser_try *)p)->target);
  parser_free_default(p);
}

struct parser *parser_create_try(parser target) {
  struct parser_try *parser = malloc(sizeof(struct parser_try));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_try;
  parser->parser.run = parser_run_try;
  parser->target = target;
  return (struct parser *)parser;
}

/**
 * Executor.
 */

struct parser_execute {
  struct parser parser;
  parser target;
  bool (*handle)(char *, void *);
  void *extra;
};

bool parser_run_execute(const struct parser *p, struct parse_state *state) {
  struct parser_execute *exe = (struct parser_execute *)p;
  char *o_temp = state->output;
  state->output = NULL;
  bool parse_success = parser_run(exe->target, state);
  if (parse_success) {
    state_add_handler(state, exe->handle, state->output, exe->extra);
    state_success_blank(state);
    char *o_temp2 = state->output;
    state->output = o_temp;
    state_output_append_str(state, o_temp2);
    free(o_temp2);
  }
  return parse_success;
}

struct parser *parser_create_execute(parser target, bool (*handle)(char *, void *), void *extra) {
  struct parser_execute *parser = malloc(sizeof(struct parser_execute));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_execute;
  parser->target = target;
  parser->handle = handle;
  parser->extra = extra;
  return (struct parser *)parser;
}

/**
 * Publically exposed run function.
 */

bool run(parser p, const char *input, char **output) {
  struct parse_state state;
  state_create(&state, input);
  bool success = parser_run(p, &state);
  if (success) {
    state_execute(&state);
    output_string_create(output);
    output_string_append_str(output, state.output);
  }
  state_destroy(&state);
  return success;
}
