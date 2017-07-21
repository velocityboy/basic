#include <ctype.h>
#include <stdio.h>
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

/* in place remove matching quotes if the string is quoted
 */
void strunquote(char *s)
{
    size_t len = strlen(s);
    
    if (len < 2) {
        return;
    }
    
    if (s[0] == '"' && s[len - 1] == '"') {
        char *p;
        for (p = s; p[2]; p++) {
            p[0] = p[1];
        }
        *p = '\0';
    }
}
