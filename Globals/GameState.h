#include <Uefi.h>
#include <IndustryStandard/Bmp.h>
#pragma once

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundBuffer;
extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *LevelBuffer;
extern UINTN LevelWidth;
extern UINTN LevelHeight;
extern BOOLEAN IsRunning;