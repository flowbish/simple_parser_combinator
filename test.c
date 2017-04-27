#include <string.h>

#include "log.h"
#include "test.h"

bool
test_should_run(
    char *test_name,
    char **valid_names,
    size_t num_valid_names)
{
  for (size_t i = 0; i < num_valid_names; i += 1) {
    if (strcmp(test_name, valid_names[i]) == 0) {
      return true;
    }
  }
  return false;
}

int
main(int argc, char **argv)
{
  size_t pass = 0, fail = 0;
  for (size_t i = 0; i < num_tests(); i++) {
    get_test_fn(test, i);
    get_test_name(name, i);

    // Run the test if test name is supplied as an argument (or if there are no
    // arguments)
    if (argc == 1 || test_should_run(*name, &argv[1], argc - 1)) {
      debug("Running test \"%s\"", *name);
      bool success = (*test)();
      if (success) {
        pass += 1;
      } else {
        error("Test \"%s\" failed.", *name);
        fail += 1;
      }
    }
  }

  info("%zu passes, %zu failures", pass, fail);
  return 0;
}
