#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Pi/PiFirmwareFile.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include "Actors/Player.h"
#include "Globals/GameState.h"
#include "Globals/Graphics.h"
#include "Globals/Sprites.h"

EFI_INPUT_KEY last;

EFI_STATUS
ClearController (
    IN Controller *This
)
{
    if (This == NULL) {
        return RETURN_INVALID_PARAMETER;
    }

    UINTN i;
    for (i = 0; i < MAX_BUTTONS; i++) {
        This->buttons[i].state = FALSE;
    }
    return RETURN_SUCCESS;
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
    ClearController(con);

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
    ExtractBuffer(SpriteSheet,
                  SpriteSheetWidth,
                  SpriteSheetHeight,
                  0, //Sprite 0
                  0, //Facing right
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
    
    if (This->controller->buttons[UP].state) {
        This->velY -= SpriteLength * LOCATION_PRECISION;
    }
    else if (This->controller->buttons[DOWN].state) {
        This->velY += SpriteLength * LOCATION_PRECISION;
    }
    else if (This->controller->buttons[LEFT].state) {
        This->velX -= SpriteLength * LOCATION_PRECISION;
    }
    else if (This->controller->buttons[RIGHT].state) {
        This->velX += SpriteLength * LOCATION_PRECISION;
    }
    else if (This->controller->buttons[QUIT].state) {
        Print(L"Quit");
        IsRunning = FALSE;
    }
    ClearController(This->controller);

    //Get new sprite and location if moving
    if (This->velX != 0 || This->velY != 0) {
        This->x += This->velX;
        This->y += This->velY;

        //Get sprite
        ExtractBuffer(SpriteSheet,
                      SpriteSheetWidth,
                      SpriteSheetHeight,
                      This->velY == 0 ? ((This->x / (LOCATION_PRECISION * 64)) % JUMP_FRAME) * SpriteLength 
                                      : JUMP_FRAME * SpriteLength, //Get jump/midair sprite
                      (UINTN)(This->velX < 0) * SpriteLength,
                      &This->sprite,
                      SpriteLength,
                      SpriteLength
                      );
    }
    //Add to level buffer
    AddToBuffer(&LevelBuffer, 
                LevelWidth, 
                LevelHeight, 
                This->sprite, 
                This->x / (LOCATION_PRECISION * SpriteLength), 
                This->y / (LOCATION_PRECISION * SpriteLength), 
                SpriteLength, 
                SpriteLength, 
                TRUE
                );
    return EFI_SUCCESS;
}