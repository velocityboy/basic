#ifndef parser_h
#define parser_h

#include <stdio.h>

typedef struct program program;
typedef struct parser parser;

extern parser *parser_alloc();
extern void parser_free(parser *p);
extern int parser_parse_file(parser *prs, FILE *fp, program *pgm);

#endif /* parser_h */
