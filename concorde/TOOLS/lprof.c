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
#include <dlfcn.h>
#include <search.h>
#include <bfd.h>
#include <libiberty.h>
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
    long count;
    long after;
} mapent;

mapent *map = (mapent *) NULL;
int mapsize = 0;
int mapcnt = 0;

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

void dumpsect (bfd *bin, asection *sect, PTR obj)
{
    printf ("Section %s index %d\n", sect->name, sect->index);
    printf ("   flags %x vma %d reloc %d linker_mark %d\n",
            sect->flags,sect->user_set_vma,sect->reloc_done,
            sect->linker_mark);
    printf ("   vma %x lma %x cooked_size %d raw_size %d\n",
            (int) sect->vma, (int) sect->lma, (int) sect->_cooked_size,
            (int) sect->_raw_size);
    printf ("   offset %x alignment %d reloc_count %d\n",
            (int) sect->output_offset, sect->alignment_power,
            sect->reloc_count);
    printf ("   filepos %d rel_filepos %d line_filepos %d lineno_count %d\n",
            (int) sect->filepos, (int) sect->rel_filepos,
            (int) sect->line_filepos, sect->lineno_count);
}

void process_symbols (bfd *bin, asymbol **symtab, int nsyms)
{
    int i;
    asymbol *a;
    int symclass;

    for (i=0; i<nsyms; i++) {
        a = symtab[i];
        symclass = bfd_decode_symclass (a);
        if (a->section->flags & SEC_CODE) {
            map[mapcnt].name = strdup (a->name);
            map[mapcnt].addr = a->value + a->section->vma;
            map[mapcnt].len = 65536;
            mapcnt++;
#if 0
            printf ("%d: %s value %lx flags %lx section %s index %d flags %lx vma %lx\n",
                    i, a->name, a->value, a->flags, a->section->name,
                    a->section->index,a->section->flags,a->section->vma);
            fflush (stdout);
#endif
        }
    }
}

void process_map (char *fname)
{
    long storage_needed;
    asymbol **symtab = (asymbol **) NULL;
    long num_sym = 0;
    asymbol **dynsymtab = (asymbol **) NULL;
    long num_dynsym = 0;
    char **matching;
    bfd *bin;
    int i;

    bfd_init();

    bin = bfd_openr (fname, (char *) NULL);
    if (!bin) {
        perror (fname);
        fprintf (stderr, "Unable to bfd_open %s for map input\n", fname);
        exit (1);
    }

    if (bfd_check_format (bin, bfd_archive) == true) {
        fprintf (stderr, "Whoa, %s is an archive\n", fname);
        exit (1);
    }

    if (!bfd_check_format_matches (bin, bfd_object, &matching)) {
        fprintf (stderr, "Error - couldn't recognize file format\n");
        exit (1);
    }

    printf ("%s: format %s\n", fname, bin->xvec->name);

#if 0
    printf ("sections: %lx\n", bin->sections);
    bfd_map_over_sections (bin, dumpsect, (void *) NULL);
#endif

    if (bfd_get_file_flags(bin) & HAS_SYMS) {
        storage_needed = bfd_get_symtab_upper_bound (bin);
        if (storage_needed) {
            symtab = (asymbol **) malloc (storage_needed);
            if (!symtab) {
                fprintf (stderr, "Out of memory\n");
                exit (1);
            }
            num_sym = bfd_canonicalize_symtab (bin, symtab);
        }
    }

    if (bfd_get_file_flags(bin) & DYNAMIC) {
        storage_needed = bfd_get_dynamic_symtab_upper_bound (bin);
        if (storage_needed) {
            dynsymtab = (asymbol **) malloc (storage_needed);
            if (!dynsymtab) {
                fprintf (stderr, "Out of memory\n");
                exit (1);
            }
            num_dynsym = bfd_canonicalize_dynamic_symtab (bin, dynsymtab);
        }
    }

    mapsize = num_sym + num_dynsym + 1;
    mapcnt = 0;

    map = (mapent *) malloc (mapsize * sizeof (mapent));
    if (!map) {
        fprintf (stderr, "Out of memory in process_map");
        exit (1);
    }
    map[mapcnt].name = strdup ("<<<BOGUS EARLY>>>");
    map[mapcnt].addr = 0;
    mapcnt++;

    if (symtab) {
        process_symbols (bin, symtab, num_sym);
        free (symtab);
    }
    if (dynsymtab) {
        process_symbols (bin, dynsymtab, num_dynsym);
        free (dynsymtab);
    }

    if (bfd_close (bin)) {
        fprintf (stderr, "bfd_close failed\n");
    }

    for (i=0; i<mapcnt; i++) {
        map[i].count = 0;
        map[i].after = 0;
    }
    qsort (map, mapcnt, sizeof (mapent), mapcmp);
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
                printf ("%16lx %-46s %8d %5.2f\n", x, m->name,
                        profspace[i], (100.0 * (double) profspace[i]) / count);
            } else {
                printf ("%16lx AFTER %-40s %8d %5.2f\n", x, m->name,
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

    printf ("\nHeavy addresses:\n");
    printf ("     Address    Subroutine                                      Hits    Pct\n");
    heavyprint (total_count);
    qsort (map, mapcnt, sizeof (mapent), countcmp);

    total_count2 = 0;

    printf ("Subroutine                                    Address       Hits    Pct  Cum\n");
    for (i=0; i<mapcnt; i++) {
        if (map[i].count || ac == 2) {
            total_count2 += map[i].count;
            printf ("%-40s %16lx %8d %5.2f %5.2f\n", map[i].name,
                    (long int) map[i].addr, (int) map[i].count,
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
            printf ("AFTER %-40s            %8d %5.2f %5.2f\n", map[i].name,
                    (int) map[i].after,
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

    return 0;
}
