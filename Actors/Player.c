#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Pi/PiFirmwareFile.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleTextInEx.h>

#include <Library/DebugLib.h>

#include "Actors/Player.h"
#include "Globals/GameState.h"
#include "Globals/Graphics.h"

#define TO_PIXEL(A) ((A) / (INTN)(LOCATION_PRECISION))
#define TO_TILE(A) ((A) / (INTN)(LOCATION_PRECISION * SpriteLength))
#define FROM_TILE(A) ((A) * LOCATION_PRECISION * SpriteLength)

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

    This->shiftState = 0;
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
    con->buttons[QUIT].scanCode  = SCAN_ESC;
    con->buttons[QUIT].unicode   = 0;
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
    //This->flags - Will be set prior to displaying
    This->controller = con;
    This->camera = cam;
    ExtractBuffer(SpriteSheet,
                  SpriteSheetWidth,
                  SpriteSheetHeight,
                  0, //Frame 1
                  0, //Facing right
                  &This->sprite,
                  SpriteLength,
                  SpriteLength
                  );
    DEBUG((EFI_D_INIT, "Initialized Player"));
    return EFI_SUCCESS;
}

BOOLEAN
 CheckCollision (
    Player *This
) 
{
    UINTN offset = LOCATION_PRECISION * (SpriteLength - 1); //other end of sprite
    BOOLEAN colliding = FALSE;
    INTN i = 0;

    UINTN prevX = This->x - This->velX;
    UINTN prevY = This->y - This->velY;
    
    //Are new X-coords colliding? (with old y)
    if (This->velX / LOCATION_PRECISION != 0) {
        if (
            LevelBuffer[TO_TILE(This->x         ) + TO_TILE(prevY         ) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(This->x + offset) + TO_TILE(prevY         ) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(This->x         ) + TO_TILE(prevY + offset) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(This->x + offset) + TO_TILE(prevY + offset) * LevelWidth] != 0 
        ) 
        {
            const BOOLEAN positive = This->velX >= 0;
            
            i = TO_TILE(This->velX);
            positive ? i++ : i--;
            while (1) {
                if (LevelBuffer[(TO_TILE(prevX) + i) + TO_TILE(prevY) * LevelWidth] == 0) {
                    This->x = FROM_TILE(TO_TILE(prevX) + i);
                    break;
                }
                //increment i towards 0
                !positive ? i++ : i--;
            }

            This->velX = 0;
            colliding = TRUE;
        }
    }

    //Are new Y-coords colliding? (with old x)
    if (This->velY / LOCATION_PRECISION != 0) {
        if ( 
            LevelBuffer[TO_TILE(prevX         ) + TO_TILE(This->y         ) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(prevX + offset) + TO_TILE(This->y         ) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(prevX         ) + TO_TILE(This->y + offset) * LevelWidth] != 0 ||
            LevelBuffer[TO_TILE(prevX + offset) + TO_TILE(This->y + offset) * LevelWidth] != 0 
        ) 
        {
            const BOOLEAN positive = This->velY >= 0;
            i = TO_TILE(This->velY);
            positive ? i++ : i--;
            while (1) {
                if (LevelBuffer[TO_TILE(prevX) + (TO_TILE(prevY) + i) * LevelWidth] == 0 || i == 0) {
                    This->y = FROM_TILE(TO_TILE(prevY) + i);
                    break;
                }
                //increment i towards 0
                !positive ? i++ : i--;
            }
            if (positive && This->flags.midair) { //Only colliding on bottom
                This->flags.midair = 0;
            }
            This->velY = 0;
            colliding = TRUE;
        }
        else {
            This->flags.midair = 1;
        }
    }

    if (colliding) {
        This->flags.colliding = 1;
    }
    else {
        This->flags.colliding = 0;
    }
    return colliding;
}

EFI_STATUS
Tick (
    IN Player *This
)
{
    EFI_STATUS Status;
    EFI_KEY_DATA KeyData;
    UINTN i;

    /*
    ##### ##### #####    ##### #   # ####  #   # #####
    #     #       #        #   ##  # #   # #   #   #  
    #  ## ####    #        #   # # # ####  #   #   #  
    #   # #       #        #   #  ## #     #   #   #  
    ##### #####   #      ##### #   # #      ###    #  
    */
    Status = Input->ReadKeyStrokeEx(Input, &KeyData);
    while (Status != EFI_NOT_READY && !EFI_ERROR(Status)) { 
        This->controller->shiftState = KeyData.KeyState.KeyShiftState;

        //Was shift pressed
        if (KeyData.Key.UnicodeChar >= u'A' && KeyData.Key.UnicodeChar <= u'Z') {
            //Make lowercase
            KeyData.Key.UnicodeChar += 0x20; //'a'(0x61) - 'A'(0x41) = 0x20
            This->controller->shiftState = This->controller->shiftState || EFI_LEFT_SHIFT_PRESSED;
        }

        for (i = 0; i < MAX_BUTTONS; i++) {
            //if scancode matches and != 0, or if unicode matches
            if ((This->controller->buttons[i].scanCode == KeyData.Key.ScanCode && KeyData.Key.ScanCode != 0)
                ||This->controller->buttons[i].unicode == KeyData.Key.UnicodeChar
            ) {
                This->controller->buttons[i].state = TRUE;
            }
        }
        Status = Input->ReadKeyStrokeEx(Input, &KeyData);
    }

    /*
    #   #  ###  #   # ####  #     #####    ##### #   # ####  #   # #####
    #   # #   # ##  # #   # #     #          #   ##  # #   # #   #   #  
    ##### ##### # # # #   # #     ####       #   # # # ####  #   #   #  
    #   # #   # #  ## #   # #     #          #   #  ## #     #   #   #  
    #   # #   # #   # ####  ##### #####    ##### #   # #      ###    #  
    */
    UINTN rate = LOCATION_PRECISION / 2;
    
    if (This->controller->buttons[UP].state) {
        This->velY -= LOCATION_PRECISION;
    }
    else if (This->controller->buttons[DOWN].state) {
        This->velY += rate;
    }
    else if (This->controller->buttons[LEFT].state) {
        This->velX -= rate;
    }
    else if (This->controller->buttons[RIGHT].state) {
        This->velX += rate;
    }
    else if (This->controller->buttons[QUIT].state) {
        IsRunning = FALSE;
    }
    ClearController(This->controller);

    //Gravity
    //if (This->flags.midair) {
        This->velY += LOCATION_PRECISION / 2;
    //}

    //Get new sprite and location if moving
    if (This->velX != 0 || This->velY != 0) {
        This->x += This->velX;
        This->y += This->velY;
        CheckCollision(This);

        //Get sprite
        ExtractBuffer(SpriteSheet,
                      SpriteSheetWidth,
                      SpriteSheetHeight,
                      This->flags.midair ? JUMP_FRAME * SpriteLength //Get jump/midair sprite
                                         : ((This->x / (LOCATION_PRECISION * 8)) % JUMP_FRAME) * SpriteLength,
                      (UINTN)(This->velX < 0) * SpriteLength,
                      &This->sprite,
                      SpriteLength,
                      SpriteLength
                      );
    }

    //Add to level buffer
    AddToBuffer(&DrawBuffer, 
                LevelWidth * SpriteLength, 
                LevelHeight * SpriteLength, 
                This->sprite, 
                TO_PIXEL(This->x), 
                TO_PIXEL(This->y), 
                SpriteLength, 
                SpriteLength, 
                TRUE
                );
    return EFI_SUCCESS;
}