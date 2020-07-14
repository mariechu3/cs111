
cp $1 corrupted.csv

for i in {1..20}
do
    num_lines=`wc -l < corrupted.csv`
    sed -i $((1 + RANDOM % num_lines))d corrupted.csv
done


num_lines=`wc -l < corrupted.csv`

for i in {1..10}
do
    num_inodes=`grep "INODE" corrupted.csv | wc -l`
    r=$((1+RANDOM%$num_inodes))
    line=`grep "INODE" -n corrupted.csv | head -n $r | tail -1 | awk -F: '{print $1}'`
    # echo line $line
    # echo `head -n $line | tail -1`
    col1=$((13+RANDOM%15))
    col2=$((13+RANDOM%15))
    val1=$((RANDOM%1000))
    val2=$((RANDOM%1000))

    awk -v val=${val1} -v col=${col1} -v line=${line} -F, '{if ($1=="INODE" && NR==line) {$col=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
    awk -v val=${val2} -v col=${col2} -v line=${line} -F, '{if ($1=="INODE" && NR==line) {$col=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
done

for i in {1..10}
do
    num_inodes=`grep "INDIRECT" corrupted.csv | wc -l`
    r=$((1+RANDOM%$num_inodes))
    line=`grep "INDIRECT" -n corrupted.csv | head -n $r | tail -1 | awk -F: '{print $1}'`

    val=$((RANDOM%1000))
    awk -v val=${val} -v line=${line} -F, '{if ($1=="INDIRECT" && NR==line) {$6=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
done

for i in {1..10}
do
    num_inodes=`grep "BFREE" corrupted.csv | wc -l`
    r=$((1+RANDOM%$num_inodes))
    line=`grep "BFREE" -n corrupted.csv | head -n $r | tail -1 | awk -F: '{print $1}'`

    val=$((RANDOM%1000))
    awk -v val=${val} -v line=${line} -F, '{if ($1=="BFREE" && NR==line) {$2=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
done

for i in {1..10}
do
    num_inodes=`grep "IFREE" corrupted.csv | wc -l`
    r=$((1+RANDOM%$num_inodes))
    line=`grep "IFREE" -n corrupted.csv | head -n $r | tail -1 | awk -F: '{print $1}'`

    val=$((RANDOM%1000))
    awk -v val=${val} -v line=${line} -F, '{if ($1=="IFREE" && NR==line) {$2=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
done

for i in {1..10}
do
    num_inodes=`grep "DIRENT" corrupted.csv | wc -l`
    r=$((1+RANDOM%$num_inodes))
    line=`grep "DIRENT" -n corrupted.csv | head -n $r | tail -1 | awk -F: '{print $1}'`
    
    col=$(((1+$RANDOM%2)*2))
    val=$((RANDOM%1000))
    awk -v col=${col} -v val=${val} -v line=${line} -F, '{if ($1=="DIRENT" && NR==line) {$col=val;print;} else {print;}}' OFS=, corrupted.csv > tmp && mv tmp corrupted.csv
done
