// dummyoutput.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

int main(int argc, char**argv)
{
	std::cout << "dummy text\n";
	std::cerr << "some error text\n";
	for (int i = 1; i < argc; i++)
	{
		std::cout << argv[1] << std::endl;
	}

    return 0;
}

