#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mupdf/fitz.h>
#include "pdf.h"
#include "renderer.h"
#include "window.h"
#include "window/callbacks.h"
#include "state.h"

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "usage: ./jumangi [pdf file]\n");
        return -1;
    }
    const char *book = argv[1];

    // open PDF document

    document pdf = { 0 };
    if(open_document(&pdf, book) < 0) {
        return -1;
    }

    // initialize window

    window win = { 0 };
    if(create_window(&win) < 0) {
        close_document(&pdf);
        return -1;
    }

    // initialize renderer
    
    renderer renderer = { 0 };
    if(init_renderer(&renderer) < 0) {
        destroy_window(&win);
        close_document(&pdf);
        return -1;
    }

    renderer.rotation_uniform = 
             glGetUniformLocation(renderer.shader_program, "rotation_matrix");
    renderer.scale_uniform = 
             glGetUniformLocation(renderer.shader_program, "scale_matrix");
    renderer.zoom_uniform = 
             glGetUniformLocation(renderer.shader_program, "zoom_matrix");

    // initialize state

    state state = { 0 };
    state.document = &pdf;
    state.renderer = &renderer;
    state.window = &win;

    renderer.displacement_uniform = 
             glGetUniformLocation(renderer.shader_program, "displacement_vec");

    state.zoom = 1.0f;
    
    // set window user pointer

    glfwSetWindowUserPointer(win.glfw_win, &state);
    win.should_redraw = 1;

    // initial render and position

    /* TODO: get initial rotation (and more position attributes) from document database */

    render_pages(&pdf, &win);

    float rotation_matrix[4];
    rotation_matrix[0] = 1.0f;
    rotation_matrix[1] = 0.0f;
    rotation_matrix[2] = 0.0f;
    rotation_matrix[3] = 1.0f;
    glUniformMatrix2fv(renderer.rotation_uniform, 1, GL_FALSE, rotation_matrix);

    float scale_matrix[4];
    scale_matrix[0] = 1.0f;
    scale_matrix[1] = 0.0f;
    scale_matrix[2] = 0.0f;
    scale_matrix[3] = 1.0f;
    glUniformMatrix2fv(renderer.scale_uniform, 1, GL_FALSE, scale_matrix);

    float zoom_matrix[4];
    zoom_matrix[0] = 1.0f;
    zoom_matrix[1] = 0.0f;
    zoom_matrix[2] = 0.0f;
    zoom_matrix[3] = 1.0f;
    glUniformMatrix2fv(renderer.zoom_uniform, 1, GL_FALSE, zoom_matrix);

    // render loop

    while(!glfwWindowShouldClose(win.glfw_win)) {
        // printf("win.should_redraw = %d\n", win.should_redraw);

        if(win.should_redraw == 1) {
            // render_pages(&pdf, &win);
            draw_pages(&win, &renderer);
        }
        
        glfwWaitEvents();
    }

    destroy_window(&win);
    close_document(&pdf);

    return 0;
}
