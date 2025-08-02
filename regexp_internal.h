/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 *
 *	$NetBSD: regexp.h,v 1.3 1997/10/09 10:21:21 lukem Exp $
 *
 * Modified for static linking - removed SAS/C register keywords
 */

#ifndef REGEXP_INTERNAL_H
#define REGEXP_INTERNAL_H

/* Standard C89 types */
typedef char *STRPTR;
typedef unsigned char UBYTE;
typedef long LONG;
typedef int BOOL;

/* Boolean constants */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Error numbers */
#define REXPERR_NULLARG -1
#define REXPERR_REXPTOOBIG -2
#define REXPERR_TOOMANYPARENS -3
#define REXPERR_UNMATCHEDPARENS -4
#define REXPERR_JUNKONEND -5
#define REXPERR_REPKLEENECBE -6
#define REXPERR_NESTEDPOSTFIXOP -7
#define REXPERR_INVALIDRANGE -8
#define REXPERR_UNMATCHEDRNGBRKT -9
#define REXPERR_INTERNALURP -10
#define REXPERR_PFOPFOLLOWSNOTHING -11
#define REXPERR_TRAILINGBKSLASH -12
#define REXPERR_CORRUPTEDPROG -13
#define REXPERR_CORRUPTEDMEM -14
#define REXPERR_CORRUPTEDPTRS -15
#define REXPERR_INTERNALFOULUP -16

/* Number of subexpressions */
#define NSUBEXP 10

/* Regexp structure */
typedef struct regexp {
    STRPTR startp[NSUBEXP];
    STRPTR endp[NSUBEXP];
    UBYTE regstart;		/* Internal use only. */
    UBYTE reganch;		/* Internal use only. */
    STRPTR regmust;		/* Internal use only. */
    LONG regmlen;		/* Internal use only. */
    UBYTE program[1];		/* Unwarranted chumminess with compiler. */
} regexp;

/* Function prototypes for static linking */
regexp *RegComp(char *exp);
void RegFree(regexp *re);
long RegExec(regexp *prog, char *string);
char *RegXlatError(long err);

#endif /* REGEXP_INTERNAL_H */
