/*
 * flipdot.h
 *
 * Created: 09.01.2019 20:26:03
 *  Author: Julian
 */ 


#ifndef FLIPDOT_H_
#define FLIPDOT_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include <pins_arduino.h>
#include <Wire.h>

#include "LAWO_MCP23017.h"


/*
 * MATRIX CONFIGURATION
 */

#define MATRIX_WIDTH 28
#define MATRIX_HEIGHT 16
#define MIN_ROW 1
#define MAX_ROW 13
#define VIEWPORT_HEIGHT (MAX_ROW - MIN_ROW + 1)
#define PANEL_WIDTH 28
#define COL_SWAP

/*
 * PIN DECLARATIONS
 */



#define ADDRESS_ROW_MCP_Y     0x20
#define ADDRESS_ROW_MCP_B     0x21

/*
 * GLOBAL CONSTANTS
 */

#define BAUDRATE 57600
#define SERIAL_TIMEOUT 5000
#define FLIP_DURATION 1000 // in microseconds
#define FLIP_PAUSE_DURATION 250 // in microseconds

enum SERIAL_STATUSES {
  SUCCESS = 0xFF,
  TIMEOUT = 0xE0,
  ERROR = 0xEE,
};

void setQuickUpdate(uint8_t state);
void setPin(uint8_t* port, uint8_t pin, uint8_t state);
void selectColumn(uint8_t colIndex);
void selectRow(uint8_t rowIndex, uint8_t yellow);
void deselect();
void flip(uint8_t panelIndex);
void setBacklight(uint8_t status);
void setPixel(uint8_t x, uint8_t y, uint8_t state);
void setMatrix(uint16_t* newBitmap, uint16_t* oldBitmap);
void clearMatrix();
void initPins();
bool initMCP();

#endif /* FLIPDOT_H_ */
