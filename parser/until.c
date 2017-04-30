#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"
#include "log.h"

/**
 * Until to apply a given parser, rolling back input if a parsing error occurs.
 */

struct parser_until {
  struct parser parser;
  struct parser *target;
};

static bool
parser_run_until(const struct parser *p, struct parse_state *state)
{
  struct parse_state _state;
  while(!state_finished(state)) {
    state_copy(&_state, state);
    bool success = parser_run(((struct parser_until *)p)->target, state);
    state_destroy(state);
    *state = _state;
    if (!success) {
      // Advance one character
      char b;
      state_getc(state, &b);
      state_success(state, b);
    } else {
      return true;
    }
  }
  state_success_blank(state);
  return true;
}

static void
parser_free_until(struct parser *p)
{
  parser_free(((struct parser_until *)p)->target);
}

struct parser *
parser_create_until(struct parser *target)
{
  struct parser_until *parser = malloc(sizeof(struct parser_until));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_until;
  parser->parser.run = parser_run_until;
  parser->target = target;
  return (struct parser *)parser;
}
