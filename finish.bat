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
set output=B:\Emulated
set qemu=B:\qemu

if not defined workspace (
    set "workspace=%CD%"
)
rem copy /Y "%workspace%\Build\OvmfX64\DEBUG_VS2019\FV\OVMF.fd" "%output%\bios.bin"
copy /Y "%workspace%\GamePkg\Assets\*.bmp" "%output%\Drive\EFI\Game\*.bmp"
copy /Y "%workspace%\Build\GamePkg\DEBUG_VS2019\X64\Game.efi" "%output%\Drive\EFI\Game\Game.efi"
:param
if /i "%1" EQU "build" (
    %workspace%\BaseTools\BinWrappers\WindowsLike\build.bat
    rem copy /Y "%workspace%\Build\OvmfX64\DEBUG_VS2019\FV\OVMF.fd" "%output%\bios.bin"
    copy /Y "%workspace%\GamePkg\Assets\*.bmp" "%output%\Drive\EFI\Game\*.bmp"
    copy /Y "%workspace%\Build\GamePkg\DEBUG_VS2019\X64\Game.efi" "%output%\Drive\EFI\Game\Game.efi"
)
if /i "%1" EQU "run" (
    cd %output%
    %qemu%\qemu-system-x86_64 -L . -bios bios.bin -drive file=fat:rw:%output%/Drive,format=raw -net none -m 512 -vga std
    goto done
)
if "%errorlevel%" EQU "1" goto done
shift
goto param

:done

endlocal
popd