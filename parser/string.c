#include <string.h>

#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Parse a specified string.
 */

struct parser_str {
  struct parser parser;
  char *str;
};

static bool
parser_run_str(const struct parser *p, struct parse_state *state)
{
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

static void
parser_free_str(struct parser *p)
{
  free(((struct parser_str *)p)->str);
}

struct parser *
parser_create_str(char *str)
{
  struct parser_str *parser = malloc(sizeof(struct parser_str));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_str;
  parser->parser.run = parser_run_str;
  parser->str = strdup(str);
  return (struct parser *)parser;
}
