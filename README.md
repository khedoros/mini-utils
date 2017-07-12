# mini-utils
Small random utilities

## dos_header
Prints out header information from DOS MZ (.exe, not .com) binaries

## ega_emu
Emulates some aspects of EGA video hardware, operating in mode 0x0d. Built to view some of the game screens from the game Captain Comic (although it should work with any EGA screens that store graphic-planar EGA data). Examples: Captain Comic, Dangerous Dave 2.

In the future, I should try to support row-planar data, byte-planar data, and linear EGA data. Those would just need new methods to perform the blits in the right order.

## expand
Implements one of the RLE formats used in the Solar Winds games. Pass in an RLE-encoded file, and it will decompress it. Pass in a non-RLE-encoded file, and it'll decompress that too (any 0xff's in the file would be interpreted as "run" commands).
