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

#ifndef SK6812_H
#define SK6812_H

#include <Arduino.h>

class SK6812{
public:

	void init();
	void setLed(uint8_t ledId, uint8_t color);
	void setLed(uint8_t ledId, uint32_t color);
	void setLed(uint8_t ledId, uint8_t rChannel, uint8_t gChannel, uint8_t bChannel);

	uint32_t getLed(uint8_t ledId);

	void update();

private:
	uint8_t ledState[16][3];

};

#endif
 