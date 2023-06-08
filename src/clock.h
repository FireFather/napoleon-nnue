#pragma once
#include <chrono>

using MS = std::chrono::milliseconds;
using t_clock = std::chrono::steady_clock;

class Clock
{
public:
	Clock() noexcept;
	void Restart();
	double elapsedMilliseconds() const;
	static Clock startNow();

private:
	t_clock::time_point begin;
	t_clock::time_point end;
};
