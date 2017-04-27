#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * EOF parser, will only pass if EOF.
 */

struct parser_eof {
  struct parser parser;
};

static bool
parser_run_eof(const struct parser *p, struct parse_state *state)
{
  (void)p;
  if (state_finished(state)) {
    state_success_blank(state);
    return true;
  }
  return false;
}

struct parser *
parser_create_eof()
{
  struct parser_eof *parser = malloc(sizeof(struct parser_eof));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_eof;
  return (struct parser *)parser;
}
