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
    outf="/var/tmp/func2code.tmp";
    cur_file = "";
}
function endfile(f)
{
    printf ("/****************************************************************************/\n") > outf;
    close (outf);
    newf = f ".new";
    system("sed -e '/EXPORTED FUNCTIONS:/r " outf "' -e '/EXPORTED FUNCTIONS:/,/\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*/d' < " f " > " newf);
    system("mv " f " " f ".orig");
    system("mv " newf " " f);
}    
    
/^BEGINFILE:/{
    if (cur_file != "") {
        endfile(cur_file);
    }
    fname = $2;
    cur_file="../" fname;
    printf ("/*    %-68s  */\n", "EXPORTED FUNCTIONS:") > outf;
    printf ("/*  %-70s  */\n", "") > outf;
    
    next;
}
/^FILENAME:/{
    if ($2 != fname) {
        printf ("Unexpected filename %s\n", $2);
    }
    desc="";
    next;
}
/^FUNCTION:/{funct=$2; tp="Function"; next;}
/^MACRO:/{funct=$2; tp="Macro"; next;}
/^PROGRAM:/{funct=$2; tp="Program"; next;}
/^DATATYPE:/{funct=$2; tp="Data Type"; next;}
/^DESC:/{desc=substr($0,6); next;}
/^HEADER:/{next;}
/^PROTOTYPE:/{
    in_proto = 1;
    next;
}
/^DESCRIPTION:/{
    in_proto = 0;
    in_desc = 1;
    next;
}
/^END_DESCRIPTION/{
    in_desc = 0;
    printf ("/*  %-70s  */\n", "") > outf;
    next;
}
in_proto > 0 {
    x=substr($0,5);
    if (length(x) > 70) {
        printf ("long string: %s\n", x);
    }
    printf ("/*  %-70s  */\n", x) > outf;
    next;
}
in_desc > 0{
    x=substr($0,5);
    if (in_desc == 1 && (x == "NONE" || x == "MISSING") && desc != "") {
        x = desc;
        in_desc = 2;
    }
    if (length(x) > 68) {
        printf ("long string: %s\n", x);
    }
    printf ("/*    %-68s  */\n", x) > outf;
    next;
}
END{
    if (cur_file != "") {
        endfile(cur_file);
    }
}
