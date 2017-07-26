#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "stringutil.h"

/* In place upper string
 */
void strupr(char *s)
{
    while (*s) {
        *s = toupper(*s);
        s++;
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

/* Pass the format string through strftime with the current
 * local time
 */
void strformattime(const char *fmt, char *out, int outlen)
{
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    
    strftime(out, outlen, fmt, tm);
}

