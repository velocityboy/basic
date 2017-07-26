#ifndef stringutil_h
#define stringutil_h

extern void strupr(char *s);
extern void strtrim(char *s);
extern void strunquote(char *s);
extern void strformattime(const char *fmt, char *out, int outlen);

#endif /* stringutil_h */
