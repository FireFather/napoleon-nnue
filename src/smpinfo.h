#pragma once
#include "position.h"

class SMPInfo
{
public:
	void updateInfo(const int depth, const int alpha, const int beta, const Pos& position, const bool ready)
	{
		this->beta_ = beta;
		this->depth_ = depth;
		this->alpha_ = alpha;
		this->position_ = position;
		ready_to_search_ = ready;
	}

	int Alpha() const
	{
		return alpha_;
	}

	int Beta() const
	{
		return beta_;
	}

	int Depth() const
	{
		return depth_;
	}

	Pos Board()
	{
		return position_;
	}

	bool Ready() const
	{
		return ready_to_search_;
	}

	void setReady(const bool ready)
	{
		ready_to_search_ = ready;
	}

private:
	int alpha_ = 0;
	int beta_ = 0;
	int depth_ = 0;
	Pos position_;
	bool ready_to_search_ = false;
};
