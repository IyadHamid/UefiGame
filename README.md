# UefiGame
Bare metal game using EDK2.
To build, clone [edk2](https://github.com/tianocore/edk2.git) and follow build instructions and change the target to GamePkg.dsc. Run using [finish](./finish.bat) after building OvmfPkg.

## Issues
|Features|Status|
|:-----------------------|-------|
|Possible memory leak|Finding|
|ConIn is sketch, need to read from 0x60|None|
|Screen protocol selector|None|

## Changelog
Made BmpSupportLib happy and no changes in that are necessary