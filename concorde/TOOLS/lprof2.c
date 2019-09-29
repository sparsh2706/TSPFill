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

#include "machdefs.h"
#include "profdata.h"
#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
#include <dlfcn.h>
#include <search.h>
#define PROFGAP (65536)

static CCproftype *profspace = (CCproftype *) NULL;
static CCprofaddr profbase = 0;
static CCprofaddr proflen = 0;
static CCproftype profextra = 0;
static CCprofaddr profsize = CC_PROFSIZE;

typedef struct mapent {
    char *name;
    CCprofaddr addr;
    CCprofaddr len;
    int firstln;
    int lastln;
    long count;
    long after;
    int goodname;
} mapent;

mapent *map = (mapent *) NULL;
int mapsize = 0;
int mapcnt = 0;

char *mapname (mapent *m)
{
    static char buf[1024];

    if (m->goodname) {
        if (m->firstln >= 0) {
            sprintf (buf, "%s (l %d-%d)", m->name, m->firstln, m->lastln);
        } else {
            sprintf (buf, "%s", m->name);
        }
    } else {
        if (m->firstln >= 0) {
            sprintf (buf, "l %d-%d after %s", m->firstln, m->lastln, m->name);
        } else {
            sprintf (buf, "l ????? after %s", m->name);
        }
    }
    return buf;
}

void readprof (char *fname)
{
    FILE *fin;
    CCprofaddr s;
    CCprofaddr b;
    CCprofaddr l;
    CCprofaddr e;
    CCproftype *profbuf;
    CCprofaddr i;

    fin = fopen (fname, "r");
    if (!fin) {
        perror (fname);
        fprintf (stderr, "Unable to open %s for profile input\n", fname);
        exit (1);
    }
    if (profspace) {
        fread (&s, sizeof (CCprofaddr), 1, fin);
        fread (&b, sizeof (CCprofaddr), 1, fin);
        fread (&l, sizeof (CCprofaddr), 1, fin);
        if (profbase != b || proflen != l || profsize != s) {
            fprintf (stderr, "Incompatible profile file.  Ignoring %s\n",
                     fname);
            fclose (fin);
            return;
        }
    } else {
        fread (&profsize, sizeof (CCprofaddr), 1, fin);
        fread (&profbase, sizeof (CCprofaddr), 1, fin);
        fread (&proflen, sizeof (CCprofaddr), 1, fin);

        profspace = (CCproftype *) calloc (proflen, sizeof (CCproftype));
        if (!profspace) {
            fprintf (stderr, "Out of memory\n");
            return;
        }
    }
    profbuf = (CCproftype *) calloc (proflen, sizeof (CCproftype));
    if (!profbuf) {
        fprintf (stderr, "Out of memory\n");
        return;
    }

    fread (&e, sizeof (CCproftype), 1, fin);
    profextra += e;

    fread (profbuf, sizeof (CCproftype), proflen, fin);
    for (i=0; i<proflen; i++) {
        profspace[i] += profbuf[i];
    }

    free (profbuf);
    fclose (fin);
}

int mapcmp (const void *a, const void *b)
{
    const mapent *ma = (const mapent *) a;
    const mapent *mb = (const mapent *) b;

    if (ma->addr < mb->addr) return -1;
    else if (ma->addr > mb->addr) return 1;
    else return 0;
}

int countcmp (const void *a, const void *b)
{
    const mapent *ma = (const mapent *) a;
    const mapent *mb = (const mapent *) b;

    if (ma->count < mb->count) return 1;
    else if (ma->count > mb->count) return -1;
    else return 0;
}

void process_map (char *fname)
{
    LDFILE *ldin;
    int nproc;
    int i;
    PDR proc;
    int maxsym;
    SYMR sym;
    ENTRY x, *y;
    mapent *m;
    char buf[1024];
    HDRR hdr;
    char *lastname;
    int goodname;
    int newent;
    char *name;

    ldin = ldopen (fname, (LDFILE *) NULL);
    if (!ldin) {
        perror (fname);
        fprintf (stderr, "Unable to ldopen %s for map input\n", fname);
        exit (1);
    }

    hdr = SYMHEADER(ldin);
    nproc = hdr.ipdMax;
    maxsym = hdr.isymMax + hdr.iextMax;
    mapsize = nproc + maxsym + 1;
    mapcnt = 0;
    map = (mapent *) malloc (mapsize * sizeof (mapent));
    if (!map) {
        fprintf (stderr, "Out of memory in process_map");
        exit (1);
    }
    map[mapcnt].name = strdup ("<<<BOGUS EARLY>>>");
    map[mapcnt].addr = 0;
    map[mapcnt].len = 0;
    map[mapcnt].firstln = -1;
    map[mapcnt].lastln = -1;
    map[mapcnt].goodname = 1;
    mapcnt++;

    if (hcreate (mapsize) == 0) {
        perror ("hcreate");
        exit (1);
    }

    for (i=0; i<nproc; i++) {
        if (ldgetpd(ldin, i, &proc) == FAILURE) {
            fprintf (stderr, "Unable to read procedure %d (max %d)\n",
                     i, nproc);
        } else {
            if (mapcnt >= mapsize) {
                fprintf (stderr, "Whoa, too many symbols\n");
                exit (1);
            }
            if (proc.isym >= 0) {
                if (ldtbread (ldin, proc.isym, &sym) == FAILURE) {
                    fprintf (stderr, "Unable to read symbol %d\n", proc.isym);
                    name = "<<ERROR>>";
                    m = &map[mapcnt++];
                    newent = 1;
                } else {
                    name = ldgetname (ldin, &sym);
                    x.key = name;
                    y = hsearch (x, FIND);
                    if (y) {
                        m = (mapent *) y->data;
                        newent = 0;
                    } else {
                        m = &map[mapcnt++];
                        newent = 1;
                    }
                }
                goodname = 1;
            } else {
                m = &map[mapcnt++];
                name = lastname;
                newent = 1;
                goodname = 0;
            }
            if (newent) {
                m->name = strdup (name);
                m->addr = proc.adr;
                m->firstln = proc.lnLow;
                m->lastln = proc.lnHigh;
                m->goodname = goodname;
                m->len = 0;
                if (strcmp (m->name, "<<ERROR>>")) {
                    x.key = m->name;
                    x.data = (char *) m;
                    if (!hsearch (x, ENTER)) {
                        fprintf (stderr, "Hash table full\n");
                        exit (1);
                    }
                }
            } else {
                if (sym.st == stEnd) {
                    if (m->len == 0) m->len = sym.value;
                } else {
                    if (m->addr == 0) m->addr = proc.adr;
                }
            }
            lastname = m->name;
        }
    }

    for (i=0; i<maxsym; i++) {
        if (ldtbread(ldin, i, &sym) == FAILURE) {
            fprintf (stderr, "Unable to read symbol %d (max %d)\n", i, maxsym);
        } else {
            if (sym.sc == scText && (sym.st == stProc ||
                                     sym.st == stGlobal ||
                                     sym.st == stLocal ||
                                     sym.st == stStaticProc ||
                                     sym.st == stEnd)) {
                name = ldgetname (ldin, &sym);
                x.key = name;
                y = hsearch (x, FIND);
                if (y) {
                    m = (mapent *) y->data;
                    if (sym.st == stEnd) {
                        if (m->len == 0) m->len = sym.value;
                    } else {
                        if (m->addr == 0) m->addr = sym.value;
                    }
                } else {
                    if (mapcnt >= mapsize) {
                        fprintf (stderr, "Too many symbols\n");
                        exit (1);
                    }
                    map[mapcnt].name = strdup (name);
                    if (sym.st == stEnd) {
                        map[mapcnt].addr = 0;
                        map[mapcnt].len = (CCprofaddr) sym.value;
                    } else {
                        map[mapcnt].addr = (CCprofaddr) sym.value;
                        map[mapcnt].len = 0;
                    }
                    map[mapcnt].firstln = -1;
                    map[mapcnt].lastln = -1;
                    map[mapcnt].goodname = 1;
                    x.key = map[mapcnt].name;
                    x.data = (char *) &map[mapcnt];
                    if (!hsearch(x, ENTER)) {
                        fprintf (stderr, "Hash table full\n");
                        exit (1);
                    }
                    mapcnt++;
                }
            }
        }
    }

    if (ldclose (ldin) == FAILURE) {
        fprintf (stderr, "ldclose failed\n");
    }
    for (i=0; i<mapcnt; i++) {
        if (map[i].len == 0) map[i].len = PROFGAP;
        map[i].count = 0;
        map[i].after = 0;
    }
    qsort (map, mapcnt, sizeof (mapent), mapcmp);
    hdestroy ();
}


mapent *findmap (CCprofaddr x)
{
    int lo, hi;
    int mid;
    static int lastmap = 0;

    if (x < map[0].addr) return &map[0];
    if (x >= map[mapcnt-1].addr) return &map[mapcnt-1];

    if (x >= map[lastmap].addr && x < map[lastmap+1].addr) {
        return &map[lastmap];
    } else if (lastmap+2 < mapcnt && x >= map[lastmap+1].addr &&
               x < map[lastmap+2].addr) {
        return &map[++lastmap];
    }

    lo = 0;
    hi = mapcnt-1;
    /* map[lo] <= x < map[hi] */
    while (lo+1 < hi) {
        mid = (lo + hi)/2;
        if (map[mid].addr <= x) lo = mid;
        else hi = mid;
    }
    lastmap = lo;
    return &map[lo];
}

void countmap (void)
{
    CCprofaddr i;
    CCprofaddr x;
    mapent *m;

    for (i=0; i<proflen; i++) {
        if (profspace[i]) {
            x = profbase + i * profsize;
            m = findmap (x);
            if (x <= m->addr + m->len) m->count += profspace[i];
            else m->after += profspace[i];
        }
    }
}

void heavyprint (CCproftype count)
{
    CCprofaddr i;
    CCprofaddr x;
    mapent *m;
    CCproftype thresh = count/100;

    for (i=0; i<proflen; i++) {
        if (profspace[i] >= thresh) {
            x = profbase + i * profsize;
            m = findmap (x);
            if (x <= m->addr + m->len) {
                printf ("%16lx %-46s %8d %5.2f\n", x, mapname(m),
                        profspace[i], (100.0 * (double) profspace[i]) / count);
            } else {
                printf ("%16lx AFTER %-40s %8d %5.2f\n", x, mapname(m),
                        profspace[i], (100.0 * (double) profspace[i]) / count);
            }
        }
    }
}


int main (int ac, char **av)
{
    int i;
    CCproftype total_count;
    CCproftype total_count2;
    CCproftype bogus;

    if (ac < 2) {
        fprintf (stderr, "Usage: %s a.out lprof.out ...\n", av[0]);
        exit (1);
    }

    for (i=2; i<ac; i++) {
        readprof (av[i]);
    }

    total_count = 0;
    for (i=0; i<proflen; i++) total_count += profspace[i];
    total_count += profextra;

    process_map (av[1]);

    countmap ();

    bogus = map[0].count + map[0].after;
    map[0].after = 0;
    map[0].count = 0;

    if (total_count == 0) total_count = 1;

    qsort (map, mapcnt, sizeof (mapent), countcmp);

    total_count2 = 0;

    printf ("Subroutine                                    Address       Hits    Pct  Cum\n");
    for (i=0; i<mapcnt; i++) {
        if (map[i].count || ac == 2) {
            total_count2 += map[i].count;
            printf ("%-40s %16lx %8d %5.2f %5.2f\n", mapname(&map[i]),
                    map[i].addr, map[i].count,
                    (100.0 * (double) map[i].count) / total_count,
                    (100.0 * (double) total_count2) / total_count);
        }
    }
    if (bogus) {
        total_count2 += bogus;
        printf ("<<BOGUS EARLY HITS>>                                      %8d %5.2f %5.2f\n",
                bogus, (100.0 * (double) bogus) / total_count,
                (100.0 * (double) total_count2) / total_count);
    }
    for (i=0; i<mapcnt; i++) {
        if (map[i].after) {
            total_count2 += map[i].after;
            printf ("AFTER %-40s            %8d %5.2f %5.2f\n",
                    mapname(&map[i]), map[i].after,
                    (100.0 * (double) map[i].after) / total_count,
                    (100.0 * (double) total_count2) / total_count);
        }
    }
    if (profextra) {
        total_count2 += profextra;
        printf ("<<SHARED LIBRARY HITS>>                                   %8d %5.2f %5.2f\n",
                profextra, (100.0 * (double) profextra) / total_count,
                (100.0 * (double) total_count2) / total_count);
    }
    if (total_count2 != total_count) {
        fprintf (stderr, "Count drifted from %d to %d\n", total_count,
                 total_count2);
    }
    printf ("Total ticks: %d\n", total_count);

    printf ("\nHeavy addresses:\n");
    printf ("     Address    Subroutine                                      Hits    Pct\n");
    heavyprint (total_count);
}
