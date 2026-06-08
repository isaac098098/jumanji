#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

typedef struct renderer renderer;

typedef struct window {
    GLFWwindow *glfw_win;
    int width;
    int height;
    int current_page;
    int rotation_deg;
    int should_redraw;
} window;

int create_window(window *win);
void destroy_window(window *win);

#endif // WINDOW_H
