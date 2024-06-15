#include "graph.h"
#include "node.h"
#include "svg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int RECT_WIDTH = 400;
const int RECT_HEIGHT = RECT_WIDTH * 0.6;

// Initialize and return a pointer to a graph struct.
// (Adj matrix representation)
graph_t* create_graph() {
  graph_t* g = malloc(sizeof(graph_t));
  if (g == NULL) {
    return NULL;
  }

  g->num_nodes = 0;
  g->capacity = 4; // Initial capacity
  g->nodes = malloc(sizeof(node_t*) * g->capacity);
  g->edges = malloc(sizeof(int*) * g->capacity);
  g->title = malloc(strlen("") + 1); // Initial empty title.
  if (g->title == NULL) {
    fprintf(stderr, "Memory allocation failed for initial title.\n");
    free(g->nodes);
    free(g->edges);
    free(g);
    return NULL;
  }
  strcpy(g->title, "");

  g->highest_level = 0;
  g->nodes_at_level = NULL;
  g->max_nodes_at_level = 0;

  for (int i = 0; i < g->capacity; i++) {
    g->edges[i] = calloc(g->capacity, sizeof(int));
  }

  return g;
}

void free_graph(graph_t* g) {
  // If no nodes or edges can just free the graph and title
  if (g->nodes == NULL || g->edges == NULL) {
    free(g->title);
    free(g);
    return;
  }

  for (int i = 0; i < g->num_nodes; i++) {
    free(g->nodes[i]->name);
    free(g->nodes[i]);
    free(g->edges[i]);
  }

  free(g->title);
  free(g->nodes);
  free(g->edges);
  if (g->nodes_at_level != NULL) {
    free(g->nodes_at_level);
  }
  free(g);
}

static void resize_graph(graph_t* g) {
  int new_capacity = g->capacity * 2; // Increase capacity by 2.
  g->nodes = realloc(g->nodes, sizeof(node_t*) * new_capacity);
  g->edges = realloc(g->edges, sizeof(int*) * new_capacity);

  for (int i = g->capacity; i < new_capacity; i++) {
    g->edges[i] = calloc(new_capacity, sizeof(int));
  }

  for (int i = 0; i < g->capacity; i++) {
    g->edges[i] = realloc(g->edges[i], sizeof(int) * new_capacity);
    memset(g->edges[i] + g->capacity, 0, sizeof(int) * (new_capacity - g->capacity));
  }

  g->capacity = new_capacity;
}

node_t* add_node(graph_t* g, const char* name, const char* text) {
  // Resize if needed.
  if (g->num_nodes >= g->capacity) {
    resize_graph(g);
  }

  // Create node.
  node_t* node = create_node(name, text);
  node->id = g->num_nodes;

  // Add node to respective index;
  g->nodes[g->num_nodes] = node;
  g->num_nodes++;

  return node;
}

// Return node in graph that has the specified name.
// (Parser prohibits same nodes with same name)
node_t* get_node(graph_t* g, const char* name) {
  for (int i = 0; i < g->num_nodes; i++) {
    if (strcmp(g->nodes[i]->name, name) == 0) {
      return g->nodes[i];
    }
  }
  return NULL;
}

// Add edge to edges matrix, using the from node's name and the to node's name.
bool add_edge(graph_t* g, const char* from_name, const char* to_name) {
  node_t* from_node = get_node(g, from_name);
  node_t* to_node = get_node(g, to_name);

  // Early return if nodes aren't in graph.
  if (from_node == NULL || to_node == NULL) {
    return false;
  }

  // If edge already exists.
  if (g->edges[from_node->id][to_node->id] == 1) {
    return true;
  }

  g->edges[from_node->id][to_node->id] = 1;

  // Setup node levels...
  
  // This is a check and should only run for the root node of the graph
  if (from_node->level == -1) {
    from_node->level = 1;
  }
  // If to_node's level hasn't been initialized it means that it is a direct child.
  // (i.e. initialize with parent's level + 1)
  if (to_node->level == -1) {
    to_node->level = from_node->level + 1;
    if (to_node->level > g->highest_level) {
      g->highest_level = to_node->level; // Update graph's highest_level.
    }
  }

  // Direct child check.
  if (to_node->level - from_node->level == 1) {
    from_node->num_children++;
    to_node->parent = from_node;
  }

  return true;
}

// Prints the layout of the overall graph.
// (Title, levels, nodes, and their edges)
void print_graph(graph_t* g) {
  if (g->title)
    printf("---%s---(%d levels)\n", g->title, g->highest_level);
  for (int from = 0; from < g->num_nodes; from++) {
    for (int to = 0; to < g->num_nodes; to++) {
      if (g->edges[from][to]) {
        printf("%s(%s) - level %d - %d children -> %s(%s) - level %d - %d children\n",
               g->nodes[from]->name, g->nodes[from]->text, g->nodes[from]->level, g->nodes[from]->num_children,
               g->nodes[to]->name, g->nodes[to]->text, g->nodes[to]->level, g->nodes[to]->num_children);
      }
    }
  }
}

// Update's graph's title.
void update_graph_title(graph_t* g, const char* title) {
  if (g->title != NULL) {
    free(g->title);
  }

  g->title = malloc(strlen(title) + 1);
  if (g->title == NULL) {
    fprintf(stderr, "Memory allocation failed for graph title.\n");
    return;
  }

  strcpy(g->title, title);
}

// Helper function to calculate each node's required width (x-space) for drawing the graph.
static void calculate_required_widths(graph_t* g) {
  const int PADDING = RECT_WIDTH * 0.10;

  // Initialize required widths
  for (int i = 0; i < g->num_nodes; i++) {
    g->nodes[i]->required_width = RECT_WIDTH + PADDING;
  }

  // Traverse from the bottom level to the top
  for (int level = g->highest_level; level >= 0; level--) {
    for (int i = 0; i < g->num_nodes; i++) {
      if (g->nodes[i]->level == level) {
        node_t* parent_node = g->nodes[i];
        double total_child_width = 0;

        for (int j = 0; j < g->num_nodes; j++) {
          if (g->edges[parent_node->id][j]) {
            if (g->nodes[j]->level - parent_node->level == 1) // Only add width of its direct children, not all edges.
              total_child_width += g->nodes[j]->required_width;
          }
        }

        if (total_child_width > parent_node->required_width) {
          parent_node->required_width = total_child_width;
        }
      }
    }
  }
}

// Function to draw the entirety of the graph.
void draw_graph(graph_t* g, char* bg_color, char* node_color, int text_size) {
  // Get important width requirements.
  calculate_required_widths(g);

  // Graph constants. (subject to change)
  const int GRAPH_PADDING = 400;
  const int WIDTH = g->nodes[0]->required_width + GRAPH_PADDING;
  const int HEIGHT = RECT_HEIGHT * g->num_nodes + GRAPH_PADDING;

  // Initialize svg.
  svg_t* svg = svg_create(WIDTH, HEIGHT);
  // Fill background.
  svg_fill(svg, bg_color);

  // Calculate and store positions of all nodes.
  for (int level = g->highest_level; level >= 0; level--) {
    double used_up_width = 0.0; // Used for x-offset if there were previous nodes on level.
    for (int i = 0; i < g->num_nodes; i++) {
      if (g->nodes[i]->level == level) {
        node_t* current_node = g->nodes[i];
        // Offset from previous used_up_width and center according to: its required width, the graph's width, and the root node's required width.
        current_node->x_pos = (used_up_width + current_node->required_width / 2) + WIDTH / 2 - g->nodes[0]->required_width / 2;
        // Place depending on its level in the graph and the graph's height.
        current_node->y_pos = ((double)level / (double)(g->highest_level + 1)) * HEIGHT;
        used_up_width += current_node->required_width; // The node's width is now used up.
      }
    }
  }

  // Draw title.
  svg_text(svg, WIDTH / 2, g->num_nodes > 0 ? g->nodes[0]->y_pos - RECT_HEIGHT / 1.2 : HEIGHT / 10, "sans-serif", text_size * 1.5, "black", "black", g->title);

  // Adjust the positions of child nodes based on their parent's position
  for (int i = 0; i < g->num_nodes; i++) {
    node_t* current_node = g->nodes[i];
    node_t* parent_node = current_node->parent;
    if (parent_node == NULL) {
      continue; // Skip if the current node does not have a parent
    }

    // Calculate the total number of children for the parent
    double total_children_width = 0.0;
    for (int j = 0; j < g->num_nodes; j++) {
      if (g->nodes[j]->parent == parent_node) {
        total_children_width += g->nodes[j]->required_width;
      }
    }

    // Calculate the starting position (leftmost position) for the children
    double start_x = parent_node->x_pos - (total_children_width / 2);

    // Position each child based on its required width
    double current_x = start_x;
    for (int j = 0; j < g->num_nodes; j++) {
      if (g->nodes[j]->parent == parent_node) {
        g->nodes[j]->x_pos = current_x + g->nodes[j]->required_width / 2;
        current_x += g->nodes[j]->required_width;
      }
    }
  }

  // Draw all edges first.
  for (int from = 0; from < g->num_nodes; from++) {
    for (int to = 0; to < g->num_nodes; to++) {
      if (g->edges[from][to]) {
        // If it was just a line it would be a simple Point A (from_node's pos) to Point B (to_node's pos)
        // but arrow's make it more complicated...
        node_t* from_node = g->nodes[from];
        node_t* to_node = g->nodes[to];

        // Find direction of edge so we know where to stop on the node so we don't go inside and can see the arrowhead.
        enum DIRECTION { DOWN, UP, LEFT, RIGHT };
        enum DIRECTION direction;
        if (to_node->y_pos == from_node->y_pos && to_node->x_pos < from_node->x_pos)
          direction = LEFT;
        else if (to_node->y_pos == from_node->y_pos && to_node->x_pos > from_node->x_pos)
          direction = RIGHT;
        if (to_node->y_pos > from_node->y_pos)
          direction = DOWN;
        else if (to_node->y_pos < from_node->y_pos)
          direction = UP;

        switch (direction) {
          case DOWN: {
            // Stop at the top side of the node.
            svg_arrow(svg, "black", 8, RECT_WIDTH / 10, from_node->x_pos, from_node->y_pos,
                     to_node->x_pos, to_node->y_pos - RECT_HEIGHT / 2);
            break;
          }
          case UP: {
            // Stop at the bottom side of the node.
            svg_arrow(svg, "black", 8, RECT_WIDTH / 10, from_node->x_pos, from_node->y_pos,
                     to_node->x_pos, to_node->y_pos + RECT_HEIGHT / 2);
            break;
          }
          case LEFT: {
            // Stop at the right side of the node.
            svg_arrow(svg, "black", 8, RECT_WIDTH / 10, from_node->x_pos, from_node->y_pos,
                     to_node->x_pos + RECT_WIDTH / 2, to_node->y_pos);
            break;
          }
          case RIGHT: {
            // Stop at the left side of the node.
            svg_arrow(svg, "black", 8, RECT_WIDTH / 10, from_node->x_pos, from_node->y_pos,
                     to_node->x_pos - RECT_WIDTH / 2, to_node->y_pos);
            break;
          }
        }
      }
    }
  }

  // Draw all nodes on top of edges.
  for (int i = 0; i < g->num_nodes; i++) {
    node_t* curr_node = g->nodes[i];
    double x = curr_node->x_pos;
    double y = curr_node->y_pos;

    // Draw rectangles centered on the nodes' positions.
    svg_rectangle(svg, RECT_WIDTH, RECT_HEIGHT, x - (RECT_WIDTH / 2), y - (RECT_HEIGHT / 2), node_color, "black", 6, 8, 8);
    // Draw the nodes' text on top.
    svg_text(svg, x, y, "sans-serif", text_size, "black", "black", curr_node->text);
  }

  // Get filename from graph's title.
  size_t filename_length = strlen(g->title) + 5;
  char* svg_filename = malloc(filename_length * sizeof(char));
  if (!svg_filename) {
    fprintf(stderr, "Memory allocation failed for svg_filename\n");
  }
  snprintf(svg_filename, filename_length, "%s.svg", g->title);

  // Finally, save and free svg.
  svg_save(svg, svg_filename);
  free(svg_filename);
}
