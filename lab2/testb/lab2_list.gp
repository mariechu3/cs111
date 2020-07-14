#!/usr/local/cs/bin/gnuplot

#purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... throughput vs number of threads for mutex and spin-lock synchronized list operations
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#	lab2b_3.png ... successful iterations vs threads for each synchronized method
#	lab2b_4.png ... throughput vs number of threads for mutex synchronized partioned lists
#       lab2b_5.png ... throughput vs number of threads for spin-lock-synchronized partitioned lists
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Throughput vs Number of Threads for synchronized list operations"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only sync with 1000 iterations
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
	title 'Mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7)  \
	title 'Spin' with linespoints lc rgb 'green'


set title "List-2: Wait for Lock Time and Average Time per Operations for Different Number of Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Wait for Lock Time and Average Time per Operations"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait-for-lock' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'avg time/op' with linespoints lc rgb 'red'
     
set title "List-3: Iterations that run without failure with --yield=id and --list=4"
set xlabel "Threads"
set xrange [0.75:]
#unset logscale x
#unset logscale y
#set xtics 0,4
#set ytics 0,10
set ylabel "successful iterations"
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "blue" title "Spin-Lock", \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "red" title "Mutex", \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "green" title "Unprotected"

set title "List-4: Throughput vs Number of Threads for mutex partitioned synchronized list operations"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_4.png'

# grep out only sync with 1000 iterations
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
        title 'lists=1' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7)  \
        title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
        title 'lists=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7)  \
        title 'lists=16' with linespoints lc rgb 'orange'


set title "List-5: Throughput vs Number of Threads for spin-lock partitioned synchronized list operations"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_5.png'

# grep out only sync with 1000 iterations
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
        title 'lists=1' with linespoints lc rgb 'red', \
      "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7)  \
        title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
        title 'lists=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7)  \
        title 'lists=16' with linespoints lc rgb 'orange'

