#include <stdio.h>
#include <glad/glad.h>
#include "callbacks.h"
#include "renderer.h"
#include "state.h"
#include "window.h"

#define MAX_ZOOM 10.0f
#define MIN_ZOOM 0.3f
#define ID_MAT2 (float[4]){ 1.0f, 0.0f, 0.0f, 1.0f }

void error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW error: %s\n", description);
}

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    state *s = glfwGetWindowUserPointer(win);

    if(key == GLFW_KEY_A && action == GLFW_PRESS) {
        // fit to height

        int winw = s->window->width;
        int winh = s->window->height;
        int docw = s->document->width;
        int doch = s->document->height;

        /* TODO: generalize to wider width or taller height cases */

        float sx = ((float)winh / doch) * ((float)docw / winw);
        int new_width = (int)(sx * winw);

        if(new_width <= winw) {
            s->zoom = 1.0f;
            s->displacement[0] = 0.0f;
            s->displacement[1] = 0.0f;

            float scale_matrix[] = { sx, 0.0f, 0.0f, 1.0f };
            glUniformMatrix2fv(s->renderer->scale_uniform, 1, GL_FALSE, scale_matrix);
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, ID_MAT2);
            glUniform2fv(s->renderer->displacement_uniform, 1, s->displacement);

            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_H && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll left 8% of window width

        for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
            s->pages_pos[i][0] += 2.0 * 0.08;
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll down whole page or 8% of window height

        if(mods & GLFW_MOD_SHIFT) {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                float stride = (2.0f + PAGE_GAP) * s->zoom;
                s->pages_pos[i][1] += stride;
            }
        }
        else {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] += 2.0 * 0.08;
            }
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll left whole page or 8% of window width

        if(mods & GLFW_MOD_SHIFT) {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                float stride = (2.0f + PAGE_GAP) * s->zoom;
                s->pages_pos[i][1] -= stride;
            }
        }
        else {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] -= 2.0 * 0.08;
            }
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll right 8% of window width

        for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
            s->pages_pos[i][0] -= 2.0 * 0.08;
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
        // close viewer

        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }

    if(key == GLFW_KEY_R && action == GLFW_PRESS) {
        // rotate ninety degrees counterclockwise

        int deg = (s->window->rotation_deg + 90) % 360;
        s->window->rotation_deg = deg;
        float rotation_matrix[] = {
            cos(deg * M_PI / 180.0), -sin(deg * M_PI / 180.0),
            sin(deg * M_PI / 180.0), cos(deg * M_PI / 180.0)
        };
        glUniformMatrix2fv(s->renderer->rotation_uniform, 1, GL_FALSE, rotation_matrix);
        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_S && action == GLFW_PRESS) {
        // fit to width

        int winw = s->window->width;
        int winh = s->window->height;
        int docw = s->document->width;
        int doch = s->document->height;

        /* TOOD: generalize to wider width or taller height cases */

        float sy = ((float)doch / winh) * ((float)winw / docw);
        int new_height = (int)(sy * winh);

        if(new_height >= winh) {
            s->zoom = 1.0f;
            s->displacement[0] = 0.0f;
            s->displacement[1] = 0.0f;

            float scale_matrix[] = { 1.0f, 0.0f, 0.0f, sy };
            glUniformMatrix2fv(s->renderer->scale_uniform, 1, GL_FALSE, scale_matrix);
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, ID_MAT2);
            glUniform2fv(s->renderer->displacement_uniform, 1, s->displacement);

            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        // zoom down

        if(s->zoom > MIN_ZOOM) {
            float old_zoom = s->zoom;
            float new_zoom = s->zoom * 0.9f;
            float ratio = new_zoom / old_zoom;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][0] = ratio * s->pages_pos[i][0];
                s->pages_pos[i][1] = ratio * s->pages_pos[i][1];
            }

            s->zoom = new_zoom;

            s->renderer->zoom_matrix[0] = new_zoom;
            s->renderer->zoom_matrix[1] = 0.0f;
            s->renderer->zoom_matrix[2] = 0.0f;
            s->renderer->zoom_matrix[3] = new_zoom;

            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, 
                               s->renderer->zoom_matrix);
            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)
                             && (mods & GLFW_MOD_SHIFT))
    {
        // zoom up

        if(s->zoom < MAX_ZOOM) {
            float old_zoom = s->zoom;
            float new_zoom = s->zoom * 1.1f;
            float ratio = new_zoom / old_zoom;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][0] = ratio * s->pages_pos[i][0];
                s->pages_pos[i][1] = ratio * s->pages_pos[i][1];
            }

            s->zoom = new_zoom;

            s->renderer->zoom_matrix[0] = new_zoom;
            s->renderer->zoom_matrix[1] = 0.0f;
            s->renderer->zoom_matrix[2] = 0.0f;
            s->renderer->zoom_matrix[3] = new_zoom;

            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, 
                               s->renderer->zoom_matrix);
            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_EQUAL && action == GLFW_RELEASE && (mods & GLFW_MOD_SHIFT))
    {
        /* TODO: rerender page */
    }
}

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    glViewport(LEFT_CORNER_X, LEFT_CORNER_Y, width, height);
    state *s = glfwGetWindowUserPointer(win);
    s->window->width = width;
    s->window->height = height;
    s->window->should_redraw = 1;
}
