#ifndef STATE_H
#define STATE_H

#include "pdf.h"
#include "renderer.h"
#include "window.h"

typedef struct state {
    document *document;  
    renderer *renderer;
    window *window;
    float displacement[2];
    float zoom;
} state;

#endif // STATE_H
