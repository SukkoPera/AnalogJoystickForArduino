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
 * See the following link for a reference on the PC Analog Joystick protocol:
 * http://www.built-to-spec.com/blog/2009/09/10/using-a-pc-joystick-with-the-arduino/
 */

#include "AnalogJoystick.h"

// Signature to validate calibration data
#define CALDATA_SIGNATURE 0xCA1B


bool AnalogJoystick::begin (const byte nAxes, const byte axisPins[], const byte nButtons, const byte buttonPins[]) {
  if (nAxes > MAX_AXES)
    return false;
  if  (nButtons > MAX_BUTTONS)
    return false;

  this -> nAxes = nAxes;

  for (int i = 0; i < nAxes; i++) {
    Axis& axis = axes[i];
    axis.pin = axisPins[i];
    axis.min = 1023;
    axis.max = 0;
    axis.rawValue = 128;    // Center
    axis.value = 0;
  }

  this -> nButtons = nButtons;
  for (int i = 0; i < nButtons; i++) {
    Button& btn = buttons[i];
    btn.pin = buttonPins[i];
    btn.pressed = false;
  }

  this -> calibrated = false;

  // Button inputs, with pullups
  for (int i = 0; i < nButtons; ++i) {
    pinMode (buttonPins[i], INPUT_PULLUP);
  }
}

bool AnalogJoystick::calibrate (const int ledPin) {
  static bool started = false;
  static long start_time = 0;
  static long led_changed = 0;
  static bool led_state = LOW;

  if (!started) {
    for (int i = 0; i < nAxes; ++i) {
      axes[i].min = 1023;
      axes[i].max = 0;
    }

    start_time = millis ();
    started = true;

    // Prepare LED
    if (ledPin >= 0)
      pinMode (ledPin, OUTPUT);
  }

  if (millis () - start_time > CALIBRATION_DURATION * 1000L) {
    // Calibration complete
    calibrated = true;

//  for (int i = 0; i < nAxes; ++i) {
//    Serial.print (axisNames[i]);
//    Serial.print (" Axis Range = " );
//    Serial.print (joyAxes[i].min);
//    Serial.print (" - ");
//    Serial.println (joyAxes[i].max);
//  }

    // Signal the user to let go of all levers
    if (ledPin >= 0)
      digitalWrite (ledPin, HIGH);

    // Wait a bit to make sure user accomplishes
    delay (2000);

    // Sample center positions
    for (int i = 0; i < nAxes; ++i) {
      axes[i].center = analogRead (axes[i].pin);
    }

    // Turn off led to signal calibration is complete
    if (ledPin >= 0)
      digitalWrite (ledPin, LOW);
  } else {
    // Do a calibration pass

    // Calculate time to blink led for
    int interval = 500;
    if (millis () - start_time >= CALIBRATION_DURATION * 1000L - 2000)
      // Blink faster as end of calibration approaches
      interval = 150;

    if (ledPin >= 0 && millis () - led_changed > interval) {
      digitalWrite (LED_BUILTIN, (led_state = !led_state));
      led_changed = millis ();
    }

    // Check if axes have mover over the current bounds
    for (int i = 0; i < nAxes; ++i) {
      int x = analogRead (axes[i].pin);

      // New maximum
      if (x > axes[i].max) {
        //Serial.print ("New max X = " );
        //Serial.println (x);

        axes[i].max = x;
      }

      // New minimum
      if (x < axes[i].min) {
        //Serial.print("New min x = " );
        //Serial.println (x);

        axes[i].min = x;
      }
    }
  }

  return calibrated;
}

bool AnalogJoystick::calibrate (const CalibrationData& calData) {
  if (isCalibrationDataValid (calData)) {
    for (int i = 0; i < nAxes * 3; i += 3) {
      Axis& axis = axes[i / 3];

      axis.min = calData.bounds[i];
      axis.center = calData.bounds[i + 1];
      axis.max = calData.bounds[i + 2];;
    }

    calibrated = true;
  }

  return calibrated;
}

void AnalogJoystick::read () {
  for (int i = 0; i < nAxes; ++i) {
    Axis& axis = axes[i];

    axis.rawValue = analogRead (axis.pin);

    // If we have calibration values, use them
    if (calibrated) {
      if (axis.rawValue > axis.center) {
        axis.value = map (axis.rawValue, axis.center, axis.max, AXIS_CENTER_VALUE + 1, AXIS_MAX_VALUE);
      } else if (axis.rawValue < axis.center) {
        axis.value = map (axis.rawValue, axis.min, axis.center, AXIS_MIN_VALUE, AXIS_CENTER_VALUE - 1);
      } else {
        // Centered
        axis.value = AXIS_CENTER_VALUE;
      }

      // Just in case the sensor value is outside the range seen during calibration
      axis.value = constrain (axis.value, AXIS_MIN_VALUE, AXIS_MAX_VALUE);
    }
  }

  for (int i = 0; i < nButtons; ++i) {
    buttons[i].pressed = (digitalRead (buttons[i].pin) == LOW);
  }
}

int AnalogJoystick::getAxis (byte axisNo) const {
  if (axisNo < nAxes)
    return axes[axisNo].value;
  else
    return AXIS_CENTER_VALUE;
}

bool AnalogJoystick::getButton (byte buttonNo) const {
  if (buttonNo < nButtons)
    return buttons[buttonNo].pressed;
  else
    return false;
}

bool AnalogJoystick::isCalibrated () const {
  return calibrated;
}

AnalogJoystick::CalibrationData AnalogJoystick::getCalibrationData () const {
  AnalogJoystick::CalibrationData calData;

  calData.nAxes = nAxes;
  for (int i = 0; i < MAX_AXES * 3; i += 3) {
    if (i < nAxes * 3) {
      const AnalogJoystick::Axis& axis = axes[i / 3];
      calData.bounds[i] = axis.min;
      calData.bounds[i + 1] = axis.center;
      calData.bounds[i + 2] = axis.max;
    } else {
      calData.bounds[i] = -1;
      calData.bounds[i + 1] = -1;
      calData.bounds[i + 2] = -1;
    }
  }
  calData.signature = CALDATA_SIGNATURE;

  return calData;
}

bool AnalogJoystick::isCalibrationDataValid (const AnalogJoystick::CalibrationData& calData) const {
  return calData.signature == CALDATA_SIGNATURE && calData.nAxes == nAxes;
}
