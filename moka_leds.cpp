//SK8612 driver library

/*
 * This is a library for driving SK6812 addressable leds (aka Neopixels), using timer interrupts
 * Copyright 2017 - Pierre-Loup Martin / le labo du troisième
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moka_leds.h"


// leds are numbered from 0 to 15, from left to right and from top to bottom
// Each led has RGB value, arranged as GRB.
// The table is ordered GRB so the data can be streamed raw, but setting methods use the classis RGB order.

// Timer2 is used to end data bits to leds, via pin3 pwm (PORTD3, OCR2B).
// It's disabled most of the time, only turned on when updating,
// and turned off again when all bits have been shifted.

// Moka run at 8MHz on the calibrated internal RC oscillator.
// 

//The top number when bit shifting. This is 16 leds * 3 colors * 8 bits


const uint16_t NUM_LED = 16;

//The table storing the current led color.
uint8_t _ml_ledColor[NUM_LED][3];
uint8_t _ml_ledData[NUM_LED][3];

//Led state, i.e. on or off: one bit per led
uint16_t _ml_ledState = 0;

//The display state, i.e. if leds are lit or shut, independently of their respective values
bool _ml_displayOn = false;

//The blink state, i.e. if the display blinks
bool _ml_displayBlink = false;
bool _ml_currentBlink = false;

uint16_t _ml_blinkOnDelay = 1000;
uint16_t _ml_blinkOffDelay = 1000;

//Init the timer for led driving
void ml_init(){
    //Set the led data pin, output, default to 0;

    DDRB |= _BV(DDB1);
    PORTB &= ~(_BV(PORTB1));

    ml_clrLeds(); 
    ml_setDisplayState(true);

}

// Set led value with 8 bits-defined color
// color is 0xAARRGGBB, and is converted into 3-bytes color value before to be stored in led table
// 1 offset on the alpha channel is for having 4 steps of luminosity.
void ml_setColor(uint8_t ledId, uint8_t color){
	uint8_t aChannel = ((color >> 6) & 0x03) + 1;
	uint8_t rChannel = (color >> 4) & 0x03;
	uint8_t gChannel = (color >> 2) & 0x03;
	uint8_t bChannel = (color >> 0) & 0x03;
	rChannel *= aChannel * 21;
	gChannel *= aChannel * 21;
	bChannel *= aChannel * 21;
	ml_setColor(ledId, rChannel, gChannel, bChannel);

}

//Set led value with 8 bits value for R, G and B channels.
void ml_setColor(uint8_t ledId, uint8_t rChannel, uint8_t gChannel, uint8_t bChannel){
	_ml_ledColor[ledId][0] = gChannel;
	_ml_ledColor[ledId][1] = rChannel;
	_ml_ledColor[ledId][2] = bChannel;
}

//Get led value on 24 bits
uint32_t ml_getColor(uint8_t ledId){
	uint8_t rChannel = _ml_ledColor[ledId][1];
	uint8_t gChannel = _ml_ledColor[ledId][0];
	uint8_t bChannel = _ml_ledColor[ledId][2];
	return (uint32_t)(((uint32_t)rChannel << 16) | ((uint32_t)gChannel << 8) | ((uint32_t)bChannel));	
}

//Update the 16 leds.
void ml_update(){
	// If display is on, then copy values from ledColor to ledData
	if(_ml_displayOn){
		// For each led, copy value if led is lit, else fill color bytes with zeros
		for(uint8_t i = 0; i < NUM_LED; i++){
			if(_ml_ledState & _BV(i)){
				memcpy(_ml_ledData[i], _ml_ledColor[i], 3);
			} else {
				memset(_ml_ledData[i], 0, 3);
			}
		}
	// If display is off, then just fill led Data with zeros.
	} else {
		// ledData is filled with 0
		memset(_ml_ledData, 0, sizeof(_ml_ledData));
	}

	// Pointer to ledData, and first byte of data to be sent to leds.
	uint8_t *ptr = &_ml_ledData[0][0];
	uint8_t curByte = *ptr;

	// Value for turning deicated port pin high or low.
	uint8_t hi = PORTB | _BV(PORTB1);
	uint8_t lo = PORTB & ~_BV(PORTB1);

	// Counters for total byte number, and for each bit in these bytes.
	uint8_t counter = 0;
	uint8_t nbByte = 0;

	// Save current state register before to disable ISR.
	uint8_t sreg = SREG;
	cli();

	asm volatile(
		"ldi 	%[nbByte], 48			\n\t" // 1		Init bytes counter
		"head%=:						\n\t" // /		Label head (new byte)
		"ld 	%[curByte], %a[ptr]+	\n\t" // 2		Load the next value
		"ldi 	%[counter], 8			\n\t" // 1		init bit counter
		"bit%=:							\n\t" // /		Label bit (next bit)
		"out 	%[port], %[hi]			\n\t" // 1		Set port pin high
		"sbrs	%[curByte], 7			\n\t" // 1/2	Skip if bit is set
		"out 	%[port], %[lo]			\n\t" // 1		Set port pin low
		"lsl	%[curByte]				\n\t" // 1		Shift register left
		"out 	%[port], %[lo]			\n\t" // 1		Set port bit low
		"dec 	%[counter]				\n\t" // 1		Decrement bit counter
		"brne	bit%=					\n\t" // 1/2	branch label bit if counter is 0
		"dec 	%[nbByte]				\n\t" // 1		Decrement byte counter
		"brne 	head%=					\n\t" // 1/2	branch label head if counter is 0

		:	[counter]	"+d"	(counter),
			[nbByte]	"+d"	(nbByte),
			[curByte]	"+r"	(curByte)
		:	[lo]		"r"		(lo),
			[hi]		"r"		(hi),
			[port]		"I"		(_SFR_IO_ADDR(PORTB)),
			[ptr]		"e"		(ptr)
	);

	// Enable ISR again.
	SREG = sreg;
}

// Set led states for a 16 bit int, each bit beeing a led
void ml_setLed(uint16_t state){
	_ml_ledState = state;
}

// Set led state for one led
void ml_setLed(uint8_t ledId, bool state){
	if(state){
		_ml_ledState |= _BV(ledId);
	} else {
		_ml_ledState &= ~_BV(ledId);
	}
}

// Set the diplay state (turned on or off)
void ml_setDisplayState(bool state){
	_ml_displayOn = state;
}

// Set the blink state (turned on or off)
void ml_setBlinkState(bool state){
	_ml_displayBlink = state;
}

//Set the duration of the on state when blinking
void ml_setBlinkOnDelay(uint16_t delay){
	_ml_blinkOnDelay = delay;
}

//Set the duration of the off state when blinking
void ml_setBlinkOffDelay(uint16_t delay){
	_ml_blinkOffDelay = delay;
}

void ml_blink(){

}

//Clear all leds (each channel of each led is set to 0)
void ml_clrLeds(){
	for(uint8_t i = 0; i < 16; i++){
		ml_setColor(i, 0, 0, 0);
	}
}

