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

BEGIN{
    in_proto = 0;
    in_desc = 0;
}
/^PROTOTYPE:/{
    in_proto = 1;
    print $0;
    next;
}
/^DESCRIPTION:/{
    in_proto = 0;
    in_desc = 1;
    print $0;
    next;
}
/^END_DESCRIPTION/{
    if (in_desc == 0) {
        printf ("    MISSING\n");
    } else {
        if (d[1] == "") {
            printf ("HELLOHELLO\n");
        }
        pref = match (d[1], "[^ ]");
        for (i=2; i<in_desc; i++) {
            p = match (d[i], "[^ ]");
            if (p > 0 && p < pref) pref = p;
        }
        for (i=1; i<in_desc; i++) {
            print "    " substr(d[i], pref);
        }
    }
    in_desc = 0;
    print $0;
    next;
}
in_proto > 0 {
    if (in_proto == 1) {
        print "    " substr($0, match ($0, "[^ ]"));
    } else {
        print "        " substr($0, match ($0, "[^ ]"));
    }
    in_proto++;
}
in_desc > 0 {
    d[in_desc] = $0;
    in_desc++;
}
in_proto == 0 && in_desc == 0 {
    print $0;
    next;
}
