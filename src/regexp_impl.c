/*
 * Core regex implementation functions for static linking
 * Based on BSD regexp implementation
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "regexp_internal.h"
#include <stdio.h>

/* Dummy variable for two-pass compilation */
static unsigned char regdummy;

/* Boolean constants if not already defined */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Magic number for regex programs */
#define MAGIC 0234

/* Opcodes */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	OPEN	20	/* no	Mark this point in input as start of #n. */
#define	CLOSE	30	/* no	Analogous to OPEN. */

/* Node access macros */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*(p+1)&0377)<<8)+(*(p+2)&0377))
#define	OPERAND(p)	((p)+3)

/* Utility macros */
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#define	FAIL(m)	{ regerror(m); return(NULL); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')

/* Flags */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Global work variables for regcomp().
 */
typedef struct RegcompGlobals
{
	char *regparse;	/* Input-scan pointer. */
	long regnpar;		/* () count. */
	char *regcode;	/* Code-emit pointer; &regdummy = don't. */
	long regsize;		/* Code size. */
} rc_globals;

/*
 * Global work variables for regexec().
 */
typedef struct RegexecGlobals
{
	char *reginput;		/* String-input pointer. */
	char *regbol;			/* Beginning of input, for ^ check. */
	char **regstartp;	/* Pointer to startp array. */
	char **regendp;		/* Ditto for endp. */
} re_globals;

/* Forward declarations */
static char *reg(rc_globals *rcglobs, long paren, long *flagp);
static char *regbranch(rc_globals *rcglobs, long *flagp);
static char *regpiece(rc_globals *rcglobs, long *flagp);
static char *regatom(rc_globals *rcglobs, long *flagp);
static char *regnode(rc_globals *rcglobs, unsigned char op);
static char *regnext(char *p);
static void regc(rc_globals *rcglobs, unsigned char b);
static void reginsert(rc_globals *rcglobs, unsigned char op, char *opnd);
static void regtail(rc_globals *rcglobs, char *p, char *val);
static void regoptail(rc_globals *rcglobs, char *p, char *val);
static int regtry(re_globals *reglobs, const regexp *prog, char *string);
static int regmatch(re_globals *reglobs, char *prog);
static long regrepeat(re_globals *reglobs, char *p);

/* Simple error function */
static void regerror(const char *msg)
{
    /* For static linking, we'll just ignore errors */
    (void)msg;
}

/* Free function for static linking */
void RegFree(regexp *re) 
{ 
    if (re) free(re); 
}

/* Compile function for static linking */
regexp *RegComp(char *exp)
{
    rc_globals rcglobs;
    regexp *r;
    char *scan;
    char *longest;
    long len;
    long flags;
    
    if (exp == NULL)
        FAIL("NULL argument");
    
    /* First pass: determine size, legality. */
    rcglobs.regparse = exp;
    rcglobs.regnpar = 1;
    rcglobs.regsize = 0L;
    rcglobs.regcode = &regdummy;  /* Use &regdummy for first pass */
    regc(&rcglobs, MAGIC);
    if (reg(&rcglobs, 0, &flags) == NULL) {
        return(NULL);
    }
    
    /* Small enough for pointer-storage convention? */
    if (rcglobs.regsize >= 32767L)	/* Probably could be 65535L. */
        FAIL("regexp too big");
    
    /* Allocate space. */
    r = (regexp *)malloc(sizeof(regexp) + (unsigned)rcglobs.regsize);
    if (r == NULL)
        FAIL("out of space");
    
    /* Second pass: emit code. */
    rcglobs.regparse = exp;
    rcglobs.regnpar = 1;
    rcglobs.regcode = r->program;
    regc(&rcglobs, MAGIC);
    if (reg(&rcglobs, 0, &flags) == NULL)
        return(NULL);
    
    /* Dig out information for optimizations. */
    r->regstart = '\0';	/* Worst-case defaults. */
    r->reganch = 0;
    r->regmust = NULL;
    r->regmlen = 0;
    scan = r->program+1;			/* First BRANCH. */
    if (OP(regnext(scan)) == END) {		/* Only one top-level choice. */
        scan = OPERAND(scan);
        
        /* Starting-point info. */
        if (OP(scan) == EXACTLY)
            r->regstart = *OPERAND(scan);
        else if (OP(scan) == BOL)
            r->reganch++;
        
        /*
         * If there's something expensive in the r.e., find the
         * longest literal string that must appear and make it the
         * regmust.  Resolve ties in favor of later strings, since
         * the regstart check works with the beginning of the r.e.
         * and avoiding duplication strengthens checking.  Not a
         * strong reason, but sufficient in the absence of others.
         */
        if (flags&SPSTART) {
            longest = NULL;
            len = 0;
            for (; scan != NULL; scan = regnext(scan))
                if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
                    longest = OPERAND(scan);
                    len = strlen(OPERAND(scan));
                }
            r->regmust = longest;
            r->regmlen = len;
        }
    }
    
    return(r);
}

/* Execute function for static linking */
long RegExec(regexp *prog, char *string)
{
    re_globals reglobs;
    char *s;
    
    /* Be paranoid... */
    if (prog == NULL || string == NULL) {
        return(1);
    }
    
    /* Check validity of program. */
    if (UCHARAT(prog->program) != MAGIC) {
        return(1);
    }
    
    /* If there is a "must appear" string, look for it. */
    if (prog->regmust != NULL) {
        s = string;
        while ((s = strchr(s, prog->regmust[0])) != NULL) {
            if (strncmp(s, prog->regmust, prog->regmlen) == 0)
                break;	/* Found it. */
            s++;
        }
        if (s == NULL)	/* Not present. */
            return(1);
    }
    
    /* Mark beginning of line for ^ . */
    reglobs.regbol = string;
    
    /* Simplest case:  anchored match need be tried only once. */
    if (prog->reganch)
        return(regtry(&reglobs, prog, string));
    
    /* Messy cases:  unanchored match. */
    s = string;
    if (prog->regstart)
        while ((s = strchr(s, prog->regstart)) != NULL) {
            if (regtry(&reglobs, prog, s))
                return(0);
            s++;
        }
    else
        do {
            if (regtry(&reglobs, prog, s))
                return(0);
        } while (*s++ != '\0');
    
    /* Failure. */
    return(1);
}

/* Error translation function */
char *RegXlatError(long err)
{
    static char *errors[] = {
        "NULL argument",
        "Regexp too big",
        "Too many parentheses",
        "Unmatched parentheses",
        "Junk on end",
        "Repetition/Kleene (+*) could be empty",
        "Nested postfix (+*?) operators",
        "Invalid range",
        "Unmatched range bracket",
        "Internal urp",
        "Postfix operator (+*?) follows nothing",
        "Trailing backslash",
        "Corrupted program",
        "Corrupted memory",
        "Corrupted pointers",
        "Internal foulup"
    };
    
    if (err >= -16 && err <= -1)
        return errors[-err - 1];
    return "Unknown error";
}

/* Emit a node */
static char *regnode(rc_globals *rcglobs, unsigned char op)
{
    char *ret;
    char *ptr;

    ret = rcglobs->regcode;
    if (ret == &regdummy) {
        rcglobs->regsize += 3;
        return(ret);
    }

    ptr = ret;
    *ptr++ = op;
    *ptr++ = '\0';		/* Null "next" pointer. */
    *ptr++ = '\0';
    rcglobs->regcode = ptr;

    return(ret);
}

/* Emit (if appropriate) a byte of code */
static void regc(rc_globals *rcglobs, unsigned char b)
{
    if (rcglobs->regcode != &regdummy)
        *(rcglobs->regcode)++ = b;
    else
        rcglobs->regsize++;
}

/* Insert an operator in front of already-emitted operand */
static void reginsert(rc_globals *rcglobs, unsigned char op, char *opnd)
{
    char *src, *dst, *place;

    if (rcglobs->regcode == &regdummy) {
        rcglobs->regsize += 3;
        return;
    }

    src = rcglobs->regcode;
    rcglobs->regcode += 3;
    dst = rcglobs->regcode;
    while (src > opnd)
        *--dst = *--src;

    place = opnd;		/* Op node, where operand used to be. */
    *place++ = op;
    *place++ = '\0';
    *place++ = '\0';
}

/* Set the next-pointer at the end of a node chain */
static void regtail(rc_globals *rcglobs, char *p, char *val)
{
    char *scan, *temp;
    long offset;

    if (p == &regdummy)
        return;

    /* Find last node. */
    scan = p;
    for (;;) {
        temp = regnext(scan);
        if (temp == NULL)
            break;
        scan = temp;
    }

    if (OP(scan) == BACK)
        offset = scan - val;
    else
        offset = val - scan;
    *(scan+1) = (offset>>8)&0377;
    *(scan+2) = offset&0377;
}

/* regtail on operand of first argument; nop if operandless */
static void regoptail(rc_globals *rcglobs, char *p, char *val)
{
    /* "Operandless" and "op != BRANCH" are synonymous in practice. */
    if (p == NULL || p == &regdummy || OP(p) != BRANCH)
        return;
    regtail(rcglobs, OPERAND(p), val);
}

/* Dig the "next" pointer out of a node */
static char *regnext(char *p)
{
    long offset;

    if (p == &regdummy)
        return(NULL);

    offset = NEXT(p);
    if (offset == 0)
        return(NULL);

    if (OP(p) == BACK)
        return(p-offset);
    else
        return(p+offset);
}

/* Regular expression, i.e. main body or parenthesized thing */
static char *reg(rc_globals *rcglobs, long paren, long *flagp)
{
    char *ret, *br, *ender;
    long parno = 0;
    long flags;

    *flagp = HASWIDTH;	/* Tentatively. */

    /* Make an OPEN node, if parenthesized. */
    if (paren) {
        if (rcglobs->regnpar >= NSUBEXP)
            FAIL("Too many parentheses");
        parno = rcglobs->regnpar;
        rcglobs->regnpar++;
        ret = regnode(rcglobs, OPEN+parno);
    } else
        ret = NULL;

    /* Pick up the branches, linking them together. */
    br = regbranch(rcglobs, &flags);
    if (br == NULL)
        return(NULL);
    if (ret != NULL)
        regtail(rcglobs, ret, br);	/* OPEN -> first. */
    else
        ret = br;
    if (!(flags&HASWIDTH))
        *flagp &= ~HASWIDTH;
    *flagp |= flags&SPSTART;
    while (*rcglobs->regparse == '|' || *rcglobs->regparse == '\n') {
        rcglobs->regparse++;
        br = regbranch(rcglobs, &flags);
        if (br == NULL)
            return(NULL);
        regtail(rcglobs, ret, br);	/* BRANCH -> BRANCH. */
        if (!(flags&HASWIDTH))
            *flagp &= ~HASWIDTH;
        *flagp |= flags&SPSTART;
    }

    /* Make a closing node, and hook it on the end. */
    ender = regnode(rcglobs, (paren) ? CLOSE+parno : END);	
    regtail(rcglobs, ret, ender);

    /* Hook the tails of the branches to the closing node. */
    for (br = ret; br != NULL; br = regnext(br))
        regoptail(rcglobs, br, ender);

    /* Check for proper termination. */
    if (paren && *rcglobs->regparse++ != ')') {
        FAIL("Unmatched parentheses");
    } else if (!paren && *rcglobs->regparse != '\0') {
        if (*rcglobs->regparse == ')') {
            FAIL("Unmatched parentheses");
        } else
            FAIL("Junk on end");	/* "Can't happen". */
    }

    return(ret);
}

/* One alternative of an | operator */
static char *regbranch(rc_globals *rcglobs, long *flagp)
{
    char *ret, *chain, *latest;
    long flags;

    *flagp = WORST;		/* Tentatively. */

    ret = regnode(rcglobs, BRANCH);
    chain = NULL;
    while (*rcglobs->regparse != '\0' && *rcglobs->regparse != ')' &&
           *rcglobs->regparse != '\n' && *rcglobs->regparse != '|') {
        latest = regpiece(rcglobs, &flags);
        if (latest == NULL)
            return(NULL);
        *flagp |= flags&HASWIDTH;
        if (chain == NULL)	/* First piece. */
            *flagp |= flags&SPSTART;
        else
            regtail(rcglobs, chain, latest);
        chain = latest;
    }
    if (chain == NULL)	/* Loop ran zero times. */
        (void) regnode(rcglobs, NOTHING);

    return(ret);
}

/* Something followed by possible [*+?] */
static char *regpiece(rc_globals *rcglobs, long *flagp)
{
    char *ret, *next;
    unsigned char op;
    long flags;

    ret = regatom(rcglobs, &flags);
    if (ret == NULL)
        return(NULL);

    op = *rcglobs->regparse;
    if (!ISMULT(op)) {
        *flagp = flags;
        return(ret);
    }

    if (!(flags&HASWIDTH) && op != '?')
        FAIL("Repetition/Kleene (+*) could be empty");
    *flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

    if (op == '*' && (flags&SIMPLE))
        reginsert(rcglobs, STAR, ret);
    else if (op == '*') {
        /* Emit x* as (x&|), where & means "self". */
        reginsert(rcglobs, BRANCH, ret);			/* Either x */
        regoptail(rcglobs, ret, regnode(rcglobs, BACK));		/* and loop */
        regoptail(rcglobs, ret, ret);			/* back */
        regtail(rcglobs, ret, regnode(rcglobs, BRANCH));		/* or */
        regtail(rcglobs, ret, regnode(rcglobs, NOTHING));		/* null. */
    } else if (op == '+' && (flags&SIMPLE))
        reginsert(rcglobs, PLUS, ret);
    else if (op == '+') {
        /* Emit x+ as x(&|), where & means "self". */
        next = regnode(rcglobs, BRANCH);			/* Either */
        regtail(rcglobs, ret, next);
        regtail(rcglobs, regnode(rcglobs, BACK), ret);		/* loop back */
        regtail(rcglobs, next, regnode(rcglobs, BRANCH));		/* or */
        regtail(rcglobs, ret, regnode(rcglobs, NOTHING));		/* null. */
    } else if (op == '?') {
        /* Emit x? as (x|) */
        reginsert(rcglobs, BRANCH, ret);			/* Either x */
        regtail(rcglobs, ret, regnode(rcglobs, BRANCH));		/* or */
        next = regnode(rcglobs, NOTHING);		/* null. */
        regtail(rcglobs, ret, next);
        regoptail(rcglobs, ret, next);
    }
    rcglobs->regparse++;
    if (ISMULT(*rcglobs->regparse))
        FAIL("Nested postfix operators");

    return(ret);
}

/* The lowest level */
static char *regatom(rc_globals *rcglobs, long *flagp)
{
    char *ret;
    long flags;

    *flagp = WORST;		/* Tentatively. */

    switch (*rcglobs->regparse++) {
    case '^':
        ret = regnode(rcglobs, BOL);
        break;
    case '$':
        ret = regnode(rcglobs, EOL);
        break;
    case '.':
        ret = regnode(rcglobs, ANY);
        *flagp |= HASWIDTH|SIMPLE;
        break;
    case '[': {
            long class, classend;

            if (*rcglobs->regparse == '^') {	/* Complement of range. */
                ret = regnode(rcglobs, ANYBUT);
                rcglobs->regparse++;
            } else
                ret = regnode(rcglobs, ANYOF);
            if (*rcglobs->regparse == ']' || *rcglobs->regparse == '-')
                regc(rcglobs, *rcglobs->regparse++);
            while (*rcglobs->regparse != '\0' && *rcglobs->regparse != ']') {
                if (*rcglobs->regparse == '-') {
                    rcglobs->regparse++;
                    if (*rcglobs->regparse == ']' || *rcglobs->regparse == '\0')
                        regc(rcglobs, '-');
                    else {
                        class = UCHARAT(rcglobs->regparse-2)+1;
                        classend = UCHARAT(rcglobs->regparse);
                        if (class > classend+1)
                            FAIL("Invalid range");
                        for (; class <= classend; class++)
                            regc(rcglobs, class);
                        rcglobs->regparse++;
                    }
                } else
                    regc(rcglobs, *rcglobs->regparse++);
            }
            regc(rcglobs, '\0');
            if (*rcglobs->regparse != ']')
                FAIL("Unmatched range bracket");
            rcglobs->regparse++;
            *flagp |= HASWIDTH|SIMPLE;
        }
        break;
    case '(':
        ret = reg(rcglobs, 1, &flags);
        if (ret == NULL)
            return(NULL);
        *flagp |= flags&(HASWIDTH|SPSTART);
        break;
    case '\0':
    case '|':
    case '\n':
    case ')':
        FAIL("Internal urp");	/* Supposed to be caught earlier. */
        break;
    case '?':
    case '+':
    case '*':
        FAIL("Postfix operator (+*?) follows nothing");
        break;
    case '\\':
        switch (*rcglobs->regparse++) {
        case '\0':
            FAIL("Trailing backslash");
            break;
        default:
            /* Handle general quoted chars in exact-match routine */
            goto de_fault;
        }
        break;
    de_fault:
    default:
        /* Encode a string of characters to be matched exactly. */
        {
            char *regprev;
            unsigned char ch;

            rcglobs->regparse--;			/* Look at cur char */
            ret = regnode(rcglobs, EXACTLY);
            for (regprev = 0; ; ) {
                ch = *rcglobs->regparse++;	/* Get current char */
                switch (*rcglobs->regparse) {	/* look at next one */
                default:
                    regc(rcglobs, ch);	/* Add cur to string */
                    break;
                case '.': case '[': case '(':
                case ')': case '|': case '\n':
                case '$': case '^':
                case '\0':
                magic:
                    regc(rcglobs, ch);	/* dump cur char */
                    goto done;	/* and we are done */
                case '?': case '+': case '*':
                    if (!regprev) 	/* If just ch in str, */
                        goto magic;	/* use it */
                    /* End mult-char string one early */
                    rcglobs->regparse = regprev; /* Back up parse */
                    goto done;
                case '\\':
                    regc(rcglobs, ch);	/* Cur char OK */
                    switch (rcglobs->regparse[1]){ /* Look after \ */
                    case '\0':
                        goto done; /* Not quoted */
                    default:
                        /* Backup point is \, scan point is after it. */
                        regprev = rcglobs->regparse;
                        rcglobs->regparse++; 
                        continue;	/* NOT break; */
                    }
                }
                regprev = rcglobs->regparse;	/* Set backup point */
            }
        done:
            regc(rcglobs, '\0');
            *flagp |= HASWIDTH;
            if (!regprev)		/* One char? */
                *flagp |= SIMPLE;
        }
        break;
    }

    return(ret);
}

/* Try match at specific point */
static int regtry(re_globals *reglobs, const regexp *prog, char *string)
{
    long i;
    char **sp;
    char **ep;

    reglobs->reginput = string;
    reglobs->regstartp = (char**)prog->startp;
    reglobs->regendp = (char**)prog->endp;

    sp = (char**)prog->startp;
    ep = (char**)prog->endp;
    for (i = NSUBEXP; i > 0; i--) {
        *sp++ = NULL;
        *ep++ = NULL;
    }
    if (regmatch(reglobs, (char*)prog->program + 1)) {
        ((regexp *)prog)->startp[0] = string;
        ((regexp *)prog)->endp[0] = reglobs->reginput;
        return TRUE;
    } else
        return FALSE;
}

/* Main matching routine */
static int regmatch(re_globals *reglobs, char *prog)
{
    unsigned char *scan;	/* Current node. */
    unsigned char *next;		/* Next node. */

    scan = prog;
    while (scan != NULL) {
        next = regnext(scan);

        switch (OP(scan)) {
        case BOL:
            if (reglobs->reginput != reglobs->regbol)
                return FALSE;
            break;
        case EOL:
            if (*reglobs->reginput != '\0')
                return FALSE;
            break;
        case ANY:
            if (*reglobs->reginput == '\0')
                return FALSE;
            reglobs->reginput++;
            break;
        case EXACTLY: {
                long len;
                unsigned char *opnd;

                opnd = OPERAND(scan);
                /* Inline the first character, for speed. */
                if (*opnd != *reglobs->reginput)
                    return FALSE;
                len = strlen(opnd);
                if (len > 1 && strncmp(opnd, reglobs->reginput, len) != 0)
                    return FALSE;
                reglobs->reginput += len;
            }
            break;
        case ANYOF:
            if (*reglobs->reginput == '\0' || strchr(OPERAND(scan), *reglobs->reginput) == NULL)
                return FALSE;
            reglobs->reginput++;
            break;
        case ANYBUT:
            if (*reglobs->reginput == '\0' || strchr(OPERAND(scan), *reglobs->reginput) != NULL)
                return FALSE;
            reglobs->reginput++;
            break;
        case NOTHING:
            break;
        case BACK:
            break;
        case BRANCH: {
                char *save;

                if (OP(next) != BRANCH)		/* No choice. */
                    next = OPERAND(scan);	/* Avoid recursion. */
                else {
                    do {
                        save = reglobs->reginput;
                        if (regmatch(reglobs, OPERAND(scan)))
                            return TRUE;
                        reglobs->reginput = save;
                        scan = regnext(scan);
                    } while (scan != NULL && OP(scan) == BRANCH);
                    return FALSE;
                }
            }
            break;
        case STAR:
        case PLUS: {
                unsigned char nextch;
                long no;
                char *save;
                long min;

                /*
                 * Lookahead to avoid useless match attempts
                 * when we know what character comes next.
                 */
                nextch = '\0';
                if (OP(next) == EXACTLY)
                    nextch = *OPERAND(next);
                min = (OP(scan) == STAR) ? 0 : 1;
                save = reglobs->reginput;
                no = regrepeat(reglobs, OPERAND(scan));
                while (no >= min) {
                    /* If it could work, try it. */
                    if (nextch == '\0' || *reglobs->reginput == nextch)
                        if (regmatch(reglobs, next))
                            return TRUE;
                    /* Couldn't or didn't -- back up. */
                    no--;
                    reglobs->reginput = save + no;
                }
                return FALSE;
            }
            break;
        case END:
            return TRUE;	/* Success! */
            break;
        default:
            return FALSE;
            break;
        }

        scan = next;
    }

    /*
     * We get here only if there's trouble -- normally "case END" is
     * the terminating point.
     */
    return FALSE;
}

/* Repeatedly match something simple, report how many */
static long regrepeat(re_globals *reglobs, char *p)
{
    long count = 0;
    char *scan;
    char *opnd;

    scan = reglobs->reginput;
    opnd = OPERAND(p);
    switch (OP(p)) {
    case ANY:
        count = strlen(scan);
        scan += count;
        break;
    case EXACTLY:
        while (*opnd == *scan) {
            count++;
            scan++;
        }
        break;
    case ANYOF:
        while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
            count++;
            scan++;
        }
        break;
    case ANYBUT:
        while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
            count++;
            scan++;
        }
        break;
    default:		/* Oh dear.  Called inappropriately. */
        count = 0;	/* Best compromise. */
        break;
    }
    reglobs->reginput = scan;

    return(count);
} 