#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Single character parser. Will attmept to parse a single, specified character
 * from the input.
 */

struct parser_char {
  struct parser parser;
  char c;
};

static bool
parser_run_char(const struct parser *p, struct parse_state *state)
{
  char b, c = ((struct parser_char *)p)->c;
  if (state_getc(state, &b) && b == c) {
    return state_success(state, b);
  }
  return state_rewind(state);
}

struct parser *
parser_create_char(char c)
{
  struct parser_char *parser = malloc(sizeof(struct parser_char));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_char;
  parser->c = c;
  return (struct parser *)parser;
}
