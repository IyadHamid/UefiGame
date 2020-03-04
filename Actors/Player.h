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
typedef struct Controller {
    Button buttons[MAX_BUTTONS];
    UINT32 shiftState;
    
    VOID (*clear)();
    VOID (*refresh)();
} Controller;

///
/// Camera information
///
typedef struct Camera {
    UINTN x;
    UINTN y;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *screen;
} Camera;

///
/// Player flags
///
typedef struct PlayerFlags {
    BOOLEAN midair : 1;
    BOOLEAN facingRight : 1;
} PlayerFlags;

///
/// Player information
///
typedef struct Player {
    ///
    /// X coord
    ///
    UINTN x;
    ///
    /// Y coord
    ///
    UINTN y;
    ///
    /// X velocity
    ///
    INTN velX;
    ///
    /// Y velocity
    ///
    INTN velY;
    ///
    /// X acceleration
    ///
    INTN accX;
    ///
    /// Y acceleration
    ///
    INTN accY;
    ///
    /// Midair, Facing Right
    ///
    PlayerFlags flags;
    ///
    /// Controller
    ///
    Controller *controller;
    ///
    /// Camera
    ///
    Camera *camera;
    ///
    /// Current sprite
    ///
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *sprite;

    VOID (*collide)();
    VOID (*tick)();
} Player;

VOID InitializePlayer (
    IN Player *this
);