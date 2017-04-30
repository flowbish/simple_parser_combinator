#pragma once

/* At some point it will be helpful to test the parsers and ensure they all
 * work as expected. */

/**
 * Represents a parser. Can be executed with run_parser.
 */
typedef struct parser *parser;

bool run(parser p, const char *input, char **o);
void parser_free(parser p);

#define blank parser_create_blank()
parser parser_create_blank();
#define null parser_create_null()
parser parser_create_null();
#define eof parser_create_eof()
parser parser_create_eof();

#define ch parser_create_char
parser parser_create_char(char c);
#define str parser_create_str
parser parser_create_str(char *str);
#define many parser_create_many
parser parser_create_many(parser target);
#define optional parser_create_optional
parser parser_create_optional(parser target);
#define try parser_create_try
parser parser_create_try(parser target);
#define until parser_create_until
parser parser_create_until(parser target);

#define or5(p, ...) or(p, or4(__VA_ARGS__))
#define or4(p, ...) or(p, or3(__VA_ARGS__))
#define or3(p, ...) or(p, or2(__VA_ARGS__))
#define or2(p, q) or(p, q)
#define or parser_create_or
parser parser_create_or(parser left, parser right);

#define and8(p, ...) and(p, and7(__VA_ARGS__))
#define and7(p, ...) and(p, and6(__VA_ARGS__))
#define and6(p, ...) and(p, and5(__VA_ARGS__))
#define and5(p, ...) and(p, and4(__VA_ARGS__))
#define and4(p, ...) and(p, and3(__VA_ARGS__))
#define and3(p, ...) and(p, and2(__VA_ARGS__))
#define and2(p, q) and(p, q)
#define and parser_create_and
parser parser_create_and(parser left, parser right);

#define exe parser_create_execute
parser parser_create_execute(parser target, bool (*handle)(char *, void *), void *extra);
