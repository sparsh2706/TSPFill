#!/bin/nawk -f
#
#   This file is part of CONCORDE
#
#   (c) Copyright 1995--1999 by David Applegate, Robert Bixby,
#   Vasek Chvatal, and William Cook
#
#   Permission is granted for academic research use.  For other uses,
#   contact the authors for licensing options.
#
#   Use at your own risk.  We make no guarantees about the
#   correctness or usefulness of this code.
#
function genhdr(f,title){
    printf ("<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">\n") > f;
    printf ("<html>\n") > f;
    printf ("<head>\n") > f;
    printf ("   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n") > f;
    printf ("   <title>%s</title>\n", title) > f;
    printf ("</head>\n") > f;
    printf ("<body>\n") > f;
    printf ("<center><h1>%s</h1></center>\n", title) > f;
    printf ("<h2>") > f;
    if (indof != f) printf ("<a href=\"%s\">Organizational index</a>&nbsp;&nbsp;&nbsp;&nbsp;\n", indof) > f;
    if (indff != f) printf ("<a href=\"%s\">Function index</a>&nbsp;&nbsp;&nbsp;&nbsp;\n", indff) > f;
    if (indpf != f) printf ("<a href=\"%s\">Program index</a>&nbsp;&nbsp;&nbsp;&nbsp;\n", indpf) > f;
    if (indmf != f) printf ("<a href=\"%s\">Macro index</a>&nbsp;&nbsp;&nbsp;&nbsp;\n", indmf) > f;
    if (inddf != f) printf ("<a href=\"%s\">Data Types index</a>&nbsp;&nbsp;&nbsp;&nbsp;\n", inddf) > f;
    printf ("</h2>") > f;
}

BEGIN{
    in_proto = 0;
    in_desc = 0;
    indof = "concorde_org.html"
    indff = "concorde_funcs.html"
    indpf = "concorde_prog.html"
    indmf = "concorde_macro.html"
    inddf = "concorde_types.html"

    genhdr(indof, "Concorde organizational index");

    genhdr(indff, "Concorde function alphabetic index");
    close (indff);
    indfp = "sort >> " indff;

    genhdr(indpf, "Concorde program alphabetic index");
    close (indpf);
    indpp = "sort >> " indpf;

    genhdr(indmf, "Concorde macro alphabetic index");
    close (indmf);
    indmp = "sort >> " indmf;

    genhdr(inddf, "Concorde data types alphabetic index");
    close (inddf);
    inddp = "sort >> " inddf;

}
/^FILENAME:/{fname=$2; desc=""; next;}
/^FUNCTION:/{funct=$2; tp="Function"; next;}
/^MACRO:/{funct=$2; tp="Macro"; next;}
/^PROGRAM:/{funct=$2; tp="Program"; next;}
/^DATATYPE:/{funct=$2; tp="Data Type"; next;}
/^DESC:/{desc=substr($0,6); next;}
/^HEADER:/{
    hdr=$2;
    if (hdrlist[hdr] == "") {
        outf = hdr;
        sub("\\.h",".html",outf);
        genhdr(outf, "Concorde " hdr " functions");
        hdrlist[hdr] = outf;
    } else {
        outf = hdrlist[hdr];
    }

    if (desc != "") descline = " - " desc;
    else descline = ""

    if (tp == "Function") {
        printf ("<a href=\"%s#%s\">%s</a>%s<br>\n",
                outf, funct, funct, descline) | indfp;
    } else if (tp == "Macro") {
        printf ("<a href=\"%s#%s\">%s</a>%s<br>\n",
                outf, funct, funct, descline) | indmp;
    } else if (tp == "Program") {
        printf ("<a href=\"%s#%s\">%s</a>%s<br>\n",
                outf, funct, funct, descline) | indpp;
    } else if (tp == "Data Type") {
        printf ("<a href=\"%s#%s\">%s</a>%s<br>\n",
                outf, funct, funct, descline) | inddp;
    }
    printf ("<a href=\"%s#%s\">%s</a>%s<br>\n",
            outf, funct, funct, descline) > indof;

    printf ("<a NAME=\"%s\"></a>\n", funct) > outf;
    printf ("<h2>%s", funct) > outf;

    printf ("</h2>\n") > outf;
    printf ("<dt>File:</dt>\n") > outf;
    printf ("<dd><tt>%s</tt></dd>\n", fname) > outf;
    printf ("<dt>Header:</dt>\n") > outf;
    printf ("<dd><tt>%s</tt></dd>\n", hdr) > outf;
    next;
}
/^BEGINFILE:/{
    printf ("<h1>%s</h1>\n", $2) > indof;
    next;
}
/^PROTOTYPE:/{
    printf ("<dt>Prototype:</dt>\n") > outf;
    in_proto = 1;
    next;
}
/^DESCRIPTION:/{
    in_proto = 0;
    printf ("<dt>Description:</dt>\n") > outf;
    in_desc = 1;
    next;
}
/^END_DESCRIPTION/{
    in_desc = 0;
    next;
}
in_proto > 0{
    x=substr($0,5);
    gsub(" ","\\&nbsp;",x);
    printf ("<dd><tt>%s</tt></dd>\n", x) > outf;
    next;
}
in_desc > 0{
    x=substr($0,5);
    if (in_desc == 1 && (x == "NONE" || x == "MISSING") && desc != "") {
        x = desc;
        in_desc = 2;
    }
    gsub(" ","\\&nbsp;",x);
    printf ("<dd><tt>%s</tt></dd>\n", x) > outf;
    next;
}
END{
    close (indfp);
    close (indpp);
    close (indmp);
    close (inddp);
    printf ("</body>\n</html>\n") >> indff;
    printf ("</body>\n</html>\n") >> indpf;
    printf ("</body>\n</html>\n") >> indmf;
    printf ("</body>\n</html>\n") >> inddf;
    printf ("</body>\n</html>\n") > indof;
    for (i in hdrlist) {
        outf = hdrlist[i];
        printf ("</body>\n</html>\n") > outf;
    }
}
