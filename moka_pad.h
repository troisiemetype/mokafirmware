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

#ifndef MOKA_PAD_H
#define MOKA_PAD_H

#include <Arduino.h>

extern bool _mp_int;

void mp_init();

void mp_setDebounceDelay(uint16_t);

bool mp_getButton(uint8_t button);
uint16_t mp_getButtons();

bool mp_update();


#endif