#ifndef safemem_h
#define safemem_h

#include <stdlib.h>

extern void *safe_calloc(size_t count, size_t size);
extern void *safe_realloc(void *buffer, size_t new_size);
extern char *safe_strdup(const char *str);

#endif /* safemem_h */
