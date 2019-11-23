#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/Shell.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BmpSupportLib.h>  
#include <IndustryStandard/Bmp.h>
#include <Pi/PiFirmwareFile.h>

#include "Graphics.h"
#include "Sprites.h"

EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SpriteSheet;
UINTN SpriteSheetSize;
UINTN SpriteSheetHeight;
UINTN SpriteSheetWidth;
UINTN SpriteLength;

EFI_STATUS
GetScreen (
	IN OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Screen
)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Out;
	UINTN HandleCount;
	EFI_HANDLE* HandleBuffer = NULL;
	//EFI_GRAPHICS_OUTPUT_BLT_PIXEL temp;
	UINTN i;
	EFI_STATUS Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);
	if (EFI_ERROR(Status)) {
		return EFI_SUCCESS;
	}
	i = 0;
	//for (i = 0; i < HandleCount; i++) {
		// Handle protocol
		gBS->HandleProtocol(HandleBuffer[i], &gEfiGraphicsOutputProtocolGuid, (VOID **)&Out);
		*Screen = Out;
		return EFI_SUCCESS;
//	}
	//return EFI_SUCCESS;
}

EFI_STATUS
ScaleBuffer(
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	IN UINTN *Width,
	IN UINTN *Height,
	IN UINTN Scale
)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *In;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Out;
	UINTN OutWidth;
	UINTN OutHeight;
	UINTN x;
	UINTN y;
	//Allocate more space
	OutWidth = *Width * Scale;
	OutHeight = *Height * Scale;
	UINTN Size = sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * OutWidth * OutHeight;
    Out = AllocatePool (Size);
	if (Out == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	In = *Buffer;
	for (y = 0; y < OutHeight; y++) {
		for (x = 0; x < OutWidth; x++) {
			Out[x + y * OutWidth] = In[(x / Scale) + (y / Scale) * *Width];	
		}
	}
	*Buffer = Out;
	*Width = OutWidth;
	*Height = OutHeight;
	return EFI_SUCCESS; 
}

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
)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Out;
	UINTN x;
	UINTN y;
	//Allocate resources
	Out = AllocatePool(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);
	if (Out == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			Out[x + y * Width] = Buffer[(x + SourceX) + (y + SourceY) * SourceWidth];
		}
	}
	FreePool(*NewBuffer);
	*NewBuffer = Out;
	return EFI_SUCCESS;
}

EFI_STATUS
AddToBuffer (
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	IN UINTN SourceWidth,
	IN UINTN SourceHeight,
	IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Addend,
	IN UINTN DestinationX,
	IN UINTN DestinationY,
	IN UINTN Width,
	IN UINTN Height,
	IN BOOLEAN Transparent
)
{
	//EFI_STATUS Status;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Out = *Buffer;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL ZeroPixel;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL Src;
	UINTN x;
	UINTN y;

	if (Transparent) {
		ZeroMem (&ZeroPixel, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	}
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			Src = Addend[x + y * Width];
			if (!(Transparent && CompareMem (&Src, &ZeroPixel, 3) == 0)) {
				Out[(x + DestinationX) + (y + DestinationY) * SourceWidth] = Src;
			}
		}
	}
	*Buffer = Out;
	return EFI_SUCCESS;
}

EFI_STATUS 
LoadBMP (
	CHAR16  *FileName
)
{
	EFI_STATUS Status = EFI_SUCCESS;
    SHELL_FILE_HANDLE FileHandle;
    CHAR16  *FullFileName;

	if (FileName == NULL) {	
        return EFI_UNSUPPORTED;
    }
	
    FullFileName = ShellFindFilePath(FileName);
    if (FullFileName == NULL) {
        Status = EFI_NOT_FOUND;
		Print(L"Failed at Find\n");
        goto Cleanup;
    }
    Status = ShellOpenFileByName(FullFileName, &FileHandle, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
		Print(L"Failed at Open\n");
        goto Cleanup;
    }
    ShellSetFilePosition(FileHandle, 0);
	EFI_FILE_INFO *FileInfo = ShellGetFileInfo(FileHandle);
	void *File = NULL;
	UINTN Size = FileInfo->FileSize;

	ShellReadFile(FileHandle, &Size, File);
	ShellCloseFile(&FileHandle);
	
	Status = TranslateBmpToGopBlt(File, Size, &SpriteSheet, &SpriteSheetSize, &SpriteSheetHeight, &SpriteSheetWidth);
	if (EFI_ERROR(Status)) {
		Print(L"Failed at Translate\n");
		if (Status == RETURN_UNSUPPORTED) {
			Print(L"Unsupported\n");
		}
		if (Status == RETURN_INVALID_PARAMETER) {
			Print(L"Invalid\n");
		}
		if (Status == RETURN_BUFFER_TOO_SMALL) {
			Print(L"To Small\n");
		}
		if (Status == RETURN_OUT_OF_RESOURCES) {
			Print(L"Out of Resources\n");
		}
		goto Cleanup;
	}
	SpriteLength = 8;
	Print(L"Didn't fail\n");
  Cleanup:
    if (FullFileName != NULL) {
       FreePool(FullFileName);
    }
	
	return Status;
}