@echo off
pushd .
::build
set output=B:\Emulated
copy "%workspace%\Build\OvmfX64\DEBUG_VS2019\FV\OVMF.fd" "%output%\bios.bin"
copy "%~dp0\sprites.bmp" "%output%\Drive\EFI\Game\sprites.bmp"
copy "%workspace%\Build\GamePkg\DEBUG_VS2019\X64\Game.efi" "%output%\Drive\EFI\Game\Game.efi"
if /i "%1" EQU "run" (
    cd B:\Emulated
    qemu-system-x86_64 -L . -bios bios.bin -drive file=fat:rw:%output%/Drive,format=raw -net none -m 512 -vga std
    goto done
)

:done
popd