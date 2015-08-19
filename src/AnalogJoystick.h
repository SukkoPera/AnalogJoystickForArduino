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

#include "Arduino.h"

#define MAX_AXES 4
#define MAX_BUTTONS 8

// Seconds
#define CALIBRATION_DURATION 8

// Calibrated value reported by the joystick axes when centered
#define AXIS_CENTER_VALUE 128

// Calibrated value reported by the joystick axes when at minimum
#define AXIS_MIN_VALUE 0

// Calibrated value reported by the joystick axes when at maximum
#define AXIS_MAX_VALUE 255

class AnalogJoystick {
public:
  bool begin (const byte nAxes, const byte axisPins[], const byte nButtons, const byte buttonPins[]);

  bool calibrate (const int ledPin = -1);

  /* WARNING: You have to make sure that the array contains a (min,center,max)
   * tuple for every axis!
   */
  void calibrate (const int axisBounds[]);

  void read ();

  struct Axis {
    // Analog pin to read values from
    byte pin;

    // Minimum value resulting from calibration
    int min;

    // Maximum value resulting from calibration
    int max;

    // Center value resulting from calibration
    int center;

    /* Raw value
     *
     * On my joystick, 0 is DOWN and RIGHT, no idea if this is standard.
     */
    int rawValue;

    /* Normalized value, taking calibration into account.
     *
     * Note that this is a byte, so ranges in 0-255, with 128 being center.
     * 0 is "inherited" from the raw readins, so DOWN and RIGHT too.
     */
    byte value;
  };

  struct Button {
    // Pin to read values from
    byte pin;

    // Guess what!
    bool pressed;
  };

  Axis axes[MAX_AXES];
  Button buttons[MAX_BUTTONS];

  // Commodities
  int getAxis (byte axisNo) const;
  bool getButton (byte buttonNo) const;
  bool isCalibrated () const;

private:
  byte nAxes;
  byte nButtons;

  bool calibrated;
};
