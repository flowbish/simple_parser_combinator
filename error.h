#pragma once

#include <string.h>
#include <stdlib.h>

struct error {
  char *message;
};

#define error_try(fn)                           \
  do {                                          \
    struct error *e = fn;                       \
    if (e != NULL)                              \
      return e;                                 \
  } while (0)

#define error_to(var, msg, ...)                 \
  if (var == NULL)                              \
    var = malloc(sizeof(struct error));         \
  asprintf(&var->message,                       \
           "%s:%d: " msg,                       \
           __FILE__,                            \
           __LINE__,                            \
           ##__VA_ARGS__)                       \

#define raise(msg, ...)                         \
  do {                                          \
    struct error *e = NULL;                     \
    error_to(e, msg, ##__VA_ARGS__);            \
    return e;                                   \
  } while(0)
