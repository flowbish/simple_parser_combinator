#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Concatenation of parser. Will attempt to execute the given parses in order,
 * succeeding only if both succeed.
 */

struct parser_and {
  struct parser parser;
  struct parser *first;
  struct parser *second;
};

static bool
parser_run_and(const struct parser *p, struct parse_state *state)
{
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

static void
parser_free_and(struct parser *p)
{
  parser_free(((struct parser_and *)p)->first);
  parser_free(((struct parser_and *)p)->second);
}

struct parser *
parser_create_and(struct parser *first, struct parser *second)
{
  struct parser_and *parser = malloc(sizeof(struct parser_and));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_and;
  parser->parser.run = parser_run_and;
  parser->first = first;
  parser->second = second;
  return (struct parser *)parser;
}
