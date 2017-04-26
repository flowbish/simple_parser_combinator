#include <stdbool.h>

#include "state.h"

struct parser {
  bool (*run)(const struct parser*, struct parse_state*);
  void (*free)(struct parser*);
};

typedef bool (*parser_run_fn)(const struct parser*, struct parse_state*, char **o);
typedef void (*parser_free_fn)(struct parser*);

void parser_set_defaults(struct parser *);
