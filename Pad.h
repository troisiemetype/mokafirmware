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

#ifndef PAD_H
#define PAD_H

#include <Arduino.h>

class Pad{
public:
	init();

	void setDebounceDelay(uint16_t);

	bool getButton(uint8_t button);
	uint16_t getButtons();

	void update();


private:
	uint16_t _now;
	uint16_t _prev;
	uint16_t _state;
	uint16_t _pState;

	uint32_t _time;
	uint16_t _debounceDelay;


};

#endif