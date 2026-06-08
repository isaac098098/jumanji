#ifndef PDF_H
#define PDF_H

#include <mupdf/fitz.h>

typedef struct document {
    fz_context *ctx;
    fz_document *doc;
    fz_colorspace *colorspace;
    fz_pixmap **pixmaps;
    int width;
    int height;
} document;

int open_document(document *pdf, const char *file_path);
void close_document(document *pdf);

#endif // PDF_H
