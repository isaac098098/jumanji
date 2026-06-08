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
        fprintf(stderr, "usage: ./jumanji [pdf file]\n");
        return -1;
    }
    const char *book = argv[1];

    // open PDF document

    document pdf = { 0 };
    if(open_document(&pdf, book) < 0) {
        return -1;
    }

    pdf.pixmaps = malloc(MAX_RENDERED_PAGES * sizeof(*pdf.pixmaps));

    // initialize window

    window win = { 0 };
    if(create_window(&win) < 0) {
        close_document(&pdf);
        return -1;
    }

    // initialize renderer
    
    renderer renderer = { 0 };

    renderer.textures = malloc(MAX_RENDERED_PAGES * sizeof(*renderer.textures));
    glGenTextures(MAX_RENDERED_PAGES, renderer.textures);

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
    renderer.displacement_uniform = 
             glGetUniformLocation(renderer.shader_program, "displacement_vec");

    // initialize state
    state state = { 0 };
    state.document = &pdf;
    state.renderer = &renderer;
    state.window = &win;
    state.zoom = 1.0f;
    
    // set window user pointer

    glfwSetWindowUserPointer(win.glfw_win, &state);

    // initial render and position
    /* TODO: get initial rotation (and more position attributes) from document database */

    initial_render(&pdf, &renderer, &win);
    win.should_redraw = 1;
    
    // calculate initial vertical page positions

    for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
        state.pages_pos[i][0] = 0.0f;
        state.pages_pos[i][1] = -i * ((2.0f + PAGE_GAP) * state.zoom);
    }

    // render loop

    int first_texture = 0;
    int last_texture = MAX_RENDERED_PAGES - 1;

    while(!glfwWindowShouldClose(win.glfw_win)) {
        if(win.should_redraw == 1) {
            glClear(GL_COLOR_BUFFER_BIT);
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[i]);
                printf("pages_pos[%d] = { %f, %f }\n", i, state.pages_pos[i][0], state.pages_pos[i][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[i]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            if(state.pages_pos[last_texture][1] > (-1.0f + state.zoom + PAGE_GAP)) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[first_texture]);
                state.pages_pos[first_texture][1] = state.pages_pos[last_texture][1] - (2.0f + PAGE_GAP) * state.zoom; 
                printf("pages_pos[first_texture] = { %f, %f }\n", state.pages_pos[first_texture][0], state.pages_pos[first_texture][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[first_texture]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                last_texture = (last_texture + 1) % MAX_RENDERED_PAGES;
                first_texture = (first_texture + 1) % MAX_RENDERED_PAGES;
            }

            if(state.pages_pos[first_texture][1] < (1.0f - state.zoom - PAGE_GAP)) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[last_texture]);
                state.pages_pos[last_texture][1] = state.pages_pos[first_texture][1] + (2.0f + PAGE_GAP) * state.zoom; 
                printf("pages_pos[last_texture] = { %f, %f }\n", state.pages_pos[last_texture][0], state.pages_pos[last_texture][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[last_texture]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                last_texture = (last_texture + 1) % MAX_RENDERED_PAGES;
                first_texture = (first_texture + 1) % MAX_RENDERED_PAGES;
            }

            glfwSwapBuffers(win.glfw_win);
            win.should_redraw = 0;
        }

        glfwWaitEvents();
    }

    free(renderer.textures);
    free(pdf.pixmaps);
    destroy_window(&win);
    close_document(&pdf);

    return 0;
}
