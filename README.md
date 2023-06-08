# napoleon
 
chess engine Napoleon by crybot (Marco Pampaloni)

![alt tag](https://raw.githubusercontent.com/FireFather/napoleon/master/napoleon.bmp)
![alt tag](https://raw.githubusercontent.com/FireFather/napoleon/master/napoleonpp.bmp)

https://github.com/crybot/Napoleon/

Had a chance to work on Marco's excellent multi-threaded engine:

- nnue added
- boost libraries dependency eliminated
- perft/divide added
- source code cleaned up, simplified, and improved
- big strength improvement
- visual studio 2022 (v143) project files included

Support added for an external NNUE (halfkp_256x2-32-32) evaluation (nn.bin) via Daniel Shawul's nnue-probe library: https://github.com/dshawul/nnue-probe.

The executable has been produced using Visual Studio 2022...the project files have been included.

Any halfkp_256x2-32-32 NNUE can be used...see:

https://github.com/FireFather/halfkp_256x2-32-32-nets or

https://tests.stockfishchess.org/nns for a different net.

SF-compatible nets start on page 72-73 (approx.) with dates of 21-05-02 22:26:43 or earlier.

The nnue file size must = 20,530 KB (halfkp_256x2-32-32).

Norman Schmidt firefather@telenet.be
