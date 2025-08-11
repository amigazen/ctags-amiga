#ifndef REGEX_H
#define REGEX_H

/* Static regex implementation based on BSD regexp */
/* No external library dependencies */

#include <string.h>
#include "regexp_internal.h"

/* POSIX regex constants */
#define REG_EXTENDED    1
#define REG_NEWLINE     2
#define REG_NOSUB       4
#define REG_ICASE       8

/* POSIX-style wrapper functions for ctags compatibility */
typedef struct {
    regexp *prog;
} regex_t;

typedef struct {
    int rm_so;  /* start of match */
    int rm_eo;  /* end of match */
} regmatch_t;

/* Function prototypes for POSIX compatibility */
int regcomp(regex_t *preg, const char *pattern, int cflags);
int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
void regfree(regex_t *preg);
size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);

/* Implementation */
int regcomp(regex_t *preg, const char *pattern, int cflags)
{
    /* Ignore REG_EXTENDED flag since our implementation is basic */
    /* Also ignore REG_NEWLINE and REG_ICASE for now */
    preg->prog = RegComp((char*)pattern);
    return (preg->prog != NULL) ? 0 : 1;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags)
{
    long result;
    int i;
    
    if (!preg->prog) return 1;
    
    result = RegExec(preg->prog, (char*)string);
    if (result == 0) {
        /* Match found - fill in pmatch if provided */
        if (pmatch && nmatch > 0) {
            /* Fill in all requested matches */
            for (i = 0; i < nmatch && i < NSUBEXP; i++) {
                if (preg->prog->startp[i] && preg->prog->endp[i]) {
                    pmatch[i].rm_so = preg->prog->startp[i] - string;
                    pmatch[i].rm_eo = preg->prog->endp[i] - string;
                } else {
                    pmatch[i].rm_so = -1;
                    pmatch[i].rm_eo = -1;
                }
            }
            /* Fill remaining matches as unused */
            for (; i < nmatch; i++) {
                pmatch[i].rm_so = -1;
                pmatch[i].rm_eo = -1;
            }
        }
        return 0;
    }
    return 1;  /* No match */
}

void regfree(regex_t *preg)
{
    if (preg->prog) {
        RegFree(preg->prog);
        preg->prog = NULL;
    }
}

size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
    const char *errmsg = "Unknown regex error";
    
    if (errcode == 0) {
        errmsg = "No error";
    } else if (errcode == 1) {
        errmsg = "Compilation failed";
    }
    
    if (errbuf && errbuf_size > 0) {
        strncpy(errbuf, errmsg, errbuf_size - 1);
        errbuf[errbuf_size - 1] = '\0';
        return strlen(errbuf);
    }
    
    return strlen(errmsg);
}

#endif /* REGEX_H */
