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

 #include "SK6812.h"


//led state
//leds are numbered from 0 to 15, from left to right and from top to bottom
//Each led has RGB value, arranged as GRB.
//this table is ordered GRB, but I2C communication from mater uses RGB.

const uint16_t topIndex = 384;
uint8_t pwmTable[topIndex];
volatile uint16_t currentIndex = 0;

ISR(TIMER2_COMPA_vect){
	if(++currentIndex == topIndex){
		TCCR2B &= ~(_BV(CS20));
		PORTD &= ~(_BV(PORTD3));
	} else {
		OCR2B = pwmTable[currentIndex];
	}
}

//Init the timer for led driving
void SK6812::init(){
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
}

//set led value with 6 bits-defined color
void SK6812::setLed(uint8_t ledId, uint8_t color){
	uint8_t rChannel = (color & 0x00110000) * 85;
	uint8_t gChannel = (color & 0x00001100) * 85;
	uint8_t bChannel = (color & 0x00000011) * 85;
	setLed(ledId, rChannel, gChannel, bChannel);

}

//Set led value with 24-bits defined color
void SK6812::setLed(uint8_t ledId, uint32_t color){
	uint8_t rChannel = (color >> 16);
	uint8_t gChannel = (color >> 8);
	uint8_t bChannel = color;
	setLed(ledId, rChannel, gChannel, bChannel);
}

//Set led value with 8 bits value for R, G and B channels.
void SK6812::setLed(uint8_t ledId, uint8_t rChannel, uint8_t gChannel, uint8_t bChannel){
	ledState[ledId][0] = gChannel;
	ledState[ledId][1] = rChannel;
	ledState[ledId][2] = bChannel;
}

//Get led value on 24 bits
uint32_t SK6812::getLed(uint8_t ledId){
	uint8_t rChannel = ledState[ledId][1];
	uint8_t gChannel = ledState[ledId][0];
	uint8_t bChannel = ledState[ledId][2];
	return (uint32_t)(((uint32_t)rChannel << 16) | ((uint32_t)gChannel << 8) | ((uint32_t)bChannel));	
}

//Update the 16 leds.
void SK6812::update(){
	//Current index is incremented at each timer TOP
	currentIndex = 0;

	//A high state for a low bit is only 5 clock cycles.
	//To save compute time during interrupt, each bit length is precomputed before turning the timer on.
	for(uint8_t led = 0; led < 16; led++){

		for(uint8_t channel = 0; channel < 3; channel++){
			
			uint8_t index = channel + led * 3;
			for(uint8_t bit = 0; bit < 8; bit++){

				if((ledState[led][channel] && (1 << bit))){
					pwmTable[index + bit] = 10;
				} else {
					pwmTable[index + bit] = 5;
				}
			}
		}
	}

	PORTD |= _BV(PORTD3);
	TCCR2B |= _BV(CS20);
}