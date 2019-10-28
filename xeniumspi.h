//Based on gathering a few commands using my logic analyser on the Xenium SPI bus, the commands seem to match a screen like this:
//https://www.crystalfontz.com/products/document/3391/CFA634_Series_Data_Sheet_Release_2015-03-30.pdf

//Xenium SPI Commands
#define XeniumCursorHome 1
#define XeniumHideDisplay 2
#define XeniumShowDisplay 3
#define XeniumHideCursor 4
#define XeniumShowUnderLineCursor 5
#define XeniumShowBlockCursor 6
#define XeniumShowInvertedCursor 7
#define XeniumBackspace 8
#define XeniumModuleConfig 9
#define XeniumLineFeed 10
#define XeniumDeleteInPlace 11
#define XeniumFormFeed 12 //Clears and resets cursor to home
#define XeniumCarriageReturn 13
#define XeniumSetBacklight 14
#define XeniumSetContrast 15
#define XeniumSetCursorPosition 17 ////The following 2 bytes after the set cursor command is column then row0x
#define XeniumDrawBarGraph 18 //Followed up graph index, style, startcol, endcol, length, row
#define XeniumScrollOn 19
#define XeniumScrollOff 20
#define XeniumWrapOn 23
#define XeniumWrapOff 24
#define XeniumCustomCharacter 25
#define XeniumReboot 26
#define XeniumCursorMove 27 //Followed by 91, then Up 65(0x41),    Down 66 (0x42),    Right 67 (0x43),   Left 68 (0x44) for up,down,right,left
#define XeniumLargeNumber 28

//SPI State Machine
#define SPI_ACTIVE 0
#define SPI_IDLE 1
#define SPI_SYNC 2
#define SPI_WAIT 3
