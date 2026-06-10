#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "window.h"
#include "state.h"
#define MAX_COMPILE_LOG_LEN 512
#define ID_MAT2 (float[4]){ 1.0f, 0.0f, 0.0f, 1.0f }

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

    char *vertex_shader_source = read_file(SHADER_DIR "/shader.vert");
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

    char *fragment_shader_source = read_file(SHADER_DIR "/shader.frag");
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

    for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
        glBindTexture(GL_TEXTURE_2D, renderer->textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, wrap_border_color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glfwSwapInterval(1);
    glClearColor(0.2f, 0.3f, 0.3f, 0.5f);

    return 0;
}

void initial_render(document *pdf, renderer *r, window *win) {
    state *s = glfwGetWindowUserPointer(win->glfw_win);

    // TODO: check if MAX_RENDERED_PAGES > fz_count_pages

    int initial_page = 0;
    for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
        float zoom = INITIAL_RENDER_ZOOM;
        fz_matrix ctm = fz_scale(zoom, zoom);

        fz_try(pdf->ctx) {
            pdf->pixmaps[i] = 
                fz_new_pixmap_from_page_number(pdf->ctx, pdf->doc, i + initial_page,
                                               ctm, fz_device_rgb(pdf->ctx), 1);
        }
        fz_catch(pdf->ctx) {
            fprintf(stderr, "could not create pixmap\n!");
            fz_drop_document(pdf->ctx, pdf->doc);
            fz_drop_context(pdf->ctx);
        }

        s->document->width = pdf->pixmaps[i]->w;
        s->document->height = pdf->pixmaps[i]->h;

        int winw = s->window->width;
        int winh = s->window->height;
        int docw = s->document->width;
        int doch = s->document->height;

        glBindTexture(GL_TEXTURE_2D, r->textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pdf->pixmaps[i]->w, pdf->pixmaps[i]->h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, pdf->pixmaps[i]->samples);
        r->pages[i].page_number = i + initial_page;

        glUniformMatrix2fv(r->rotation_uniform, 1, GL_FALSE, ID_MAT2);
        float sx = ((float)winh / doch) * ((float)docw / winw);
        s->renderer->pages[i].aspect = sx;
        float scale_matrix[] = { sx, 0.0f, 0.0f, 1.0f };
        glUniformMatrix2fv(r->scale_uniform, 1, GL_FALSE, scale_matrix);

        r->zoom_matrix[0] = 1.0f;
        r->zoom_matrix[1] = 0.0f;
        r->zoom_matrix[2] = 0.0f;
        r->zoom_matrix[3] = 1.0f;
        glUniformMatrix2fv(r->zoom_uniform, 1, GL_FALSE, r->zoom_matrix);

        // glUniform2fv(r->displacement_uniform, 1, s->displacement);
    }

    /* TODO: set last displacement, zoom and rotation of document */
}

void render_pages(document *pdf, window *win, renderer *renderer) {
    pdf->width = pdf->pixmaps[0]->w;
    pdf->height = pdf->pixmaps[0]->h;

    for(int i = 0; i < MAX_RENDERED_PAGES; i++) {
        fz_drop_pixmap(pdf->ctx, pdf->pixmaps[i]);
    }
}

void draw_pages(window *win, renderer *r) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(win->glfw_win);
    win->should_redraw = 0;
}
