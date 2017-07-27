#include <stdio.h>
#include <stdarg.h>

#include "output.h"
#include "safemem.h"

static const int TAB_SIZE = 8;

struct output
{
    int col;
    int buflen;
    char *buffer;
};

static void output_tab(output *out);

/* Allocate and oupput stream
 */
output *output_alloc()
{
    output *out = safe_calloc(1, sizeof(output));
    out->buflen = 80;
    out->buffer = safe_calloc(out->buflen, 1);
    return out;
}

/* Free an output stream
 */
void output_free(output *out)
{
    if (out) {
        free(out->buffer);
    }
    free(out);
}

/* Directly set the column
 */
void output_set_col(output *out, int col)
{
    out->col = col;
}

/* Output a string, counting columns
 */
void output_print(output *out, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(out->buffer, out->buflen, fmt, args);
    va_end(args);
    
    if (n >= out->buflen) {
        out->buflen = n + 1;
        out->buffer = safe_realloc(out->buffer, out->buflen);
    }

    va_start(args, fmt);
    vsnprintf(out->buffer, out->buflen, fmt, args);
    va_end(args);
    
    for (const char *p = out->buffer; *p; p++) {
        switch (*p) {
        case '\n':
            putchar('\r');
            putchar('\n');
            out->col = 0;
            break;
            
        case '\r':
            putchar('\r');
            out->col = 0;
            break;
            
        case '\b':
        case 127:
            putchar(*p);
            if (out->col) {
                out->col--;
            }
            break;
            
        case '\t':
            output_tab(out);
            continue;
            
        default:
            out->col++;
            putchar(*p);
            break;
        }
    }
}

/* output a tab character as spaces 
 */
void output_tab(output *out)
{
    int n = out->col % TAB_SIZE;
    int spaces = TAB_SIZE - n;
    
    out->col += spaces;
    while (spaces--) {
        putchar(' ');
    }
}

