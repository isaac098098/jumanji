#ifndef RENDERER_H
#define RENDERER_H

#include "pdf.h"

typedef struct window window;

typedef struct renderer {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shader_program;
    GLuint texture;
    GLuint aspect_uniform;
    GLuint rotation_uniform;
    GLuint scale_uniform;
    GLuint zoom_uniform;
    GLuint displacement_uniform;
    int should_rerender;
} renderer;

int init_renderer(renderer *renderer);
void render_pages(document *pdf, window *win);
void draw_pages(window *win, renderer *renderer);

#endif // RENDERER_H
