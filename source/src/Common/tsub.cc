#include <string>
#include <iostream>
using namespace std;
int main ()
{
	string s = "AGSet 'TOTO` already exists!";
	unsigned int pos = s.find_first_of("`'");
	if ( pos != string::npos ) {
		string sub = s.substr(pos+1, s.find_first_of("`'", pos+1)-pos-1);
		cout << "SUB = " << sub << endl;
	}


}

