#pragma once

#define CONCAT(a, ...) _CONCAT(a, __VA_ARGS__)
#define _CONCAT(a, ...) a ## __VA_ARGS__

#define _NARGS_8(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define COUNT_VARARGS(...) _NARGS_8(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

#define EXPAND(...) __VA_ARGS__
