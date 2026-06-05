#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer.h"
#include "window.h"

#define MAX_COMPILE_LOG_LEN 512

const float wrap_border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

char* read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
        fprintf(stderr, "could not open %s!\n", path);
        return NULL;
    }

    char c;
    size_t fp_c = 0;
    while((c = fgetc(fp)) != EOF) fp_c++;
    rewind(fp);

    char *buffer =  malloc(fp_c * sizeof(*buffer) + 1);
    if(buffer == NULL) {
        fprintf(stderr, "failed to allocate memory for file %s!\n", path);
        return NULL;
    }
    fread(buffer, sizeof(char), fp_c, fp);
    buffer[fp_c] = '\0';

    fclose(fp);

    return buffer;
}

int compile_and_get_shaders(GLuint *shader_program) {
    // compile vertex shader

    char *vertex_shader_source = read_file("src/shaders/shader.vert");
    const char *src = vertex_shader_source;
    GLuint vertex_shader = 0;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &src, NULL);
    glCompileShader(vertex_shader);
    int res = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &res);

    if(!res) {
        char *compile_log = malloc(MAX_COMPILE_LOG_LEN * sizeof(*compile_log));
        glGetShaderInfoLog(vertex_shader, MAX_COMPILE_LOG_LEN, NULL, compile_log);
        fprintf(stderr, "vertex shader compilation failed: %s\n", compile_log);
        free(compile_log);
        return -1;
    }

    free(vertex_shader_source);

    // compile fragment shader

    char *fragment_shader_source = read_file("src/shaders/shader.frag");
    src = fragment_shader_source;
    unsigned int fragment_shader = 0;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &res);

    if(!res) {
        char *compile_log = malloc(MAX_COMPILE_LOG_LEN * sizeof(*compile_log));
        glGetShaderInfoLog(fragment_shader, MAX_COMPILE_LOG_LEN, NULL, compile_log);
        fprintf(stderr, "fragment shader compilation failed: %s\n", compile_log);
        free(compile_log);
        return -1;
    }

    free(fragment_shader_source);

    // link shaders

    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, vertex_shader);
    glAttachShader(*shader_program, fragment_shader);
    glLinkProgram(*shader_program);
    glGetProgramiv(*shader_program, GL_LINK_STATUS, &res);

    if(!res) {
        char *compile_log = malloc(MAX_COMPILE_LOG_LEN * sizeof(*compile_log));
        glGetProgramInfoLog(*shader_program, MAX_COMPILE_LOG_LEN, NULL, compile_log);
        fprintf(stderr, "shader linking failed: %s\n", compile_log);
        free(compile_log);
        return -1;
    }

    // cleanup

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return 0;
}

int init_renderer(renderer *renderer) {
    // compile and get shader program
    
    if(compile_and_get_shaders(&renderer->shader_program) < 0) {
        return -1;
    }

    glUseProgram(renderer->shader_program);

    // send vertex data to GPU

    // send vertices to GPU memory through VBOs,
    // configure how openGL should interpret them
    // and specify how to send data to the GPU

    float initial_pos[] = {
        //positions  // texture coords
        1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f, 0.0f,
       -1.0f,  1.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_pos), initial_pos, GL_STATIC_DRAW);

    glGenBuffers(1, &renderer->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // how OpenGL should interpret the vertex data

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // set textures configuration

    glGenTextures(1, &renderer->texture);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, wrap_border_color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glfwSwapInterval(1);
    glClearColor(0.2f, 0.3f, 0.3f, 0.5f);

    return 0;
}

void render_pages(document *pdf, window *win) {
    // new pixmap from page

    float zoom = 3.0;
    fz_matrix ctm = fz_scale(zoom, zoom);

    fz_try(pdf->ctx) {
        pdf->pixmap = fz_new_pixmap_from_page_number(pdf->ctx, pdf->doc, 4, ctm,
                fz_device_rgb(pdf->ctx), 1);
    }
    fz_catch(pdf->ctx) {
        fprintf(stderr, "could not create pixmap\n!");
        fz_drop_document(pdf->ctx, pdf->doc);
        fz_drop_context(pdf->ctx);
    }

    pdf->width = pdf->pixmap->w;
    pdf->height = pdf->pixmap->h;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pdf->pixmap->w, pdf->pixmap->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pdf->pixmap->samples);

    fz_drop_pixmap(pdf->ctx, pdf->pixmap);
}

void draw_pages(window *win, renderer *renderer) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(win->glfw_win);
    win->should_redraw = 0;
}
