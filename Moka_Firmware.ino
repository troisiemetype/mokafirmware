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
 * PD2: int signal: output, low. NOTA: not implemented yet, due to board misconception leading to short.
 * PD3: led data: output, low.
 * PD4: button col 1: input pullup
 * PD5: button col 2: input pullup
 * PD6: button col 3: input pullup
 * PD7: button col 4: input pullup
 *
 */

//includes
#include "moka_leds.h"
#include "moka_pad.h"
#include "moka_twi.h"

//Constants definition
//const uint8_t nbLed = 16;

/*
uint8_t rColor = 1;
uint8_t gColor = 1;
uint8_t bColor = 1;
*/
uint8_t color = 0b01110100;


void setup(){
    //Setting up the pins


    ml_init();

    for(uint8_t i = 0; i < 16; ++i){
        ml_setColor(i, color);
    }
/*
    for(uint8_t i = 0; i < 16; ++i){
        ml_setLed(i, true);
        ml_update();
        delay(20);
    }

    ml_update();

    delay(500);

    for(uint8_t i = 0; i < 16; ++i){
        ml_setLed(i, false);
        ml_update();
        delay(20);
    }

    ml_update();
*/
    //Pad setting
    mp_init();

    mw_init();

}

void loop(){
//    test();
    mp_update();
//    ml_update();
}

//Test function
void test(){
    if(mp_update()){
        
        uint16_t buttonState = mp_getButtons();
        for(uint8_t i = 0; i < 16; i++){
            
            if(buttonState & _BV(i)){
                ml_setLed(i, true);
                ml_setColor(i, color);
            } else {
                ml_setLed(i, false);
            }
            
            ml_update();
        }
     
    }
    delay(10);
}
