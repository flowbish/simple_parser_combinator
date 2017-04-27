#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Blank parser, will consume no input and always succeed.
 */

struct parser_blank {
  struct parser parser;
};

static bool
parser_run_blank(const struct parser *p, struct parse_state *state)
{
  (void)p;
  return state_success_blank(state);
}

struct parser *
parser_create_blank()
{
  struct parser_blank *parser = malloc(sizeof(struct parser_blank));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_blank;
  return (struct parser *)parser;
}
