#include "Demo1\Demo_1.h"
#include "Demo2\Demo_2.h"
#include "DemoVK1\DemoVK_1.h"

int main(int argc, char* argv[]) {
	//Demo_1 d;
	//Demo_2 d;
	DemoVK_1 d;

	int r = d.Execute();
	if (r < 0) {
		std::cin.get();
	}
	return r;
}