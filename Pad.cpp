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

Pad::init(){
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

	_prev = 0;
	_now = 0;
	_state = 0;
	_ptate = 0;

	_time = millis();
	_debounceDelay = 5;

}

//set a cutom debounce delay. Defaut is 5ms.
void Pad::setdebounceDelay(uint16_t debounce){
	_debounceDelay = debounce;
}

//Get the value for a button
//The returned value is true when pushed, i.e. pulled low.
bool Pad::getButton(uint8_t button){
	return ~(buttonState & _BV(button));
}

//Get the values for the whole grid
//The returned value is true when pushed.
uint16_t Pad::getbuttons(){
	return ~buttonState;
}

//update the pad reading. Debounce is applied to the whole pad, not a specific button.
bool Pad::update(){

	_prev = _now;
	_now = 0;
	for(uint8_t row = 0; row < 4; row++){
		PORTC |= 0x0F;
		//Buttons are active low, so the row to be read is turned low.
		PORTC &= ~(_BV(row));
		//Add a delay before reading?
		uint8_t reading = (PIND & 0xF0) >> 4;
		_now |= ((reading & 0x0F) << row);
	}

	if(_prev != _now){
		_time = millis();
		return false;
	}

	if((_state != _now) && ((millis() - _time) > _debounceDelay)){
		_pState = _state;
		_state = _now;
		return true;
	}
	return false;
}