/****************************************************************************/
/*                                                                          */
/*  This file is part of CONCORDE                                           */
/*                                                                          */
/*  (c) Copyright 1995--1999 by David Applegate, Robert Bixby,              */
/*  Vasek Chvatal, and William Cook                                         */
/*                                                                          */
/*  Permission is granted for academic research use.  For other uses,       */
/*  contact the authors for licensing options.                              */
/*                                                                          */
/*  Use at your own risk.  We make no guarantees about the                  */
/*  correctness or usefulness of this code.                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*    EXPORTED FUNCTIONS:                                                   */
/*                                                                          */
/*  void CCprof_init (void)                                                 */
/*    MISSING                                                               */
/*                                                                          */
/*  void CCprof_start (void)                                                */
/*    MISSING                                                               */
/*                                                                          */
/*  void CCprof_stop (void)                                                 */
/*    MISSING                                                               */
/*                                                                          */
/*  void CCprof_end (void)                                                  */
/*    MISSING                                                               */
/*                                                                          */
/****************************************************************************/

#include "machdefs.h"
#include "profrus.h"

#ifdef  LONG_PROFILE
#include "profdata.h"
#include "util.h"

static proftype *profspace = (proftype *) NULL;
static profaddr profbase = 0;
static profaddr proflen = 0;
static proftype profextra = 0;
#define PROFSTAT_INIT (0)
#define PROFSTAT_ERROR (-1)
#define PROFSTAT_READY (1)
#define PROFSTAT_RUNNING (2)
static int profstat = PROFSTAT_INIT;

#ifdef SYSGNULINUX_ALPHA
#define _ftext _start
#endif


static void
    proftimer (int sig, int code, struct sigcontext *sc),
    writeprof (char *profname);


static void proftimer (int sig, int code, struct sigcontext *sc)
{
    profaddr x = (((profaddr) sc->sc_pc) - profbase) / CC_PROFSIZE;

    if (x >= proflen) profextra++;
    else profspace[x]++;
}

extern unsigned long _etext, _ftext;

void CCprof_init (void)
{
    profaddr i;

    if (profstat != PROFSTAT_INIT) return;

    profbase = (profaddr) &_ftext;
    proflen = (((profaddr) &_etext) - profbase) / CC_PROFSIZE + 1;

/*
    printf ("profbase %lx proflen %ld\n", profbase, proflen);
*/

    profspace = CC_SAFE_MALLOC (proflen, proftype);
    if (!profspace) {
        fprintf (stderr, "Out of memory - not starting profiler\n");
        profstat = PROFSTAT_ERROR;
        return;
    }

    for (i=0; i<proflen; i++) profspace[i] = 0;
    profextra = 0;

#ifdef SYSGNULINUX_ALPHA
    if (signal (SIGPROF, (void (*)(int)) proftimer)) {
#else
    if (signal (SIGPROF, proftimer)) {
#endif
        perror ("signal");
        CC_FREE (profspace, proftype);
        profstat = PROFSTAT_ERROR;
        return;
    }
    profstat = PROFSTAT_READY;
}

static void writeprof (char *profname)
{
    FILE *fout;
    profaddr x;

    if (profstat == PROFSTAT_RUNNING) CCprof_stop ();
    if (profstat != PROFSTAT_READY) return;

    fout = fopen (profname, "w");
    if (!fout) {
        perror (profname);
        fprintf (stderr, "Unable to open %s for profile output\n", profname);
        return;
    }
    x = CC_PROFSIZE;
    fwrite (&x, sizeof (profaddr), 1, fout);
    fwrite (&profbase, sizeof (profaddr), 1, fout);
    fwrite (&proflen, sizeof (profaddr), 1, fout);
    fwrite (&profextra, sizeof (proftype), 1, fout);
    fwrite (profspace, sizeof (proftype), proflen, fout);

    fclose (fout);
}

void CCprof_start (void)
{
    struct itimerval newtimer;
    newtimer.it_interval.tv_sec = 0;
    newtimer.it_interval.tv_usec = CC_PROFTIME;
    newtimer.it_value.tv_sec = 0;
    newtimer.it_value.tv_usec = CC_PROFTIME;

    if (profstat == PROFSTAT_INIT) CCprof_init();

    if (profstat == PROFSTAT_READY) {
        if (setitimer (ITIMER_PROF, &newtimer, (struct itimerval *) NULL)) {
            perror ("setitimer");
        }
        profstat = PROFSTAT_RUNNING;
    }
}

void CCprof_stop (void)
{
    struct itimerval newtimer;

    newtimer.it_interval.tv_sec = 0;
    newtimer.it_interval.tv_usec = 0;
    newtimer.it_value.tv_sec = 0;
    newtimer.it_value.tv_usec = 0;

    if (profstat == PROFSTAT_RUNNING) {
        if (setitimer (ITIMER_PROF, &newtimer, (struct itimerval *) NULL)) {
            perror ("setitimer");
        }
        profstat = PROFSTAT_READY;
    }
}

void CCprof_end (void)
{
    char nam[1024];

    CCprof_stop();
    sprintf (nam, "lmon.out.%d", (int) getpid());
    writeprof (nam);

    CC_IFFREE (profspace, proftype);
    profbase = 0;
    proflen = 0;
    profextra = 0;
    profstat = PROFSTAT_INIT;
}

#endif  /* LONG_PROFILE */

#ifdef SHORT_PROFILE

#include <mon.h>

void CCprof_init (void)
{
#if defined(SYSRS6000)||defined(SYSGNURS6000)||defined(SYSRS6000X)
     extern struct monglobal _mondata;

    _mondata.prof_type = _PROF_TYPE_IS_P;

    if (monstartup ((caddr_t) -1, (caddr_t) 0)) {
        perror ("monstartup");
    }
#else
    extern unsigned long _etext, _ftext;
    if (monstartup ((caddr_t) &_ftext, (caddr_t) &_etext)) {
        perror ("monstartup");
    }
#endif
    moncontrol (0);
}

void CCprof_start (void)
{
    moncontrol (1);
}

void CCprof_stop (void)
{
    moncontrol (0);
}

void CCprof_end (void)
{
    CCprof_stop();
}

#endif  /* SHORT_PROFILE */

#if !(defined(LONG_PROFILE)||defined(SHORT_PROFILE))

void CCprof_init (void)
{
}

void CCprof_start (void)
{
}

void CCprof_stop (void)
{
}

void CCprof_end (void)
{
}

#endif /* !(LONG_PROFILE||SHORT_PROFILE) */
