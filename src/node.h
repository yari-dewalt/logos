#ifndef NODE_H
#define NODE_H

// Graph node representation
typedef struct node {
  struct node* parent;
  char* name;
  char* text;
  int id;
  int level;
  int num_children;
  double x_pos;
  double y_pos;
  double required_width;
} node_t;

// Creates and initializes node with name and text.
node_t* create_node(const char* name, const char* text);

#endif
