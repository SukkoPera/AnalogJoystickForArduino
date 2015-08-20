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
 * This sketch is specific for one of my joypads, from Xtreme. Even though
 * it's nothing fancy, it was my first PlayStation-like joypad, so I'm
 * particularly fond of it.
 * 
 * Besides useless autofire functions, this joypad has 8 buttons. The first
 * four buttons are reported directly, while the other ones are reported as
 * two more axes. So, at least this will show how to convert axes to buttons.
 * 
 * Note that due to the way they work, if L1 and L2 are pressed
 * simultaneously, only L1 will be reported. Ditto for R1 and R2.
 */

#include <EEPROM.h>
#include <AnalogJoystick.h>
#include <UnoJoy.h>

#define N_BUTTONS 4
#define N_AXES 4

/* The minimum deviation from center to report buttons 5/6/7/8 as pressed. The
 * values that are reported are basically 0-128-255, so just try to work around
 * A/D conversion noise
 */
#define MIN_DELTA 30

struct CalibrationData {
  // A (min, center, max) tuple for every axis
  int bounds[N_AXES * 3];

  /* A signature, having it at the end ensures data has the correct number of
   * axes
   */
  word signature;
};

#define SIGNATURE 0xCA1B

#define EEPROM_ADDR 0


const byte buttonPins[N_BUTTONS] = {
  2, 3, 4, 5
};

const byte axisPins[N_AXES] = {
  A0, A1, A2, A3
};

AnalogJoystick joystick;

void setup () {
  joystick.begin (N_AXES, axisPins, N_BUTTONS, buttonPins);

  bool doCalibration = false;

  // If button 1 is pressed, force calibration
  joystick.read ();
  if (joystick.getButton (0))
    doCalibration = true;

  // See if we have calibration data handy
  if (!doCalibration) {
    CalibrationData calData;
    EEPROM.get (EEPROM_ADDR, calData);
    if (calData.signature == SIGNATURE)
      joystick.calibrate (calData.bounds);
    else
      doCalibration = true;
  }

  if (doCalibration) {
    // Calibrate
    while (!joystick.isCalibrated ())
      joystick.calibrate (LED_BUILTIN);

    // Save calibration data for next time
    CalibrationData calData;
    for (int i = 0; i < N_AXES * 3; i += 3) {
      AnalogJoystick::Axis& axis = joystick.axes[i / 3];
      calData.bounds[i] = axis.min;
      calData.bounds[i + 1] = axis.center;
      calData.bounds[i + 2] = axis.max;
    }
    calData.signature = SIGNATURE;
    EEPROM.put (EEPROM_ADDR, calData);
  }

  setupUnoJoy ();

  // For the LED, in case it wasn't done by calibrate()
  pinMode (LED_BUILTIN, OUTPUT);
}

void loop () {
  // Update joystick values
  joystick.read ();

  // Make up data for UnoJoy
  dataForController_t controllerData = getBlankDataForController ();
  controllerData.leftStickX = (uint8_t) abs (joystick.getAxis (0) - 255);
  controllerData.leftStickY = (uint8_t) abs (joystick.getAxis (1) - 255);
  controllerData.squareOn = joystick.getButton (0);
  controllerData.triangleOn = joystick.getButton (1);
  controllerData.crossOn = joystick.getButton (2);
  controllerData.circleOn = joystick.getButton (3);

  // Convert axes 2 and 3 to buttons
  controllerData.l1On = (joystick.getAxis (2) > AXIS_CENTER_VALUE + MIN_DELTA);
  controllerData.l2On = (joystick.getAxis (2) < AXIS_CENTER_VALUE - MIN_DELTA);
  controllerData.r1On = (joystick.getAxis (3) > AXIS_CENTER_VALUE + MIN_DELTA);
  controllerData.r2On = (joystick.getAxis (3) < AXIS_CENTER_VALUE - MIN_DELTA);

  // Flash led with buttons, I like this
  digitalWrite (LED_BUILTIN, controllerData.squareOn | controllerData.triangleOn |
                             controllerData.crossOn | controllerData.circleOn |
                             controllerData.l1On | controllerData.l2On |
                             controllerData.r1On | controllerData.r2On);
  
  setControllerData (controllerData);
}

