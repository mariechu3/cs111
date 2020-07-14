#!/bin/bash
#NAME: Marie Chu
#UID: 905116878
#EMAIL: mariechu@ucla.edu

#png 1
it=0
mult=0
while [ $it -ne 16 ]
do
it=$((2**mult))
if [ $it -eq 16 ]; then
./lab2_list --threads=12 --iterations=1000 --sync=m >>lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >>lab2b_list.csv
fi
./lab2_list --threads=$it --iterations=1000 --sync=m >>lab2b_list.csv
./lab2_list --threads=$it --iterations=1000 --sync=s >>lab2b_list.csv
mult=$((mult+1))
done
./lab2_list --threads=24 --iterations=1000 --sync=m >>lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >>lab2b_list.csv
#png 2
it=0
mult=0
while [ $it -ne 16 ]
do
it=$((2**mult))
./lab2_list --threads=1 --iterations=$it --lists=4 --yield=id >>lab2b_list.csv
for i in {4..16..4}
do
./lab2_list --threads=$i --iterations=$it --lists=4 --yield=id  >>lab2b_list.csv
done
mult=$((mult+1))
done
it=0
mult=0
while [ $it -ne 80 ]
do
it=$((2**mult))
it=$((it*10))
./lab2_list --threads=1 --iterations=$it --lists=4 --sync=s --yield=id >>lab2b_list.csv
./lab2_list --threads=1 --iterations=$it --lists=4 --sync=m --yield=id >>lab2b_list.csv
for i in {4..16..4}
do
./lab2_list --threads=$i --iterations=$it --lists=4 --yield=id --sync=s >>lab2b_list.csv
./lab2_list --threads=$i --iterations=$it --lists=4 --yield=id --sync=m >>lab2b_list.csv
done
mult=$((mult+1))
done

#png 3
it=0
mult=0
while [ $it -ne 8 ]
do
it=$((2**mult))
for i in {4..16..4}
do
if [ $i -ne 12 ]; then
./lab2_list --threads=$it --lists=$i --iterations=1000 --sync=m >>lab2b_list.csv
./lab2_list --threads=$it --lists=$i --iterations=1000 --sync=s >>lab2b_list.csv
if [ $it -eq 8 ]; then
./lab2_list --threads=12 --lists=$i --iterations=1000 --sync=m >>lab2b_list.csv
./lab2_list --threads=12 --lists=$i --iterations=1000 --sync=s >>lab2b_list.csv
fi
fi
done
mult=$((mult+1))
done

