#ifndef RENDERER_H
#define RENDERER_H

#include "pdf.h"

#define MAX_RENDERED_PAGES 4
#define PAGE_GAP 0.01f

typedef struct window window;

typedef struct page {
    int texture_index;
    int page_number;
    float aspect;
} page;

typedef struct renderer {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shader_program;
    GLuint *textures;
    page *pages;
    GLuint rotation_uniform;
    GLuint scale_uniform;
    GLuint zoom_uniform;
    float zoom_matrix[4];
    GLuint displacement_uniform;
    int should_rerender;
} renderer;

int init_renderer(renderer *renderer);
void initial_render(document *pdf, renderer *r, window *win);
void render_pages(document *pdf, window *win, renderer *renderer);
void draw_pages(window *win, renderer *r);

#endif // RENDERER_H
