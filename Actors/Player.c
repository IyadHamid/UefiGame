#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Pi/PiFirmwareFile.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include "GameState.h"
#include "Graphics.h"
#include "Player.h"
#include "Sprites.h"

EFI_INPUT_KEY last;

EFI_STATUS
ClearController (
    IN Controller *This
)
{

}

EFI_STATUS
Init (
    IN Player *This
)
{
    last.ScanCode = SCAN_NULL;
    last.UnicodeChar = u' ';

    Controller *con;
    con = AllocatePool(sizeof(Controller));
    con->buttons[UP].scanCode    = SCAN_NULL;
    con->buttons[UP].unicode     = u'w';
    con->buttons[DOWN].scanCode  = SCAN_NULL;
    con->buttons[DOWN].unicode   = u's';
    con->buttons[LEFT].scanCode  = SCAN_NULL; 
    con->buttons[LEFT].unicode   = u'a';
    con->buttons[RIGHT].scanCode = SCAN_NULL; 
    con->buttons[RIGHT].unicode  = u'd';
    con->buttons[QUIT].scanCode  = SCAN_NULL;
    con->buttons[QUIT].unicode   = u'Q';

    Camera *cam;
    cam = AllocatePool(sizeof(Camera));
    cam->x = 0;
    cam->y = 0;
    cam->screen;
    GetScreen(&cam->screen);

    This->x = 0;
    This->y = 0;
    This->velX = 0;
    This->velY = 0;
    This->controller = con;
    This->camera = cam;
    This->frame = 0;
    This->isLeft = 0;
    ExtractBuffer(SpriteSheet,
                      SpriteSheetWidth,
                      SpriteSheetHeight,
                      (This->frame / FRAME_DURATION) * SpriteLength,
                      (UINTN)This->isLeft * SpriteLength,
                      &This->sprite,
                      SpriteLength,
                      SpriteLength
                      );
    return EFI_SUCCESS;
}

EFI_STATUS
Tick (
    IN Player *This,
    IN BOOLEAN Input
)
{
    EFI_INPUT_KEY Key;
    UINTN i;
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != EFI_NOT_READY) { 
        for (i = 0; i < MAX_BUTTONS; i++) {
            if (This->controller->buttons[i].scanCode == Key.ScanCode) {
                if (Key.ScanCode != 0 ||
                    This->controller->buttons[i].unicode == Key.UnicodeChar
                ) {
                    This->controller->buttons[i].state = TRUE;
                    break;
                }
            }
        }    
    }

    gST->ConIn->Reset(gST->ConIn, 0);
    if (This->controller->buttons[UP].state) {
        This->velY--;
    }
    else if (This->controller->buttons[DOWN].state) {
        This->velY++;
    }
    else if (This->controller->buttons[LEFT].state) {
        This->velX--;
    }
    else if (This->controller->buttons[RIGHT].state) {
        This->velX++;
    }
    else if (This->controller->buttons[QUIT].state) {
        Print(L"Quit");
        IsRunning = FALSE;
    }

    for (i = 0; i < MAX_BUTTONS; i++) {
        This->controller->buttons[i].state = FALSE;
    }


    if (This->velX != 0 || This->velY != 0 || This->frame / FRAME_DURATION == JUMP_FRAME) {
        This->x += This->velX;
        This->y += This->velY;

        //Which frame to choose
        if (This->velY != 0) {
            This->frame = JUMP_FRAME * FRAME_DURATION;
        }
        else {
            This->frame = (This->frame + 1) % (JUMP_FRAME * FRAME_DURATION);
        }

        //Which direction is This traveling in
        if (This->velX > 0) {
            This->isLeft = FALSE;
        }
        else if (This->velX < 0) {
            This->isLeft = TRUE;
        }

        //Get and add to level
        ExtractBuffer(SpriteSheet,
                      SpriteSheetWidth,
                      SpriteSheetHeight,
                      (This->frame / FRAME_DURATION) * SpriteLength,
                      (UINTN)This->isLeft * SpriteLength,
                      &This->sprite,
                      SpriteLength,
                      SpriteLength
                      );
    }
    AddToBuffer(&LevelBuffer, LevelWidth, LevelHeight, This->sprite, This->x, This->y, SpriteLength, SpriteLength, TRUE);
    return EFI_SUCCESS;
}