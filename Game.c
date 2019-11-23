/** @file
  Brief Description of UEFI MyHelloWorld
  Detailed Description of UEFI MyHelloWorld
  Copyright for UEFI MyHelloWorld
  License for UEFI MyHelloWorld
**/

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/BmpSupportLib.h>
#include <IndustryStandard/Bmp.h>

#include "GameState.h"
#include "Graphics.h"
#include "Player.h"
#include "Sprites.h"

BOOLEAN IsRunning;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *LevelBuffer;
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
  IsRunning = TRUE;

  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);
  Status = LoadBMP(L"EFI\\Game\\sprites.bmp");
  if (EFI_ERROR(Status)) {
    Print(L"Failed at Load\n");
    goto Cleanup;
  }

  ScaleBuffer(&SpriteSheet, &SpriteSheetWidth, &SpriteSheetHeight, 4);
  SpriteLength *= 4;
  
  EFI_EVENT TickEvent;
  EFI_EVENT TickList[1];
  UINTN eventId;

  gBS->CreateEvent(EVT_TIMER, TPL_NOTIFY, NULL, NULL, &TickEvent);
	gBS->SetTimer(TickEvent, TimerPeriodic, 50000 * 10);

  TickList[0] = TickEvent;
  //TickList[1] = gST->ConIn->WaitForKey;





  EFI_GRAPHICS_OUTPUT_BLT_PIXEL a = {25, 100, 48, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *b;
  b = AllocatePool(500 * 500 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  for (UINTN i = 0; i < 500 * 500; i++) {
    if (i%2 == 0)
    b[i] = a;
  }
  
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *temp;
  LevelWidth = 2048;
  LevelHeight = 1024;
  LevelBuffer = AllocatePool(LevelWidth * LevelHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  Player *player;
  player = AllocatePool(sizeof(player));

  Init(player);
  while (IsRunning) {
    gBS->WaitForEvent(1, TickList, &eventId);
    Tick(player, (BOOLEAN)eventId);
    ExtractBuffer(LevelBuffer, LevelWidth, LevelHeight, 0, 0, &temp, 600, 600);
    player->camera->screen->Blt(player->camera->screen, temp, EfiBltBufferToVideo, 0, 0, 0, 0, 600, 600, 0);
  }
Cleanup:
  gST->ConOut->EnableCursor(gST->ConOut, TRUE);
  return Status;
}