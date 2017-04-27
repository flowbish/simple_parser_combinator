#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Union of parsers. Will attempt to execute the given parsers in order,
 * succeeding if either succeed.
 */

struct parser_or {
  struct parser parser;
  struct parser *first;
  struct parser *second;
};

static bool
parser_run_or(const struct parser *p, struct parse_state *state)
{
  bool first = parser_run(((struct parser_or *)p)->first, state);
  if (first) {
    return true;
  }
  bool second = parser_run(((struct parser_or *)p)->second, state);
  if (second) {
    return true;
  }
  return false;
}

static void parser_free_or(struct parser *p)
{
  parser_free(((struct parser_or *)p)->first);
  parser_free(((struct parser_or *)p)->second);
}

struct parser *
parser_create_or(struct parser *first, struct parser *second)
{
  struct parser_or *parser = malloc(sizeof(struct parser_or));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_or;
  parser->parser.run = parser_run_or;
  parser->first = first;
  parser->second = second;
  return (struct parser *)parser;
}
