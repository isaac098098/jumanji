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

        int texture = s->current_page.texture_index;
        float stride = s->pages_pos[texture][1];

        for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
            s->pages_pos[i][0] = 0.0f;
            s->pages_pos[i][1] -= stride;
        }

        glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, ID_MAT2);
        s->zoom = 1.0f;

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_H && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // go to page top or scroll left 8% of window width

        if(mods & GLFW_MOD_SHIFT) {
            int texture = s->current_page.texture_index;
            float target = 1.0f / s->zoom - 1.0f;
            float stride = target - s->pages_pos[texture][1];

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] += stride;
            }

            s->window->should_redraw = 1;
        }
        else {
            int texture = s->current_page.texture_index;
            float pos_x = s->pages_pos[texture][0];
            float aspect = s->renderer->pages[texture].aspect;
            float left = aspect * s->zoom * (-1.0f + pos_x);

            if(left < -1.0f) {
                float stride = 2.0 * 0.08 / s->zoom;
                float dx = -1.0f - left;
                float pos_x = dx / (aspect * s->zoom);

                if(pos_x < stride) {
                    stride = pos_x;
                }

                for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                    s->pages_pos[i][0] += stride;
                }

                s->window->should_redraw = 1;
            }
        }
    }

    if(key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll down whole page or 8% of window height

        if(mods & GLFW_MOD_SHIFT) {
            int texture = (s->current_page.texture_index + 1) % MAX_RENDERED_PAGES;
            s->current_page.texture_index = texture;
            float stride = s->pages_pos[texture][1];

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] -= stride;
            }
        }
        else {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] += 2.0 * 0.08 / s->zoom;
            }
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll up whole page or 8% of window width
        if(mods & GLFW_MOD_SHIFT) {
            int texture = s->current_page.texture_index;
            float page_stride = s->pages_pos[texture][1];
            float stride = 2.0f + PAGE_GAP;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] -= stride + page_stride;
            }
        }
        else {
            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] -= 2.0 * 0.08 / s->zoom;
            }
        }

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // go to page bottom or scroll right 8% of window width

        if(mods & GLFW_MOD_SHIFT) {
            int texture = s->current_page.texture_index;
            float target = 1.0f - 1.0f / s->zoom;
            float stride = target - s->pages_pos[texture][1];

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                s->pages_pos[i][1] += stride;
            }

            s->window->should_redraw = 1;
        }
        else {
            int texture = s->current_page.texture_index;
            float pos_x = s->pages_pos[texture][0];
            float aspect = s->renderer->pages[texture].aspect;

            float right = aspect * s->zoom * (1.0f + pos_x);

            if(right > 1.0f) {
                float stride = 2.0f * 0.08f / s->zoom;

                float dx = right - 1.0f;
                float pos_dx = dx / (aspect * s->zoom);

                if(pos_dx < stride) {
                    stride = pos_dx;
                }

                for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                    s->pages_pos[i][0] -= stride;
                }

                s->window->should_redraw = 1;
            }
        }
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

        int texture = s->current_page.texture_index;
        float stride = s->pages_pos[texture][0];
        float aspect = s->renderer->pages[texture].aspect;

        for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
            s->pages_pos[i][0] -= stride;
        }

        s->zoom = 1.0f / aspect;
        s->renderer->zoom_matrix[0] = s->zoom;
        s->renderer->zoom_matrix[3] = s->zoom;
        glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, 
                           s->renderer->zoom_matrix);

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        // zoom out

        if(s->zoom > MIN_ZOOM) {
            s->zoom *= 0.9f;
            s->renderer->zoom_matrix[0] = s->zoom;
            s->renderer->zoom_matrix[3] = s->zoom;
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, 
                               s->renderer->zoom_matrix);

            int texture = s->current_page.texture_index;
            float aspect = s->renderer->pages[texture].aspect;
            float limit = 1.0f - 1.0f / (aspect * s->zoom);

            if(limit < 0.0f) limit = 0.0f;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                if(s->pages_pos[i][0] > limit)
                    s->pages_pos[i][0] = limit;

                if(s->pages_pos[i][0] < -limit)
                    s->pages_pos[i][0] = -limit;
            }

            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)
                             && (mods & GLFW_MOD_SHIFT))
    {
        // zoom in

        if(s->zoom < MAX_ZOOM) {
            s->zoom *= 1.1f;
            s->renderer->zoom_matrix[0] = s->zoom;
            s->renderer->zoom_matrix[3] = s->zoom;
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, 
                               s->renderer->zoom_matrix);

            int texture = s->current_page.texture_index;
            float aspect = s->renderer->pages[texture].aspect;
            float limit = 1.0f - 1.0f / (aspect * s->zoom);

            if(limit < 0.0f) limit = 0.0f;

            for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
                if(s->pages_pos[i][0] > limit)
                    s->pages_pos[i][0] = limit;

                if(s->pages_pos[i][0] < -limit)
                    s->pages_pos[i][0] = -limit;
            }

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
