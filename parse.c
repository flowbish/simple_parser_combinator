#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "parser/parser_internal.h"
#include "parse.h"
#include "state.h"
#include "log.h"

/**
 * Publically exposed run function.
 */

bool
run(struct parser *p, const char *input, char **output)
{
  struct parse_state state;
  state_create(&state, input);
  bool success = parser_run(p, &state);
  if (success) {
    state_execute(&state);
    *output = malloc(strlen(state.output));
    strcpy(*output, state.output);
  }
  state_destroy(&state);
  return success;
}
