#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "table.h"
#include "graph.h"

// Interpret result representation.
typedef struct {
  bool had_error;
  graph_t* graph;
} interpret_result_t;

// Parser struct
typedef struct {
  lexer_t* lexer;
  token curr;
  token next;
  table_t* variables;
  bool had_error;
  bool panic_mode;
} parser_t;

// Initializes global parser. (doesn't return pointer)
void init_parser(const char* source);
// Scans and goes to next token.
void next_token();
// Checks current token type.
bool check_token(token_type type);
// Checks next token type.
bool check_peek(token_type type);
// Consumes current token if type matches, else returns.
void match(token_type type);
// Loop to parse all tokens.
void parse();
// Calls parse and returns the interpret result.
interpret_result_t interpret();

#endif
