#pragma once

/* At some point it will be helpful to test the parsers and ensure they all
 * work as expected. */

/**
 * Represents a parser. Can be executed with run_parser.
 */
typedef struct parser *parser;

bool run(parser p, const char *input, char **o);

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
#define try parser_create_try
parser parser_create_try(parser target);

#define or parser_create_or
parser parser_create_or(parser left, parser right);
#define and parser_create_and
parser parser_create_and(parser left, parser right);

#define exe parser_create_execute
parser parser_create_execute(parser target, bool (*handle)(char *, void *), void *extra);
