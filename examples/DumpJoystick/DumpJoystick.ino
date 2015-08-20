/***************************************************************************
 * This file is part of AnalogJoystick for Arduino.                        *
 *                                                                         *
 * Copyright (C) 2015 by SukkoPera                                         *
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
 * Sketch that shows basic usage of the library, reading a 2-axis 2-button
 * joystick and continuously reporting its state.
 *
 * The calibration procedure works like this:
 * - When the LED starts blinking move all the joypad levers to their extremes a
 *   couple of times.
 * - When the LED starts blinking faster prepare to let go of all levers.
 * - When the LED remains on permanently, immediately return all levers to
 *   their center position and don't touch them anymore.
 * - When the LED turns off, calibration is complete.
 */

#include <AnalogJoystick.h>

#define N_BUTTONS 2
#define N_AXES 2

const byte buttonPins[N_BUTTONS] = {
  2, 3
};

const char *buttonNames[N_BUTTONS] = {
  "B1",
  "B2"
};

const byte axisPins[N_AXES] = {
  A0, A1
};

const char *axisNames[N_AXES] = {
  "X",
  "Y"
};

AnalogJoystick joystick;

void setup () {
  Serial.begin (9600);

  Serial.println (F(
    "Press a button on your joystick to start the calibration procedure:\n"
    "- When the LED starts blinking move all the joypad levers to their\n"
    "  extremes a couple of times.\n"
    "- When the LED starts blinking faster prepare to let go of all levers.\n"
    "- When the LED remains on permanently, immediately return all levers to\n"
    "  their center position and don't touch them anymore.\n"
    "- When the LED turns off, calibration is complete\n"
  ));

  // Initialize the AnalogJoystick library
  joystick.begin (N_AXES, axisPins, N_BUTTONS, buttonPins);

  // Note that buttons are digital and thus can be read before calibration
  bool buttonPressed = false;
  while (!buttonPressed) {
    for (int i = 0; !buttonPressed && i < N_BUTTONS; ++i) {
      joystick.read ();
      buttonPressed = joystick.getButton (i);
    }
  }
}

void loop () {
  if (!joystick.isCalibrated ()) {
    joystick.calibrate (LED_BUILTIN);
  } else {
    // Update joystick values
    joystick.read ();

    // Show the raw readings
    for (int i = 0; i < N_AXES; ++i) {
      Serial.print (axisNames[i]);
      Serial.print ("=");
      Serial.print (joystick.getAxis (i));

      // If you want to see the raw value too, uncomment the following lines
      //Serial.print (" (");
      //Serial.print (joystick.axes[i].rawValue);
      //Serial.print (")");

      Serial.print (" ");
    }

    Serial.print ("Buttons=");
    for (int i = 0; i < N_BUTTONS; ++i) {
      if (joystick.getButton (i)) {
        Serial.print (buttonNames[i]);
        Serial.print (" ");
      }
    }
    Serial.println ("");

    // Wait a bit
    delay (100);
  }
}
