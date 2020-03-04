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

#include "B:/UefiGame/Actors/Player.h"
#include "B:/UefiGame/Globals/GameState.h"
#include "B:/UefiGame/Globals/Graphics.h"

#define TO_PIXEL(A) ((A) / (INTN)(LOCATION_PRECISION))
#define TO_TILE(A) ((A) / (INTN)(LOCATION_PRECISION * SpriteLength))
#define FROM_TILE(A) ((A) * LOCATION_PRECISION * SpriteLength)

EFI_INPUT_KEY last;

VOID Clear (
    IN Controller *this
) {
    UINTN i;
    for (i = 0; i < MAX_BUTTONS; i++) {
        this->buttons[i].state = FALSE;
    }

    this->shiftState = 0;
}

VOID Collide (
    Player *this
) {
    //Change velocity
    //Check collisions
    //Set flags
    return;
}

VOID Refresh (
    IN Controller *this
) {
    UINTN i;
    EFI_KEY_DATA KeyData;
    EFI_STATUS Status = Input->ReadKeyStrokeEx(Input, &KeyData);
    while (Status != EFI_NOT_READY && !EFI_ERROR(Status)) { 
        this->shiftState = KeyData.KeyState.KeyShiftState;

        //Was shift pressed
        if (KeyData.Key.UnicodeChar >= u'A' && KeyData.Key.UnicodeChar <= u'Z') {
            //Make lowercase
            KeyData.Key.UnicodeChar += 0x20; //'a'(0x61) - 'A'(0x41) = 0x20
            this->shiftState = this->shiftState || EFI_LEFT_SHIFT_PRESSED;
        }

        for (i = 0; i < MAX_BUTTONS; i++) {
            //if scancode matches and != 0, or if unicode matches
            if ((this->buttons[i].scanCode == KeyData.Key.ScanCode && KeyData.Key.ScanCode != 0)
                ||this->buttons[i].unicode == KeyData.Key.UnicodeChar
            ) {
                this->buttons[i].state = TRUE;
            }
        }
        Status = Input->ReadKeyStrokeEx(Input, &KeyData);
    }
}

VOID Tick (
    IN Player *this
) {
    this->controller->refresh(this->controller);
    UINTN rate = LOCATION_PRECISION;
    
    if (this->controller->buttons[UP].state && !this->flags.midair) {
        this->velY -= LOCATION_PRECISION * 10;
    }
    else if (this->controller->buttons[DOWN].state) {
        this->velY += rate;
    }
    else if (this->controller->buttons[LEFT].state) {
        this->flags.facingRight = 0;
        this->velX -= rate;
    }
    else if (this->controller->buttons[RIGHT].state) {
        this->flags.facingRight = 1;
        this->velX += rate;
    }
    else if (this->controller->buttons[QUIT].state) {
        IsRunning = FALSE;
    }

    if (!this->controller->buttons[LEFT].state && !this->controller->buttons[RIGHT].state && this->velX != 0) {
        this->velX += rate * -this->flags.facingRight;
    }
    this->controller->clear(this->controller);

    //Gravity
    this->velY += LOCATION_PRECISION;
    //Max speed
    if (this->velX > 32) {
        this->velX = 32;
    }
    else if (this->velX < -32) {
        this->velX = -32;
    }

    //Get new sprite and location if moving
    if (this->velX/LOCATION_PRECISION != 0 || this->velY/LOCATION_PRECISION != 0) {
        this->collide(this);

        //Get sprite
        ExtractBuffer(SpriteSheet,
                      SpriteSheetWidth,
                      SpriteSheetHeight,
                      this->flags.midair ? JUMP_FRAME * SpriteLength //Get jump/midair sprite
                                         : ((this->x / (LOCATION_PRECISION * 8)) % JUMP_FRAME) * SpriteLength,
                      (!this->flags.facingRight) * SpriteLength, //Go down one tile (in the sprite sheet) if facing left
                      &this->sprite,
                      SpriteLength,
                      SpriteLength
                      );
    }

    //Add to level buffer
    AddToBuffer(&DrawBuffer, 
                LevelWidth * SpriteLength, 
                LevelHeight * SpriteLength, 
                this->sprite, 
                TO_PIXEL(this->x), 
                TO_PIXEL(this->y), 
                SpriteLength, 
                SpriteLength, 
                TRUE
                );
}

VOID InitializePlayer (
    IN Player *this
) {
    this->tick = &Tick;
    this->collide = &Collide;

    last.ScanCode = SCAN_NULL;
    last.UnicodeChar = u' ';

    Controller *con;
    con = AllocatePool(sizeof(Controller));

    con->clear = &Clear;
    con->refresh = &Refresh;
    
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
    con->clear(con);

    Camera *cam;
    cam = AllocatePool(sizeof(Camera));
    cam->x = 0;
    cam->y = 0;
    cam->screen;
    GetScreen(&cam->screen);

    this->x = FROM_TILE(1);
    this->y = FROM_TILE(1);
    this->velX = 0;
    this->velY = 0;
    //this->flags - Will be set prior to displaying
    this->controller = con;
    this->camera = cam;
    ExtractBuffer(SpriteSheet,
                  SpriteSheetWidth,
                  SpriteSheetHeight,
                  0, //Frame 1
                  0, //Facing right
                  &this->sprite,
                  SpriteLength,
                  SpriteLength
                  );
}
