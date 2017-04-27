#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Executor.
 */

struct parser_execute {
  struct parser parser;
  struct parser *target;
  bool (*handle)(char *, void *);
  void *extra;
};

static bool
parser_run_execute(const struct parser *p, struct parse_state *state)
{
  struct parser_execute *exe = (struct parser_execute *)p;
  char *o_temp = state->output;
  state->output = NULL;
  bool parse_success = parser_run(exe->target, state);
  char *o_temp2 = state->output;
  state->output = o_temp;
  if (parse_success) {
    state_add_handler(state, exe->handle, o_temp2, exe->extra);
    state_success_blank(state);
    state_output_append_str(state, o_temp2);
  }
  free(o_temp2);
  return parse_success;
}

static void parser_free_execute(struct parser *p)
{
  parser_free(((struct parser_execute *)p)->target);
}

struct parser *
parser_create_execute(
    struct parser *target,
    bool (*handle)(char *, void *),
    void *extra)
{
  struct parser_execute *parser = malloc(sizeof(struct parser_execute));
  parser_set_defaults(&parser->parser);
  parser->parser.free = parser_free_execute;
  parser->parser.run = parser_run_execute;
  parser->target = target;
  parser->handle = handle;
  parser->extra = extra;
  return (struct parser *)parser;
}
