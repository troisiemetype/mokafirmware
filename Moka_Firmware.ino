/*
 * Moka is a board that manage 16 RGB leds and 16 soft buttons as an user interface with visual feedback.
 * Copyright 2017 - Pierre-Loup Martin / le labo du troisième
 *
 * This program is part of Moka.
 *
 * Moka is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Infos:
 * https://www.lelabodutroisieme.fr
 * https://www.github.com/troisiemetype/mokaboard for hardware reference
 * https://www.github.com/troisiemetype/mokafirmware for software reference
 * https://www.github.com/troisiemetype/moka for user library
 */


/*
 * Pin defition
 * These are given for information. Direct port manipulation is used to speed up things.
 * Pin settings are done in include files.

 * PORTB mapping:
 * PB0: addr0: input pullup
 * PB1: addr1: input pullup
 * PB2: addr2: input pullup
 * PB3: MOSI
 * PB4: MISO
 * PB5: SCK
 * PB6: NC
 * PB7: NC
 *
 * PORTC mapping:
 * PC0: button row 1 output, high.
 * PC1: button row 2 output, high.
 * PC2: button row 3 output, high.
 * PC3: button row 4 output, high.
 * PC4: SDA: input, external pullup.
 * PC5: SCL: input, external pullup.
 * PC6: RESET: input, external pullup.
 * PC7: Not implemented by atemga 328P.
 *
 * PORTD mapping:
 * PD0: addr3: input pullup
 * PD1: addr4: input pullup
 * PD2: int signal: output, low.
 * PD3: led data: output, low.
 * PD4: button col 1: input pullup
 * PD5: button col 2: input pullup
 * PD6: button col 3: input pullup
 * PD7: button col 4: input pullup
 *
 */

//includes
#include "SK6812.h"
#include "Pad.h"

//instanciations of include objects
SK6812 leds = SK6812();
Pad pad = Pad();


//Constants definition
const uint8_t nbLed = 16;


void setup(){
    //Setting up the pins
/*
    DDRB = 0x00;            //All port B pins are input
    PORTB = 0x07;           //PB0, PB1 and PB2 with pullup

    DDRC = 0x0F;            //PC0 - PC3 are Output, PC4 - PC6 are input.
    PORTC = 0x0F;           //PC0 - PC3 are HIGH, PC4 - PC6 with no pullup.

    //DDRD = 0b00001100;
    DDRD = 0x0C;
    //PORTD = 0b11110011;
    PORTD = 0xF3;
    */

    //Led timer setting
    leds.init();

    //Pad setting
    pad.init();


    //
    //Led init
    randomSeed(3224);
}

void loop(){
    pad.update();
    testRandomLed();

}

//Test function
void testRandomLed(){
    for (uint8_t i = 0; i < 16; i++){
        leds.setLed(i, random(256), random(256), random(256));
    }
    leds.update();

    delay(1000);
}

