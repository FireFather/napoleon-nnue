#include <random>
#include "search.h"
#include "eval.h"
#include "movepick.h"
#include "searchterms.h"

hashTable Search::Hash;
bool Search::pondering = false;
std::atomic<bool> Search::ponderHit(false);
std::atomic<bool> Search::stopSignal(true);
std::atomic<bool> Search::quit(false);
int Search::gameTime[2];
int Search::moveTime;
thread_local bool Search::sendOutput = false;
thread_local SearchInfo Search::searchInfo;
std::vector<std::thread> Search::threads;
SMPInfo Search::smpInfo;
std::condition_variable Search::smp;
std::mutex mux;
int Search::depth_limit = 100;
int Search::cores;
const int Search::defaultCores = 1;

int Search::razorMargin(const int depth)
{
	return (rzMultiplier * (depth - 1) + rzMargin);
}

int Search::futilityMargin(const int depth)
{
	return (fpMargin * depth);
}

int Search::predictTime(const uint8_t color)
{
	const int GameTime = gameTime[color];
	return GameTime / 30 - (GameTime / (60 * 1000));
}

Move Search::startThinking(const SearchType type, Pos& position, const bool verbose)
{
	Hash.Clear();

	sendOutput = verbose;
	stopSignal = false;
	pondering = false;
	ponderHit = false;
	searchInfo.setDepthLimit(depth_limit);

	if (type == SearchType::Infinite || type == SearchType::Ponder)
	{
		if (type == SearchType::Ponder)
			pondering = true;
		searchInfo.newSearch();
	}
	else
	{
		int time;

		if (type == SearchType::TimePerGame)
		{
			const int GameTime = gameTime[position.getSideToMove()];
			time = GameTime / 30 - (GameTime / (60 * 1000));
		}
		else
		{
			time = moveTime;
		}

		searchInfo.newSearch(time);
	}

	const Move move = iterativeSearch(position);

	if (sendOutput)
	{
		const Move ponder = getPonderMove(position, move);

		if (ponder.isNull())
			std::cout << "bestmove " << move.toAlgebraic() << std::endl;
		else
			std::cout << "bestmove " << move.toAlgebraic() << " ponder " << ponder.toAlgebraic() << std::endl;
	}
	searchInfo.stopSearch();
	return move;
}

void Search::stopThinking()
{
	stopSignal = true;
	smpInfo.setReady(false);
}

void Search::killThreads()
{
	quit = true;
	smp.notify_all();

	for (auto& t : threads)
		t.join();
	threads.clear();
	quit = false;
}

void Search::initThreads(const int num_threads)
{
	killThreads();
	{
		std::lock_guard<std::mutex> lock(mux);
		smpInfo.setReady(false);
	}
	cores = num_threads;
	for (int i = 1; i < cores; i++)
		threads.emplace_back(smpSearch);
}

void Search::signalThreads(const int depth, const int alpha, const int beta, const Pos& position, const bool ready)
{
	std::unique_lock<std::mutex> lock(mux);
	smpInfo.updateInfo(depth, alpha, beta, position, ready);
	lock.unlock();
	smp.notify_all();
}

void Search::smpSearch()
{
	std::default_random_engine eng;
	std::uniform_int_distribution<int> score_dist(0, 25); // to tune

	/* thread local information */
	sendOutput = false;
	searchInfo.newSearch();

	const auto move = new Move();
	const auto position = new Pos();
	while (!quit)
	{
		std::unique_lock<std::mutex> lock(mux);
		smp.wait(lock, [] { return quit || smpInfo.Ready(); });
		auto info = smpInfo;
		lock.unlock();
		if (quit)
			break;
		const int rand_window = score_dist(eng);
		auto fen = info.Board().getFen();

		if (position->getFen() != fen)
		{
			searchInfo.newSearch();
			*position = info.Board();
		}
		searchRoot(info.Depth(), info.Alpha() - rand_window, info.Beta() + rand_window, std::ref(*move),
			std::ref(*position));
	}
}

// iterative deepening
Move Search::iterativeSearch(Pos& position)
{
	Move move;
	Move toMake = nullMove;

	int score = searchRoot(searchInfo.maxDepth(), -Infinity, Infinity, move, position);
	searchInfo.incrementDepth();

	while ((searchInfo.maxDepth() < 100 && !searchInfo.timeOver()) || pondering)
	{
		if (stopSignal)
			break;

		if (ponderHit)
		{
			searchInfo.setGameTime(predictTime(position.getSideToMove()));
			ponderHit = false;
			pondering = false;
		}

		searchInfo.SelDepth = 0;
		searchInfo.resetNodes();

		if (searchInfo.maxDepth() > 5 && cores > 1)
			signalThreads(searchInfo.maxDepth(), -Infinity, Infinity, position, true);

		// aspiration search
		int temp = searchRoot(searchInfo.maxDepth(), score - aspirationValue, score + aspirationValue, move, position);

		if (temp <= score - aspirationValue)
			temp = searchRoot(searchInfo.maxDepth(), -Infinity, score + aspirationValue, move, position);

		if (temp >= score + aspirationValue)
			temp = searchRoot(searchInfo.maxDepth(), score - aspirationValue, Infinity, move, position);

		score = temp;

		if (score != Unknown)
			toMake = move;

		searchInfo.incrementDepth();
	}

	stopThinking();

	return toMake;
}

int Search::searchRoot(const int depth, int alpha, const int beta, Move& moveToMake, Pos& position)
{
	int score;
	const int startTime = static_cast<int>(searchInfo.elapsedTime());

	MovePick moves(position, searchInfo);
	moveGen::getLegalMoves(moves.moves, moves.count, position);

	// chopper pruning
	if (moves.count == 1)
	{
		moveToMake = moves.Next();
		return alpha;
	}

	moves.Sort<false>();

	int i = 0;

	for (auto move = moves.First(); !move.isNull(); move = moves.Next(), i++)
	{
		if ((searchInfo.timeOver() || stopSignal))
			return Unknown;

		position.makeMove(move);

		if (i == 0)
			score = -Search::search<NodeType::PV>(depth - 1, -beta, -alpha, 1, position, false);
		else
		{
			score = -search<NodeType::NONPV>(depth - 1, -alpha - 1, -alpha, 1, position, true);

			if (score > alpha)
				score = -search<NodeType::PV>(depth - 1, -beta, -alpha, 1, position, false);
		}
		position.undoMove(move);

		if (score > alpha)
		{
			moveToMake = move;

			if (score >= beta)
			{
				if (sendOutput)
					std::cout << "info " << getInfo(position, moveToMake, beta, depth, startTime) << std::endl;
				return beta;
			}

			alpha = score;
		}
	}

	if (sendOutput)
		std::cout << "info " << getInfo(position, moveToMake, alpha, depth, startTime) << std::endl;

	return alpha;
}

template <NodeType node_type>
int Search::search(int depth, int alpha, int beta, const int ply, Pos& position, const bool cut_node)
{
	searchInfo.visitNode();

	auto bound = ScoreType::Alpha;
	const bool pv = node_type == NodeType::PV;
	bool futility = false;
	bool extension = false;
	int score;
	int legal = 0;

	if (ply > searchInfo.SelDepth)
		searchInfo.SelDepth = ply;

	if (searchInfo.Nodes() % 10000 == 0 && sendOutput)
		if (searchInfo.timeOver())
			stopSignal = true;

	if (stopSignal)
		return alpha;

	// mate distance pruning
	alpha = std::max(alpha, -Mate + ply);
	beta = std::min(beta, Mate - ply - 1);

	if (alpha >= beta)
		return alpha;

	// Hash table lookup
	auto hashHit = Hash.Probe(position.zobrist, depth, alpha, beta);

	if ((score = hashHit.first) != hashTable::Unknown)
		return score;
	Move best = hashHit.second;

	const uint64_t attackers = position.kingAttackers(position.getKingSquare(position.getSideToMove()),
		position.getSideToMove());

	if (attackers)
	{
		extension = true;
		++depth;
	}

	if (depth == 0)
		return quiescence(alpha, beta, position);

	if (position.isRepetition())
		return 0;

	const int eval = Eval::evaluate(position);

	// cutoff
	if (depth <= coDepth
		&& !pv
		&& !attackers
		&& std::abs(alpha) < Mate - maxPly
		&& std::abs(beta) < Mate - maxPly
		&& eval - coMultiplier * depth >= beta)
	{
		return beta;
	}

	// null move pruning
	if (position.getAllowNullMove()
		&& !pv
		&& depth >= nmpDepth
		&& !attackers
		&& !position.EG())
	{
		const int R = depth >= nmpReductionDepth ? nmpReduction1 : nmpReduction2;

		// cut node
		position.makeNullMove();

		// make a null-window search
		score = -search<NodeType::NONPV>(depth - R - 1, -beta, -beta + 1, ply, position, !cut_node);
		position.undoNullMove();

		if (score >= beta)
			return beta;
	}

	// internal iterative deepening (IID)
	if (depth >= iidDepth && best.isNull() && pv)
	{
		const int R = iidReduction;

		if (position.getAllowNullMove())
			position.toggleNullMove();

		search<node_type>(depth - R - 1, alpha, beta, ply, position, cut_node);

		if (!position.getAllowNullMove())
			position.toggleNullMove();

		//hash table lookup
		hashHit = Hash.Probe(position.zobrist, depth, alpha, beta);

		best = hashHit.second;
	}

	// razoring
	if (!pv
		&& depth <= rzDepth
		&& eval + razorMargin(depth) <= alpha)
	{
		const int res = quiescence(alpha - razorMargin(depth), beta - razorMargin(depth), position);

		if (res + razorMargin(depth) <= alpha)
			depth--;

		if (depth <= 0)
			return alpha;
	}

	// futility pruning
	if (!pv
		&& depth <= fpDepth
		&& eval + futilityMargin(depth) <= alpha)
		futility = true;

	// extended futility pruning
	if (!pv
		&& depth > efpDepth
		&& eval + efpMargin <= alpha)
		futility = true;

	MovePick moves(position, searchInfo);

	moveGen::getPseudoLegalMoves<false>(moves.moves, moves.count, attackers, position); // get captures and non-captures

	moves.Sort<false>(ply);
	moves.hashMove = best;

	// principal variation search
	bool pruned = false;

	int moveNumber = 0;
	int newDepth = depth;
	const uint64_t pinned = position.pinnedPieces();

	for (auto move = moves.First(); !move.isNull(); move = moves.Next())
	{
		if (position.isMoveLegal(move, pinned))
		{
			legal++;
			constexpr int E = 0;
			newDepth = depth + E;
			const bool capture = position.isCapture(move);
			position.makeMove(move);

			// futility pruning
			if (futility
				&& moveNumber > 0
				&& !capture
				&& !move.isPromotion()
				&& !position.kingAttackers(position.getKingSquare(position.getSideToMove()), position.getSideToMove()))
			{
				pruned = true;
				position.undoMove(move);
				continue;
			}

			if (moveNumber == 0)
			{
				score = -search<node_type>(newDepth - 1, -beta, -alpha, ply + 1, position, !cut_node);
			}
			else
			{
				int R = 0;
				const int lmrN = newDepth >= lmrNewDepth ? lmrN1 : lmrN2;

				// late move reduction
				if (moveNumber >= lmrN
					&& newDepth >= lmrDepth
					&& !extension
					&& !capture
					&& !move.isPromotion()
					&& !attackers
					&& move != searchInfo.firstKiller(ply)
					&& move != searchInfo.secondKiller(ply)
					&& !position.kingAttackers(position.getKingSquare(position.getSideToMove()),
						position.getSideToMove()))
				{
					R = lmr1;
					if (moveNumber > lmrMoveNumber)
						R = lmr2;
				}

				newDepth = std::max(1, depth - R);

				score = -search<NodeType::NONPV>(newDepth - 1, -alpha - 1, -alpha, ply + 1, position, !cut_node);

				if (score > alpha)
				{
					newDepth = depth;
					score = -search<NodeType::PV>(newDepth - 1, -beta, -alpha, ply + 1, position, !cut_node);
				}
			}

			position.undoMove(move);

			if (score >= beta)
			{
				if (move == best) // we don't want to save our hash move also as a killer move
					return beta;

				//killer moves and history heuristic
				if (!position.isCapture(move))
				{
					searchInfo.setKillers(move, ply);
					searchInfo.setHistory(move, position.getSideToMove(), newDepth);
				}

				// for safety, we don't save forward pruned nodes inside transposition table
				if (!pruned)
					Hash.Save(position.zobrist, newDepth, beta, best, ScoreType::Beta);

				return beta; //  fail hard beta-cutoff
			}

			if (score > alpha)
			{
				bound = ScoreType::Exact;
				alpha = score; // alpha acts like max in MiniMax
				best = move;
			}

			moveNumber++;
		}
	}

	// check for stalemate and checkmate
	if (legal == 0)
	{
		if (position.getIsCheck())
			alpha = -Mate + ply; // return best score for the deepest mate
		else
			alpha = 0; // return draw score
	}

	// check for fifty moves rule
	if (position.getHalfMoveClock() >= 100)
		alpha = 0;

	// for safety, we don't save forward pruned nodes inside transposition table
	if (!pruned)
		Hash.Save(position.zobrist, newDepth, alpha, best, bound);

	return alpha;
}

// quiescence is called at horizon nodes (depth = 0)
int Search::quiescence(int alpha, const int beta, Pos& position)
{
	searchInfo.visitNode();

	const uint64_t attackers = position.kingAttackers(position.getKingSquare(position.getSideToMove()),
		position.getSideToMove());
	const bool inCheck = attackers;
	int stand_pat = 0; // to suppress warning

	if (!inCheck)
	{
		stand_pat = Eval::evaluate(position);

		if (stand_pat >= beta)
			return beta;

		int Delta = pieceValue[Queen];

		if (position.isPromotingPawn())
			Delta += pieceValue[Queen] - pieceValue[Pawn];

		// big delta futility pruning
		if (stand_pat < alpha - Delta)
			return alpha;

		if (alpha < stand_pat)
			alpha = stand_pat;
	}

	// TO TEST
	if (position.isRepetition())
		return 0;

	const uint64_t pinned = position.pinnedPieces();

	MovePick moves(position, searchInfo);

	if (!inCheck)
		moveGen::getPseudoLegalMoves<true>(moves.moves, moves.count, attackers, position); // get all capture moves
	else
		moveGen::getPseudoLegalMoves<false>(moves.moves, moves.count, attackers, position); // get all evading moves

	moves.Sort<true>();

	for (auto move = moves.First(); !move.isNull(); move = moves.Next())
	{
		// delta futility pruning
		if (!inCheck)
		{
			if (!move.isPromotion() && (pieceValue[position.pieceOnSquare(move.toSquare()).Type]
				+ stand_pat + dfpMargin <= alpha || position.See(move) < 0))
				continue;
		}

		if (position.isMoveLegal(move, pinned))
		{
			position.makeMove(move);
			const int score = -quiescence(-beta, -alpha, position);
			position.undoMove(move);

			if (score >= beta)
				return beta;

			if (score > alpha)
				alpha = score;
		}
	}

	return alpha;
}

std::string Search::getPV(Pos& position, const Move toMake, const int depth)
{
	std::string pv;

	if (toMake.isNull() || depth == 0)
		return pv;
	pv = toMake.toAlgebraic() + " ";

	position.makeMove(toMake);
	pv += getPV(position, Hash.getPV(position.zobrist), depth - 1);
	position.undoMove(toMake);

	return pv;
}

std::string Search::getInfo(Pos& position, const Move toMake, const int score, const int depth, const int startTime)
{
	std::ostringstream info;
	const double delta = searchInfo.elapsedTime() - startTime;
	const double nps = (delta > 0
		? searchInfo.Nodes() * static_cast<double>(cores) / delta
		: searchInfo.Nodes() * static_cast<double>(cores) / 1) * static_cast<double>(1000);

	info << "depth " << depth << " seldepth " << searchInfo.SelDepth;

	if (std::abs(score) >= Mate - maxPly)
	{
		int plies = Mate - std::abs(score) + 1;

		if (score < 0) // mated
			plies *= -1;

		info << " score mate " << plies / 2;
	}
	else
		info << " score cp " << score;

	info << " time " << searchInfo.elapsedTime()
		<< " nodes " << searchInfo.Nodes() * cores
		<< " nps " << static_cast<int>(nps)
		<< " pv " << getPV(position, toMake, depth);

	return info.str();
}

Move Search::getPonderMove(Pos& position, const Move toMake)
{
	position.makeMove(toMake);
	const Move move = Hash.getPV(position.zobrist);
	position.undoMove(toMake);

	return move;
}
