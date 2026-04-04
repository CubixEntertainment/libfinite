#include <finite/input.h>

typedef struct {
    FiniteGamepad *gamepads[MAX_GAMEPADS];
    int _gamepads;
} FiniteGamepadInfo;

int initializeGamepadSocket();
void *handle_input();