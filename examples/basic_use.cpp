#include <sos\sos.h>

#include <iostream>
#include <string>
#include <atomic>
#include <cassert>

using namespace std;
using namespace mgb;


struct MyType {
	static std::atomic_int i;
	std::string message;
	MyType() { i++; }
	MyType(std::string s)
		: message(std::move(s))
	{
		i++;
	}

	~MyType()
	{
		i--;
	}
	static int getCount() { return i; }
};

std::atomic_int MyType::i{ 0 };

int main() {

	sos::SharedObjectStore<MyType,10> store;
	{
		auto handle1 = store.create("Hello1");
		auto handle2 = store.create("Hello2");
		assert(MyType::getCount() == 2);
		auto handle3 = handle2;
		handle1 = handle1;
		handle2->message = "Hello World";
		assert(handle1->message == "Hello1");
	}
	assert(MyType::getCount() == 0);
	std::cout << "Complete!" << std::endl;
	cin.get();
}