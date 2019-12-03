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
/// Player information
///
typedef struct {
    UINTN x;
    UINTN y;
    INTN velX;
    INTN velY;
    Controller *controller;
    Camera *camera;
    ///
    /// Frame timer, frame/sprite * FRAME_DURATION
    ///
    INT8 frame;
    BOOLEAN isLeft;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *sprite;
} Player;

EFI_STATUS
Init (
    IN Player *This
);

EFI_STATUS
Tick(
    IN Player *This,
    IN BOOLEAN Input
);