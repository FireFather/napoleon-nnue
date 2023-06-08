#pragma once
#include <sstream>
#include <iostream>
#include <thread>

class Pos;

namespace Uci
{
	void Start();
	void Go(std::istringstream&);
	void engineInfo();
	extern Pos position;
	extern std::thread search;
}
