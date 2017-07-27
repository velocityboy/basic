#ifndef output_h
#define output_h

typedef struct output output;

extern output *output_alloc();
extern void output_free(output *out);
extern void output_set_col(output *out, int col);
extern void output_print(output *out, const char *fmt, ...);

#endif /* output_h */
