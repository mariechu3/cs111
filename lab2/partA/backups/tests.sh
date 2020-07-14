#!/bin/bash
#NAME: Marie Chu
#UID: 905116878
#EMAIL: mariechu@ucla.edu

it=10
mult=10
while [ $it -lt 1000000 ]
do
./lab2_add --threads=1 --iterations=$it >>lab2_add.csv
for i in {2..12..2}
do
    if [ $i -ne 6 ] || [ $i -ne 10 ]; then
    ./lab2_add --threads=$i --iterations=$it >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --yield >> lab2_add.csv
    fi
done
it=$((it*mult))
done
for i in {2..12..2}
do
    if [ $i -ne 6 ] || [ $i -ne 10 ]; then
    ./lab2_add --threads=$i --iterations=20 >> lab2_add.csv
    ./lab2_add --threads=$i --iterations=40 >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=80 >> lab2_add.csv 
    ./lab2_add --threads=$i --iterations=20 --yield >> lab2_add.csv
    ./lab2_add --threads=$i --iterations=40 --yield >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=80 --yield >> lab2_add.csv
    fi
done
