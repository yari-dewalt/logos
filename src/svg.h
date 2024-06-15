#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

// svg struct
typedef struct {
  char* svg;
  int height;
  int width;
  bool finalized;
} svg_t;

// Creates, initializes, and returns svg.
svg_t* svg_create(int width, int height);
// Ends svg tag and updates finalized state.
void svg_finalize(svg_t* svg);
// Prints the svg text.
void svg_print(svg_t* svg);
// Saves svg file.
void svg_save(svg_t* svg, char* file_path);
// Frees svg memory.
void svg_free(svg_t* svg);
// Adds circle element to svg.
void svg_circle(svg_t* svg, char* stroke, int stroke_width, char* fill, int r, int cx, int cy);
// Adds line element to svg.
void svg_line(svg_t* svg, char* stroke, int stroke_width, int x1, int y1, int x2, int y2);
// Adds arrow element to svg.
void svg_arrow(svg_t* svg, char* stroke, int stroke_width, int arrow_length, int x1, int y1, int x2, int y2);
// Adds rectangle element to svg.
void svg_rectangle(svg_t* svg, int width, int height, int x, int y, char* fill, char* stroke, int stroke_width, int radius_x, int radius_y);
// Fills background of svg.
void svg_fill(svg_t* svg, char* fill);
// Draws text.
void svg_text(svg_t* svg, int x, int y, char* font_family, int font_size, char* fill, char* stroke, char* text);
// Adds ellipse element to svg.
void svg_ellipse(svg_t* svg, int cx, int cy, int rx, int ry, char* fill, char* stroke, int stroke_width); 
