#include "svg.h"
#include <string.h>
#include <math.h>

// Helper to append string to svg text.
static void appendstringtosvg(svg_t* svg, char* text) {
  int l = strlen(svg->svg) + strlen(text) + 1;

  char* p = realloc(svg->svg, l);

  if (p) {
    svg->svg = p;
  }

  strcat(svg->svg, text);
}

// Helper to append number to svg text.
static void appendnumbertosvg(svg_t* svg, int n) {
  char sn[16];

  sprintf(sn, "%d", n);

  appendstringtosvg(svg, sn);
}

// Creates, initializes, and returns svg.
svg_t* svg_create(int width, int height) {
  svg_t* svg = malloc(sizeof(svg_t));

  if (svg != NULL) {
    svg->svg = NULL;
    svg->finalized = false;
    svg->width = width;
    svg->height = height;

    svg->svg = malloc(1);

    sprintf(svg->svg, "%s", "\0");

    appendstringtosvg(svg, "<svg width='");
    appendnumbertosvg(svg, width);
    appendstringtosvg(svg, "px' height='");
    appendnumbertosvg(svg, height);
    appendstringtosvg(svg, "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n");

    return svg;
  } else {
    return NULL;
  }
}

// Ends svg tag and updates finalized state.
void svg_finalize(svg_t* svg) {
  appendstringtosvg(svg, "</svg>");

  svg->finalized = true;
}

// Prints the svg text.
void svg_print(svg_t* svg) {
  printf("%s\n", svg->svg);
}

// Saves svg file.
void svg_save(svg_t* svg, char* file_path) {
  if (!svg->finalized) {
    svg_finalize(svg);
  }

  FILE* fp;

  fp = fopen(file_path, "w");
  if (fp != NULL) {
    fwrite(svg->svg, 1, strlen(svg->svg), fp);
    fclose(fp);
  }
}

// Frees svg memory.
void svg_free(svg_t* svg) {
  free(svg->svg);
  free(svg);
}

// Adds rectangle element to svg.
void svg_rectangle(svg_t* svg, int width, int height,
                   int x, int y, char* fill, char* stroke,
                   int stroke_width, int radius_x, int radius_y) {
  appendstringtosvg(svg, "  <rect fill='");
  appendstringtosvg(svg, fill);
  appendstringtosvg(svg, "' stroke='");
  appendstringtosvg(svg, stroke);
  appendstringtosvg(svg, "' stroke-width='");
  appendnumbertosvg(svg, stroke_width);
  appendstringtosvg(svg, "px' width='");
  appendnumbertosvg(svg, width);
  appendstringtosvg(svg, "' height='");
  appendnumbertosvg(svg, height);
  appendstringtosvg(svg, "' y='");
  appendnumbertosvg(svg, y);
  appendstringtosvg(svg, "' x='");
  appendnumbertosvg(svg, x);
  appendstringtosvg(svg, "' ry='");
  appendnumbertosvg(svg, radius_y);
  appendstringtosvg(svg, "' rx='");
  appendnumbertosvg(svg, radius_x);
  appendstringtosvg(svg, "'/>\n");
}

// Fills background of svg.
void svg_fill(svg_t* svg, char* fill) {
  svg_rectangle(svg, svg->width, svg->height, 0, 0, fill, fill, 0, 0, 0);
}

// Adds line element to svg.
void svg_line(svg_t* svg, char* stroke, int stroke_width,
              int x1, int y1, int x2, int y2) {
  appendstringtosvg(svg, "  <line stroke='");
  appendstringtosvg(svg, stroke);
  appendstringtosvg(svg, "' stroke-width='");
  appendnumbertosvg(svg, stroke_width);
  appendstringtosvg(svg, "px' y2='");
  appendnumbertosvg(svg, y2);
  appendstringtosvg(svg, "' x2='");
  appendnumbertosvg(svg, x2);
  appendstringtosvg(svg, "' y1='");
  appendnumbertosvg(svg, y1);
  appendstringtosvg(svg, "' x1='");
  appendnumbertosvg(svg, x1);
  appendstringtosvg(svg, "'/>\n");
}

// Adds arrow element to svg.
void svg_arrow(svg_t* svg, char* stroke, int stroke_width, int arrow_length,
               int x1, int y1, int x2, int y2) {
  // Draw the main line
  svg_line(svg, stroke, stroke_width, x1, y1, x2, y2);

  // Calculate the direction vector of the line
  double dx = x2 - x1;
  double dy = y2 - y1;
  double length = sqrt(dx * dx + dy * dy);
  double unit_dx = dx / length;
  double unit_dy = dy / length;

  // Arrowhead parameters
  double arrow_angle = M_PI / 6; // 30 degrees in radians

  // Calculate the points of the arrowhead
  int arrow_x1 = x2 - arrow_length * (unit_dx * cos(arrow_angle) - unit_dy * sin(arrow_angle));
  int arrow_y1 = y2 - arrow_length * (unit_dy * cos(arrow_angle) + unit_dx * sin(arrow_angle));
  int arrow_x2 = x2 - arrow_length * (unit_dx * cos(-arrow_angle) - unit_dy * sin(-arrow_angle));
  int arrow_y2 = y2 - arrow_length * (unit_dy * cos(-arrow_angle) + unit_dx * sin(-arrow_angle));

  // Draw the arrowhead lines
  svg_line(svg, stroke, stroke_width, x2, y2, arrow_x1, arrow_y1);
  svg_line(svg, stroke, stroke_width, x2, y2, arrow_x2, arrow_y2);
}

// Draws text.
void svg_text(svg_t* svg, int x, int y, char* font_family,
              int font_size, char* fill, char* stroke, char* text) {
  appendstringtosvg(svg, "  <text x='");
  appendnumbertosvg(svg, x);
  appendstringtosvg(svg, "' y='");
  appendnumbertosvg(svg, y);
  appendstringtosvg(svg, "' font-family='");
  appendstringtosvg(svg, font_family);
  appendstringtosvg(svg, "' stroke='");
  appendstringtosvg(svg, stroke);
  appendstringtosvg(svg, "' fill='");
  appendstringtosvg(svg, fill);
  appendstringtosvg(svg, "' font-size='");
  appendnumbertosvg(svg, font_size);
  appendstringtosvg(svg, "px");
  appendstringtosvg(svg, "' text-anchor='middle' dominant-baseline='middle'>");
  appendstringtosvg(svg, text);
  appendstringtosvg(svg, "</text>\n");
}

// Adds circle element to svg.
void svg_circle(svg_t* svg, char* stroke, int stroke_width, char* fill, int r, int cx, int cy) {
  appendstringtosvg(svg, "  <circle stroke='");
  appendstringtosvg(svg, stroke);
  appendstringtosvg(svg, "' stroke-width='");
  appendnumbertosvg(svg, stroke_width);
  appendstringtosvg(svg, "px' fill='");
  appendstringtosvg(svg, fill);
  appendstringtosvg(svg, "' r='");
  appendnumbertosvg(svg, r);
  appendstringtosvg(svg, "' cy='");
  appendnumbertosvg(svg, cy);
  appendstringtosvg(svg, "' cx='");
  appendnumbertosvg(svg, cx);
  appendstringtosvg(svg, "'/>\n");
}

// Adds ellipse element to svg.
void svg_ellipse(svg_t* svg, int cx, int cy, int rx, int ry, char* fill, char* stroke, int stroke_width) {
  appendstringtosvg(svg, "  <ellipse cx='");
  appendnumbertosvg(svg, cx);
  appendstringtosvg(svg, "' cy='");
  appendnumbertosvg(svg, cy);
  appendstringtosvg(svg, "' rx='");
  appendnumbertosvg(svg, rx);
  appendstringtosvg(svg, "' ry='");
  appendnumbertosvg(svg, ry);
  appendstringtosvg(svg, "' fill='");
  appendstringtosvg(svg, fill);
  appendstringtosvg(svg, "' stroke='");
  appendstringtosvg(svg, stroke);
  appendstringtosvg(svg, "' stroke-width='");
  appendnumbertosvg(svg, stroke_width);
  appendstringtosvg(svg, "'/>\n");
}
