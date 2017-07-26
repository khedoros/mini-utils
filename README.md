# mini-utils
Small random utilities. Mostly related to playing with data files in old computer games.

## dos_header
Prints out header information from DOS MZ (.exe, not .com) binaries

## ega_emu
Emulates some aspects of EGA video hardware, operating in mode 0x0d. Built to view some of the game screens from the game Captain Comic (although it should work with any EGA screens that store graphic-planar EGA data). Examples: Captain Comic, Dangerous Dave 2.

In the future, I should try to support row-planar data, byte-planar data, and linear EGA data. Those would just need new methods to perform the blits in the right order.

## expand
Implements one of the RLE formats used in the Solar Winds games. Pass in an RLE-encoded file, and it will decompress it. Pass in a non-RLE-encoded file, and it'll decompress that too (any 0xff's in the file would be interpreted as "run" commands).

## data-view
Implements *two* of the RLE formats used in the Solar Winds games, as well as raw file data. Provide a 768-byte palette file and some other data file. The data file's bytes will be rendered in the window. w-a-s-d resize the window (Useful for visualizing data sizes and alignment), and i-j-k-l shift by one byte or one row of bytes. f switches raw-rle1-rle2. Bonus: Sometimes it crashes if the file isn't actually rle-encoded.

## mandel
Mandelbrot generator. Displays the set at 1024x1024 resolution for 10 seconds, then exits.

## life
Conway's game of life. Give it a width, height, and desired framerate. Display is capped at 60fps, I think, but the generation simulation should only be limited by your CPU. Spacebar shows a readout of how many cells change between generations. 'r' restarts the simulation (seeds with a new random number, regenerates the map, starts the simulation). Any other key should exit.

## bfi
BrainF\*\*\* programming language interpreter, along with some test/example code. 
- test1.bf: take input of two characters, output their sum.
- test2.bf: add 2 and 5, then convert the output value to ASCII
- test3.bf and test4.bf: alternate implementations of "hello world" programs
