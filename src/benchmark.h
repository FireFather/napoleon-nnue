#pragma once

class Pos;

class Benchmark
{
public:
	explicit Benchmark(Pos&);
	void runPerft(int);
	uint64_t Perft(int);
	void runDivide(int);
	uint64_t Divide(int);
	uint64_t Loop(int);
	void perftTest();
	void ttdTest(int) const;

private:
	Pos& position;
};
