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
//Coord->Pixel
#define TO_PIXEL(A) ((A) / (INTN)(LOCATION_PRECISION))
//Coord->Tile
#define TO_TILE(A) ((A) / (INTN)(LOCATION_PRECISION * SpriteLength))
//Tile->Coord
#define FROM_TILE(A) ((A) * LOCATION_PRECISION * SpriteLength)
//Pixel->Coord
#define FROM_PIXEL(A) ((A) * LOCATION_PRECISION)
//Pixel->Tile
#define PIXEL_TO_TILE(A) ((A) / SpriteLength)
//Tile->Pixel
#define TILE_TO_PIXEL(A) ((A) * SpriteLength)

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
    
    UINTN newX = this->x + this->velX;
    UINTN newY = this->y + this->velY;
    /* Sonic like detection (arrows are each ray)
    http://info.sonicretro.org/SPG:Solid_Tiles#Sensors

    .^....^.
    .^....^.
    .^....^.
    .^....^.
    <<<<>>>>
    .v....v.
    .v....v.
    .v....v.
    */
    
    const UINTN halfLength = SpriteLength / 2;
    const UINTN halfScaled = halfLength / SCALE;
    const INTN midX = TO_PIXEL(newX) + halfLength;
    const INTN midY = TO_PIXEL(newY) + halfLength;
    const INTN leftX = TO_PIXEL(newX) + (1 * SpriteLength / 8);
    const INTN rightX = TO_PIXEL(newX) + (7 * SpriteLength / 8);

    UINTN left, right, upLeft, upRight, downLeft, downRight;
    left = right = upLeft = upRight = downLeft = downRight = halfScaled + 1;

    for (UINTN i = 0; i <= halfLength / SCALE + 1; i++) {
        if ( //Midpoint to left
            this->velX < 0 &&
            LevelBuffer[PIXEL_TO_TILE((midX - i * SCALE)) + 
                        PIXEL_TO_TILE(midY) * LevelWidth
                        ] == 0
        ) {
            left = i;
        }
        else if ( //Midpoint to right
            this->velX > 0 &&
            LevelBuffer[PIXEL_TO_TILE(midX + (i - 1) * SCALE) + 
                        PIXEL_TO_TILE(midY) * LevelWidth
                        ] == 0
        ) {
            right = i;
        }

        if (this->velY < 0) {
            if ( //Left midpoint to up
                LevelBuffer[
                            PIXEL_TO_TILE(leftX) +
                            PIXEL_TO_TILE(midY - i * SCALE) * LevelWidth
                            ] == 0
            ) {
                upLeft = i;
            }
            if ( //Right midpoint to up
                LevelBuffer[
                            PIXEL_TO_TILE(rightX) +
                            PIXEL_TO_TILE(midY - i * SCALE) * LevelWidth
                            ] == 0
            ) {
                upRight = i;
            }
        }
        else if (this->velY >= 0) { //Need to check for ledges (<=)
            if ( //Left midpoint to down
                LevelBuffer[
                            PIXEL_TO_TILE(leftX) +
                            PIXEL_TO_TILE(midY + (i - 1) * SCALE) * LevelWidth
                            ] == 0
            ) {
                downLeft = i;
            }
            if ( //Right midpoint to down
                LevelBuffer[
                            PIXEL_TO_TILE(rightX) +
                            PIXEL_TO_TILE(midY + (i - 1) * SCALE) * LevelWidth
                            ] == 0
            ) {
                downRight = i;
            }
        }

    }
    //DEBUG/////////////
    gST->ConOut->SetCursorPosition(gST->ConOut, 65, 0);
    Print(L"%d, %d   \t", upLeft, upRight);
    gST->ConOut->SetCursorPosition(gST->ConOut, 65, 1);
    Print(L"%d, %d   \t", left, right);
    gST->ConOut->SetCursorPosition(gST->ConOut, 65, 2);
    Print(L"%d, %d   \t", downLeft, downRight);
    gST->ConOut->SetCursorPosition(gST->ConOut, 65, 3);
    Print(L"%d, %d   \t", this->velX != 0, this->velY != 0);

    if (left <= halfScaled) {
        newX += FROM_PIXEL(halfLength - left * SCALE);
        this->velX = 0;
    }
    else if (right <= halfScaled) {
        newX -= FROM_PIXEL(halfLength - right * SCALE);
        this->velX = 0;    
    }

    if (downLeft <= halfScaled || downRight <= halfScaled) {
        if (downLeft > downRight) {
            newY -= FROM_PIXEL(halfLength - downLeft * SCALE);
        }
        else {
            newY -= FROM_PIXEL(halfLength - downRight * SCALE);
        }
        this->velY = 0;
        this->flags.midair = 0;
    }
    else {
        this->flags.midair = 1;
    }

    if (upLeft <= halfScaled || upRight <= halfScaled) {
        if (upLeft > upRight) {
            newY += FROM_PIXEL(halfLength - upLeft * SCALE);
        }
        else {
            newY += FROM_PIXEL(halfLength - upRight * SCALE);
        }
        this->velY = 0;
    }

    this->x=newX; 
    this->y=newY;
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

    //INTN acc = 0;
    this->accX = this->accY = 0;
    if (this->controller->buttons[UP].state && !this->flags.midair) {
        this->accY -= FROM_PIXEL(10);
    }
    else if (this->controller->buttons[DOWN].state) {
        this->accY += FROM_PIXEL(2);
    }
    else if (this->controller->buttons[LEFT].state) {
        this->flags.facingRight = 0;
        this->accX -= FROM_PIXEL(2);
        //acc -= FROM_PIXEL(2);
    }
    else if (this->controller->buttons[RIGHT].state) {
        this->flags.facingRight = 1;
        this->accX += FROM_PIXEL(2);
        //acc += FROM_PIXEL(2);
    }
    else if (this->controller->buttons[QUIT].state) {
        IsRunning = FALSE;
    }

    this->controller->clear(this->controller);

    this->velX += this->accX ;//+ acc;
    this->velY += this->accY;


    //Gravity
    //this->accY += FROM_PIXEL(1);
    //Max speed
    //if (this->velX > 32) {
    //    this->velX = 32;
    //}
    //else if (this->velX < -32) {
    //    this->velX = -32;
    //}

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
