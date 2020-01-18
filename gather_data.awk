#!/usr/bin/awk -f

FNR == 1 {
    split(FILENAME, parts, /_|\./);
    printf "%s,", parts[4];
}

/user/{ printf "%s,", $2; }

/.*maximum resident set size/{ print $1; }
