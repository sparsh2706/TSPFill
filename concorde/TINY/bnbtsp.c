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
/*  int CCtiny_bnb_tsp (int nnodes, int nedges, int *elist, int *weight,    */
/*      int *lbound, int *ubound, double *objlimit, int objdir,             */
/*      double *objval, int *xsol, int searchlimit)                         */
/*    MISSING                                                               */
/*                                                                          */
/****************************************************************************/

#include "machdefs.h"
#include "util.h"
#include "tinytsp.h"

#define BIGDOUBLE (1e30)

/*#define DEBUG 1*/
#undef TIMINGS

#define SEARCHLIMIT (1<<30)

typedef struct node {
    int availdeg;
    int deg;
    int pathend;
    int pathlen;
    struct edge *adjvec;
    int adjlist;
} node;

typedef struct edge {
    int next;
    int prev;
    int weight;
    int value;
    int bestvalue;
} edge;

typedef struct histent {
    int from;
    int to;
    int pfrom;
    int pto;
    int plen;
} histent;

typedef struct tspsearch {
    int nnodes;
    node *nodes;
    histent *searchhist;
    edge *adjspace;
    int histtop;
    double currentval;
    double bestval;
    int searchcount;
    int searchlimit;
} tspsearch;

static int checkout_node (tspsearch *s, int i);
static int build_paths (tspsearch *s);
static void build_adj (tspsearch *s);
static void search (tspsearch *s);
static int set_edge(tspsearch *s, int n0, int n1, int v);
static void unset_edge(tspsearch *s, histent *h);
static int select_edge (tspsearch *s, int *n0, int *n1);
static void savetour (tspsearch *s);
static void unroll_stack (tspsearch *s, int smark);
static double lowerbound (tspsearch *s);

int CCtiny_bnb_tsp (int nnodes, int nedges, int *elist, int *weight,
             int *lbound, int *ubound, double *objlimit, int objdir,
             double *objval, int *xsol, int searchlimit)
{
    tspsearch s;
    int i, j;
    int rval;
    int n0, n1;
    double initlimit;
#ifdef TIMINGS
    double sz = CCutil_zeit();
#endif

    rval = CC_TINYTSP_ERROR; /* Failures in this part are from out of memory */
    s.nnodes = nnodes;
    s.nodes = (node *) NULL;
    s.searchhist = (histent *) NULL;
    s.histtop = 0;
    s.currentval = 0.0;
    if (objlimit) {
        initlimit = objdir * (*objlimit);
    } else {
        initlimit = BIGDOUBLE;
    }
    s.bestval = initlimit;

    if (searchlimit < 0) {
        s.searchlimit = SEARCHLIMIT;
    } else {
        s.searchlimit = searchlimit;
    }

    s.nodes = CC_SAFE_MALLOC (nnodes, node);
    s.adjspace = CC_SAFE_MALLOC (nnodes * nnodes, edge);
    s.searchhist = CC_SAFE_MALLOC (nnodes * nnodes, histent);
    if (s.nodes == (node *) NULL ||
        s.adjspace == (edge *) NULL ||
        s.searchhist == (histent *) NULL) {
        goto CLEANUP;
    }
    for (i=0; i<nnodes; i++) {
        s.nodes[i].adjvec = s.adjspace + i*nnodes;
    }

    for (i=0; i<nnodes; i++) {
        s.nodes[i].pathend = i;
        s.nodes[i].pathlen = 1;
        for (j=0; j<nnodes; j++) {
            s.nodes[i].adjvec[j].weight = 0;
            s.nodes[i].adjvec[j].value = 0;
            s.nodes[i].adjvec[j].bestvalue = 0;
        }
    }

    for (i=0; i<nedges; i++) {
        n0 = elist[2*i];
        n1 = elist[2*i+1];
        s.nodes[n0].adjvec[n1].weight = objdir * weight[i];
        s.nodes[n1].adjvec[n0].weight = objdir * weight[i];
        if (lbound[i] == ubound[i] && (lbound[i] == 0 || lbound[i] == 1)) {
            s.nodes[n0].adjvec[n1].value = lbound[i];
            s.nodes[n0].adjvec[n1].bestvalue = lbound[i];
            s.nodes[n1].adjvec[n0].value = lbound[i];
            s.nodes[n1].adjvec[n0].bestvalue = lbound[i];
        } else if (lbound[i] == 0 && ubound[i] == 1) {
            s.nodes[n0].adjvec[n1].value = -1;
            s.nodes[n0].adjvec[n1].bestvalue = -1;
            s.nodes[n1].adjvec[n0].value = -1;
            s.nodes[n1].adjvec[n0].bestvalue = -1;
        } else {
            fprintf (stderr, "Invalid bounds (%d,%d) on edge %d\n",
                     lbound[i], ubound[i], i);
            goto CLEANUP;
        }
    }

    build_adj (&s);

#ifdef DEBUG
    printf ("Performing initial checks\n");
#endif
    if (build_paths (&s)) {
        rval = CC_TINYTSP_INFEASIBLE;
        goto CLEANUP;
    }

    for (i=0; i<nnodes; i++) {
        if (checkout_node (&s, i)) {
            rval = CC_TINYTSP_INFEASIBLE;
            goto CLEANUP;
        }
        if (s.nodes[i].deg == 1 &&
            s.nodes[i].adjvec[s.nodes[i].pathend].value == -1) {
            if (s.nodes[i].pathlen == nnodes) {
#ifdef DEBUG
                printf ("node %d pathend, forcing %d to 1\n",
                        i,s.nodes[i].pathend);
#endif
                if (set_edge (&s, i, s.nodes[i].pathend, 1)) {
                    rval = CC_TINYTSP_INFEASIBLE;
                    goto CLEANUP;
                }
            } else {
#ifdef DEBUG
                printf ("node %d pathend, forcing %d to 0\n",
                        i,s.nodes[i].pathend);
#endif
                if (set_edge (&s, i, s.nodes[i].pathend, 0)) {
                    rval = CC_TINYTSP_INFEASIBLE;
                    goto CLEANUP;
                }
            }
        }
    }

#ifdef DEBUG
    printf ("Beginning search\n");
#endif
    s.searchcount = 0;
    search (&s);
#ifdef DEBUG
    printf ("Search finished\n");
#endif

    if (s.searchcount >= s.searchlimit) {
#ifdef DEBUG
        fprintf (stderr, "search node limit %d exceeded\n", s.searchlimit);
        fprintf (stderr, "SNLE obj:\n");
        fprintf (stderr, "%d %d\n", nnodes, nedges);
        for (i=0; i<nedges; i++) {
            fprintf (stderr, "%d %d %d %d %d\n", elist[2*i], elist[2*i+1],
                     lbound[i], ubound[i], objdir * weight[i]);
        }
#endif
#ifdef TIMINGS
        printf ("BNB Search Limit Nodes: %d   Time %.3f\n",
                s.searchcount, CCutil_zeit() - sz); fflush (stdout);
#endif
        rval = CC_TINYTSP_SEARCHLIMITEXCEEDED;
        goto CLEANUP;
    }

#ifdef TIMINGS
    printf ("BNB Search in Nodes: %d   Time %.3f\n",
            s.searchcount, CCutil_zeit() - sz); fflush (stdout);
#endif
#ifdef DEBUG
    printf ("bnbtsp search finished in %d nodes\n", s.searchcount);
    fflush (stdout);
#endif

    if (objval) *objval = objdir * s.bestval;
    if (s.bestval >= initlimit) {
        rval = CC_TINYTSP_INFEASIBLE;
        goto CLEANUP;
    }

    if (xsol) {
        for (i=0; i<nedges; i++) {
            xsol[i] = s.nodes[elist[2*i]].adjvec[elist[2*i+1]].bestvalue;
        }
    }
    rval = 0;

  CLEANUP:
    CC_IFFREE (s.nodes, node);
    CC_IFFREE (s.adjspace, edge);
    CC_IFFREE (s.searchhist, histent);
    return rval;
}

static int checkout_node (tspsearch *s, int i)
{
    node *nodes = s->nodes;
    int j;

    if (nodes[i].availdeg < 2 || nodes[i].deg > 2) return 1;
    if (nodes[i].availdeg == 2) {
        while ((j = nodes[i].adjlist) != -1) {
#ifdef DEBUG
            printf ("node %d avail 2, forcing %d to 1\n", i,j);
#endif
            if (set_edge (s, i, j, 1)) return 1;
        }
    }
    if (nodes[i].deg == 2) {
        while ((j = nodes[i].adjlist) != -1) {
#ifdef DEBUG
            printf ("node %d deg 2, forcing %d to 0\n", i,j);
#endif
            if (set_edge (s, i, j, 0)) return 1;
        }
    }
    return 0;
}

static int build_paths (tspsearch *s)
{
    int i, j;
    int pi, pj;
    int pl;
    int nnodes = s->nnodes;
    node *nodes = s->nodes;

    for (i=0; i<nnodes; i++) {
        nodes[i].pathend = i;
        nodes[i].pathlen = 1;
    }
    for (i=0; i<nnodes; i++) {
        for (j=i+1; j<nnodes; j++) {
            if (s->nodes[i].adjvec[j].value == 1) {
                if (nodes[i].pathend == j && nodes[i].pathlen < nnodes)
                    return 1;
                pi = nodes[i].pathend;
                pj = nodes[j].pathend;
                nodes[i].pathend = pj;
                if (pi != i) nodes[pi].pathend = pj;
                nodes[j].pathend = pi;
                if (pj != j) nodes[pj].pathend = pi;
                pl = nodes[pi].pathlen + nodes[pj].pathlen;
                nodes[pi].pathlen = pl;
                nodes[pj].pathlen = pl;
            }
        }
    }
    return 0;
}

static void build_adj (tspsearch *s)
{
    int i, j;
    int p;
    int nnodes = s->nnodes;
    node *nodes = s->nodes;

    for (i=0; i<nnodes; i++) {
        p = -1;
        nodes[i].adjlist = -1;
        nodes[i].deg = 0;
        nodes[i].availdeg = 0;
        for (j=0; j<nnodes; j++) {
            if (nodes[i].adjvec[j].value == 1) {
                nodes[i].deg++;
                nodes[i].availdeg++;
                if (i <= j) { /* only add in each edge once */
                    s->currentval += nodes[i].adjvec[j].weight;
                }
            } else if (nodes[i].adjvec[j].value == -1) {
                nodes[i].availdeg++;
                nodes[i].adjvec[j].prev = p;
                nodes[i].adjvec[j].next = -1;
                if (p == -1) {
                    nodes[i].adjlist = j;
                } else {
                    nodes[i].adjvec[p].next = j;
                }
                p = j;
            }
        }
    }
}

/* minimization */
static void search (tspsearch *s)
{
    int n0, n1;
    int smark;

    s->searchcount++;
    if (s->searchcount >= s->searchlimit) return;

    if (s->currentval + lowerbound(s) >= s->bestval) {
#ifdef DEBUG
        printf ("search, currentval %.0f lowerbound %.0f, bestval %.0f, returning\n",
                s->currentval, lowerbound(s), s->bestval);
#endif
        return;
    }
    if (!select_edge (s, &n0, &n1)) { /* every edge set without failure */
                                      /* => tour */
        savetour(s);
        return;
    }

    smark = s->histtop;
#ifdef DEBUG
    printf ("branching on %d-%d\n", n0,n1);
#endif
    if (s->nodes[n0].adjvec[n1].weight > 0) {
        if (!set_edge (s, n0, n1, 0)) {
            search (s);
        }
        unroll_stack (s, smark);
        if (!set_edge (s, n0, n1, 1)) {
            search (s);
        }
        unroll_stack (s, smark);
    } else {
        if (!set_edge (s, n0, n1, 1)) {
            search (s);
        }
        unroll_stack (s, smark);
        if (!set_edge (s, n0, n1, 0)) {
            search (s);
        }
        unroll_stack (s, smark);
    }
#ifdef DEBUG
    printf ("done branching on %d-%d\n", n0,n1);
#endif
    return;
}

/* 1 means contradiction found */
static int set_edge(tspsearch *s, int n0, int n1, int v)
{
    int p0, p1;
    int pl0=0, pl1;
    edge *e;
    node *nodes = s->nodes;

#ifdef DEBUG
    printf ("set_edge %d-%d = %d\n",n0,n1,v);
#endif
    if (nodes[n0].adjvec[n1].value != -1) {
        if (nodes[n0].adjvec[n1].value == v) return 0;
        else return 1;
    }
    if (nodes[n0].deg + v > 2) return 1;
    if (nodes[n1].deg + v > 2) return 1;
    if (nodes[n0].availdeg - 1 + v < 2) return 1;
    if (nodes[n1].availdeg - 1 + v < 2) return 1;
    p0 = nodes[n0].pathend;
    p1 = nodes[n1].pathend;
    if (v == 1 && p0 == n1 && nodes[n0].pathlen < s->nnodes) return 1;

    nodes[n0].deg += v;
    nodes[n1].deg += v;
    nodes[n0].availdeg += v - 1;
    nodes[n1].availdeg += v - 1;

    if (v == 1) { /* The only situation in which pathends matter */
        nodes[p0].pathend = p1;
        nodes[p1].pathend = p0;
        pl0 = nodes[p0].pathlen;
        pl1 = nodes[p1].pathlen;
        nodes[p0].pathlen = pl0 + pl1;
        nodes[p1].pathlen = pl0 + pl1;
    }

    e = &nodes[n0].adjvec[n1];
    if (e->next != -1) nodes[n0].adjvec[e->next].prev = e->prev;
    if (e->prev == -1) {
        nodes[n0].adjlist = e->next;
    } else {
        nodes[n0].adjvec[e->prev].next = e->next;
    }
    e->value = v;

    e = &nodes[n1].adjvec[n0];
    if (e->next != -1) nodes[n1].adjvec[e->next].prev = e->prev;
    if (e->prev == -1) {
        nodes[n1].adjlist = e->next;
    } else {
        nodes[n1].adjvec[e->prev].next = e->next;
    }
    e->value = v;

    s->currentval += v * e->weight;

    s->searchhist[s->histtop].from = n0;
    s->searchhist[s->histtop].to = n1;
    s->searchhist[s->histtop].pfrom = p0;
    s->searchhist[s->histtop].pto = p1;
    s->searchhist[s->histtop].plen = pl0;
    s->histtop++;

    if (v == 1) {
        if ((p0 != n0 || p1 != n1) && nodes[n0].pathlen != s->nnodes
            && n0 != p1) {
#ifdef DEBUG
            printf ("Avoiding subtour, forcing %d-%d to 0\n",
                    p0,p1);
#endif
            if (set_edge (s, p0, p1, 0)) return 1;
        }
    }
    checkout_node (s, n0);
    checkout_node (s, n1);

    return 0;
}

static void unset_edge(tspsearch *s, histent *h)
{
    int n0 = h->from;
    int n1 = h->to;
    edge *e;
    node *nodes = s->nodes;
    int v = nodes[n0].adjvec[n1].value;

    if (v == -1) return;

#ifdef DEBUG
    printf ("unset_edge %d-%d from %d\n",n0,n1,v);
#endif
    nodes[n0].deg -= v;
    nodes[n1].deg -= v;
    nodes[n0].availdeg += 1 - v;
    nodes[n1].availdeg += 1 - v;

    if (v == 1) { /* The only situation in which pathends matter */
        nodes[n0].pathend = h->pfrom;
        nodes[n1].pathend = h->pto;
        if (h->pfrom != n0) nodes[h->pfrom].pathend = n0;
        if (h->pto != n1)   nodes[h->pto].pathend = n1;
        nodes[h->pto].pathlen = nodes[h->pto].pathlen - h->plen;
        nodes[h->pfrom].pathlen = h->plen;
    }

    e = &nodes[n0].adjvec[n1];
    if (e->next != -1) nodes[n0].adjvec[e->next].prev = n1;
    if (e->prev == -1) {
        nodes[n0].adjlist = n1;
    } else {
        nodes[n0].adjvec[e->prev].next = n1;
    }
    e->value = -1;

    e = &nodes[n1].adjvec[n0];
    if (e->next != -1) nodes[n1].adjvec[e->next].prev = n0;
    if (e->prev == -1) {
        nodes[n1].adjlist = n0;
    } else {
        nodes[n1].adjvec[e->prev].next = n0;
    }
    e->value = -1;

    s->currentval -= v * e->weight;
}

static int select_edge (tspsearch *s, int *n0, int *n1)
{
    int i, j;
    int bestn0 = -1, bestn1 = -1;
    int bestval;
    node *nodes = s->nodes;
    int nnodes = s->nnodes;

    bestval = -1;

    for (i=0; i<nnodes; i++) {
        for (j=nodes[i].adjlist; j != -1; j = nodes[i].adjvec[j].next) {
            if (nodes[i].adjvec[j].weight > bestval) {
                bestn0 = i;
                bestn1 = j;
                bestval = nodes[i].adjvec[j].weight;
            } else if (-nodes[i].adjvec[j].weight > bestval) {
                bestn0 = i;
                bestn1 = j;
                bestval = -nodes[i].adjvec[j].weight;
            }
        }
    }
    if (bestval >= 0) {
        *n0 = bestn0;
        *n1 = bestn1;
        return 1;
    } else {
        return 0;
    }
}

static void savetour (tspsearch *s)
{
    int i, j;

#ifdef DEBUG
    printf ("Solution found, saving tour, value %.0f\n", s->currentval);
#endif

    s->bestval = s->currentval;
    for (i=0; i<s->nnodes; i++) {
        for (j=0; j<s->nnodes; j++) {
            s->nodes[i].adjvec[j].bestvalue = s->nodes[i].adjvec[j].value;
        }
    }
}

static void unroll_stack (tspsearch *s, int smark)
{
    int i = s->histtop;

    while (i > smark) {
        i--;
        unset_edge (s, &s->searchhist[i]);
    }
    s->histtop = smark;
}

static double lowerbound (tspsearch *s)
{
    int i, j;
    node *nodes = s->nodes;
    int nnodes = s->nnodes;
    double lb = 0.0;
    int min1, min2;

    for (i=0; i<nnodes; i++) {
        if (nodes[i].deg == 0) {
            min1 = INT_MAX;
            min2 = INT_MAX;
            for (j=nodes[i].adjlist; j != -1; j = nodes[i].adjvec[j].next) {
                if (nodes[i].adjvec[j].weight < min2) {
                    if (nodes[i].adjvec[j].weight < min1) {
                        min2 = min1;
                        min1 = nodes[i].adjvec[j].weight;
                    } else {
                        min2 = nodes[i].adjvec[j].weight;
                    }
                }
            }
            lb += min1 + min2;
        } else if (nodes[i].deg == 1) {
            min1 = INT_MAX;
            for (j=nodes[i].adjlist; j != -1; j = nodes[i].adjvec[j].next) {
                if (nodes[i].adjvec[j].weight < min1) {
                    min1 = nodes[i].adjvec[j].weight;
                }
            }
            lb += min1;
        }
    }
    return ceil(lb/2.0);
}
