#include "clock.h"

Clock::Clock() noexcept :
	begin(t_clock::now())
{
}

void Clock::Restart()
{
	begin = t_clock::now();
}

double Clock::elapsedMilliseconds() const
{
	return static_cast<double>(std::chrono::duration_cast<MS>(t_clock::now() - begin).count());
}

Clock Clock::startNow()
{
	Clock watch;
	watch.Restart();
	return watch;
}
