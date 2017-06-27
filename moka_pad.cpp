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

uint16_t _mp_now;
uint16_t _mp_prev;
uint16_t _mp_state;
uint16_t _mp_pState;

uint32_t _mp_time;
uint16_t _mp_debounceDelay;

//bool _mp_int = false;

void mp_init(){
/*
 * PC0: button row 1 output, high.
 * PC1: button row 2 output, high.
 * PC2: button row 3 output, high.
 * PC3: button row 4 output, high.
 * PD4: button col 1: input pullup
 * PD5: button col 2: input pullup
 * PD6: button col 3: input pullup
 * PD7: button col 4: input pullup
 */

	DDRC |= 0x0F;
	PORTC |= 0x0F;

	DDRD &= ~(0xF0);
	PORTD |= 0xF0;

	_mp_prev = 0;
	_mp_now = 0;
	_mp_state = 0;
	_mp_pState = 0;

	_mp_time = millis();
	_mp_debounceDelay = 5;

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

//update the pad reading. Debounce is applied to the whole pad, not a specific button.
bool mp_update(){

	_mp_prev = _mp_now;
	_mp_now = 0;
	for(uint8_t row = 0; row < 4; row++){
		PORTC |= 0x0F;
		//Buttons are active low, so the row to be read is turned low.
		PORTC &= ~(_BV(row));
		//Add a delay before reading?
		uint8_t reading = (PIND & 0xF0) >> 4;
		_mp_now |= ((reading & 0x0F) << row);
	}

	if(_mp_prev != _mp_now){
		_mp_time = millis();
		return false;
	}

	if((_mp_state != _mp_now) && ((millis() - _mp_time) > _mp_debounceDelay)){
		_mp_pState = _mp_state;
		_mp_state = _mp_now;
//		_mp_int = true;
		return true;
	}
	return false;
}