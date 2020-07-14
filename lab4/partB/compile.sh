uname -a | grep 905116878 
if [ $? == 0 ];then
    echo "BEAGLE"
    gcc -g -std=c11 -Wall -Wextra -o lab4b lab4.c -lm -lmraa 
else
    echo "DUMMY"
    gcc -g -std=c11 -Wall -Wextra -D DUMMY -o lab4b lab4.c -lm 
fi
