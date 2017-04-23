#include <string.h>

#include "test.h"

bool test_should_run(char *test_name, char **valid_names, size_t num_valid_names) {
  for (size_t i = 0; i < num_valid_names; i += 1) {
    if (strcmp(test_name, valid_names[i]) == 0) {
      return true;
    }
  }
  return false;
}
