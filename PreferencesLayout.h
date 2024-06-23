
/** $VER: PreferencesLayout.h (2024.06.23) **/

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

#pragma region Name

// Label
#define X_D11   0
#define Y_D11   0
#define W_D11   76
#define H_D11   H_LBL

// EditBox
#define X_D12   X_D11 + W_D11 + IX
#define Y_D12   Y_D11
#define W_D12   160
#define H_D12   H_EBX

#pragma endregion

#pragma region User Data Folder Path

// Label
#define X_D13   0
#define Y_D13   Y_D12 + H_D12 + IY
#define W_D13   76
#define H_D13   H_LBL

// EditBox
#define X_D14   X_D13
#define Y_D14   Y_D13 + H_D13 + IY
#define W_D14   240
#define H_D14   H_EBX

// Button: Select
#define X_D15    X_D14 + W_D14 + IX
#define Y_D15    Y_D14
#define W_D15    16
#define H_D15    H_BTN

#pragma endregion

#pragma region Template File Path

// Label
#define X_D16   0
#define Y_D16   Y_D14 + H_D14 + IY
#define W_D16   76
#define H_D16   H_LBL

// EditBox
#define X_D17   X_D16
#define Y_D17   Y_D16 + H_D16 + IY
#define W_D17   240
#define H_D17   H_EBX

// Button: Select
#define X_D18    X_D17 + W_D17 + IX
#define Y_D18    Y_D17
#define W_D18    16
#define H_D18    H_BTN

// Button: Edit
#define X_D19    X_D18 + W_D18 + IX
#define Y_D19    Y_D18
#define W_D19    W_BTN
#define H_D19    H_BTN

#pragma endregion
