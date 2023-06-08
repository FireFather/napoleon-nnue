#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>
#include <cctype>

#include "../pragma.h"
#include "misc.h"

FD open_file(const char* name)
{
#ifndef _WIN32
	return open(name, O_RDONLY);
#else
	return CreateFile(name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_FLAG_RANDOM_ACCESS, nullptr);
#endif
}

void close_file(FD fd)
{
#ifndef _WIN32
	close(fd);
#else
	CloseHandle(fd);
#endif
}

size_t file_size(FD fd)
{
#ifndef _WIN32
	struct stat statbuf;
	fstat(fd, &statbuf);
	return statbuf.st_size;
#else
	DWORD sizeHigh;
	const DWORD sizeLow = GetFileSize(fd, &sizeHigh);
	return (static_cast<uint64_t>(sizeHigh) << 32) | sizeLow;
#endif
}

const void* map_file(FD fd, map_t* map)
{
#ifndef _WIN32

	* map = file_size(fd);
	void* data = mmap(NULL, *map, PROT_READ, MAP_SHARED, fd, 0);
#ifdef MADV_RANDOM
	madvise(data, *map, MADV_RANDOM);
#endif
	return data == MAP_FAILED ? NULL : data;

#else

	DWORD sizeHigh;
	const DWORD sizeLow = GetFileSize(fd, &sizeHigh);
	*map = CreateFileMapping(fd, nullptr, PAGE_READONLY, sizeHigh, sizeLow, nullptr);
	if (*map == nullptr)
		return nullptr;
	return MapViewOfFile(*map, FILE_MAP_READ, 0, 0, 0);

#endif
}

void unmap_file(const void* data, map_t map)
{
	if (!data) return;

#ifndef _WIN32

	munmap((void*)data, map);

#else

	UnmapViewOfFile(data);
	CloseHandle(map);

#endif
}

/*
FEN
*/
static constexpr char piece_name[] = "_KQRBNPkqrbnp_";
static constexpr char rank_name[] = "12345678";
static constexpr char file_name[] = "abcdefgh";
static constexpr char col_name[] = "WwBb";
static constexpr char cas_name[] = "KQkq";

void decode_fen(const char* fen_str, int* player, int* castle,
	int* fifty, int* move_number, int* piece, int* square)
{
	/*decode fen*/
	int index = 2;
	const char* p = fen_str, * pfen;
	for (int r = 7; r >= 0; r--)
	{
		for (int f = 0; f <= 7; f++)
		{
			const int sq = r * 8 + f;
			if ((pfen = strchr(piece_name, *p)) != nullptr)
			{
				const int pc = static_cast<int>(strchr(piece_name, *pfen) - piece_name);
				if (pc == 1)
				{
					piece[0] = pc;
					square[0] = sq;
				}
				else if (pc == 7)
				{
					piece[1] = pc;
					square[1] = sq;
				}
				else
				{
					piece[index] = pc;
					square[index] = sq;
					index++;
				}
			}
			else if ((pfen = strchr(rank_name, *p)) != nullptr)
			{
				for (int i = 0; i < pfen - rank_name; i++)
				{
					f++;
				}
			}
			p++;
		}
		p++;
	}
	piece[index] = 0;
	square[index] = 0;

	/*player*/
	if ((pfen = strchr(col_name, *p)) != nullptr)
		*player = ((pfen - col_name) >= 2);
	p++;
	p++;

	/*castling rights*/
	*castle = 0;
	if (*p == '-')
	{
		p++;
	}
	else
	{
		while ((pfen = strchr(cas_name, *p)) != nullptr)
		{
			*castle |= (1 << (pfen - cas_name));
			p++;
		}
	}
	/*epsquare*/
	int epsquare;
	p++;
	if (*p == '-')
	{
		epsquare = 0;
		p++;
	}
	else
	{
		epsquare = static_cast<int>(strchr(file_name, *p) - file_name);
		p++;
		epsquare += 16 * static_cast<int>(strchr(rank_name, *p) - rank_name);
		p++;
	}
	square[index] = epsquare;

	/*fifty & hply*/
	p++;
	if (*p && *(p + 1) && isdigit(*p) && (isdigit(*(p + 1)) || *(p + 1) == ' '))
	{
		sscanf(p, "%d %d", fifty, move_number);
		if (*move_number <= 0) *move_number = 1;
	}
	else
	{
		*fifty = 0;
		*move_number = 1;
	}
}
