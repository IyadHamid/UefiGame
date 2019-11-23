#include <Uefi.h>
#include <IndustryStandard/Bmp.h>
#pragma once

#define FRAME_DURATION 10
#define JUMP_FRAME 3

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SpriteSheet;
extern UINTN SpriteSheetSize;
extern UINTN SpriteSheetHeight;
extern UINTN SpriteSheetWidth;
extern UINTN SpriteLength;
