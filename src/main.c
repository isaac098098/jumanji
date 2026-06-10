#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mupdf/fitz.h>
#include <float.h>

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
    pdf.base_page_heights = malloc(MAX_RENDERED_PAGES * sizeof(*pdf.base_page_heights));

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

            // rerender pages if visible pages if zoom has changed and
            // mark all the other pages as rerender pending

            if(state.changed_zoom) {
                for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                    float center = state.zoom * state.pages_pos[i][1];
                    float bottom = center - state.zoom;
                    float top = center + state.zoom;

                    if(top > -1.0f && bottom < 1.0f)
                    {
                        if(renderer.pages[i].should_rerender == 1) {
                            glBindTexture(GL_TEXTURE_2D, renderer.textures[i]);
                            state.render_zoom = (state.window->height * state.zoom * 1.5f) / state.document->base_page_heights[i];
                            fz_matrix ctm = fz_scale(state.render_zoom, state.render_zoom);
                            fz_drop_pixmap(pdf.ctx, pdf.pixmaps[i]);
                            pdf.pixmaps[i] = 
                                fz_new_pixmap_from_page_number(pdf.ctx, pdf.doc,
                                        renderer.pages[i].page_number,
                                        ctm,
                                        fz_device_rgb(pdf.ctx),
                                        1);
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pdf.pixmaps[i]->w, 
                                    pdf.pixmaps[i]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                                    pdf.pixmaps[i]->samples);

                            renderer.pages[i].should_rerender = 0;
                            win.should_redraw = 1;
                        }

                    }
                }
            }

            // draw next page if needed and check if it's the
            // last page of the document

            while(state.zoom * state.pages_pos[last_page][1] > (-1.0f + PAGE_GAP)) {
                if(renderer.pages[last_page].page_number < state.document->pages_num - 1) {
                    glBindTexture(GL_TEXTURE_2D, renderer.textures[first_page]);
                    state.pages_pos[first_page][1] = state.pages_pos[last_page][1] - (2.0f + PAGE_GAP); 
                    // printf("pages_pos[first_page] = { %f, %f }\n", state.zoom * state.pages_pos[first_page][0], state.zoom * state.pages_pos[first_page][1]);

                    if(renderer.pages[first_page].page_number != renderer.pages[last_page].page_number + 1) {
                        state.render_zoom = (state.window->height * state.zoom * 1.5f) / state.document->base_page_heights[first_page];
                        fz_matrix ctm = fz_scale(state.render_zoom, state.render_zoom);
                        fz_drop_pixmap(pdf.ctx, pdf.pixmaps[first_page]);
                        pdf.pixmaps[first_page] = 
                            fz_new_pixmap_from_page_number(pdf.ctx, pdf.doc,
                                    renderer.pages[last_page].page_number + 1,
                                    ctm,
                                    fz_device_rgb(pdf.ctx),
                                    1);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pdf.pixmaps[first_page]->w,
                                     pdf.pixmaps[first_page]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                                     pdf.pixmaps[first_page]->samples);

                        renderer.pages[first_page].page_number = renderer.pages[last_page].page_number + 1;
                    }

                    glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[first_page]);
                    last_page = (last_page + 1) % MAX_RENDERED_PAGES;
                    first_page = (first_page + 1) % MAX_RENDERED_PAGES;
                }
                else {
                    break;
                }
            }

            // draw previous page if needed and check if it's the
            // first page of the document

            while(state.zoom * state.pages_pos[first_page][1] < (1.0f - PAGE_GAP)) {
                if(renderer.pages[first_page].page_number > 0) {
                    glBindTexture(GL_TEXTURE_2D, renderer.textures[last_page]);
                    state.pages_pos[last_page][1] = state.pages_pos[first_page][1] + (2.0f + PAGE_GAP); 

                    if(renderer.pages[last_page].page_number != renderer.pages[first_page].page_number - 1) {
                        state.render_zoom = (state.window->height * state.zoom * 1.5f) / state.document->base_page_heights[last_page];
                        fz_matrix ctm = fz_scale(state.render_zoom, state.render_zoom);
                        fz_drop_pixmap(pdf.ctx, pdf.pixmaps[last_page]);
                        pdf.pixmaps[last_page] = 
                              fz_new_pixmap_from_page_number(pdf.ctx, pdf.doc,
                                                             renderer.pages[first_page].page_number - 1,
                                                             ctm,
                                                             fz_device_rgb(pdf.ctx),
                                                             1);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pdf.pixmaps[last_page]->w,
                                     pdf.pixmaps[last_page]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                                     pdf.pixmaps[last_page]->samples);

                        renderer.pages[last_page].page_number = renderer.pages[first_page].page_number - 1;
                    }

                    glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[last_page]);
                    last_page = (last_page + 1) % MAX_RENDERED_PAGES;
                    first_page = (first_page + 1) % MAX_RENDERED_PAGES;
                }
                else {
                    break;
                }
            }

            // draw pages

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                glBindTexture(GL_TEXTURE_2D, renderer.textures[i]);
                glUniform2fv(renderer.displacement_uniform, 1, state.pages_pos[i]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            
            // calculate current page

            float best_dist = MAX_ZOOM * 2.0f;
            int best_page = -1;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                float center = state.pages_pos[i][1];

                float dist = fabsf(center);

                if(dist < best_dist) {
                    best_dist = dist;
                    best_page = i;
                }
            }

            if(best_page >= 0) {
                state.current_page.page_number =
                    renderer.pages[best_page].page_number;

                state.current_page.texture_index =
                    renderer.pages[best_page].texture_index;
            }

            glfwSwapBuffers(win.glfw_win);
            win.should_redraw = 0;
        }

        glfwWaitEvents();
    }

    free(renderer.textures);
    free(pdf.base_page_heights);
    free(pdf.pixmaps);
    destroy_window(&win);
    close_document(&pdf);

    return 0;
}
