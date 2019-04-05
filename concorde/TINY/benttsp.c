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
/*  int CCtiny_benttsp_elist (int ncount, int ecount, int *elist,           */
/*      int *elen, double *upbound, double *optval, int *foundtour,         */
/*      int anytour, int searchlimit, int silent)                           */
/*    MISSING                                                               */
/*                                                                          */
/****************************************************************************/

#include "machdefs.h"
#include "util.h"
#include "tinytsp.h"
#include "macrorus.h"

typedef int Distance;

typedef struct searchinf {
    int ncount;
    Distance **dist;
    int *perm;
    int foundtour;
    int anytour;
    int bbcount;
    int bblimit;
    Distance upbound;
} searchinf;


static int
    search (searchinf *s, int n, Distance sum);

static Distance
    mstdist (searchinf *s, int n);


int CCtiny_benttsp_elist (int ncount, int ecount, int *elist, int *elen,
        double *upbound, double *optval, int *foundtour, int anytour,
        int searchlimit, CC_UNUSED int silent)
{
    Distance **dist = (Distance **) NULL;
    Distance *distspace = (Distance *) NULL;
    int *perm = (int *) NULL;
    int i;
    int rval;
    Distance sum;
    searchinf s;

    dist = CC_SAFE_MALLOC (ncount, Distance *);
    distspace = CC_SAFE_MALLOC (ncount*ncount, Distance);
    perm = CC_SAFE_MALLOC (ncount, int);
    if (dist == (Distance **) NULL ||
        distspace == (Distance *) NULL ||
        perm == (int *) NULL) {
        fprintf (stderr, "Out of memory in CCbenttsp_elist\n");
        rval = CC_TINYTSP_ERROR; goto CLEANUP;
    }

    for (i=0; i<ncount; i++) {
        dist[i] = distspace + i*ncount;
        perm[i] = i;
    }
    for (i=0; i<ecount; i++) {
        dist[elist[2*i]][elist[2*i+1]] = elen[i];
        dist[elist[2*i+1]][elist[2*i]] = elen[i];
    }
    s.perm = perm;
    s.dist = dist;
    s.foundtour = 0;
    s.anytour = anytour;
    s.bbcount = 0;
    s.bblimit = searchlimit;
    s.ncount = ncount;
    if (upbound == (double *) NULL) {
        sum = dist[0][ncount-1];
        for (i=1; i<ncount; i++) sum += dist[i-1][i];
        s.upbound = sum;
    } else {
        s.upbound = (Distance) *upbound;
    }

    rval = search (&s, ncount-1, (Distance) 0);
    if (foundtour != (int *) NULL) *foundtour = s.foundtour;
    if (optval != (double *) NULL) *optval = (double) s.upbound;
    if (anytour && rval == 2) {
        rval = 0;
    }
    if (rval == 0) {
        printf ("Search completed in %d bbnodes\n", s.bbcount);
        fflush (stdout);
    }
        
    
  CLEANUP:
    CC_IFFREE (dist, Distance *);
    CC_IFFREE (distspace, Distance);
    CC_IFFREE (perm, int);
    return rval;
}

static int search (searchinf *s, int n, Distance sum)
{
    int i;
    int u;
    int *perm = s->perm;
    Distance *d = s->dist[perm[n]];
    int rval;

    s->bbcount++;
    if (s->bbcount >= s->bblimit && s->bblimit > 0)
        return CC_TINYTSP_SEARCHLIMITEXCEEDED;
    
    if (sum + mstdist (s, n) >= s->upbound) return 0;
    
    if (n == 1) {
        d = s->dist[perm[0]];
        sum += d[perm[1]] + d[perm[s->ncount - 1]];
        if (sum < s->upbound) {
            s->foundtour = 1;
            s->upbound = sum;
            if (s->anytour) return 2;
        }
        return 0;
    } else {
        for (i=0; i<n; i++) {
            u = perm[i];
            perm[i] = perm[n-1];
            perm[n-1] = u;
            rval = search (s, n-1, sum + d[u]);
            if (rval) return rval;
            u = perm[n-1];
            perm[n-1] = perm[i];
            perm[i] = u;
        }
    }
    return 0;
}

static Distance mstdist (CC_UNUSED searchinf *s, CC_UNUSED int n)
{
    return 0;
}
