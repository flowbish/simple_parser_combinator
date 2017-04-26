#include <stdbool.h>

#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Null parser, will consume no input and always succeed.
 */

struct parser_null {
  struct parser parser;
};

bool parser_run_null(const struct parser *p, struct parse_state *state) {
  (void)p;
  (void)state;
  return false;
}

struct parser *parser_create_null() {
  struct parser_null *parser = malloc(sizeof(struct parser_null));
  parser_set_defaults(&parser->parser);
  parser->parser.run = parser_run_null;
  return (struct parser *)parser;
}
