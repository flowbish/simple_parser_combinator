#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "parse.h"

#define debug(string, ...) fprintf (stderr, "[DEBUG] "string "\n", ##__VA_ARGS__)

struct parse_state {
  const char *input;
  size_t input_len;
  size_t pos;
};

struct parser {
  bool (*run)(const struct parser*, struct parse_state*, char **o);
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
  state->input = input;
  state->input_len = strlen(input);
  state->pos = 0;
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
  if (o == NULL) {
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

char *output_string_save(char **o) {
  if (o == NULL || *o == NULL)
    return NULL;
  return strdup(*o);
}

bool state_success(struct parse_state *state, char c, char **o) {
  if (o != NULL) {
    if (*o == NULL) {
      output_string_create(o);
    }
    output_string_append_char(o, c);
  }
  state->pos += 1;
  return true;
}

bool state_success_blank(struct parse_state *state, char **o) {
  (void)state;
  if (*o == NULL) {
    output_string_create(o);
  }
  return true;
}

/**
 * Generic interface to executing a parser.
 */
bool parser_run(const struct parser *p, struct parse_state *state, char **o) {
  return (p->run)(p, state, o);
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

bool parser_run_default(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  (void)state;
  (void)o;
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

bool parser_run_blank(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  return state_success_blank(state, o);
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

bool parser_run_null(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  (void)state;
  (void)o;
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

bool parser_run_eof(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  (void)o;
  if (state_finished(state)) {
    state_success_blank(state, o);
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

bool parser_run_char(const struct parser *p, struct parse_state *state, char **o) {
  char b, c = ((struct parser_char *)p)->c;
  if (state_getc(state, &b) && b == c) {
    return state_success(state, b, o);
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

bool parser_run_str(const struct parser *p, struct parse_state *state, char **o) {
  char *str = ((struct parser_str *)p)->str;
  char cur;
  while (*str && state_getc(state, &cur)) {
    if (cur != *(str++)) {
      return false;
    }
    state_success(state, cur, o);
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

bool parser_run_or(const struct parser *p, struct parse_state *state, char **o) {
  bool first = parser_run(((struct parser_or *)p)->first, state, o);
  if (first) {
    return true;
  }
  bool second = parser_run(((struct parser_or *)p)->second, state, o);
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

bool parser_run_and(const struct parser *p, struct parse_state *state, char **o) {
  bool first = parser_run(((struct parser_and *)p)->first, state, o);
  if (!first) {
    return false;
  }
  bool second = parser_run(((struct parser_and *)p)->second, state, o);
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

bool parser_run_many(const struct parser *p, struct parse_state *state, char **o) {
  state_success_blank(state, o);
  bool success = true;
  parser target = ((struct parser_many *)p)->target;
  do {
    success = parser_run(target, state, o);
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

bool parser_run_try(const struct parser *p, struct parse_state *state, char **o) {
  struct parse_state state_save = *state;
  char *o_save = output_string_save(o);
  bool success = parser_run(((struct parser_try *)p)->target, state, o);
  if (!success) {
    *state = state_save;
    output_string_replace(o_save, o);
  } else {
    free(o_save);
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

bool parser_run_execute(const struct parser *p, struct parse_state *state, char **o) {
  (void)o;
  struct parser_execute *exe = (struct parser_execute *)p;
  char *o_temp = NULL;
  bool parse_success = parser_run(exe->target, state, &o_temp);
  if (parse_success) {
    exe->handle(o_temp, exe->extra);
    state_success_blank(state, o);
    output_string_append_str(o, o_temp);
  }
  free(o_temp);
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

bool run(parser p, const char *input, char **output) {
  struct parse_state state;
  state_create(&state, input);
  return parser_run(p, &state, output);
}
