/*
 * game.c
 *
 * Created: 30.05.2019 15:36:48
 *  Author: Julian
 */ 

#include "main.h"
#include "game.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

uint16_t playfield[MATRIX_WIDTH] = {0};
uint16_t oldPlayfield[MATRIX_WIDTH] = {0};
uint16_t adcResult = 0;
volatile uint64_t lastSysTicks = 0;
	
t_direction getDPad() {
  if (sysTicks - lastSysTicks > 50) {
    lastSysTicks = sysTicks;
    adcResult = ADCL;
	  adcResult |= ((uint16_t)ADCH << 8);
  }

	if(adcResult > 120 && adcResult < 230) return UP;
	if(adcResult > 300 && adcResult < 350) return DOWN;
	if(adcResult > 880 && adcResult < 980) return LEFT;
	if(adcResult > 450 && adcResult < 550) return RIGHT;
	return INVALID;
}

uint8_t getRandomNumber(uint8_t min, uint8_t max) {
	return ((uint32_t)rand() * (max - min) / RAND_MAX) + min;
}

void clearPlayfield() {
	memset(&playfield, 0, MATRIX_WIDTH*2);
}

void overlayPlayfield(uint16_t* buf) {
	for(uint16_t i = 0; i < MATRIX_WIDTH; i++) {
		playfield[i] |= buf[i];
	}
}

void outputPlayfield() {
	setMatrix(playfield, oldPlayfield);
	memcpy(&oldPlayfield, &playfield, MATRIX_WIDTH*2);
}

void restoreOldPlayfield() {
	memcpy(&playfield, &oldPlayfield, MATRIX_WIDTH*2);
}
