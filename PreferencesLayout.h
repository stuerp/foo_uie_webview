
/** $VER: PreferencesLayout.h (2024.06.02) **/

#pragma once

#define W_A00    332 // Dialog width as set by foobar2000, in dialog units
#define H_A00    288 // Dialog height as set by foobar2000, in dialog units

#define H_LBL        8 // Label

#define W_BTN       50 // Button
#define H_BTN       14 // Button

#define H_EBX       14 // Edit box
#define H_CBX       14 // Combo box

#define W_CHB       10 // Check box
#define H_CHB       10 // Check box

#define DX           7
#define DY           7

#define IX           4 // Spacing between two related controls
#define IY           3

#pragma region File Path

// Label
#define X_D11   0
#define Y_D11   0
#define W_D11   62
#define H_D11   H_LBL

// EditBox
#define X_D12   X_D11
#define Y_D12   Y_D11 + H_D11 + IY
#define W_D12   240
#define H_D12   H_EBX

// Button: Select
#define X_D13    X_D12 + W_D12 + IX
#define Y_D13    Y_D12
#define W_D13    16
#define H_D13    H_BTN

// Button: Edit
#define X_D14    X_D13 + W_D13 + IX
#define Y_D14    Y_D13
#define W_D14    W_BTN
#define H_D14    H_BTN

#pragma endregion

#pragma region Groupbox Callbacks

#define X_D20   X_D11
#define Y_D20   Y_D12 + H_D12 + IY

#pragma region Playback Starting

// Label
#define X_D21   X_D20 + 5
#define Y_D21   Y_D20 + 11
#define W_D21   300
#define H_D21   H_LBL

// EditBox
#define X_D22   X_D21
#define Y_D22   Y_D21 + H_D21 + IY
#define W_D22   60
#define H_D22   H_EBX

#pragma endregion

#pragma region Playback New Track

// Label
#define X_D23   X_D22
#define Y_D23   Y_D22 + H_D22 + IY
#define W_D23   300
#define H_D23   H_LBL

// EditBox
#define X_D24   X_D23
#define Y_D24   Y_D23 + H_D23 + IY
#define W_D24   60
#define H_D24   H_EBX

#pragma endregion

#define W_D20   100
#define H_D20   11 + H_D21 + IY + H_D22 + IY + H_D23 + IY + H_D24 + 7

#pragma endregion
