#include <stdio.h>
#include <mupdf/fitz.h>
#include "pdf.h"

extern fz_document_handler pdf_document_handler;

int open_document(document *pdf, const char *file_path) {
    pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if(pdf->ctx == NULL) {
        fprintf(stderr, "could not create MuPDF context!\n");
        return -1;
    }

    fz_set_warning_callback(pdf->ctx, NULL, NULL);
    fz_set_error_callback(pdf->ctx, NULL, NULL);

    fz_try(pdf->ctx) {
        fz_register_document_handler(pdf->ctx, &pdf_document_handler);
    }
    fz_catch(pdf->ctx) {
        fprintf(stderr, "could not register PDF document handler!\n");
        fz_drop_context(pdf->ctx);
        return -1;
    }

    // open document

    fz_try(pdf->ctx) {
        pdf->doc = fz_open_document(pdf->ctx, file_path);
    }
    fz_catch(pdf->ctx) {
        fprintf(stderr, "could not open file %s\n", file_path);
        fz_drop_context(pdf->ctx);
        return -1;
    }

    // count number of pages

    // int pages_c = fz_count_pages(ctx, doc);
    // printf("PDF has %d pages\n", pages_c);

    // get document colorspace

    fz_try(pdf->ctx) {
        pdf->colorspace = fz_document_output_intent(pdf->ctx, pdf->doc);
    }
    fz_catch(pdf->ctx) {
        fprintf(stderr, "could not get output intent of %s\n", file_path);
        fz_drop_document(pdf->ctx, pdf->doc);
        fz_drop_context(pdf->ctx);
        return -1;
    }

    // fz_page *page = fz_load_page(ctx, doc, 0);
    // if(page == NULL) {
        // fprintf(stderr, "could not load page!\n");
        // fz_drop_pixmap(ctx, pixmap);
        // fz_drop_colorspace(ctx, colorspace);
        // fz_drop_document(ctx, doc);
        // fz_drop_context(ctx);
        // return -1;
    // }

    // fz_rect rect = fz_bound_page(ctx, page);
    // printf("x0 = %.7f, y0 = %.7f, x1 = %.7f, y1 = %.7f\n", rect.x0, rect.y0, rect.x1, rect.y1);

    return 0;
}

void close_document(document *pdf) {
    // fz_drop_page(ctx, page);
    // fz_drop_pixmap(pdf->ctx, pdf->pixmap);
    fz_drop_colorspace(pdf->ctx, pdf->colorspace);
    fz_drop_document(pdf->ctx, pdf->doc);
    fz_drop_context(pdf->ctx);
}
