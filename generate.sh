#!/bin/bash

size=$1

echo -n "#define DATA "

seq $size |
    while read n
    do
        echo -n "$RANDOM, "
    done
echo 0
         
