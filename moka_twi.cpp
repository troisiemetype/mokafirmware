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
 *
 * There are a certain amount of features to be implemented
 * System features:
 * Reset 								Reset
 * Debounce delay 						DebounceDelay + 1 byte
 * Leds ON 								DisplayState | (bool)state
 * Blink 								BlinkState | (bool)state
 * Blinking ON delay 					BlinkOnDelay + 2 bytes
 * Blinking OFF delay 					BlinkOffDelay + 2 bytes
 *
 * Led features:
 * Clear display 						ClrDisplay
 * Set a led color (8 bits)				SetLed8 | LedNumber + 1 byte
 * Set a led color (24 bits)			SetLed24 | LedNumber + 3 bytes
 * Set global color (8 bits)			SetColor8 + 1 byte
 * Set global color (24 bits)			SetColor24 + 3 bytes
 * Get a led color (8 bits)				GetLed8 | LedNumber + 1 byte to master
 * Get a led color (24 bits)			GetLed24 | LedNumber + 3 bytes to master
 * 
 * Button features:
 * Read button 							ReadButton | ButtonNumber + 1 byte to master
 * Read buttons							ReadButtons + 1 byte to master
 */

//Led register
const uint8_t SET_ONE_LED = 0x00;
const uint8_t SET_ALL_LED = 0x40;

const uint8_t COLOR_8 = 0x00;
const uint8_t COLOR_24 = 0x10;

const uint8_t OFF = 0x20;
const uint8_t ON = 0x30;

//Buttons register
const uint8_t GET_BUTTON = 0x80;
const uint8_t GET_BUTTONS = 0x90;

//System registers
const uint8_t DISPLAY_STATE = 0xA0;
const uint8_t BLINK_STATE = 0xA2;
const uint8_t BLINK_ON_DELAY = 0xA4;
const uint8_t BLINK_OFF_DELAY = 0xA5;
const uint8_t DEBOUNCE_DELAY = 0xA6;

const uint8_t HAS_CHANGED = 0xB0;		//Ask if INT was emmited

const uint8_t CLR_DISPLAY = 0xF0;
const uint8_t UPDATE_LEDS = 0xF5;

const uint8_t RESET = 0xFF;


const uint8_t baseAddress = 10;
uint8_t twiAddress = baseAddress;


void mw_init(){
    // Setting address pins
    // PB0: addr0: input pullup
    // PB1: addr1: input pullup
    // PB2: addr2: input pullup
    // PD0: addr3: input pullup
    // PD1: addr4: input pullup

    DDRB &= ~(0x00);
    PORTB |= 0x07;

    DDRD &= ~(0x00);
    PORTD |= 0x03;

    //setting the TWI address of the board, using the five address jumpers.
    uint8_t twiOffset = 0;
    twiOffset |= (bit_is_clear(PINB, 0) << 0);
    twiOffset |= (bit_is_clear(PINB, 1) << 1);
    twiOffset |= (bit_is_clear(PINB, 2) << 2);
    twiOffset |= (bit_is_clear(PIND, 0) << 3);
    twiOffset |= (bit_is_clear(PIND, 1) << 4);

    twiAddress = baseAddress + twiOffset;

    Wire.begin(twiAddress);

    Wire.onReceived(mw_receiveHandler);
    Wire.onRequest(mw_requestHandler);
}

void mw_receiveHandler(uint8_t bytes){

}

void mw_requestHandler(){

}