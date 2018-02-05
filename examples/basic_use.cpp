#include <sos/sos.h>

#include <iostream>
#include <string>
#include <atomic>
#include <cassert>
#include <vector>

using namespace std;
using namespace mgb;


struct MyType {
	std::string message;
	MyType(const MyType& other) = delete;
	MyType(std::string s)
		: message(std::move(s))
	{
	}
};

struct Generator {
	int i = 0;
	int operator()() {
		return i++;
	}
};

int main() {
	std::vector<sos::ConstHandle<MyType>> queue1;
	std::vector<sos::ConstHandle<MyType>> queue2;

	Generator gen;
	sos::SharedObjectStore<MyType,10> store;

	while (store.live_objects_approx() < 10) {
		auto h = store.create(std::to_string(gen()));
		h->message = "Hello" + h->message;
		auto ch = std::move(h).lock();
		queue1.push_back(ch);
		queue2.push_back(ch);
	}
	{
		for (const auto& e : queue1) {
			std::cout << e->message << std::endl;
		}
		for (const auto& e : queue2) {
			std::cout << e->message << std::endl;
		}
	}

}