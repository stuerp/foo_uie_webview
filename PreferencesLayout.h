
/** $VER: PreferencesLayout.h (2024.05.25) **/

#pragma once

#define W_A00    332 // Dialog width as set by foobar2000, in dialog units
#define H_A00    288 // Dialog height as set by foobar2000, in dialog units

#define H_LBL        8 // Label

#define W_BTN       50 // Button
#define H_BTN       14 // Button

#define H_EBX       12 // Edit box
#define H_CBX       14 // Combo box

#define W_CHB       10 // Check box
#define H_CHB       10 // Check box

#define DX           7
#define DY           7

#define IX           4 // Spacing between two related controls
#define IY           3

#pragma region File Path

// Label
#define X_D11    0
#define Y_D11    0
#define W_D11    62
#define H_D11    H_LBL

// EditBox
#define X_D12    X_D11
#define Y_D12    Y_D11 + H_D11 + IY
#define W_D12    240
#define H_D12    H_EBX

#pragma endregion
