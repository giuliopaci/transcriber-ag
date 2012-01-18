#include <iostream>
#include <math.h>
#include <iterator>
#include "StringOps.h"
//#include "Parameters.h"

using namespace std;

int main()
{

/*
	Parameters glop;
	glop.load("toto");

	glop.save();

*/

	string t1="A_Jout";
	unsigned int pos;
	pos = t1.find("_");
	t1.erase(pos, 1);
	cout << " erase _ " << t1 << endl;

	string t2 = "event";
	t1 = "event,truc";
	cout << t1.substr(t2.length()+1) << endl;

/*	
	string up=" [Il Fait Beau\n";
	
	cout << "TRIM="<< StringOps(up).trim(" \t\n[]") <<"==" << endl;

	cout << "IS OK beau = " << StringOps(up).contains("beau") << endl;
	cout << "IS OK bel = " << StringOps(up).contains("bel") << endl;


	//string val = " toto=truc; machin=bidule; zorro=beau";
	string val = "  [pron=val]";

	vector<string> v;
	vector<string>::iterator it;

	cout << "INT = " << StringOps().fromInt(12) << endl;
	cout << "float = " << StringOps().fromFloat(1.234) << endl;
	
	StringOps ops(val);
	cout << " VAL =" << val << "--" << endl;
	cout << " cnt1 = " << ops.getTokenCount() << endl;
	cout << " cnt2 = " << ops.getTokenCount(";") << endl;

	val += "  ";
	cout << " VAL =" << val << "--" << endl;
	cout << " cnt1 = " << ops.getTokenCount() << endl;
	cout << " cnt2 = " << ops.getTokenCount(";= ") << endl;
	ops.split(v , " [=] ", true);

	cout << "SPLIT : " << endl;
	copy(v.begin(), v.end(), ostream_iterator<string>(cout, "\n"));

	string m,n;
	unsigned int start  = 0;
	ops.getToken(start, m , " []=");
	cout << "--" << m << "--" << endl;
	ops.getToken(start, n , " =[]; ");
	cout << "--" << n << "--" << endl;
	
	m += 12;
	cout << "--" << m << "--" << endl;
	n = m + 23;
	cout << "--" << n << "--" << endl;

	m += " ";
	m += 12.3456789;
	cout << "--" << m << "--" << endl;

	char buf[20];
	sprintf(buf, "%.3f", roundf(12.3456789));
	cout << "BUF=" << buf << "----" << endl;

	
	string as;
	cout << "FROM FLOAT= " << StringOps(as).fromFloat(12.5678, "%f.2") << endl;
	cout << "FROM FLOAT2= " << as << endl;
	cout << "CHECK FLOAT2= " << StringOps(as).toFloat() << endl;
  */

	return 0;
}
