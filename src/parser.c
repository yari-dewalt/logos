#include "parser.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Globals...but this way don't have to pass around parser and interpret_result everywhere.
interpret_result_t interpret_result;
parser_t parser;

// Reports an error at the token.
static void error_at(token token, const char* message) {
  // If we are already in panic mode we don't report anymore errors until we are out of it.
  if (parser.panic_mode) return;
  // Set panic mode to on.
  parser.panic_mode = true;
  fprintf(stderr, "[line %d] Error", token.line);

  if (token.type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token.type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token.length, token.start);
  }

  fprintf(stderr, ": %s\n", message);
  // Error flags so we know we had error when parsing / interpreting.
  parser.had_error = true;
  interpret_result.had_error = true;
}

// Easy helper to report error at current.
static void error(const char* message) {
  error_at(parser.curr, message);
}

// Scans and goes to next token. Updates curr and next.
void next_token() {
  parser.curr = parser.next;

  for (;;) {
    parser.next = scan_token(parser.lexer);
    if (parser.next.type != TOKEN_ERROR) break;

    error_at(parser.next, parser.next.start);
  }
}

static void init_interpret_result() {
  interpret_result.graph = create_graph();
  interpret_result.had_error = false;
}

void init_parser(const char* source) {
  init_interpret_result();
  parser.lexer = init_lexer(source);
  parser.variables = create_table();
  parser.had_error = false;
  parser.panic_mode = false;
  next_token();
  next_token();
}

// Returns if current token type matches specified type.
bool check_token(token_type type) {
  return type == parser.curr.type;
}

// Returns if next token type matches specified type.
bool check_peek(token_type type) {
  return type == parser.next.type;
}

// If current token type matches type move on, else return.
void match(token_type type) {
  if (!check_token(type)) return;
  next_token();
}

// Skips newline tokens until it finds not a newline token.
static void new_line() {
  match(TOKEN_NEWLINE);
  while (check_token(TOKEN_NEWLINE)) {
    next_token();
  }
}

// Checks if variable is declared.
static bool is_declared(const char* key) {
  return table_get(parser.variables, key) != NULL;
}

// Declares variable and puts its name and value into variable table.
static void declare_variable(const char* name, char* value) {
  table_set(parser.variables, name, value);
  node_t* node = get_node(interpret_result.graph, name);
  if (node != NULL) {
    // Update the node's text if the node has already been defined.
    node->text = value;
  }
}

// Adds edge to graph between two nodes.
static void add_edge_to_graph(const char* from, const char* to) {
  if (!add_edge(interpret_result.graph, from, to)) {
    error("Invalid nodes for edge creation.");
  }
}

static void assignment(char* prev_name, bool add_edge, bool add_two_edges);

// Arrow statement parsing.
static void arrow(char* prev_name) {
  // Get name.
  char* name = strndup(parser.curr.start, parser.curr.length);
  if (!is_declared(name))
    error("Undefined variable.");

  // prev_name from inline or chaining.
  if (prev_name)
    name = prev_name;
  if (!prev_name) next_token();
  if (check_token(TOKEN_ARROW)) {
    // Skip past arrow.
    next_token();
    // Get target's name
    char* target_name = strndup(parser.curr.start, parser.curr.length);
    if (check_token(TOKEN_IDENTIFIER)) {
      if (check_peek(TOKEN_EQUAL)) {
        // Chained assignment.
        assignment(name, true, false);
      } else if (check_peek(TOKEN_ARROW)) {
        // Chained arrow.
        // Subject to change but works for now.
        if (!get_node(interpret_result.graph, name)) {
          char* variable_value = table_get(parser.variables, name);
          add_node(interpret_result.graph, name, variable_value);
        }
        if (is_declared(target_name)) {
          char* variable_value = table_get(parser.variables, target_name);
          if (!get_node(interpret_result.graph, target_name))
            add_node(interpret_result.graph, target_name, variable_value);
          // Add edge.
          add_edge_to_graph(name, target_name);
        }
        arrow(NULL);
      } else if (check_peek(TOKEN_DOUBLE_ARROW)) {
        // Chained double arrow.
        // Subject to change but works for now.
        if (!get_node(interpret_result.graph, name)) {
          char* variable_value = table_get(parser.variables, name);
          add_node(interpret_result.graph, name, variable_value);
        }
        if (is_declared(target_name)) {
          char* variable_value = table_get(parser.variables, target_name);
          if (!get_node(interpret_result.graph, target_name))
            add_node(interpret_result.graph, target_name, variable_value);
          // Add double edge.
          add_edge_to_graph(name, target_name);
          add_edge_to_graph(target_name, name);
        }
        arrow(NULL);
      }
      // Create node if needed.
      if (!get_node(interpret_result.graph, name)) {
        char* variable_value = table_get(parser.variables, name);
        add_node(interpret_result.graph, name, variable_value);
      }
      if (is_declared(target_name)) {
        // If declared, add edge.
        char* variable_value = table_get(parser.variables, target_name);
        // Add node if the node doesn't exist already.
        if (!get_node(interpret_result.graph, target_name))
          add_node(interpret_result.graph, target_name, variable_value);
        add_edge_to_graph(name, target_name);
      } else {
        error("Undefined variable.");
      }
    }
    else {
      error("Expected identifier.");
    }
  } else if (check_token(TOKEN_DOUBLE_ARROW)) {
    // Same logic as regular arrow, but will add double edge.
    next_token();
    char* target_name = strndup(parser.curr.start, parser.curr.length);
    if (check_token(TOKEN_IDENTIFIER)) {
      if (check_peek(TOKEN_EQUAL)) {
        assignment(name, false, true);
      } else if (check_peek(TOKEN_ARROW)) {
        // This works for now but keep eye on it. ----
        if (!get_node(interpret_result.graph, name)) {
          char* variable_value = table_get(parser.variables, name);
          add_node(interpret_result.graph, name, variable_value);
        }
        if (is_declared(target_name)) {
          char* variable_value = table_get(parser.variables, target_name);
          if (!get_node(interpret_result.graph, target_name))
            add_node(interpret_result.graph, target_name, variable_value);
          add_edge_to_graph(name, target_name);
        }
        // ------
        arrow(NULL);
      } else if (check_peek(TOKEN_DOUBLE_ARROW)) {
        if (!get_node(interpret_result.graph, name)) {
          char* variable_value = table_get(parser.variables, name);
          add_node(interpret_result.graph, name, variable_value);
        }
        if (is_declared(target_name)) {
          char* variable_value = table_get(parser.variables, target_name);
          if (!get_node(interpret_result.graph, target_name))
            add_node(interpret_result.graph, target_name, variable_value);
          add_edge_to_graph(name, target_name);
          add_edge_to_graph(target_name, name);
        }
        arrow(NULL);
      }
      if (!get_node(interpret_result.graph, name)) {
        char* variable_value = table_get(parser.variables, name);
        add_node(interpret_result.graph, name, variable_value);
      }
      if (is_declared(target_name)) {
        char* variable_value = table_get(parser.variables, target_name);
        if (!get_node(interpret_result.graph, target_name))
          add_node(interpret_result.graph, target_name, variable_value);
        add_edge_to_graph(name, target_name);
        add_edge_to_graph(target_name, name);
      } else {
        error("Undefined variable.");
      }
    }
    else {
      error("Expected identifier.");
    }
  }
}

// Assignment parsing.
static void assignment(char* prev_name, bool add_edge, bool add_two_edges) {
  // Get name.
  char* name = strndup(parser.curr.start, parser.curr.length);
  // Inline but not inline with arrows.
  if (prev_name && !add_edge && !add_two_edges)
    name = prev_name;
  if (!prev_name || add_edge || add_two_edges) next_token();
  if (check_token(TOKEN_EQUAL)) {
    // Move past equal sign.
    next_token();
    // Get value.
    char* value = strndup(parser.curr.start, parser.curr.length);

    if (check_token(TOKEN_STRING)) {
      // Assign to string.
      declare_variable(name, value);
      // Inline with arrow.
      if (add_edge) {
        if (!get_node(interpret_result.graph, prev_name))
          add_node(interpret_result.graph, prev_name, value);
        if (!get_node(interpret_result.graph, name))
          add_node(interpret_result.graph, name, value);
        add_edge_to_graph(prev_name, name);
      // Inline with double arrow.
      } else if (add_two_edges) {
        if (!get_node(interpret_result.graph, prev_name))
          add_node(interpret_result.graph, prev_name, value);
        if (!get_node(interpret_result.graph, name))
          add_node(interpret_result.graph, name, value);
        add_edge_to_graph(prev_name, name);
        add_edge_to_graph(name, prev_name);
      }
      if (check_peek(TOKEN_EQUAL)) {
        // String is followed by equal sign.
        error("Cannot assign to literal.");
        next_token();
        next_token();
      } else if (check_peek(TOKEN_ARROW) || check_peek(TOKEN_DOUBLE_ARROW)) {
        // Inline with arrows.
        next_token();
        arrow(name);
      } else {
        next_token();
      }
    } else if (check_token(TOKEN_IDENTIFIER)) {
      // Inline with arrow.
      if (add_edge || add_two_edges) {
        error("Cannot assign to identifier while adding edge.");
      }
      if (check_peek(TOKEN_EQUAL)) {
        // Chained assignment.
        assignment(NULL, false, false);
      } else if (check_peek(TOKEN_ARROW) || check_peek(TOKEN_DOUBLE_ARROW)) {
        arrow(NULL);
      }
      // Get value of identifier and assign.
      char* variable_value = table_get(parser.variables, value);
      if (variable_value != NULL) declare_variable(name, variable_value);
      else {
        error("Undefined variable.");
      }
      next_token();
    } else {
      error("Expected string or identifier.");
    }
  }
}

// Synchronize from panic mode so we don't report chain of errors.
static void synchronize() {
  parser.panic_mode = false;
  // Move to next line.
  while (parser.next.type != TOKEN_EOF) {
    if (parser.curr.type == TOKEN_NEWLINE) return;
    next_token();
  }
}

// Statement parsing, either title, assignment, or arrow.
static void statement() {
  if (check_token(TOKEN_LEFT_BRACE)) {
    next_token();
    if (check_token(TOKEN_STRING)) {
      char* title = strndup(parser.curr.start, parser.curr.length);
      update_graph_title(interpret_result.graph, title);
      next_token();
      if (check_token(TOKEN_RIGHT_BRACE)) {
        next_token();
      }
    } else {
      error("Expected string.");
    }
  }

  else if (check_token(TOKEN_IDENTIFIER)) {
    // Identifier gets either assigned or points to another identifier.
    if (check_peek(TOKEN_EQUAL)) {
      assignment(NULL, false, false);
      next_token();
    }
    else if (check_peek(TOKEN_ARROW) || check_peek(TOKEN_DOUBLE_ARROW)) {
      arrow(NULL);
      next_token();
    }
    else {
      error("Expected either assignment or arrow to node.");
    }
  }

  new_line();
}

// Loops through and parses all tokens.
void parse() {
  while (check_token(TOKEN_NEWLINE)) {
    next_token();
  }

  while (!check_token(TOKEN_EOF)) {
    statement();
    if (parser.panic_mode) synchronize();
  }
}

// Returns the overall interpret result.
interpret_result_t interpret() {
  parse();
  return interpret_result;
}
