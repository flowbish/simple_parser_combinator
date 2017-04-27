#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"

/**
 * Default free function for a parser, used if no other free'd data is needed.
 */

static void
parser_free_default(struct parser *p)
{
  free(p);
}

/**
 * Generic interface to executing a parser.
 */
bool
parser_run(const struct parser *p, struct parse_state *state)
{
  if (p->run)
    return (p->run)(p, state);
  return true;
}

void
parser_free(struct parser *p)
{
  if (p->free)
    (p->free)(p);
  parser_free_default(p);
}

void
parser_set_defaults(struct parser *p)
{
  p->free = NULL;
  p->run = NULL;
}
