#pragma once

/* At some point it will be helpful to test the parsers and ensure they all
 * work as expected. */

/**
 * Represents a parser. Can be executed with run_parser.
 */
typedef struct parser *parser;

bool run(parser p, const char *input, char **o);

parser parser_create_blank();
parser parser_create_null();

parser parser_create_char(char c);
parser parser_create_str(char *str);
parser parser_create_or(parser left, parser right);
parser parser_create_and(parser left, parser right);

