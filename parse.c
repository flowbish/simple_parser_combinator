#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"
#include "log.h"

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
 * This parser fails if and only if the given parser fails and consumes input.
 */

struct parser_optional {
  struct parser parser;
  parser target;
};

bool parser_run_optional(const struct parser *p, struct parse_state *state) {
  size_t _pos = state->pos;
  bool success = parser_run(((struct parser_optional *)p)->target, state);
  if (!success && _pos != state->pos) {
    return false;
  }
  state_success_blank(state);
  return true;
}

void parser_free_optional(parser p) {
  parser_free(((struct parser_optional *)p)->target);
  parser_free_default(p);
}

struct parser *parser_create_optional(parser target) {
  struct parser_optional *parser = malloc(sizeof(struct parser_optional));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_optional;
  parser->parser.run = parser_run_optional;
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
  char *o_temp2 = state->output;
  state->output = o_temp;
  if (parse_success) {
    state_add_handler(state, exe->handle, o_temp2, exe->extra);
    state_success_blank(state);
    state_output_append_str(state, o_temp2);
  }
  free(o_temp2);
  return parse_success;
}

void parser_free_execute(struct parser *p) {
  parser_free(((struct parser_execute *)p)->target);
  parser_free_default(p);
}

struct parser *parser_create_execute(parser target, bool (*handle)(char *, void *), void *extra) {
  struct parser_execute *parser = malloc(sizeof(struct parser_execute));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_execute;
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
    *output = malloc(strlen(state.output));
    strcpy(*output, state.output);
  }
  state_destroy(&state);
  return success;
}
