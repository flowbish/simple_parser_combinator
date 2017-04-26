#pragma once

#include <stdio.h>

/* Printing macros */
#define error(string, ...) fprintf (stderr, "[ERROR] "string "\n", ##__VA_ARGS__)
#define info(string, ...) fprintf (stderr, "[INFO] "string "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define debug(string, ...) fprintf (stderr, "[DEBUG] "string "\n", ##__VA_ARGS__)
#else
#define debug(...)
#endif
