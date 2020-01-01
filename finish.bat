@echo off
pushd .
::Notice, this is very specific to my builds and file format
::Change variable output to where file should go
::At output add this tree
::-%output%
::|-bios.bin (will be automatically copied from OVMG.fd)
::|-EFI             (ADD THIS FOLDER)
::  |-Game          (ADD THIS FOLDER)
::    |-Game.efi    (will be copied from build)
::    |-sprites.bmp (will be copied from this directory)
::    |-map.bmp     (will be copied from this directory)
setlocal
set output=%~p0\Emulated
set qemu=B:\qemu
if not defined workspace (
    if /i "%1" NEQ "run" (
        rem note this is a custom script to setup the enviroment for edk2
        edkhelp
    )
)
if defined workspace (
    copy /Y "%~p0\Assets\*.bmp" "%output%\Drive\EFI\Game\*.bmp"
    copy /Y "%workspace%\Build\GamePkg\DEBUG_VS2019\X64\Game.efi" "%output%\Drive\EFI\Game\Game.efi"
)
:param
if /i "%1" EQU "build" (
    call %workspace%\BaseTools\BinWrappers\WindowsLike\build.bat -p ..\UefiGame\GamePkg.dsc
    if "%errorlevel%" EQU "1" goto done
    copy /Y "%workspace%\Build\OvmfX64\DEBUG_VS2019\FV\OVMF.fd" "%output%\bios.bin"
    copy /Y "%workspace%\Build\GamePkg\DEBUG_VS2019\X64\Game.efi" "%output%\Drive\EFI\Game\Game.efi"
    goto shift
)
if /i "%1" EQU "run" (
    cd %output%
    start %qemu%\qemu-system-x86_64 -L . -bios bios.bin -drive file=fat:rw:%output%/Drive,format=raw -net none -m 512 -vga std -debugcon file:debug.log -global isa-debugcon.iobase=0x402
    goto shift
)
:shift
shift /1
if "%1" NEQ "" goto param

:done

endlocal
popd