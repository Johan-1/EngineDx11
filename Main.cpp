#include <iostream>
#include "Framework.h"

int main()
{
	// create framework
	Framework* framework = new Framework();

	// delete framework
	delete framework;

	// exit message
	std::printf("framework deleted, press key followed by enter to exit\n");

	// get input and then exit application
	int kk = 0;
	std::cin >> kk;

	return 0;
}