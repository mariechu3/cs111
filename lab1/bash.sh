#!/bin/bash

#test 1
#sed 's/ //g' <input.txt 2> test1err.txt | sort 2>test1err.txt | tr [A-Z] [a-z] >translate.txt 2> test1err.txt

#test 2
#sleep 1 < input.txt > test1out.txt 2> test1err.txt
#cat <input.txt 2> test1err.txt | wc 2> test1err.txt >test1out.txt 

#test3
tr " " "\n" < input.txt 2>err.txt | head -n 20 2> err.txt |grep -c n 2> err.txt
times
