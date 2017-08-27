#pragma once

#include "macros.h"

/* At some point it will be helpful to test the parsers and ensure they all
 * work as expected. */

/**
 * Represents a parser. Can be executed with run_parser.
 */
struct parser;

bool run(struct parser *p, const char *input, char **o);
void parser_free(struct parser *p);

#define blank parser_create_blank()
struct parser *
parser_create_blank();

#define null parser_create_null()
struct parser *
parser_create_null();

#define eof parser_create_eof()
struct parser *
parser_create_eof();

#define ch parser_create_char
struct parser *
parser_create_char(char c);

#define str parser_create_str
struct parser *
parser_create_str(char *str);

#define many parser_create_many
struct parser *
parser_create_many(struct parser *target);

#define optional parser_create_optional
struct parser *
parser_create_optional(struct parser *target);

#define try parser_create_try
struct parser *
parser_create_try(struct parser *target);

#define until parser_create_until
struct parser *
parser_create_until(struct parser *target);

#define or8(p, ...) parser_create_or(p, or7(__VA_ARGS__))
#define or7(p, ...) parser_create_or(p, or6(__VA_ARGS__))
#define or6(p, ...) parser_create_or(p, or5(__VA_ARGS__))
#define or5(p, ...) parser_create_or(p, or4(__VA_ARGS__))
#define or4(p, ...) parser_create_or(p, or3(__VA_ARGS__))
#define or3(p, ...) parser_create_or(p, or2(__VA_ARGS__))
#define or2(p, q) parser_create_or(p, q)
#define or(...) \
    _NARGS_8(__VA_ARGS__, or8, or7, or6, or5, or4, or3, or2)(__VA_ARGS__)
struct parser *
parser_create_or(struct parser *left, struct parser *right);

#define and8(p, ...) parser_create_and(p, and7(__VA_ARGS__))
#define and7(p, ...) parser_create_and(p, and6(__VA_ARGS__))
#define and6(p, ...) parser_create_and(p, and5(__VA_ARGS__))
#define and5(p, ...) parser_create_and(p, and4(__VA_ARGS__))
#define and4(p, ...) parser_create_and(p, and3(__VA_ARGS__))
#define and3(p, ...) parser_create_and(p, and2(__VA_ARGS__))
#define and2(p, q) parser_create_and(p, q)
#define and(...) \
    _NARGS_8(__VA_ARGS__, and8, and7, and6, and5, and4, and3, and2)(__VA_ARGS__)
struct parser *
parser_create_and(struct parser *left, struct parser *right);

#define exe parser_create_execute
struct parser *
parser_create_execute(
    struct parser *target,
    bool (*handle)(char *, void *),
    void *extra);
