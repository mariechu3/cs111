#!/bin/bash
#NAME: Marie Chu
#UID: 905116878
#EMAIL: mariechu@ucla.edu
#PART 1
#:<<'END'
it=10
mult=10
while [ $it -lt 1000000 ]
do
./lab2_add --threads=1 --iterations=$it >>lab2_add.csv
./lab2_add --threads=1 --iterations=$it --yield >>lab2_add.csv
./lab2_add --threads=1 --iterations=$it --sync=m >> lab2_add.csv
./lab2_add --threads=1 --iterations=$it --sync=s >>lab2_add.csv
./lab2_add --threads=1 --iterations=$it --sync=c >> lab2_add.csv
for i in {2..12..2}
do
    if [ $i -ne 6 ] && [ $i -ne 10 ]; then
    ./lab2_add --threads=$i --iterations=$it >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --yield >> lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --sync=m >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --sync=s >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --sync=c >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --sync=m --yield >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=$it --sync=c --yield >>lab2_add.csv
    if [ $it -lt 10000 ]; then
    ./lab2_add --threads=$i --iterations=$it --sync=s --yield >>lab2_add.csv
    fi
    fi
done
it=$((it*mult))
done
for i in {2..12..2}
do
    if [ $i -ne 6 ] && [ $i -ne 10 ]; then
    ./lab2_add --threads=$i --iterations=20 >> lab2_add.csv
    ./lab2_add --threads=$i --iterations=40 >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=80 >> lab2_add.csv 
    ./lab2_add --threads=$i --iterations=20 --yield >> lab2_add.csv
    ./lab2_add --threads=$i --iterations=40 --yield >>lab2_add.csv
    ./lab2_add --threads=$i --iterations=80 --yield >> lab2_add.csv
    fi
done
#END
#PART 2
it=10
mult=10
while [ $it -lt 1000000 ]
do
./lab2_list --threads=1 --iterations=$it >>lab2_list.csv
it=$((it*mult))
done
./lab2_list --threads=1 --iterations=20000 >>lab2_list.csv
it=1
while [ $it -lt 10000 ]
do
for i in {2..12..2}
do
    if [ $i -ne 6 ] && [ $i -ne 10 ]; then
    ./lab2_list --threads=$i --iterations=$it >>lab2_list.csv
    fi
done
it=$((it*mult))
done
mult=0
it=0
while [ $it -lt 64 ]
do
it=$((2**mult))
for i in {2..12..2}
do
    if [ $i -ne 6 ] && [ $i -ne 10 ]; then
    ./lab2_list --threads=$i --iterations=$it --yield=i >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=d >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=il >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=dl >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=i --sync=m >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=d --sync=m >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=il --sync=m >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=dl --sync=m >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=i --sync=s >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=d --sync=s >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=il --sync=s >> lab2_list.csv
    ./lab2_list --threads=$i --iterations=$it --yield=dl --sync=s >> lab2_list.csv
    fi
done
mult=$((mult+1));
done
it=0
mult=0
while [ $it -lt 32 ]
do
it=$((2**mult))
./lab2_list --threads=$it --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=$it --iterations=1000 --sync=s >>lab2_list.csv
mult=$((mult+1))
done
./lab2_list --threads=12 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >>lab2_list.csv

