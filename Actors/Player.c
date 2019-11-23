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

BOOLEAN
IsKeyEqual (
    IN EFI_INPUT_KEY a,
    IN EFI_INPUT_KEY b
) 
{
    return (a.ScanCode == b.ScanCode) && (a.UnicodeChar == b.UnicodeChar);
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
    con->up.ScanCode = SCAN_NULL;
    con->up.UnicodeChar = u'w';
    con->down.ScanCode = SCAN_NULL;
    con->down.UnicodeChar = u's';
    con->left.ScanCode = SCAN_NULL; 
    con->left.UnicodeChar = u'a';
    con->right.ScanCode = SCAN_NULL; 
    con->right.UnicodeChar = u'd';
    con->quit.ScanCode = SCAN_ESC; 
    con->quit.UnicodeChar = u' ';

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
    //if (Input) {
    if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != EFI_NOT_READY) { 

        gST->ConIn->Reset(gST->ConIn, 0);
        if (IsKeyEqual(Key, This->controller->up)) {
            This->velY--;
        }
        else if (IsKeyEqual(Key, This->controller->down)) {
            This->velY++;
        }
        else if (IsKeyEqual(Key, This->controller->left)) {
            This->velX--;
        }
        else if (IsKeyEqual(Key, This->controller->right)) {
            This->velX++;
        }
        else if (IsKeyEqual(Key, This->controller->quit)) {
            Print(L"Quit");
            IsRunning = FALSE;
        }
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