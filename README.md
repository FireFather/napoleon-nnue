# napoleon
 
**chess engine Napoleon by crybot (Marco Pampaloni)

![alt tag](https://raw.githubusercontent.com/FireFather/napoleon/master/logos/napoleon.bmp)
![alt tag](https://raw.githubusercontent.com/FireFather/napoleon/master/logos/napoleonpp.bmp)

https://github.com/crybot/Napoleon/

**some improvements to Marco's excellent multi-threaded engine**

- nnue added
- boost libraries dependency eliminated
- perft/divide added
- source code cleaned up, simplified, and improved
- re-factored and optimized according to analysis by Clang, ReSharper C++, and Visual Studio Code
- big strength improvement
- visual studio 2022 (v143) project files included

Support has been added for an external NNUE (halfkp_256x2-32-32) evaluation (nn.bin) via Daniel Shawul's nnue-probe library: https://github.com/dshawul/nnue-probe.

**strength estimate**

| 					  |       |       |                   |       |
| --------------------| ----- |------ | ----------------- | ----- |
|  napoleon-nnue      | 2624 |  16384 |(+6857,=6639,-2888)| 62.1 %|
|  vs.                |      | games |(    +,    =,    -)|   (%) |
|  quazar-0.4         | 2727 |   2048 |(+926,=781,-341)   | 64.3 %|
|  leorik-2.2         | 2706 |   2048 |(+780,=958,-310)   | 61.5 %|
|  coiled-1.1         | 2547 |   2048 |(+345,=917,-786)   | 39.2 %|
|  barbarossa-0.6.0   | 2502 |   2048 |(+246,=870,-932)   | 33.3 %|
|  amyan-1.72         | 2495 |   2048 |(+173,=981,-894)   | 32.4 %|
|  ifrit-m1.8         | 2476 |   2048 |(+188,=857,-1003)  | 30.1 %|
|  chispa-4.0.3       | 2429 |   2048 |(+154,=705,-1189)  | 24.7 %|
|  kingfisher-1.0     | 2354 |   2048 |(+76,=570,-1402)   | 17.6 %|

Games Completed = 16384 of 16384 (Avg game length = 6.593 sec)
Settings = Gauntlet/32MB/1000ms+100ms/M 500cp for 6 moves, D 120 moves/EPD:book.epd(31526)	

Visual Studio 2022 used...the project files have been included.
The executable has been produced using MSYS2 mingw-w64-x86_64-toolchain.

**Any halfkp_256x2-32-32 NNUE can be used...see**

https://github.com/FireFather/halfkp_256x2-32-32-nets or

https://tests.stockfishchess.org/nns for a different net.

Compatible nets start on page 72-73 (approx.) with dates of 21-05-02 22:26:43 or earlier.

The nnue file size must = 20,530 KB (halfkp_256x2-32-32).

Norman Schmidt firefather@telenet.be
