# UefiGame
Bare metal game using EDK2 by [Iyad H](https://github.com/IyadHamid).
To build, clone [edk2](https://github.com/tianocore/edk2.git) and follow build instructions and change the target to GamePkg.dsc. Run using [finish](./finish.bat) after building OvmfPkg with Qemu. Or alternatively take the drive path constructed in [finish](./finish.bat) and place it in a USB (FAT32) and boot Game.efi.

The game loads a a sprite sheet that can easily be changed however the tile size is defined to be 8x8 in the [gamestate](.\globals\gamestate.h)

![sprites.bmp](.\Assets\sprites.bmp "Sprites")
![tiles.bmp](.\Assets\tiles.bmp "Tiles")

## Issues
|Features|Status|
|:-----------------------|-------|
|Overall|Nonfunctional|
|Collisions|Not implemented|
|Level creation|Minimal, undocumented|
|Camera Changes|None|

## Changelog (Latest first)
### v0.1.0
- Made player more similar to a class
- Made 16-bit
### v0.0.0
- Added Collisions w/ Gravity
- Changed controller to be less "console-y"
- Modularized LoadBMP
- Found and squashed memory leak
- Made BmpSupportLib happy and no changes in that are necessary