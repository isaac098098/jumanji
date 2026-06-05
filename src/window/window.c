#include <stdio.h>
#include <math.h>
#include <glad/glad.h>
#include "callbacks.h"
#include "window.h"
#include "renderer.h"

#define LEFT_CORNER_X 0
#define LEFT_CORNER_Y 0
#define WIN_WIDTH 840
#define WIN_HEIGHT 680

int create_window(window *win) {
    if(!glfwInit()) {
        fprintf(stderr, "failed to initialize GLFW!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    win->glfw_win = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "triangle", NULL, NULL);
    if(!win->glfw_win) {
        fprintf(stderr, "failed to create GLFW window!\n");
        glfwTerminate();
        return -1;
    }

    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(win->glfw_win, key_callback);
    glfwSetFramebufferSizeCallback(win->glfw_win, framebuffer_size_callback);
    glfwGetWindowSize(win->glfw_win, &win->width, &win->height);

    glfwMakeContextCurrent(win->glfw_win);
    gladLoadGL();
    glViewport(LEFT_CORNER_X, LEFT_CORNER_Y, WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void destroy_window(window *win) {
    glfwDestroyWindow(win->glfw_win);
    glfwTerminate();
}
