
/** $VER: PreferencesLayout.h (2024.08.04) **/

#pragma once

#define W_A00   332 // Dialog width as set by foobar2000, in dialog units
#define H_A00   288 // Dialog height as set by foobar2000, in dialog units

#define H_LBL     8 // Label

#define W_BTN    50 // Button
#define H_BTN    14 // Button

#define H_EBX    14 // Edit box
#define H_CBX    14 // Combo box

#define W_CHB    10 // Check box
#define H_CHB    10 // Check box

#define DX        7
#define DY        7

#define IX        4 // Spacing between two related controls
#define IY        3

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

#pragma region Window Size

// Label
#define X_D20   0
#define Y_D20   Y_D17 + H_D17 + IY
#define W_D20   76
#define H_D20   H_LBL

// EditBox
#define X_D21   X_D20 + W_D20 + IX
#define Y_D21   Y_D20
#define W_D21   30
#define H_D21   H_EBX

// ComboBox
#define X_D22   X_D21 + W_D21 + IX
#define Y_D22   Y_D20
#define W_D22   44
#define H_D22   H_CBX

#pragma endregion

#pragma region Reaction Alignment

// Label
#define X_D23   0
#define Y_D23   Y_D21 + H_D21 + IY
#define W_D23   76
#define H_D23   H_LBL

// EditBox
#define X_D24   X_D23 + W_D23 + IX
#define Y_D24   Y_D23
#define W_D24   30
#define H_D24   H_EBX

// Label
#define X_D25   X_D24 + W_D24 + IX
#define Y_D25   Y_D24
#define W_D25   100
#define H_D25   H_LBL

#pragma endregion

// Checkbox: Clear browsing data on startup
#define X_D26   0
#define Y_D26   Y_D24 + H_D24 + IY
#define W_D26   160
#define H_D26   H_LBL

// Checkbox: In Private mode
#define X_D27   0
#define Y_D27   Y_D26 + H_D26 + IY
#define W_D27   160
#define H_D27   H_LBL

// Checkbox: Fluent scrollbar style
#define X_D28   0
#define Y_D28   Y_D27 + H_D27 + IY
#define W_D28   160
#define H_D28   H_LBL

// Warning
#define X_D99   0
#define Y_D99   H_A00 - H_LBL
#define W_D99   W_A00
#define H_D99   H_LBL
