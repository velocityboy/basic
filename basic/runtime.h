#ifndef runtime_h
#define runtime_h

typedef struct program program;
typedef struct runtime runtime;

runtime *runtime_alloc();
void runtime_free(runtime *rt);
void runtime_run(runtime *rt, program *pgm);


#endif /* runtime_h */
