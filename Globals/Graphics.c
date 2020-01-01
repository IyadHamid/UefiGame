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

#include "Globals/GameState.h"
#include "Globals/Graphics.h"

EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SpriteSheet;
UINTN SpriteSheetSize;
UINTN SpriteSheetHeight;
UINTN SpriteSheetWidth;
UINTN SpriteLength;

EFI_STATUS
GetScreen (
	IN OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Screen
)
{	//TBA Screen select
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Out;
	UINTN HandleCount;
	EFI_HANDLE* HandleBuffer = NULL;
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
	IN OUT UINTN *Width,
	IN OUT UINTN *Height,
	IN UINTN Scale
)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Out;
	UINTN OutWidth;
	UINTN OutHeight;
	UINTN x;
	UINTN y;

	//Allocate more space/get new sizes
	OutWidth = *Width * Scale;
	OutHeight = *Height * Scale;
    Out = AllocatePool (sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * OutWidth * OutHeight);
	if (Out == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	for (y = 0; y < OutHeight; y++) {
		for (x = 0; x < OutWidth; x++) {
			Out[x + y * OutWidth] = (*Buffer)[(x / Scale) + (y / Scale) * *Width];	
		}
	}

	FreePool(*Buffer);
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
	UINTN x;
	UINTN y;
	//Allocate resources
	*NewBuffer = AllocatePool(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);
	if (*NewBuffer == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			(*NewBuffer)[x + y * Width] = Buffer[(x + SourceX) + (y + SourceY) * SourceWidth];
		}
	}
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
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL Src;
	UINTN x;
	UINTN y;

	for (y = 0; y < Height && y + DestinationY < SourceHeight; y++) {
		for (x = 0; x < Width && y + DestinationX < SourceHeight; x++) {
			Src = Addend[x + y * Width];
			//If (not in transparent mode) and (not a zero pixel), add
			if (!(Transparent && CompareMem(&Src, &ZeroPixel, 3) == 0)) {
				(*Buffer)[(x + DestinationX) + (y + DestinationY) * SourceWidth] = Src;
			}
		}
	}
	return EFI_SUCCESS;
}

EFI_STATUS 
LoadBMP (
	IN CHAR16  *FileName,
	OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **Buffer,
	OUT UINTN *Height,
	OUT UINTN *Width,
	OUT UINTN *Size
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
        goto Cleanup;
    }
    Status = ShellOpenFileByName(FullFileName, &FileHandle, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        goto Cleanup;
    }

    ShellSetFilePosition(FileHandle, 0);
	EFI_FILE_INFO *FileInfo = ShellGetFileInfo(FileHandle);

	void *File = (void *) 1; //To make BmpSupportLib happy
	//UINTN Size = FileInfo->FileSize;

	ShellReadFile(FileHandle, &FileInfo->FileSize, File);
	if (EFI_ERROR(Status)) {
        goto Cleanup;
    }

	*Buffer = 0;
	*Height = 0;
	*Width = 0;

	Status = TranslateBmpToGopBlt(File, FileInfo->FileSize, Buffer, Size, Height, Width);
	if (EFI_ERROR(Status)) {
		goto Cleanup;
	}
  Cleanup:
  	ShellCloseFile(&FileHandle);
    if (FullFileName != NULL) {
       FreePool(FullFileName);
    }

	return Status;
}

EFI_STATUS
InitBackground (
) 
{
	EFI_STATUS Status;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelMap;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Tile;

	Status = LoadBMP(L"EFI\\Game\\map.bmp", &PixelMap, &LevelHeight, &LevelWidth, &SpriteSheetSize); //Using SpriteSheetSize as temporary
	if (EFI_ERROR(Status)) {
	  goto Cleanup;
	}

	BackgroundBuffer = AllocatePool(LevelWidth * SpriteLength * LevelHeight * SpriteLength * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	LevelBuffer = AllocatePool(LevelWidth * LevelHeight * sizeof(UINT8));

	Tile = AllocatePool(SpriteLength * SpriteLength * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
	for (UINTN i = 0; i < LevelWidth * LevelHeight; i++) {
		LevelBuffer[i] = PixelMap[i].Red;
		if (PixelMap[i].Red != 0) { 
			if (PixelMap[i].Red != PixelMap[i - 1].Red) {
				if (Tile != NULL) {
					FreePool(Tile);
				}
	    		ExtractBuffer(SpriteSheet, 
	                    	  SpriteSheetWidth, 
	                    	  SpriteSheetHeight, 
	                    	  (PixelMap[i].Red - 1) * SpriteLength, 
	                    	  2 * SpriteLength, 
	                    	  &Tile, 
	                    	  SpriteLength, 
	                    	  SpriteLength
	                    	  );
	    }
	    AddToBuffer(&BackgroundBuffer, 
	                LevelWidth * SpriteLength, 
	                LevelHeight * SpriteLength, 
	                Tile, 
	                (i % LevelWidth) * SpriteLength, 
	                (i / LevelWidth) * SpriteLength, 
	                SpriteLength, 
	                SpriteLength, 
	                FALSE
	                );
	  }
	}
	if (Tile != NULL) {
		FreePool(Tile);
	}
	FreePool(PixelMap);	
  Cleanup:
  	return Status;
}