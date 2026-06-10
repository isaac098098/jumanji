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
    float pages_pos[MAX_RENDERED_PAGES][2];
    page current_page;
} state;

#endif // STATE_H
