#ifndef program_h
#define program_h

typedef struct program program;
typedef struct statement statement;

struct program
{
  statement *head;
  statement *tail;
};

extern program *program_alloc();
extern void program_free(program *pgm);
extern void program_new(program *pgm);
extern void program_insert_statement(program *pgm, statement *stmt);

#endif /* program_h */
