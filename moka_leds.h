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

#ifndef MOKA_LEDS_H
#define MOKA_LEDS_H

#include <Arduino.h>

void ml_init();
void ml_setColor(uint8_t ledId, uint8_t color);
void ml_setColor(uint8_t ledId, uint8_t rChannel, uint8_t gChannel, uint8_t bChannel);

void ml_setLed(uint16_t state);
void ml_setLed(uint8_t ledId, bool state);
void ml_setDisplayState(bool state);
void ml_setBlinkState(bool state);

void ml_setBlinkOnDelay(uint16_t delay);
void ml_setBlinkOffDelay(uint16_t delay);

void ml_clrLeds();

uint32_t ml_getLed(uint8_t ledId);

void ml_update();

#endif
 