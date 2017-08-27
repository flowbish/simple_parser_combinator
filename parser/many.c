#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Kleene star. Will attempt to match the first parser as many times as
 * possible.
 */

struct parser_many {
  struct parser parser;
  struct parser *target;
};

static bool
parser_run_many(const struct parser *p, struct parse_state *state)
{
  state_success_blank(state);
  bool success = true;
  struct parser *target = ((struct parser_many *)p)->target;
  do {
    success = parser_run(target, state);
  } while (success == true);
  return true;
}

static void
parser_free_many(struct parser *p)
{
  parser_free(((struct parser_many *)p)->target);
}

struct parser *
parser_create_many(struct parser *target)
{
  struct parser_many *parser = malloc(sizeof(struct parser_many));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_many;
  parser->parser.run = parser_run_many;
  parser->target = target;
  return (struct parser *)parser;
}
