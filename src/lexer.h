#ifndef LEXER_H
#define LEXER_H

// Token types
typedef enum {
  // Single-character tokens.
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_EQUAL, TOKEN_NEWLINE, // TOKEN_COMMA
  // Multi-character tokens.
  TOKEN_ARROW, TOKEN_DOUBLE_ARROW,
  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING,
  // Misc.
  TOKEN_ERROR, TOKEN_EOF
} token_type;

// Token struct
typedef struct {
  token_type type;
  const char* start;
  int length;
  int line;
} token;

// Lexer struct
typedef struct {
  const char* start;
  const char* current;
  int line;
} lexer_t;

// Creates and returns initialized lexer.
lexer_t* init_lexer(const char* source);
// Scans and returns token from source text.
token scan_token();

#endif
