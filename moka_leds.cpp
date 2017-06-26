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

//The top number when bit shifting. This is 16 leds * 3 colors * 8 bits
const uint16_t _ml_top = 384;

//The table storing the current led color.
uint8_t _ml_ledColor[16][3];
//The table storing the timer value for each bit
uint8_t _ml_pwmTable[_ml_top];
//Led state, i.e. on or off: one bit per led
uint16_t _ml_ledState = 0;

//The current bit index when bitshifting
volatile uint16_t _ml_current = 0;

//The display state, i.e. if leds are lit or shut, independently of their respective values
bool _ml_displayOn = false;

//The blink state, i.e. if the display blinks
bool _ml_displayBlink = false;
bool _ml_currentBlink = false;

uint16_t _ml_blinkOnDelay = 1000;
uint16_t _ml_displayOffDelay = 1000;

//Init the timer for led driving
void ml_init(){
	//Disable interrupts when setting registers
    cli();

    //The SK6812 leds are driven by only one pin, which is both data and clock.
    //A 0 is sent by driven the pin high for 0.3us and low for 0.9us.
    //A 1 is sent by driven the pin high for 0.6us and low for 0.6us.
    //At 16MHz, 0.3us is 4.8 clock cycles
    //          0.6us is 9.6 clock cycles
    //          0.9us is 14.4 clock cycles
    //The mean time of a bit is given to be 1.25us, that is 20 clock cycles.
    //So we drive pin high 5 clock cycle for a 0, 10 for a 1, and clear counter at 20.

    //Timer 2 setting (see atmel datasheet, chapter 18, Timer 2):
    //fast PWM
    //counting from bottom to OCR2A
    //OCR2B clear on compare match, set on bottom.

    TCCR2A = 0x23;          //COM2A pin disable, COM2B pin clear on compare match, WGM2 fast pwm
    TCCR2B = 0x08;          //WGM22 TOP = OCR2A, TIMER disable, no prescalling.
    TIMSK2 = 0x02;			//OCIE2A enable. It must be set for the compare interrupt to fire.
    TCNT2 = 0;              //Init timer to 0
    OCR2A = 20;             //One bit of led data is 1.25 us, that is 20 clock cycles
    OCR2B = 5;              //Defaut to 0. See K6812 datasheet

    //Set interrupt again;
    sei();

    //Set the led data pin, output, default to 0;

    DDRD |= _BV(DDD3);
    PORTD &= ~(_BV(PORTD3));

}

// Set led value with 8 bits-defined color
// color is 0xAARRGGBB, and is converted into 3-bytes color value before to be stored in led table
// 1 offset on the alpha channel is for having 4 steps of luminosity.
void ml_setLed(uint8_t ledId, uint8_t color){
	uint8_t aChannel = ((color >> 6) & 0x03) + 1;
	uint8_t rChannel = (color >> 4) & 0x03;
	uint8_t gChannel = (color >> 2) & 0x03;
	uint8_t bChannel = (color >> 0) & 0x03;
	rChannel *= aChannel * 21;
	gChannel *= aChannel * 21;
	bChannel *= aChannel * 21;
	ml_setLed(ledId, rChannel, gChannel, bChannel);

}

//Set led value with 8 bits value for R, G and B channels.
void ml_setLed(uint8_t ledId, uint8_t rChannel, uint8_t gChannel, uint8_t bChannel){
	_ml_ledColor[ledId][0] = gChannel;
	_ml_ledColor[ledId][1] = rChannel;
	_ml_ledColor[ledId][2] = bChannel;
}

//Get led value on 24 bits
uint32_t ml_getLed(uint8_t ledId){
	uint8_t rChannel = _ml_ledColor[ledId][1];
	uint8_t gChannel = _ml_ledColor[ledId][0];
	uint8_t bChannel = _ml_ledColor[ledId][2];
	return (uint32_t)(((uint32_t)rChannel << 16) | ((uint32_t)gChannel << 8) | ((uint32_t)bChannel));	
}

//Update the 16 leds.
void ml_update(){
	//Current index is incremented at each timer TOP
	_ml_current = 0;

	//A high state for a low bit is only 5 clock cycles.
	//To save compute time during interrupt, each bit length is precomputed before turning the timer on.
	// If diplay is Off, or blink is On and current blink is Off, we send only zeros.
	if(!_ml_displayOn || (_ml_displayBlink && !_ml_currentBlink)){
		for(uint16_t index = 0; i < _ml_top; i++){
			_ml_pwmTable[index] = 5;
		}
	// Else, we can send data corresponding to pixels
	} else {
		for(uint8_t led = 0; led < 16; led++){
			//If led is on, we compute each bit length for bit shifting
			if(_ml_ledState & _BV(led)){
				for(uint8_t channel = 0; channel < 3; channel++){
					
					uint8_t index = channel + led * 3;
					for(uint8_t bit = 0; bit < 8; bit++){

						if((_ml_ledColor[led][channel]) & (1 << bit) & ledState){
							_ml_pwmTable[index + bit] = 10;

						} else {
							_ml_pwmTable[index + bit] = 5;

						}
					}
				}
			//But if the led is independently turned Off, we directly set 0 for it.
			} else {
				for(uint8_t i = 0; i < 24; i++){
							_ml_pwmTable[index + bit] = 5;					
				}
			}
		}
	}

	//Timer is turned on (no prescaller), OCR2B pin is set high
	PORTD |= _BV(PORTD3);
	TCCR2B |= _BV(CS20);
}

//Timer COMPA interrupt
ISR(TIMER2_COMPA_vect){
	//Test current bit, stop the timer if bit shifting is finished, or load the next bit length in OCR2B
	if(++_ml_current == _ml_top){
		TCCR2B &= ~(_BV(CS20));
		PORTD &= ~(_BV(PORTD3));
	} else {
		OCR2B = _ml_pwmTable[_ml_current];
	}
}

// Set led states for a 16 bit int, each bit beeing a led
void ml_setLed(uint16_t state){
	_ml_ledState = state;
}

// Set led state for one led
void ml_setLed(uint8_t ledId, bool state){
	if(state){
		_ml_ledState |= ledId;
	} else {
		_ml_ledState &= ~(ledId);
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

//Clear all leds (each channel of each led is set to 0)
void ml_clrLeds(){
	for(uint8_t i = 0; i < 16; i++){
		ml_setColor(i, 0, 0, 0);
	}
}

