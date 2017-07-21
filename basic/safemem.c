#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safemem.h"

/* Common out of memory hook
 */
static void out_of_memory()
{
    fprintf(stderr, "Out of memory\n");
    exit(1);
}

/* calloc() but cannot return NULL
 */
void *safe_calloc(size_t count, size_t size)
{
    void *p = calloc(count, size);
    if (p == NULL) {
        out_of_memory();
    }
    return p;
}

/* malloc() but cannot return NULL
 */
void *safe_malloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        out_of_memory();
    }
    return p;
}

/* realloc() but cannot return NULL
 */
void *safe_realloc(void *buffer, size_t new_size)
{
    void *p = realloc(buffer, new_size);
    if (!p) {
        out_of_memory();
    }
    return p;
}

/* strdup() but cannot return NULL
 */
char *safe_strdup(const char *str)
{
    char *s = strdup(str);
    if (s == NULL) {
        out_of_memory();
    }
    return s;
}
