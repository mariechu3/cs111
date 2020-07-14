#include <iostream>
#include <string>
using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2)
	cout << "Wrong number of arguments.\n";
    string arg = argv[1];
    if(arg.substr(arg.length()-4,4) != ".csv")
	cout << "Must specify a csv file.\n";  

}
