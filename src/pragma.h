/*
  Fire is a freeware UCI chess playing engine authored by Norman Schmidt.

  Fire utilizes many state-of-the-art chess programming ideas and techniques
  which have been documented in detail at https://www.chessprogramming.org/
  and demonstrated via the very strong open-source chess engine Stockfish...
  https://github.com/official-stockfish/Stockfish.

  Fire is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  You should have received a copy of the GNU General Public License with
  this program: copying.txt.  If not, see <http://www.gnu.org/licenses/>.
*/

// disable specific compiler warnings
#pragma once
#ifdef _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4146) // unary minus operator applied to unsigned type
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#pragma warning(disable: 4244) // 'initializing' : conversion from '' to '', possible loss of data
#pragma warning(disable: 4458) // declaration of 'depth' hides class member
#pragma warning(disable: 4706) // assignment within conditional expression
#pragma warning(disable: 6031) // Return value ignored: 'sscanf'
#pragma warning(disable: 6326) // Potential comparison of a constant with another constant
#pragma warning(disable: 6386) // Buffer overrun while writing to 'this->kingSquare'
#else
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

