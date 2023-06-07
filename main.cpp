#include "search.h"
#include "nnue-probe/nnue.h"

int main()
{
	Uci::engineInfo();
	Search::Hash.setSize(32);
	Search::initThreads();
	nnue_init("nn.bin");
	Uci::Start();
	return 0;
}
