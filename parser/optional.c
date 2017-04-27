#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * This parser fails if and only if the given parser fails and consumes input.
 */

struct parser_optional {
  struct parser parser;
  struct parser *target;
};

static bool
parser_run_optional(const struct parser *p, struct parse_state *state)
{
  size_t _pos = state->pos;
  bool success = parser_run(((struct parser_optional *)p)->target, state);
  if (!success && _pos != state->pos) {
    return false;
  }
  state_success_blank(state);
  return true;
}

static void
parser_free_optional(struct parser *p)
{
  parser_free(((struct parser_optional *)p)->target);
}

struct parser *
parser_create_optional(struct parser *target)
{
  struct parser_optional *parser = malloc(sizeof(struct parser_optional));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_optional;
  parser->parser.run = parser_run_optional;
  parser->target = target;
  return (struct parser *)parser;
}
