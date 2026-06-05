#include <stdio.h>
#include <glad/glad.h>
#include "callbacks.h"
#include "renderer.h"
#include "state.h"
#include "window.h"

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

        /* TOOD: generalize to wider width or taller height cases */

        printf("window width = %d\n", winw);
        printf("window height = %d\n", winh);
        printf("document width = %d\n", docw);
        printf("document height = %d\n", doch);

        float sx = ((float)winh / doch) * ((float)docw / winw);
        int new_width = (int)(sx * winw);
        printf("new_width = %d\n", new_width);

        if(new_width <= winw) {
            float scale_matrix[4];
            scale_matrix[0] = sx;
            scale_matrix[1] = 0.0f;
            scale_matrix[2] = 0.0f;
            scale_matrix[3] = 1.0f;
            glUniformMatrix2fv(s->renderer->scale_uniform, 1, GL_FALSE, scale_matrix);

            float zoom_matrix[4];
            zoom_matrix[0] = 1.0f;
            zoom_matrix[1] = 0.0f;
            zoom_matrix[2] = 0.0f;
            zoom_matrix[3] = 1.0f;
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, zoom_matrix);

            s->zoom = 1.0f;

            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll down

        int winw = s->window->width;
        int winh = s->window->height;

        // scroll 8% of window width

        s->displacement[1] += 2.0 * 0.08;
        glUniform2fv(s->renderer->displacement_uniform, 1, s->displacement);

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT )) {
        // scroll up

        int winw = s->window->width;
        int winh = s->window->height;
        int docw = s->document->width;
        int doch = s->document->height;

        // scroll 8% of window width

        s->displacement[1] -= 2.0 * 0.08;
        glUniform2fv(s->renderer->displacement_uniform, 1, s->displacement);

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
        // close viewer

        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }

    if(key == GLFW_KEY_R && action == GLFW_PRESS) {
        // rotate ninety degrees counterclockwise

        float rotation_matrix[4];
        int deg = (s->window->rotation_deg + 90) % 360;
        s->window->rotation_deg = deg;
        rotation_matrix[0] = cos(deg * M_PI / 180.0);
        rotation_matrix[1] = -sin(deg * M_PI / 180.0);
        rotation_matrix[2] = sin(deg * M_PI / 180.0);
        rotation_matrix[3] = cos(deg * M_PI / 180.0);
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

        printf("window width = %d\n", winw);
        printf("window height = %d\n", winh);
        printf("document width = %d\n", docw);
        printf("document height = %d\n", doch);

        float sy = ((float)doch / winh) * ((float)winw / docw);
        int new_height = (int)(sy * winh);
        printf("new_height = %d\n", new_height);

        if(new_height >= winh) {
            float scale_matrix[4];
            scale_matrix[0] = 1.0f;
            scale_matrix[1] = 0.0f;
            scale_matrix[2] = 0.0f;
            scale_matrix[3] = sy;
            glUniformMatrix2fv(s->renderer->scale_uniform, 1, GL_FALSE, scale_matrix);

            float zoom_matrix[4];
            zoom_matrix[0] = 1.0f;
            zoom_matrix[1] = 0.0f;
            zoom_matrix[2] = 0.0f;
            zoom_matrix[3] = 1.0f;
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, zoom_matrix);

            s->zoom = 1.0f;
            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        // zoom dowm

        if(s->zoom >= 0.1f) {
            float new_zoom = s->zoom - 0.1f;
            s->zoom = new_zoom;

            float zoom_matrix[4];
            zoom_matrix[0] = new_zoom;
            zoom_matrix[1] = 0.0f;
            zoom_matrix[2] = 0.0f;
            zoom_matrix[3] = new_zoom;
            glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, zoom_matrix);

            s->window->should_redraw = 1;
        }
    }

    if(key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)
                             && (mods & GLFW_MOD_SHIFT))
    {
        // zoom up

        float new_zoom = s->zoom + 0.1f;
        s->zoom = new_zoom;

        float zoom_matrix[4];
        zoom_matrix[0] = new_zoom;
        zoom_matrix[1] = 0.0f;
        zoom_matrix[2] = 0.0f;
        zoom_matrix[3] = new_zoom;
        glUniformMatrix2fv(s->renderer->zoom_uniform, 1, GL_FALSE, zoom_matrix);

        s->window->should_redraw = 1;
    }

    if(key == GLFW_KEY_EQUAL && action == GLFW_RELEASE && (mods & GLFW_MOD_SHIFT))
    {
        // zoom up

        printf("aquí es cuando renderizas\n");
    }
}

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    glViewport(LEFT_CORNER_X, LEFT_CORNER_Y, width, height);
    state *s = glfwGetWindowUserPointer(win);
    s->window->width = width;
    s->window->height = height;
    s->window->should_redraw = 1;
}
