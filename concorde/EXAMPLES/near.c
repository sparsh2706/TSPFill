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
/*            DEMO OF CONCORDE LIBRARY - a nearest neighbor code            */
/*                                                                          */
/*  Written by:  Applegate, Bixby, Chvatal, and Cook                        */
/*  Date: November 14, 1997                                                 */
/*                                                                          */
/****************************************************************************/

#include "concorde.h"

static int seed = 0;
static int printcycle = 0;
static char *tspfile = (char *) NULL;

int
    main (int, char **);

static int
    parseargs (int, char **),
    nncycle (int ncount, CCdatagroup *dat, int *cyc, double *val,
        CCrandstate *rstate);

static void
    usage (char *f);


int main (int ac, char **av)
{
    int rval = 0;
    int ncount;
    int *cycle = (int *) NULL;
    double startzeit, val;
    CCdatagroup dat;
    CCrandstate rstate;

    CCutil_init_datagroup (&dat);
    
    seed = (int) CCutil_real_zeit ();
    if (parseargs (ac, av))
        return 1;
    CCutil_sprand (seed, &rstate);

    if (!tspfile) {
        usage (av[0]);
        return 1;
    }

    printf ("Nearest Neighbor with with seed %d\n", seed);
    fflush (stdout);

    startzeit = CCutil_zeit ();

    dat.x = (double *) NULL;
    dat.y = (double *) NULL;
    dat.z = (double *) NULL;
    dat.adj = (int **) NULL;

    rval = CCutil_gettsplib (tspfile, &ncount, &dat);
    if (rval) {
        fprintf (stderr, "could not read the TSPLIB file\n");
        goto CLEANUP;
    }

    cycle = CC_SAFE_MALLOC (ncount, int);
    if (cycle == (int *) NULL) {
        fprintf (stderr, "out of memory in main\n");
        rval = 1; goto CLEANUP;
    }

    rval = nncycle (ncount, &dat, cycle, &val, &rstate);
    if (rval) {
        fprintf (stderr, "nncycle failed\n");
        goto CLEANUP;
    }

    printf ("Length of Tour: %.0f\n", val); 
    printf ("Running Time: %.2f (seconds)\n", CCutil_zeit () - startzeit);
    fflush (stdout);

    if (printcycle) {
        int i;
    
        for (i = 0; i < ncount; i++) {
            printf ("%2d ", cycle[i]); fflush (stdout);
            if (i % 10 == 9) printf ("\n");
        }
        printf ("\n");
    }

CLEANUP:

    CC_IFFREE (cycle, int);
    CCutil_freedatagroup (&dat);

    return rval;
}

static int nncycle (int ncount, CCdatagroup *dat, int *cyc, double *val,
        CCrandstate *rstate)
{
    int rval = 0;
    int i, j;
    int len, bestlen, best, first, current;
    char *marks = (char *) NULL;

    if (val) *val = 0.0;
    
    marks = CC_SAFE_MALLOC (ncount, char);
    if (marks == (char *) NULL) {
        fprintf (stderr, "out of memory in nncycle\n");
        rval = 1; goto CLEANUP;
    }

    for (i = 0; i < ncount; i++) {
        marks[i] = 0;
    }
    

    first = CCutil_lprand (rstate) % ncount;
    printf ("Start tour at Node %d\n", first);

    if (cyc) cyc[0] = first;
    marks[first] = 1;
    current = first;

    for (i = 1; i < ncount; i++) {
        bestlen = (1 << 30);
        best = -1;
        for (j = 0; j < ncount; j++) {
            if (marks[j] == 0) {
                len = CCutil_dat_edgelen (current, j, dat);
                if (len < bestlen) {
                    best = j;
                    bestlen = len;
                }
            }
        }
        current = best;
        marks[current] = 1;
        if (val) (*val) += (double) bestlen;
        if (cyc) cyc[i] = best;
    }
    if (val) (*val) += (double) CCutil_dat_edgelen (first, current, dat);

CLEANUP:

    CC_IFFREE (marks, char);
    return rval;
}


static int parseargs (int ac, char **av)
{
    int c;
    int optind = 1;
    char *optarg = (char *) NULL;

    while ((c = CCutil_bix_getopt (ac, av, "s:f", &optind, &optarg)) != EOF)
        switch (c) {
        case 'f':
            printcycle = 1;
            break;
        case 's':
            seed = atoi (optarg);
            break;
        case CC_BIX_GETOPT_UNKNOWN:
        case '?':
        default:
            usage (av[0]);
            return 1;
        }
    if (optind >= ac) {
        usage (av[0]);
        return 1;
    }

    tspfile = av[optind++];

    if (optind < ac) {
        usage (av[0]);
        return 1;
    }
    return 0;
}

static void usage (char *f)
{
    fprintf (stderr, "usage: %s [- see below -] tsplib_file\n", f);
    fprintf (stderr, "   -f    print the cycle\n");
    fprintf (stderr, "   -s #  random number seed\n");
}
