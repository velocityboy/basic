#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef struct userent userent;

const int MAX_PASSWD = 8;
const int MAX_UID = 999999999;

struct userent
{
    int uid;
    char passwd[MAX_PASSWD + 1];
    char home[PATH_MAX];
    char shell[PATH_MAX];
};

static jmp_buf restart;

static void rtrim(char *buf);

int parse_userent(char *line, userent *ent)
{
    char *p = strchr(line, ',');
    if (!p) {
        return -1;
    }
    
    *p = '\0';
    char *q;
    
    long uid = strtol(line, &q, 10);
    if ((uid == 0 && errno == EINVAL) || q != p) {
        return -1;
    }
    
    if (uid > MAX_UID) {
        return -1;
    }
    
    ent->uid = (int)uid;
    
    p++;
    q = strchr(p, ',');
    
    if (!q || (q - p) >= sizeof(ent->passwd)) {
        return -1;
    }
    
    *q++ = '\0';
    
    strncpy(ent->passwd, p, sizeof(ent->passwd));
    
    p = q;
    q = strchr(p, ',');
    
    if (!q || (q - p) >= sizeof(ent->home)) {
        return -1;
    }

    *q++ = '\0';
    
    strncpy(ent->home, p, sizeof(ent->home));
    
    if (strlen(q) >= sizeof(ent->shell)) {
        return -1;
    }
    
    strncpy(ent->shell, q, sizeof(ent->shell));
    
    return 0;
}

int find_userent(const char *root, int uid, userent *ent)
{
    char fn[PATH_MAX];
    snprintf(fn, sizeof(fn), "%s/etc/passwd", root);
    
    FILE *fp = fopen(fn, "r");
    if (!fp) {
        return -1;
    }
    
    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        rtrim(line);
        if (parse_userent(line, ent) == 0) {
            if (ent->uid == uid) {
                fclose(fp);
                return 0;
            }
        }
    }
    
    fclose(fp);
    
    return -1;
}

void banner(const char *root)
{
    char fn[PATH_MAX];
    char line[200];
    char line2[200];
    snprintf(fn, sizeof(fn), "%s/etc/banner", root);
    
    FILE *fp = fopen(fn, "r");
    if (!fp) {
        return;
    }
    
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    
    
    while (fgets(line, sizeof(line), fp)) {
        if (strchr(line, '%')) {
            strftime(line2, sizeof(line2), line, tm);
            printf("%s", line2);
        } else {
            printf("%s", line);
        }
    }
    
    fclose(fp);
}

void find_root(const char *pgm, char *path)
{
    const char *slash = strrchr(pgm, '/');
    if (slash == NULL) {
        pgm = "./";
        slash = &pgm[1];
    }
    
    ptrdiff_t n = slash - pgm;
    assert(n < PATH_MAX);
    
    char pgmpath[PATH_MAX];
    strncpy(pgmpath, pgm, n);
    pgmpath[n] = '\0';
    
    strcat(pgmpath, "/..");
    
    realpath(pgmpath, path);
}

static void rtrim(char *buf)
{
    size_t n = strlen(buf);
    
    while (n && isspace(buf[n-1])) {
        --n;
    }
    
    buf[n] = '\0';
}

static int isalldigits(char *buf)
{
    for (char *p = buf; *p; p++) {
        if (!isdigit(*p)) {
            return 0;
        }
    }
    
    return 1;
}

int getuser()
{
    char buf[20];
    while (1) {
        printf("USERID: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            exit(1);
        }
        
        rtrim(buf);
        
        int user = 0;
        if (buf[0] == '\0' || !isalldigits(buf) || sscanf(buf, "%d", &user) == 0) {
            printf("INVALID USERID\n");
            continue;
        }
        
        return user;
    }
}

void getpasswd(char *pwbuf, int len)
{
    static const char *mask = "@#$%X*MP@#$%X*MP";
    printf("PASSWORD:\n");
    
    for (int i = 0; i < 3; i++) {
        printf("%8.8s\r", mask + 2 * i);
    }
    
    fgets(pwbuf, len, stdin);
    rtrim(pwbuf);
}

void sigint()
{
    longjmp(restart, 1);
}

int main(int argc, const char * argv[])
{
    char root[PATH_MAX];
    find_root(argv[0], root);
        
    banner(root);
    
    struct termios termios;
    tcgetattr(0, &termios);
    termios.c_cc[VEOF] = 0xff;
    tcsetattr(0, TCSANOW, &termios);

    setjmp(restart);
    signal(SIGINT, &sigint);
    printf("\n");
    
    while (1) {
        int uid = getuser();
        char pw[MAX_PASSWD + 1];
        getpasswd(pw, sizeof(pw));
        
        userent ent;
        if (find_userent(root, uid, &ent) == -1 ||
            strcmp(pw, ent.passwd) != 0) {
            printf("INVALID USERID/PASSWORD COMBINATION\n");
            continue;
        }
        
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s%s", root, ent.home);
        if (chdir(path) == -1) {
            printf("WORKSPACE %d COULD NOT BE OPENED\n", uid);
            continue;
        }
        
        snprintf(path, sizeof(path), "%s%s", root, ent.shell);

        if (execl(path, path, NULL) == -1) {
            printf("FAILED TO RUN INTERPRETER");
            continue;
        }
    }
    
}
