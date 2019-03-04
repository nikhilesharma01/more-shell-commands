#!/bin/bash

echo 'Running ls -l'
gcc -o ls_l ls_l.c
./ls_l "$1"


echo 'Running find'
gcc -o findexe findc.c
./findexe $2 $3

echo 'Running grep'
gcc -o grepexe grepc.c
./grepexe $4 $5

echo 'Running more'
gcc -o morecexe morec.c

if [ "$#" == 6 ];  then
    ./morecexe $6
else
    ./morecexe $6 $7
fi
 
