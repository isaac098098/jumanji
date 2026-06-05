#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <GLFW/glfw3.h>

#define LEFT_CORNER_X 0
#define LEFT_CORNER_Y 0
#define WIN_WIDTH 840
#define WIN_HEIGHT 680

void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

#endif // CALLBACKS_H
