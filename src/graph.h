#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include "node.h"

// Graph type.
typedef struct {
  char* title;
  node_t** nodes;
  int** edges;
  int num_nodes;
  int capacity;
  int highest_level;
  int* nodes_at_level;
  int max_nodes_at_level;
} graph_t;


// Creates and returns initialized graph.
graph_t* create_graph();
// Handles freeing memory used by graph.
void free_graph(graph_t* g);
// Creates and adds node to graph with name and text.
node_t* add_node(graph_t* g, const char* name, const char* text);
// Returns node if found in graph, else NULL.
node_t* get_node(graph_t* g, const char* name);
// Prints the layout of the graph.
void print_graph(graph_t* g);
// Adds edge to adj matrix of graph between two nodes defined by name.
// Returns true if edge added, else false.
bool add_edge(graph_t* g, const char* from_name, const char* to_name);
// Frees and then changes graph's title.
void update_graph_title(graph_t* g, const char* title);
// Creates svg drawing of graph.
void draw_graph(graph_t* graph, char* bg_color, char* node_color, int text_size);

#endif
