# UefiGame
Bare metal game using EDK2 by [Iyad H](https://github.com/IyadHamid).
To build, clone [edk2](https://github.com/tianocore/edk2.git) and follow build instructions and change the target to GamePkg.dsc. Run using [finish](./finish.bat) after building OvmfPkg with Qemu. Or alternatively take the drive path constructed in [finish](./finish.bat) and place it in a USB (FAT32) and boot Game.efi.

## Issues
|Features|Status|
|:-----------------------|-------|
|Overall|Working (as intended)|
|Controller needs to be redone/fixed|In progress|
|Physics (Collision/Gravity/Friction)|None|
|Scrolling Camera|None|
|Screen protocol selector|None|
|Modularize LoadBmp|None|

## Changelog (Latest first)
### v0.0.0
- Found and squashed memory leak
- Made BmpSupportLib happy and no changes in that are necessary