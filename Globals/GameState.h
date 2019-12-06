#include <Uefi.h>
#include <IndustryStandard/Bmp.h>
#pragma once

#define LOCATION_PRECISION 10
#define BMP_TILE_LENGTH 8
#define JUMP_FRAME 3


extern EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *Input;

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundBuffer;
extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *LevelBuffer;
extern UINTN LevelWidth;
extern UINTN LevelHeight;

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SpriteSheet;
extern UINTN SpriteSheetSize;
extern UINTN SpriteSheetHeight;
extern UINTN SpriteSheetWidth;
extern UINTN SpriteLength;

extern BOOLEAN IsRunning;