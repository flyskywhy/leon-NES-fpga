Li Zheng <flyskywhy@gmail.com>

2006.11.27

# SLNES
这是我结合了各种网上流传的 NES 模拟器（ C 以及汇编）再加以优化，用 C 语言所编写的 NES 模拟器，自信是目前运行速度最快的 C 语言 NES 模拟器（以及 6502 内核）了，而且移植方便。 Windows 平台的 SLNES 请用 `VS.net2003` 打开 `win32\SLNES.sln` 。

## doc/士兰捷视项目游戏功能实现方案
文档目录，主要请看 `士兰捷视项目游戏功能实现方案 2.0.doc` ，它是各种参考资料的筛选和总结，通过它以及参考文档，就可以了解 NES 模拟器的源代码（ C 或者汇编）的编写原理。

## doc/gamefile
`doc/gamefile/test/` 目录中是专门用于测试 NES 模拟器是否能正常运行的游戏 ROM ； `doc/gamefile/GameDisk/` 目录中的是市面上流传的一种 VCD 游戏光盘中的 `.bin` 文件的 `.nes` 对应游戏 ROM ；其它目录中的是我从网上下的一些 mapper0、1、2、3、4、7、11 的游戏 ROM 。

## doc/PocketNES
PocketNES 是一个在任天堂掌上游戏机 GBA 上的 NES 游戏模拟器，它是用 ARM 汇编写成的（因为 GBA 中使用 CPU 是 ARM ），游戏速度很快，且有完善的 OSD 菜单（在游戏中同时按下 L、R 键来呼出），如果想要用汇编编写 NES 模拟器，这是一个很好的参考。

`Builder for PocketNES` 是一个将 PocketNES 模拟器及 `.nes` 文件合并为 `.gba` 文件的工具，这个 `.gba` 文件可以通过烧录器烧进 GBA 游戏机所用的游戏卡中，这样 GBA 游戏机就可以玩 `NES` 游戏了，或者通过 GBA 模拟器打开 `.gba` 文件，这样在其它平台如 PC 上也可以玩 NES 游戏，当然“PC->GBA模拟器->NES模拟器”相对于“PC->NES模拟器”来说显然有点多此一举，但对于想通过 PocketNES 的单步调试来了解 PocketNES 的运行方式，以便于用其它汇编语言编写 NES 模拟器的程序员来说，可能是有参考作用的。

## doc/VisualBoyAdvance
GBA 模拟器，这里只提供了普通版本（好像也有 GDB 功能？），网上应该找得到 Debug 版本，可用于调试 `.gba` 文件。

## doc/GBA编程参考
提供了一些 GBA 的软硬件资料，对于理解 PocketNES 的代码很有帮助。

## doc/fceuxd
一个可以调试 `.nes` 文件的 NES 模拟器，这是我当初用来调试 VCD 游戏光盘中的 `.bin` 文件与相应 `.nes` 文件内容不同的“地雷”所用的。
