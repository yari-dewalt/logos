#include "node.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node_t* create_node(const char* name, const char* text) {
  node_t* new_node = malloc(sizeof(node_t));
  new_node->parent = NULL;
  new_node->name = strdup(name);
  new_node->text = strdup(text);
  new_node->level = -1;
  new_node->num_children = 0;
  new_node->x_pos = -1.0;
  new_node->y_pos = -1.0;
  new_node->required_width = 0.0;
  return new_node;
}
