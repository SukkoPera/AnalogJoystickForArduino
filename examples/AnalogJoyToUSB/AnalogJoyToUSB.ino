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
 * This sketch shows how the library can be used, together with UnoJoy, to turn
 * a Game Port joystick into an USB one.
 *
 * It also shows how calibration data can be saved to the EEPROM, so that the
 * joystick does not have to be calibrated every time. To force recalibration,
 * keep button 1 pressed at startup/reset.
 *
 * For details on UnoJoy, see https://github.com/AlanChatham/UnoJoy.
 */

#include <EEPROM.h>
#include <AnalogJoystick.h>
#include <UnoJoy.h>

#define N_BUTTONS 2
#define N_AXES 2

// EEPROM address at which calibration data is saved/loaded
#define EEPROM_ADDR 0


const byte buttonPins[N_BUTTONS] = {
  2, 3
};

const byte axisPins[N_AXES] = {
  A0, A1
};

AnalogJoystick joystick;

void setup () {
  joystick.begin (N_AXES, axisPins, N_BUTTONS, buttonPins);

  bool calibrationNeeded = false;

  // If button 1 is pressed, force calibration
  joystick.read ();
  if (joystick.getButton (0))
    calibrationNeeded = true;

  // See if we have calibration data handy
  if (!calibrationNeeded) {
    AnalogJoystick::CalibrationData calData;
    EEPROM.get (EEPROM_ADDR, calData);
    calibrationNeeded = !joystick.calibrate (calData);
  }

  if (calibrationNeeded) {
    // Calibrate
    while (!joystick.isCalibrated ())
      joystick.calibrate (LED_BUILTIN);

    // Save calibration data for next time
    AnalogJoystick::CalibrationData calData = joystick.getCalibrationData ();
    EEPROM.put (EEPROM_ADDR, calData);
  }

  // For the LED, in case it wasn't done by calibrate()
  pinMode (LED_BUILTIN, OUTPUT);

  // Init UnoJoy
  setupUnoJoy ();
}

void loop () {
  // Update joystick values
  joystick.read ();

  // Flash led with buttons, I like this
  digitalWrite (LED_BUILTIN, joystick.getButton (0) | joystick.getButton (1));

  // Make up data for UnoJoy
  dataForController_t controllerData = getBlankDataForController ();
  controllerData.leftStickX = (uint8_t) abs (joystick.getAxis (0) - 255);
  controllerData.leftStickY = (uint8_t) abs (joystick.getAxis (1) - 255);
  controllerData.squareOn = joystick.getButton (0);
  controllerData.crossOn = joystick.getButton (1);
  setControllerData (controllerData);
}
