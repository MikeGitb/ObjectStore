#include <sos/sos.h>

#include <iostream>

using namespace std;
using namespace mgb;
int main() {
	cout << "Hello World. My name is \"" << sos::my_name()<< "\"" << std::endl;
	cin.get();
}