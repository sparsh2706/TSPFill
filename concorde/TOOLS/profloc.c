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
#include <a.out.h>

#define BUFSIZE (32768)
int main (int ac, char **av)
{
        FILHDR fh;
        SYMENT sym;
        AUXENT aux;
        char buf[BUFSIZE];
        int ncopy, n;
        int i;
        int j;

        fread (&fh, FILHSZ, 1, stdin);
        fwrite (&fh, FILHSZ, 1, stdout);

        ncopy = fh.f_symptr - sizeof (fh);
        while (ncopy) {
                n = ncopy;
                if (n > sizeof (buf)) n = sizeof (buf);
                fread (buf, n, 1, stdin);
                fwrite (buf, n, 1, stdout);
                ncopy -= n;
        }

        for (i=0; i<fh.f_nsyms; i++) {
            fread (&sym, SYMESZ, 1, stdin);
            if (sym.n_sclass == 107 && sym.n_scnum == 2) {
                sym.n_sclass = 2;
            }
            fwrite (&sym, SYMESZ, 1, stdout);
            for (j=0; j<sym.n_numaux; j++) {
                fread (&aux, AUXESZ, 1, stdin);
                fwrite (&aux, AUXESZ, 1, stdout);
                i++;
            }
        }
        fread (&ncopy, sizeof (int), 1, stdin);
        fwrite (&ncopy, sizeof (int), 1, stdout);
        ncopy -= sizeof (int);
        while (ncopy) {
                n = ncopy;
                if (n > sizeof (buf)) n = sizeof (buf);
                fread (buf, n, 1, stdin);
                fwrite (buf, n, 1, stdout);
                ncopy -= n;
        }

}
