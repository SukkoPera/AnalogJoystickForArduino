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
 ***************************************************************************/

#include <AnalogJoystick.h>

#define N_BUTTONS 2
#define N_AXES 2

const byte buttonPins[N_BUTTONS] = {
  2, 3
};

const char *buttonNames[N_AXES] = {
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
  // initialize serial communications at 9600 bps:
  Serial.begin (9600);

  joystick.begin (N_AXES, axisPins, N_BUTTONS, buttonPins);
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
