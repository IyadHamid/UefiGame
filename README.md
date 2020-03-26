# UefiGame
Bare metal game using EDK2 by [Iyad H](https://github.com/IyadHamid).
To build, clone [edk2](https://github.com/tianocore/edk2.git) and follow build instructions and change the target to GamePkg.dsc. Run using the path [Drive](./Emulated/Drive/EFI) and run it in QEMU or copy it into a USB (FAT32) and run Game.efi through [UEFI Shell](./Emulated/Drive/EFI/Boot/Shell.efi).

The game loads a a sprite sheet that can easily be changed however the tile size is defined to be 8x8 in the [gamestate](.\Globals\gamestate.h)

![sprites.bmp](.\Assets\sprites.bmp "Sprites")
![tiles.bmp](.\Assets\tiles.bmp "Tiles")

## Issues/Goals
|Features|Status|
|:-----------------------|-------|
|Overall|half-functional|
|Collisions|Not tested|
|Level creation|Minimal, undocumented|
|Slopes|not implemented|
|Camera Changes|None|

## Changelog (Latest first)
### v0.1.0
- Used more optimized sensors similar to retro-Sonic
- Made player more similar to a class
- Made 16-bit images
### v0.0.0
- Added Collisions w/ Gravity
- Changed controller to be less "console-y"
- Modularized LoadBMP
- Found and squashed memory leak
- Made BmpSupportLib happy and no changes in that are necessary