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
    renderer.pages = malloc(MAX_RENDERED_PAGES * sizeof(*renderer.pages));
    for(int i = 0; i < MAX_RENDERED_PAGES; i++) { renderer.pages[i].texture_index = i; }

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

    int first_page = 0;
    int last_page = MAX_RENDERED_PAGES - 1;

    while(!glfwWindowShouldClose(win.glfw_win)) {
        if(win.should_redraw == 1) {
            glClear(GL_COLOR_BUFFER_BIT);
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[i]);
                // printf("pages_pos[%d] = { %f, %f }\n", i, state.zoom * state.pages_pos[i][0], state.zoom * state.pages_pos[i][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[i]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            while(state.zoom * state.pages_pos[last_page][1] > (-1.0f + PAGE_GAP)) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[first_page]);
                state.pages_pos[first_page][1] = state.pages_pos[last_page][1] - (2.0f + PAGE_GAP); 
                // printf("pages_pos[first_page] = { %f, %f }\n", state.zoom * state.pages_pos[first_page][0], state.zoom * state.pages_pos[first_page][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[first_page]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                last_page = (last_page + 1) % MAX_RENDERED_PAGES;
                first_page = (first_page + 1) % MAX_RENDERED_PAGES;
            }

            while(state.zoom * state.pages_pos[first_page][1] < (1.0f - PAGE_GAP)) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[last_page]);
                state.pages_pos[last_page][1] = state.pages_pos[first_page][1] + (2.0f + PAGE_GAP); 
                // printf("pages_pos[last_page] = { %f, %f }\n", state.pages_pos[last_page][0], state.pages_pos[last_page][1]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[last_page]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                last_page = (last_page + 1) % MAX_RENDERED_PAGES;
                first_page = (first_page + 1) % MAX_RENDERED_PAGES;
            }

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                float pos_y = state.pages_pos[i][1];
                float half_bottom = -0.5f * state.zoom;
                float half_top = 0.5f * state.zoom;
                if(pos_y > half_bottom && pos_y < half_top) {
                    state.current_page.page_number = renderer.pages[i].page_number;
                    state.current_page.texture_index = renderer.pages[i].texture_index;
                    // printf("current_page_number = %d\n", state.current_page.page_number);
                    // printf("current_texture_index = %d\n", state.current_page.texture_index);
                    break;
                }
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
