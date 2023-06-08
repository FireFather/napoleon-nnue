#include "searchinfo.h"

SearchInfo::SearchInfo(const int time, const int depth, const int nodes) noexcept :
	depth(depth),
	nodes(nodes)
{
	SelDepth = 0;
	allocatedTime = time;
	setDepthLimit(100);
}

int SearchInfo::incrementDepth()
{
	return depth++;
}

int SearchInfo::maxDepth() const
{
	return depth;
}

void SearchInfo::setDepthLimit(const int depth)
{
	depthLimit = depth;
}

void SearchInfo::newSearch(const int time)
{
	resetNodes();
	allocatedTime = time;
	depth = 1;
	memset(history, 0, sizeof(history));
	memset(killers, 0, sizeof(killers));
	timer.Restart();
}

void SearchInfo::stopSearch()
{
	setDepthLimit(100);
}

void SearchInfo::resetNodes()
{
	nodes = 0;
}

void SearchInfo::setGameTime(const int time)
{
	allocatedTime = time;
	timer.Restart();
}
