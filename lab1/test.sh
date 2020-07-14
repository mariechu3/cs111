#Name: Marie Chu
#email: mariechu@ucla.edu
#UID: 905116878
#!/bin/bash

#checks the wronly works
printf "" >output.txt
./simpsh --wronly output.txt
if [ $? -ne 0 ] ; then
	echo "Exit code should be 0, not $?"l
fi
#checks that rdonly exits with status 0
./simpsh --rdonly output.txt
if [ $? -ne 0 ]; then
	echo "Exit code should be 0, not $?"
fi
#file does not exist so should exit with status 1
./simpsh --rdonly input.txt --verbose --wronly output.txt > output.txt 
if [ $? -ne 1 ]; then
	echo "Exit code should be 1, not $?";
fi
#check if verbose works
if ! grep -s wronly output.txt; then
	echo "--verbose did not work";
fi
echo "This is input" > input.txt
printf "" > error.txt
printf "" > output.txt
#check that command works
./simpsh --rdonly input.txt --wronly output.txt --rdwr error.txt --command 0 1 2 cat
cmp -s input.txt output.txt;
if [ $? -ne 0 ]; then
	echo "--command unsuccessful";
fi
#checks that command is unable to work with invalid index
./simpsh --rdonly input.txt --wronly output.txt --rdwr error.txt --command 0 1 3 cat
if [ $? -ne 1 ]; then
	echo "Invalid index number for command. Exit status should be 1 not $?";
fi
printf "ALL CAPS" > hi.txt
printf "" > bye.txt
printf "" > fail.txt
./simpsh --rdonly input.txt --wronly output.txt --rdwr error.txt --command 0 1 2 cat --rdonly hi.txt --wronly bye.txt --rdwr fail.txt --command 3 4 5 tr [A-Z] [a-z]
OUTPUT=`cat output.txt`
if [[ "$OUTPUT" =~ +[A-Z] ]]; then
	echo "Did not tr upper to lowercase";
fi
rm output.txt input.txt error.txt hi.txt bye.txt fail.txt
echo "PART 2 Test Cases"
#checking fd flags
./simpsh --creat --rdonly new.txt
if [ $? -ne 0 ]; then
	echo "Error with create. Exit status should be 0 not $?"
fi
./simpsh --creat --excl --rdonly new.txt
if [ $? -ne 1 ]; then
	echo "File already exists. Exit status should be 1 not $?"
fi
#checking --abort/--ignore/--default/--catch
./simpsh --abort --catch
if [ $? -ne 139 ]; then
	echo "--abort specified, supposed to exit with 139 not $?"
fi
./simpsh --catch 11 --abort --creat --rdwr output.txt 2> output.txt
printf "11 caught\n" > new.txt
cmp -s output.txt new.txt
if [ $? -ne 0 ]; then
	echo "--catch specified, supposed to catch";
fi
./simpsh --ignore 11 --default 11 --abort
if [ $? -ne 139 ]; then
	echo "--ignore specified, should exit with status 0, not $?";
fi
rm -f output.txt new.txt
rm -rf dir
mkdir dir
./simpsh --chdir dir --creat --rdonly sub.txt --chdir .. command 0 0 0 cat
if [ $? -ne 0 ]; then
	echo "exit status should be 0, not $?"
fi
./simpsh --chdir dir --chdir dir
if [ $? -ne 1 ]; then
	echo "Directory did not exit, should exit with status 1, not $?"
fi
#checking wait
./simpsh --creat --rdonly a.txt --creat --wronly b.txt --creat --rdwr c.txt --command 0 1 2 ./grading_helper 11 --wait
if [ $? -ne 139 ]; then
	echo "child terminated with signal 11, should exit with exit code 139, not $?"
fi

rm -f a.txt b.txt c.txt
