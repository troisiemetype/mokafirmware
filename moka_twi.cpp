//TWI as slave for the moka board

/*
 * This is a library for managing TWI on the moka board
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

#include "moka_twi.h"

#include "moka_leds.h"
#include "moka_pad.h"

#include <Wire.h>

/* This file manages TWI communication, and dispatch requests from master to slave functions
 * The master can send or request data
 * Data sent is managed by mw_receivedhandler()
 * Data request is managed by mw_requestHandler();
 *
 * A data send will be typically an instruction byte from master,
 * followed by one or more data byte(s) from master to slave
 * A data request will be typically an instruction byte,
 * followed by one or more data byte(s) from slave to master.
 */

//Led register

const uint8_t SET_ONE_LED = 0x00;		// SET_ONE_LED | LedNumber + 1/3 bytes
const uint8_t SET_GLOBAL_LED = 0x10;	// SET_GLOBAL + 1/3 bytes
const uint8_t SET_ALL_LED = 0x20;		// SET_ALL + 16/72 bytes
//const uint8_t GET_BUTTON = 0x30;		// GET_BUTTON | ButtonNumber + 1 byte from slave to master
const uint8_t GET_BUTTONS = 0x40;		// GET_BUTTONS + 1 byte from slave to master
const uint8_t LED_STATE = 0x50; 		// LED_STATE + 2 byte

//System registers
const uint8_t DISPLAY_STATE = 0x60;		// DISPLAY_STATE | State
const uint8_t BLINK_STATE = 0x70;		// BLINK_STATE | State
const uint8_t BLINK_ON_DELAY = 0x80;	// BLINK_ON_DELAY + 2 bytes
const uint8_t BLINK_OFF_DELAY = 0x81;	// BLINK_OFF_DELAY + 2 bytes
const uint8_t DEBOUNCE_DELAY = 0x82;	// DEBOUNCE_DELAY + 1 byte

const uint8_t HAS_CHANGED = 0x83;		// HAS_CHANGED + 1 byte from slave to master // INT emitted

const uint8_t COLOR_MODE = 0x84; 		// COLOR_MODE | mode

const uint8_t CLR_DISPLAY = 0xF0;		// CLR_DISPLAY
const uint8_t UPDATE_LEDS = 0xF5;		// UPDATE_LEDS

const uint8_t RESET = 0xFF;				// RESET

//TWI states
const uint8_t TWI_SEND_IDLE = 0;
const uint8_t TWI_SEND_BUTTON = 0x10;
const uint8_t TWI_SEND_BUTTONS = 0x20;
const uint8_t TWI_SEND_INT = 3;

uint8_t _mw_twiState = TWI_SEND_IDLE;

//color mode
const uint8_t COLOR_MODE_8 = 0;
const uint8_t COLOR_MODE_24 = 1;

uint8_t _mw_colorMode = COLOR_MODE_8;


const uint8_t baseAddress = 10;
uint8_t twiAddress = baseAddress;



// TWI init function.
void mw_init(){
    // Setting address pins
    // PD4: addr0: input pullup
    // PD5: addr1: input pullup
    // PD6: addr2: input pullup
    // PD7: addr3: input pullup
    // PB0: addr4: input pullup

    DDRB &= ~(0x01);
    PORTB |= 0x01;

    DDRD &= ~(0xF0);
    PORTD |= 0xF0;

    delay(1);

    // Setting the TWI address of the board, using the five address jumpers.
    uint8_t twiOffset = 0;
    twiOffset |= (bit_is_clear(PIND, 4) << 0);
    twiOffset |= (bit_is_clear(PIND, 5) << 1);
    twiOffset |= (bit_is_clear(PIND, 6) << 2);
    twiOffset |= (bit_is_clear(PIND, 7) << 3);
    twiOffset |= (bit_is_clear(PINB, 0) << 4);

    // Set the address ports back as high-Z so they don't draw current.
    PORTB &= ~0x01;
    PORTD &= ~0xF0;

    // Add the jumper offset to the device address.
    twiAddress = baseAddress + twiOffset;

    Wire.begin(twiAddress);

    // Enable general call recognition, for broadcast adressing with address 0.
    TWAR |= _BV(TWGCE);

    Wire.onReceive(mw_receiveHandler);
    Wire.onRequest(mw_requestHandler);
}

void mw_receiveHandler(int bytes){
	uint8_t command = Wire.read();

	if(_mw_colorMode == COLOR_MODE_8){
		if(((command ^ SET_ONE_LED) & 0xF0) == 0){
			uint8_t value = Wire.read();
			ml_setColor(command & 0x0F, value);

		} else if(((command ^ SET_GLOBAL_LED) & 0xF0) == 0){
			uint8_t value = Wire.read();
			for(uint8_t i = 0; i < 16; i++){
				ml_setColor(i, value);				
			}

		} else if(((command ^ SET_ALL_LED) & 0xF0) == 0){
			for(uint8_t i = 0; i < 16; i++){
				uint8_t value = Wire.read();
				ml_setColor(i, value);				
			}
		}

	} else if (_mw_colorMode == COLOR_MODE_24){
		if(((command ^ SET_ONE_LED) & 0xF0) == 0){
			uint8_t rValue = Wire.read();
			uint8_t gValue = Wire.read();
			uint8_t bValue = Wire.read();
			ml_setColor(command & 0x0F, rValue, gValue, bValue);

		} else if(((command ^ SET_GLOBAL_LED) & 0xF0) == 0){
			uint8_t rValue = Wire.read();
			uint8_t gValue = Wire.read();
			uint8_t bValue = Wire.read();
			for(uint8_t i = 0; i < 16; i++){
				ml_setColor(i, rValue, gValue, bValue);				
			}

		} else if(((command ^ SET_ALL_LED) & 0xF0) == 0){
			for(uint8_t i = 0; i < 16; i++){
				uint8_t rValue = Wire.read();
				uint8_t gValue = Wire.read();
				uint8_t bValue = Wire.read();
				ml_setColor(i, rValue, gValue, bValue);				
			}
		}
	}

	if(((command ^ GET_BUTTONS) & 0xF0) == 0){
	 	_mw_twiState = TWI_SEND_BUTTONS;

	} else if((command ^ LED_STATE) == 0){
		uint16_t data = ((uint16_t)Wire.read() << 8);
		data |= Wire.read();
		ml_setLed(data);

	} else if(((command ^ DISPLAY_STATE) & 0xF0) == 0){
		ml_setDisplayState((bool)(command & 0x01));

	} else if(((command ^ BLINK_STATE) & 0xF0) == 0){
		ml_setBlinkState((bool)(command & 0x01));

	} else if((command ^ BLINK_ON_DELAY) == 0){
		uint16_t delay = 0;
		delay |= ((uint16_t)Wire.read() << 8);
		delay |= (Wire.read());
		ml_setBlinkOnDelay(delay);

	} else if((command ^ BLINK_OFF_DELAY) == 0){
		uint16_t delay = 0;
		delay |= ((uint16_t)Wire.read() << 8);
		delay |= (Wire.read());
		ml_setBlinkOffDelay(delay);
		
	} else if((command ^ DEBOUNCE_DELAY) == 0){
		uint8_t delay = Wire.read();
		mp_setDebounceDelay(delay);

	} else if((command ^ HAS_CHANGED) == 0){
		_mw_twiState = TWI_SEND_INT;

		// TODO: see if it work like that, or if it will have to be changed.
	} else if(((command ^ COLOR_MODE) & 0xFE) == 0){
		_mw_colorMode = (command & 0x01);
	} else if((command ^ CLR_DISPLAY) == 0){
		ml_clrLeds();
	} else if((command ^ UPDATE_LEDS) == 0){
		ml_update();
	} else if((command ^ RESET) == 0){
		//reset boad
	}

}

// Request handler.
void mw_requestHandler(){

	uint16_t buttons = 0;
	switch (_mw_twiState){
		case TWI_SEND_BUTTONS:
			// Get a reading from buttons
			buttons = mp_getButtons();
			// Wire implementation causes only one write() can be made per request,
			// so a buffer is set to send both bytes (int16 cut in two) at once. 
			uint8_t but[2];
			but[0] = (uint8_t)(buttons >> 8);
			but[1] = (uint8_t)(buttons & 0xFF);
			Wire.write(but, 2);
			_mw_twiState = TWI_SEND_IDLE;
			break;
/*		case TWI_SEND_INT:
			Wire.write(_mp_int);
			_mp_int = false;
			_mw_twiState = TWI_SEND_IDLE;
			break;
*/		default:
			break;
	}

}