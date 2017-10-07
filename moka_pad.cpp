//Moka 4x4 buttons driver library

/*
 * This is a library for reading Moka buttons pad.
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

// Button state are stored as a 16 bits value, each bit beeing a button state.

#include "moka_pad.h"
#include "moka_leds.h"

uint16_t _mp_now;
uint16_t _mp_prev;
uint16_t _mp_state;
uint16_t _mp_pState;

//independent debounce
uint32_t _mp_time[16];

uint16_t _mp_debounceDelay;

//bool _mp_int = false;

// Init all that is linked to buttons.
void mp_init(){
/*
 * PC0: button col 1: input pullup.
 * PC1: button col 2: input pullup.
 * PC2: button col 3: input pullup.
 * PC3: button col 4: input pullup.
 * PD0: button row 1: output high / input.
 * PD1: button row 2: output high / input.
 * PD2: button row 3: output high / input.
 * PD3: button row 4: output high / input.
 */

	DDRC &= ~(0x0F);
	PORTC |= 0x0F;

	DDRD &= ~(0x0F);
	PORTD &= ~(0x0F);

	_mp_prev = 0xFFFF;
	_mp_now = 0xFFFF;
	_mp_state = 0xFFFF;
	_mp_pState = 0xFFFF;

	uint32_t _mp_tTime = millis();

	for(uint8_t i = 0; i < 16; ++i){
		_mp_time[i] = _mp_tTime;
	}

	_mp_debounceDelay = 3;

}

//set a cutom debounce delay. Defaut is 5ms.
void mp_setDebounceDelay(uint16_t debounce){
	_mp_debounceDelay = debounce;
}

//Get the value for a button
//The returned value is true when pushed, i.e. pulled low.
bool mp_getButton(uint8_t button){
	return ~(_mp_state & _BV(button));
}

//Get the values for the whole grid
//The returned value is true when pushed.
uint16_t mp_getButtons(){
//	_mp_int = false;
	return ~_mp_state;
}

//update the pad reading.
bool mp_update(){

	_mp_prev = _mp_now;
	_mp_now = 0;
	for(uint8_t row = 0; row < 4; row++){
		// Turn port D as input.
		DDRD &= ~(0x0F);
		PORTD &= ~(0x0F);
		//Buttons are active low, so the row to be read is turned output low.
		DDRD |= _BV(row);
		PORTD &= ~(_BV(row));
		//Add a delay before reading?
		delay(1);
		uint8_t reading = 0;
		reading = (PINC & 0x0F);
		_mp_now |= ((reading) << (row * 4));
	}

	// Turn port D as input
	DDRD &= ~(0x0F);
	PORTD &= ~(0x0F);

	// Store the current time, so the same is used for every button.
	uint32_t _mp_tTime = millis();
	bool change = false;

//	ml_setLed(~_mp_now);

//	ml_setLed(0);

	// Debounce each button.
	for(uint8_t i = 0; i < 16; ++i){
		// Bit of each button is kept for instant previous and present state,
		// and for current active state.
		bool prev = _mp_prev & _BV(i);
		bool now = _mp_now & _BV(i);
		bool state = _mp_state & _BV(i);
		bool pState = _mp_pState & _BV(i);

		// If there is a difference between instant previous read and current one,
		// then init the counter. 

		if(prev != now){
			_mp_time[i] = _mp_tTime;
//			ml_setLed(i, true);
		}

		// If the delay of this button is more than debounce delay since last change, 
		// we update the current active value.
		if((now != pState) && ((_mp_tTime - _mp_time[i]) > _mp_debounceDelay)){
//			ml_setLed(i, ~now);
			// Copy the previous state.
			_mp_pState = _mp_state;
			uint16_t tempState = _mp_pState;
			// Replace the right bit by the new value. We use a temp value to avoid problems with
			// interrupts firing during reading.
			tempState &= ~_BV(i);
			tempState |= (_mp_now & _BV(i));

			// Then we update the global value.
			uint8_t sreg = SREG;
			cli();
			_mp_state = tempState;
			SREG = sreg;

			// And set a flag for return value.
			change = true;
		}
	}

//	ml_update();
	return change;
}