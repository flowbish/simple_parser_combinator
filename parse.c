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

bool output_string_append(char c, char **o) {
  if (o == NULL) {
    return false;
  } else {
    char buffer[] = {c, '\0'};
    *o = realloc(*o, strlen(*o) + 2);
    strcat(*o, buffer);
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
    output_string_append(c, o);
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
 * Blank parser, will consume no input and always succeed.
 */

bool parser_run_blank(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  return state_success_blank(state, o);
}

struct parser *parser_create_blank() {
  struct parser *parser = malloc(sizeof(struct parser));
  parser->free = parser_free_default;
  parser->run = parser_run_blank;
  return (struct parser *)parser;
}

/**
 * Null parser, will consume no input and always succeed.
 */

bool parser_run_null(const struct parser *p, struct parse_state *state, char **o) {
  (void)p;
  (void)state;
  (void)o;
  return false;
}

struct parser *parser_create_null() {
  struct parser *parser = malloc(sizeof(struct parser));
  parser->free = parser_free_default;
  parser->run = parser_run_null;
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
  parser->parser.free = parser_free_default;
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
  struct parse_state state_save = *state;
  char *o_save = output_string_save(o);
  char cur;
  while (*str && state_getc(state, &cur)) {
    if (cur != *(str++)) {
      *state = state_save;
      output_string_replace(o_save, o);
      free(o_save);
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
  parser->parser.free = parser_free_str;
  parser->parser.run = parser_run_str;
  parser->str = strdup(str);
  return (struct parser *)parser;
}

/**
 * Union of parsers. Will attempt to execute the given parsers in order,
 * succeeding if either succeed.
 */

struct parser_and_or {
  struct parser parser;
  parser left;
  parser right;
};

bool parser_run_or(const struct parser *p, struct parse_state *state, char **o) {
  struct parse_state state_save = *state;
  char *o_save = output_string_save(o);
  bool left = parser_run(((struct parser_and_or *)p)->left, state, o);
  if (left) {
    free(o_save);
    return true;
  }
  *state = state_save;
  output_string_replace(o_save, o);
  bool right = parser_run(((struct parser_and_or *)p)->right, state, o);
  if (right) {
    free(o_save);
    return true;
  }
  *state = state_save;
  output_string_replace(o_save, o);
  free(o_save);
  return false;
}

void parser_free_or(parser p) {
  parser_free(((struct parser_and_or *)p)->left);
  parser_free(((struct parser_and_or *)p)->right);
  parser_free_default(p);
}

struct parser *parser_create_or(parser left, parser right) {
  struct parser_and_or *parser = malloc(sizeof(struct parser_and_or));
  parser->parser.free = parser_free_or;
  parser->parser.run = parser_run_or;
  parser->left = left;
  parser->right = right;
  return (struct parser *)parser;
}

/**
 * Concatenation of parser. Will attempt to execute the given parses in order,
 * succeeding only if both succeed.
 */

bool parser_run_and(const struct parser *p, struct parse_state *state, char **o) {
  struct parse_state state_save = *state;
  char *o_save = output_string_save(o);
  bool left = parser_run(((struct parser_and_or *)p)->left, state, o);
  if (!left) {
    *state = state_save;
    output_string_replace(o_save, o);
    free(o_save);
    return false;
  }
  bool right = parser_run(((struct parser_and_or *)p)->right, state, o);
  if (!right) {
    *state = state_save;
    output_string_replace(o_save, o);
    free(o_save);
    return false;
  }
  free(o_save);
  return true;
}

void parser_free_and(parser p) {
  parser_free_or(p);
}

struct parser *parser_create_and(parser left, parser right) {
  struct parser_and_or *parser = malloc(sizeof(struct parser_and_or));
  parser->parser.free = parser_free_and;
  parser->parser.run = parser_run_and;
  parser->left = left;
  parser->right = right;
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
  parser->parser.free = parser_free_many;
  parser->parser.run = parser_run_many;
  parser->target = target;
  return (struct parser *)parser;
}

bool run(parser p, const char *input, char **output) {
  struct parse_state state;
  state_create(&state, input);
  return parser_run(p, &state, output);
}
