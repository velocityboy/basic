#include <ctype.h>
#include <string.h>

#include "stringutil.h"

/* in place upper case a string
 */
void strupper(char *s)
{
    for (; *s; s++) {
        *s = toupper(*s);
    }
}

/* in place remove leading and trailing whitespace
 */
void strtrim(char *s)
{
    char *p = s + strlen(s) - 1;

    /* trim trailing space 
     */
    while (p >= s && isspace(*p)) {
        p--;
    }
    *++p = '\0';
    
    /* trim leading space
     */
    char *d = s;
    while (*s && isspace(*s)) {
        s++;
    }
    
    if (d < s) {
        while (*s) {
            *d++ = *s++;
        }
        *d++ = '\0';
    }
}


