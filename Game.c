#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/BmpSupportLib.h>
#include <IndustryStandard/Bmp.h>

#include <Protocol/SimpleTextInEx.h>

#include "Actors/Player.h"
#include "Globals/GameState.h"
#include "Globals/Graphics.h"

#define SCALE 4
#define T Print(L"%d", zxc); zxc++;

BOOLEAN IsRunning;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL ZeroPixel;
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *Input;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundBuffer;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DrawBuffer;
UINT8 *LevelBuffer; 
UINTN LevelWidth;
UINTN LevelHeight;

/**
  Entry point for the game.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{ 
  EFI_STATUS Status = EFI_SUCCESS;

  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);
  Print(L"Hello World!\n");
  ZeroMem(&ZeroPixel, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  //Get input
  Input = (void *)1;
  Status = gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (VOID **)&Input);
  if (EFI_ERROR(Status)) {
    ErrorPrint(L"Unable to get gEfiSimpleTextInputExProtocol\n");
  	goto Cleanup;
  }

  //Get sprites
  Status = LoadBMP(L"EFI\\Game\\sprites.bmp", &SpriteSheet, &SpriteSheetHeight, &SpriteSheetWidth, &SpriteSheetSize);
  if (EFI_ERROR(Status)) {
    ErrorPrint(L"Unable to find sprites.bmp\n");
    goto Cleanup;
  }

	SpriteLength = BMP_TILE_LENGTH;

  //Scale sprites up
  ScaleBuffer(&SpriteSheet, &SpriteSheetWidth, &SpriteSheetHeight, SCALE);
  SpriteLength *= SCALE;
  InitBackground();
  //Initialize Player
  Player *player;
  player = AllocatePool(sizeof(player));
  Init(player);

  //Setup tick loop
  EFI_EVENT TickEvent;
  UINTN eventId;
  gBS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TickEvent);
	gBS->SetTimer(TickEvent, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(10));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *temp;

  IsRunning = TRUE;
  while (IsRunning) {
  
    //Copy Background to DrawBuffer
    DrawBuffer = AllocateCopyPool(LevelWidth * SpriteLength * LevelHeight * SpriteLength, BackgroundBuffer);
    //Wait for tick
    gBS->WaitForEvent(1, &TickEvent, &eventId);
    Tick(player);
    ExtractBuffer(DrawBuffer, LevelWidth * SpriteLength, LevelHeight * SpriteLength, 0, 0, &temp, 512, 512);
    
    player->camera->screen->Blt(player->camera->screen, temp, EfiBltBufferToVideo, 0, 0, 0, 0, 512, 512, 0);
    //Free screen and temporary
    FreePool(temp);
    FreePool(DrawBuffer);
  }
Cleanup:
  gBS->CloseEvent(TickEvent);
  gST->ConOut->EnableCursor(gST->ConOut, TRUE);
  if (!EFI_ERROR(Status)) {
    gST->ConOut->ClearScreen(gST->ConOut);
  }
  return Status;
}