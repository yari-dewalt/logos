#include <string.h>
#include <stdbool.h>
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

// Initialize lexer
lexer_t* init_lexer(const char* source) {
  lexer_t* lexer = malloc(sizeof(lexer_t*));
  if (lexer == NULL) {
    return NULL;
  }
  lexer->start = source;
  lexer->current = source;
  lexer->line = 1;
  return lexer;
}

// Helper to check if char is alphanumeric.
static bool is_alphanum(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
          c == '_';
}

// Helper to check if at end of source.
static bool is_at_end(lexer_t* lexer) {
  return *lexer->current == '\0';
}

// Advances to next char in source.
static char advance(lexer_t* lexer) {
  lexer->current++;
  return lexer->current[-1];
}

// Peek but don't move on to next char.
static char peek(lexer_t* lexer) {
  return *lexer->current;
}

// Peek at upcoming next char.
static char peek_next(lexer_t* lexer) {
  if (is_at_end(lexer)) return '\0';
  return lexer->current[1];
}

// Check if lexer's current matches the expected char, if so, advance.
static bool match(lexer_t* lexer, char expected) {
  if (is_at_end(lexer)) return false;
  if (*lexer->current != expected) return false;
  lexer->current++;
  return true;
}

// Creates and returns token with specified type.
static token make_token(lexer_t* lexer, token_type type) {
  token token;
  token.type = type;
  if (type == TOKEN_STRING) {
    token.start = lexer->start + 1;
    token.length = (int)(lexer->current - lexer->start - 2);
  } else {
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
  }
  token.line = lexer->line;
  return token;
}

// Creates special error token with error message.
static token error_token(lexer_t* lexer, const char* message) {
  token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = lexer->line;
  return token;
}

// Skip all whitespace except for newlines.
static void skip_whitespace(lexer_t* lexer) {
  for (;;) {
    char c = peek(lexer);
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance(lexer);
        break;
      case '/':
        if (peek_next(lexer) == '/') {
          // A comment goes until the end of the line.
          while (peek(lexer) != '\n' && !is_at_end(lexer)) advance(lexer);
        } else if (peek_next(lexer) == '*') {
          // Advance past the "/*"
          advance(lexer);
          advance(lexer);
          // A comment goes until we find the closing "*/"
          while ((peek(lexer) != '*' || peek_next(lexer) != '/') && !is_at_end(lexer)) {
            advance(lexer);
          }
          // Advance past the closing "*/"
          if (!is_at_end(lexer)) {
            advance(lexer);
            advance(lexer);
          }
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

// Consumes alphanumeric characters to create identifier.
static token identifier(lexer_t* lexer) {
  while (is_alphanum(peek(lexer))) advance(lexer);
  return make_token(lexer, TOKEN_IDENTIFIER);
}

// Consumes up to quotes to create string.
static token string(lexer_t* lexer, const char quote) {
  while (peek(lexer) != quote && !is_at_end(lexer)) {
    if (peek(lexer) == '\n') return error_token(lexer, "Unterminated string.");
    advance(lexer);
  }

  if (is_at_end(lexer)) return error_token(lexer, "Unterminated string.");

  // Advance past the closing quote.
  advance(lexer);
  return make_token(lexer, TOKEN_STRING);
}

// Scans for and creates the next token from the source. 
token scan_token(lexer_t* lexer) {
  // Before anything, skip whitespace.
  skip_whitespace(lexer);
  // Update start.
  lexer->start = lexer->current;

  if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

  // Advance to next char.
  char c = advance(lexer);
  if (is_alphanum(c)) return identifier(lexer);

  switch (c) {
    case '{': return make_token(lexer, TOKEN_LEFT_BRACE);
    case '}': return make_token(lexer, TOKEN_RIGHT_BRACE);
    case '=': return make_token(lexer, TOKEN_EQUAL);
    case '<': {
      if (match(lexer, '-') && match(lexer, '>')) return make_token(lexer, TOKEN_DOUBLE_ARROW);
      else return error_token(lexer, "Unexpected character.");
    }
    case '-': {
      if (match(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
      else return error_token(lexer, "Unexpected character.");
    }
    case '"': return string(lexer, '"');
    case '\'': return string(lexer, '\'');
    case '\n':
      lexer->line++;
      return make_token(lexer, TOKEN_NEWLINE);
  }

  return error_token(lexer, "Unexpected character.");
}
