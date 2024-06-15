#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "graph.h"
#include "lexer.h"
#include "parser.h"
#include "svg.h"

#define VERSION "1.0.0"
#define DEBUG_MODE false

// Reads file from path and returns source text.
static char* read_file(const char* path) {
  FILE* file = fopen(path, "rb");
  // Couldn't open file, most likely due to improper path.
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  // Something went wrong when reading the file.
  if (bytes_read < file_size) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

// Read file, interpret, and draw graph if successful.
static void run_file(const char* path, char* bg_color, char* node_color, int text_size) {
  char* source = read_file(path);

  init_parser(source);
  interpret_result_t result = interpret();

  if (!result.had_error) {
  #if DEBUG_MODE
    print_graph(result.graph);
  #endif
    draw_graph(result.graph, bg_color, node_color, text_size);
  }

  free_graph(result.graph);
  free(source);
}

// Prints help info.
void print_help() {
  printf("Usage: logos <path> [...options]\n");
  printf("Options:\n");
  printf("  -bgc, --background-color <color>  Set the background color (default: white)\n");
  printf("  -nc, --node-color <color>         Set the node color (default: white)\n");
  printf("  -ts, --text-size <size>           Set the text size (default: 16)\n");
  printf("  --version                         Show the version information\n");
  printf("  --help                            Show this help message\n");
  printf("\nFor more help please visit: https://github.com/yari-dewalt/logos\n");
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: logos <path> [...options]\n");
    exit(64);
  }

  char* path = NULL;
  char* bg_color = "white";
  char* node_color = "white";
  int text_size = 16;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--version") == 0) {
      printf("logos version %s\n", VERSION); // Print version and don't run.
      exit(0);
    } else if (strcmp(argv[i], "--help") == 0) {
      print_help(); // Print help and don't run.
      exit(0);
    } else if (i == 1) {
      path = argv[i]; // Path is always first argument.
    // Option parsing.
    } else if ((strcmp(argv[i], "-bgc") == 0 || strcmp(argv[i], "--background-color") == 0) && i + 1 < argc) {
      bg_color = argv[++i];
    } else if ((strcmp(argv[i], "-nc") == 0 || strcmp(argv[i], "--node-color") == 0) && i + 1 < argc) {
      node_color = argv[++i];
    } else if ((strcmp(argv[i], "-ts") == 0 || strcmp(argv[i], "--text-size") == 0) && i + 1 < argc) {
      text_size = atoi(argv[++i]);
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      printf("Usage: logos <path> [...options]\n");
      exit(64);
    }
  }

  if (path == NULL) {
    fprintf(stderr, "Error: <path> is required.\n");
    printf("Usage: logos <path> [...options]\n");
    exit(64);
  }

  run_file(path, bg_color, node_color, text_size);
  return 0;
}
