#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Try to apply a given parser, rolling back input if a parsing error occurs.
 */

struct parser_try {
  struct parser parser;
  struct parser *target;
};

static bool
parser_run_try(const struct parser *p, struct parse_state *state)
{
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

static void
parser_free_try(struct parser *p)
{
  parser_free(((struct parser_try *)p)->target);
}

struct parser *
parser_create_try(struct parser *target)
{
  struct parser_try *parser = malloc(sizeof(struct parser_try));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_try;
  parser->parser.run = parser_run_try;
  parser->target = target;
  return (struct parser *)parser;
}
