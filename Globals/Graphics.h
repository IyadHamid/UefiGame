#include <Uefi.h>
#include <IndustryStandard/Bmp.h>
#pragma once

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL ZeroPixel;

EFI_STATUS
GetScreen(
	IN OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Screen
);

EFI_STATUS
ScaleBuffer(
	IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	IN UINTN *Width,
	IN UINTN *Height,
	IN UINTN Scale
);

EFI_STATUS
AddToBuffer(
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	IN UINTN SourceWidth,
	IN UINTN SourceHeight,
	IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Addend,
	IN UINTN DestinationX,
	IN UINTN DestinationY,
	IN UINTN Width,
	IN UINTN Height,
	IN BOOLEAN Transparent
);

EFI_STATUS
ExtractBuffer(
	IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Buffer,
	IN UINTN SourceWidth,
	IN UINTN SourceHeight,
	IN UINTN SourceX,
	IN UINTN SourceY,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **NewBuffer,
	IN UINTN Width,
	IN UINTN Height
);


EFI_STATUS 
LoadBMP (
	IN CHAR16  *FileName,
	OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	OUT UINTN *Height,
	OUT UINTN *Width,
	OUT UINTN *Size
);

EFI_STATUS
InitBackground (
);