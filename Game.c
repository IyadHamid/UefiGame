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

#include <Protocol/SimpleTextInEx.h>

#include "Actors/Player.h"
#include "Globals/GameState.h"
#include "Globals/Graphics.h"

BOOLEAN IsRunning;
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *Input;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundBuffer;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DrawBuffer;
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

  //Get input
  Input = (void *)1;
  Status = gBS->HandleProtocol(gST->ConsoleInHandle, &gEfiSimpleTextInputExProtocolGuid, (VOID **)&Input);
  if (EFI_ERROR(Status)) {
  	goto Cleanup;
  }

  //Get sprites
  Status = LoadBMP(L"EFI\\Game\\sprites.bmp", &SpriteSheet, &SpriteSheetHeight, &SpriteSheetWidth, &SpriteSheetSize);
  if (EFI_ERROR(Status)) {
    goto Cleanup;
  }
	SpriteLength = BMP_TILE_LENGTH;

  //Get map
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelMap;
  Status = LoadBMP(L"EFI\\Game\\map.bmp", &PixelMap, &LevelHeight, &LevelWidth, &SpriteSheetSize); //Using SpriteSheetSize as temporary
  if (EFI_ERROR(Status)) {
    goto Cleanup;
  }

  //Scale sprites up
  ScaleBuffer(&SpriteSheet, &SpriteSheetWidth, &SpriteSheetHeight, 4);
  SpriteLength *= 4;

  //Initialize Background
  BackgroundBuffer = AllocatePool(LevelWidth * SpriteLength * LevelHeight * SpriteLength * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *temp;
  temp = AllocatePool(SpriteLength * SpriteLength * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  for (UINTN i = 0; i < LevelWidth * LevelHeight; i++) {
    if (PixelMap[i].Red != 0) { 
      if (PixelMap[i].Red != PixelMap[i - 1].Red) {
        ExtractBuffer(SpriteSheet, 
                      SpriteSheetWidth, 
                      SpriteSheetHeight, 
                      (PixelMap[i].Red - 1) * SpriteLength, 
                      2 * SpriteLength, 
                      &temp, 
                      SpriteLength, 
                      SpriteLength
                      );
      }
      AddToBuffer(&BackgroundBuffer, 
                  LevelWidth * SpriteLength, 
                  LevelHeight * SpriteLength, 
                  temp, 
                  (i % LevelWidth) * SpriteLength, 
                  (i / LevelWidth) * SpriteLength, 
                  SpriteLength, 
                  SpriteLength, 
                  FALSE
                  );
    }
  }
  FreePool(temp);
  FreePool(PixelMap);

  //Initialize Player
  Player *player;
  player = AllocatePool(sizeof(player));
  Init(player);

  //Setup tick loop
  EFI_EVENT TickEvent;
  UINTN eventId;

  gBS->CreateEvent(EVT_TIMER, TPL_NOTIFY, NULL, NULL, &TickEvent);
	gBS->SetTimer(TickEvent, TimerPeriodic, 1000 * 50);
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
  gST->ConOut->EnableCursor(gST->ConOut, TRUE);
  gST->ConOut->ClearScreen(gST->ConOut);
  return Status;
}