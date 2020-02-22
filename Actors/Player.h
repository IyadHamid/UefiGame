#pragma once
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Pi/PiFirmwareFile.h>

enum {UP, DOWN, LEFT, RIGHT, QUIT, MAX_BUTTONS};

typedef struct {
    BOOLEAN state;
    CHAR16 unicode;
    UINT16 scanCode;
} Button;

///
/// Controls with scancodes
///
typedef struct {
    Button buttons[MAX_BUTTONS];
    UINT32 shiftState;
} Controller;

///
/// Camera information
///
typedef struct {
    UINTN x;
    UINTN y;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *screen;
} Camera;

///
/// Player flags
///
typedef struct {
    BOOLEAN colliding : 1;
    BOOLEAN midair : 1;
    BOOLEAN facingRight : 1;
} playerFlags;

///
/// Player information
///
typedef struct {
    ///
    /// X coord
    ///
    UINTN x;
    ///
    /// Y coord
    ///
    UINTN y;
    INTN velX;
    INTN velY;
    ///
    /// Colliding, Midair
    ///
    playerFlags flags;
    Controller *controller;
    Camera *camera;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *sprite;
} Player;

VOID
ClearController (
    IN Controller *This
);

VOID RefreshController(
    IN Player *This
);

VOID
Init (
    IN Player *This
);

VOID
Tick(
    IN Player *This
);