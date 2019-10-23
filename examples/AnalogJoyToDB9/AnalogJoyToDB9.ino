/***************************************************************************
 * This file is part of AnalogJoystick for Arduino.                        *
 *                                                                         *
 * Copyright (C) 2019 by SukkoPera                                         *
 *                                                                         *
 * AnalogJoystick is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * AnalogJoystick is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with AnalogJoystick. If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************
 *
 * Sketch that turns a 2-axis 2-button joystick into an Atari-style one with
 * 4 directions and 2 buttons which can be safely used on Commodore computers:
 * Vic 20, C-64, C-16 (through an adapter), Amiga...
 *
 * The calibration procedure works like this:
 * - When the LED starts blinking move all the joypad levers to their extremes a
 *   couple of times.
 * - When the LED starts blinking faster prepare to let go of all levers.
 * - When the LED remains on permanently, immediately return all levers to
 *   their center position and don't touch them anymore.
 * - When the LED turns off, calibration is complete.
 */

#include <EEPROM.h>
#include <AnalogJoystick.h>

#define N_BUTTONS 2
#define N_AXES 2

// EEPROM address at which calibration data is saved/loaded
#define EEPROM_ADDR 42

const byte buttonPins[N_BUTTONS] = {
	2, 3
};

const byte axisPins[N_AXES] = {
	A0, A1
};

// OUTPUT pins, connected to the DB-9 port
const byte PIN_UP = 4;    // DB-9 Pin 1
const byte PIN_DOWN = 5;  // DB-9 Pin 2
const byte PIN_LEFT = 6;  // DB-9 Pin 3
const byte PIN_RIGHT = 7; // DB-9 Pin 4
const byte PIN_BTN1 = 8;  // DB-9 Pin 6
const byte PIN_BTN2 = 9;  // DB-9 Pin 9

/** \brief Axis dead zone
 *  
 * If the analog stick moves less than this value from the center position, it
 * is considered still.
 */
const uint8_t ANALOG_DEAD_ZONE = 30;

/******************************************************************************/

AnalogJoystick joystick;

inline void buttonPress (byte pin) {
	/* Drive pins in open-collector style, so that we are compatible with the
	 * C64 too
	 */
	pinMode (pin, OUTPUT);  // Low is implicit
}

inline void buttonRelease (byte pin) {
	pinMode (pin, INPUT); 	// Hi-Z
}

void setup () {
	// Initialize the AnalogJoystick library
	joystick.begin (N_AXES, axisPins, N_BUTTONS, buttonPins);

	boolean calibrationNeeded = false;

	// If button 1 is pressed, force calibration
	joystick.read ();
	if (joystick.getButton (0)) {
		calibrationNeeded = true;
	}

	// See if we have calibration data handy
	if (!calibrationNeeded) {
		AnalogJoystick::CalibrationData calData;
		EEPROM.get (EEPROM_ADDR, calData);
		calibrationNeeded = !joystick.calibrate (calData);
	}

	if (calibrationNeeded) {
		// Calibrate
		while (!joystick.isCalibrated ()) {
			joystick.calibrate (LED_BUILTIN);
		}

		// Save calibration data for next time
		AnalogJoystick::CalibrationData calData = joystick.getCalibrationData ();
		EEPROM.put (EEPROM_ADDR, calData);
	}

	// For the LED, in case it wasn't done by calibrate()
	pinMode (LED_BUILTIN, OUTPUT);
	
	// Outputs: All pins to Hi-Z without pull-up
	pinMode (PIN_UP, INPUT);
	pinMode (PIN_DOWN, INPUT);
	pinMode (PIN_LEFT, INPUT);
	pinMode (PIN_RIGHT, INPUT);
	pinMode (PIN_BTN1, INPUT);
	pinMode (PIN_BTN2, INPUT);	
}

void loop () {
	// Update joystick values
	joystick.read ();

	int8_t deltaX = joystick.getAxis (0) - AXIS_CENTER_VALUE;		// 0 ... 255 --> -128 ... +127

	if (deltaX < -ANALOG_DEAD_ZONE) {
		buttonPress (PIN_LEFT);
	} else {
		buttonRelease (PIN_LEFT);
	}

	if (deltaX > +ANALOG_DEAD_ZONE) {
		buttonPress (PIN_RIGHT);
	} else {
		buttonRelease (PIN_RIGHT);
	}
	
	int8_t deltaY = joystick.getAxis (1) - AXIS_CENTER_VALUE;

	if (deltaY < -ANALOG_DEAD_ZONE) {
		buttonPress (PIN_UP);
	} else {
		buttonRelease (PIN_UP);
	}

	if (deltaY > +ANALOG_DEAD_ZONE) {
		buttonPress (PIN_DOWN);
	} else {
		buttonRelease (PIN_DOWN);
	}
	
	if (joystick.getButton (0)) {
		buttonPress (PIN_BTN1);
	} else {
		buttonRelease (PIN_BTN1);
	}

	if (joystick.getButton (1)) {
		buttonPress (PIN_BTN2);
	} else {
		buttonRelease (PIN_BTN2);
	}
	
	// Flash led with buttons, I like this
	digitalWrite (LED_BUILTIN, joystick.getButton (0) | joystick.getButton (1));
}
