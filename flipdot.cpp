/*
   flipdot.c

   Created: 09.01.2019 20:24:56
    Author: Julian
*/

#include "flipdot.h"
#include <stdint.h>

#include <Arduino.h>

// MATRIX CONFIGURATION
uint8_t PANEL_LINES[1] = {0};

MCP23017Type<ADDRESS_ROW_MCP_Y> ROW_MCP_Y;
MCP23017Type<ADDRESS_ROW_MCP_B> ROW_MCP_B;

uint8_t COLUMN_LINES      [ 5] = { 10 , 4, 11, 12, 8 /* 5 goes to 2 on PCB */ };  // Column Lines COL_A0...COL_A4
uint8_t E_LINES           [ 5] = { 3, 3, 3, 3, 3 };
static const uint8_t D         =  5;
static const uint8_t LED       =  6;
static const uint8_t MCP_RESET =  13;

uint8_t hasHalfPanelOffset(uint8_t colIndex) {
  /*
     Determine if the given column index belongs to a panel which was preceded by a 14-col half panel.
  */

  return 0;
}

uint8_t matrixClean = 0;
uint8_t pixelInverting = 0;
uint8_t activeState = 1;
uint8_t quickUpdate = 1;

/*
   HELPER FUNCTIONS
*/

void setQuickUpdate(uint8_t state) {
  quickUpdate = state;
}

/*
   PROGRAM CODE
*/

void initPins() {
  Serial.println("initPins");
  pinMode(MCP_RESET, OUTPUT);
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(COLUMN_LINES[i], OUTPUT);
    pinMode(E_LINES[i], OUTPUT);
  }
  pinMode(D, OUTPUT);
  pinMode(LED, OUTPUT);
}

bool initMCP() {

  //strobe RESET to reset the chip
  digitalWrite(MCP_RESET, LOW);
  delay(2);
  digitalWrite(MCP_RESET, HIGH);
  delay(25);
  //this initializes the MCP, sets all pins to OUTPUT and puts state LOW
  bool mcp_y_ok = ROW_MCP_Y.init();
  Serial.println("initMCP");
  bool mcp_b_ok = ROW_MCP_B.init();
  bool mcp_ok = (mcp_y_ok && mcp_b_ok) ;

  if (!mcp_ok) {
    Serial.println("Error while initializing MCP23017");
    return false;
  }
  Serial.println("Found MCP23017 ok");
  return true;
}

void selectColumn(uint8_t colIndex) {
  /*
     Select the appropriate panel for the specified column index and set the column address pins accordingly.
  */
#ifdef COL_SWAP
      colIndex = MATRIX_WIDTH - colIndex - 1;
#endif
  // In the case of a matrix with a 14-col panel at the end instead of a 28-col one, we need to remember that our panel index is off by half a panel, so flip the MSB
  uint8_t halfPanelOffset = hasHalfPanelOffset(colIndex);

  // Additionally, the address needs to be reversed because of how the panels are connected
  colIndex = MATRIX_WIDTH - colIndex - 1;

  // Since addresses start from the beginning in every panel, we need to wrap around after reaching the end of a panel
  uint8_t address = colIndex % PANEL_WIDTH;

  // A quirk of the FP2800 chip used to drive the columns is that addresses divisible by 8 are not used, so we need to skip those
  address += (address / 7) + 1;

  digitalWrite(COLUMN_LINES[0], address & 1);
  digitalWrite(COLUMN_LINES[1], address & 2);
  digitalWrite(COLUMN_LINES[2], address & 4);
  digitalWrite(COLUMN_LINES[3], address & 8);
  digitalWrite(COLUMN_LINES[4], halfPanelOffset ? !(address & 16) : address & 16);
}

void selectRow(uint8_t rowIndex, uint8_t yellow) {
#ifdef ROW_SWAP
  rowIndex = MATRIX_HEIGHT - rowIndex - 1;
#endif
  ROW_MCP_Y.writeAll(0);
  ROW_MCP_B.writeAll(0);
  if (yellow)
    ROW_MCP_Y.writePin(rowIndex, HIGH);
  else
    ROW_MCP_B.writePin(rowIndex, HIGH);

  digitalWrite(D, !yellow);
}

void deselect() {
  /*
     Deselect all rows by inhibiting the line decoders
     and deselect the column by setting col address 0
  */

  ROW_MCP_Y.writeAll(LOW);
  ROW_MCP_B.writeAll(LOW);

  digitalWrite(COLUMN_LINES[0], LOW);
  digitalWrite(COLUMN_LINES[1], LOW);
  digitalWrite(COLUMN_LINES[2], LOW);
  digitalWrite(COLUMN_LINES[3], LOW);
  digitalWrite(COLUMN_LINES[4], LOW);
}

void flip(uint8_t panelIndex) {
  /*
     Send an impulse to the specified panel to flip the currently selected dot.
  */

  // Get the enable line for the specified panel
  byte e = E_LINES[PANEL_LINES[panelIndex / PANEL_WIDTH]];

  digitalWrite(e, HIGH);
  delayMicroseconds(FLIP_DURATION );

  digitalWrite(e, LOW);
  delayMicroseconds(FLIP_PAUSE_DURATION );
}

void setBacklight(uint8_t status) {
  /*
     Enable or disable the LED backlight of the matrix.
  */

  digitalWrite(LED, !status);
}

void setPixel(uint8_t x, uint8_t y, uint8_t state) {
  selectColumn(y);
  selectRow(x, state);
  flip(x / PANEL_WIDTH);
  deselect();
}

void setMatrix(uint16_t* newBitmap, uint16_t* oldBitmap) {
  /*
     Write a bitmap to the matrix.
  */

  for (int16_t col = 0; col < MATRIX_WIDTH; col++) {
    uint16_t newColData = newBitmap[col];
    uint16_t oldColData;
    uint8_t colChanged;

    // Determine whether the current column has been changed
    if (quickUpdate && oldBitmap) {
      // We're in delta mode, compare the two bitmaps and refresh only the pixels that have changed
      oldColData = oldBitmap[col];
      colChanged = newColData != oldColData;
    } else {
      // We don't have anything to compare or Quick Update is disabled, so just do a full refresh
      oldColData = 0x0000;
      colChanged = 1;
    }
    if (!colChanged) continue;

    // Which panel are we on?
    uint8_t panel = col / PANEL_WIDTH;

    selectColumn(col);
    for (int16_t row = 0; row < MATRIX_HEIGHT; row++) {
      // Determine whether the current pixel has been changed
      uint8_t pixelChanged = !quickUpdate || !oldBitmap || (oldColData & (1 << row)) != (newColData & (1 << row));
      if (!pixelChanged) continue;
      uint8_t newPixelValue = !!(newColData & (1 << row));
      newPixelValue ^= pixelInverting;
      selectRow(MATRIX_HEIGHT - row - 1, newPixelValue);
      flip(panel);
    }
  }
  deselect();
  if (!oldBitmap) matrixClean = 1;
}

void clearMatrix() {
  for (int16_t col = 0; col < MATRIX_WIDTH; col++) {
    // Which panel are we on?
    uint8_t panel = col / PANEL_WIDTH;

    selectColumn(col);
    for (int16_t row = 0; row < MATRIX_HEIGHT; row++) {
      selectRow(MATRIX_HEIGHT - row - 1, 0);
      flip(panel);
    }
  }
  deselect();
}
